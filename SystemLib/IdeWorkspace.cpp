#include "IdeWorkspace.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <thread>
#include <sstream>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
namespace fs=std::filesystem;
namespace {
std::string uniqueSuffix() {
    static std::atomic<unsigned> sequence{0};
    return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(sequence++);
}
void writeText(const fs::path& path,const std::string& text) {
    std::ofstream out(path,std::ios::binary|std::ios::trunc);
    if(!out || !out.write(text.data(),static_cast<std::streamsize>(text.size())) || !out.flush())
        throw std::runtime_error("Cannot write "+path.string());
}
bool validName(const std::string& name) {
    return std::regex_match(name,std::regex("[A-Za-z0-9_][A-Za-z0-9_.-]*\\.asm")) && name.find("..") == std::string::npos;
}
int runAssembler(const fs::path& executable,const fs::path& cwd,
                 const fs::path& input,const fs::path& output,const fs::path& log,const fs::path& listing) {
#ifdef _WIN32
    auto quote=[](const std::wstring& s) {
        std::wstring result=L"\"";unsigned slashes=0;
        for(wchar_t c:s) {
            if(c==L'\\') {++slashes;continue;}
            result.append(c==L'"'?slashes*2+1:slashes,L'\\');slashes=0;result+=c;
        }
        result.append(slashes*2,L'\\');return result+L'"';
    };
    std::wstring command=quote(executable.wstring())+L" -Fbin -dotdir -wdc02 -L "+quote(listing.wstring())+L" -o "+quote(output.wstring())+L" "+quote(input.wstring());
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES),nullptr,TRUE};
    HANDLE file=CreateFileW(log.c_str(),GENERIC_WRITE,FILE_SHARE_READ,&security,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create assembler log");
    STARTUPINFOW startup{};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESTDHANDLES;
    startup.hStdOutput=startup.hStdError=file;startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION process{};
    BOOL started=CreateProcessW(executable.c_str(),&command[0],nullptr,nullptr,TRUE,CREATE_NO_WINDOW,nullptr,cwd.c_str(),&startup,&process);
    CloseHandle(file);
    if(!started) throw std::runtime_error("Cannot start vasm (Windows error "+std::to_string(GetLastError())+")");
    auto result=WaitForSingleObject(process.hProcess,10000);
    if(result!=WAIT_OBJECT_0) {TerminateProcess(process.hProcess,124);WaitForSingleObject(process.hProcess,INFINITE);}
    DWORD status=1;GetExitCodeProcess(process.hProcess,&status);
    CloseHandle(process.hThread);CloseHandle(process.hProcess);return static_cast<int>(status);
#else
    // No shell: paths and source cannot become shell commands. Prepare all
    // storage before fork; the child uses only async-signal-safe operations.
    std::vector<std::string> values={executable.string(),"-Fbin","-dotdir","-wdc02","-L",listing.string(),"-o",output.string(),input.string()};
    std::vector<char*> args;for(auto& v:values) args.push_back(&v[0]);args.push_back(nullptr);
    const auto cwdString=cwd.string();
    int fd=::open(log.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);
    if(fd<0) throw std::runtime_error("Cannot create assembler log");
    pid_t pid=fork();
    if(pid==0) {
        if(chdir(cwdString.c_str()) || dup2(fd,STDOUT_FILENO)<0 || dup2(fd,STDERR_FILENO)<0) _exit(126);
        close(fd);execv(args[0],args.data());
        const char error[]="Cannot execute vasm. Check executable permissions and CPU architecture.\n";
        write(STDERR_FILENO,error,sizeof(error)-1);_exit(127);
    }
    close(fd);if(pid<0) throw std::runtime_error("Cannot start assembler process");
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    int status=0;
    for(;;) {
        pid_t result=waitpid(pid,&status,WNOHANG);
        if(result==pid) return WIFEXITED(status)?WEXITSTATUS(status):128;
        if(result<0 && errno!=EINTR) throw std::runtime_error("Cannot wait for assembler process");
        if(std::chrono::steady_clock::now()>=deadline) {
            kill(pid,SIGKILL);while(waitpid(pid,&status,0)<0 && errno==EINTR) {}
            return 124;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
#endif
}
}
IdeWorkspace::IdeWorkspace(fs::path directory):root(fs::absolute(std::move(directory))) {
    if(!fs::is_directory(root)) throw std::runtime_error("Program directory does not exist: "+root.string());
}
fs::path IdeWorkspace::pathFor(const std::string& name) const {
    if(!validName(name)) throw std::runtime_error("Use a simple .asm filename with letters, numbers, _, - or .");
    const auto path=root/name;
    if(fs::is_symlink(path)) throw std::runtime_error("Symbolic links are not editable in this workspace");
    return path;
}
std::vector<std::string> IdeWorkspace::programs() const {
    std::vector<std::string> names;
    for(const auto& entry:fs::directory_iterator(root)) {
        auto name=entry.path().filename().string();
        if(entry.is_regular_file() && !entry.is_symlink() && validName(name)) names.push_back(name);
    }
    std::sort(names.begin(),names.end());return names;
}
std::string IdeWorkspace::readText(const fs::path& path) {
    std::ifstream in(path,std::ios::binary|std::ios::ate);
    if(!in) throw std::runtime_error("Cannot open "+path.string());
    auto size=in.tellg();
    if(size<0 || size>1024*1024) throw std::runtime_error("File is unreadable or exceeds 1 MiB: "+path.string());
    std::string text(static_cast<size_t>(size),'\0');in.seekg(0);
    if(!text.empty() && !in.read(&text[0],size)) throw std::runtime_error("Cannot read "+path.string());
    return text;
}
AsmDocument IdeWorkspace::open(const std::string& name) const { return AsmDocument(readText(pathFor(name))); }
void IdeWorkspace::validateNewName(const std::string& name) const {
    if(fs::exists(pathFor(name))) throw std::runtime_error("A program with that name already exists");
}
void IdeWorkspace::save(const std::string& name,AsmDocument& doc) const {
    auto path=pathFor(name);
    if(doc.onDisk) {
        if(!fs::exists(path) || AsmDocument::normalize(readText(path))!=doc.savedText())
            throw std::runtime_error("File changed outside the IDE. Reload it before saving: "+name);
    } else if(fs::exists(path)) throw std::runtime_error("File already exists: "+name);
    auto temporary=root/("."+name+".ide-"+uniqueSuffix());
    try {
        writeText(temporary,doc.text());
#ifdef _WIN32
        if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot replace "+name);
#else
        fs::rename(temporary,path);
#endif
    } catch(...) { std::error_code ignored;fs::remove(temporary,ignored);throw; }
    doc.markSaved();
}
std::string IdeWorkspace::starterProgram() {
    return "; W65C02 program. Build & Run (F5) loads this ROM into the board.\n"
           "; RAM: $0000-$3fff   VIA: $6000   ROM: $8000-$ffff\n\n"
           "    .org $8000\n\nreset:\n    sei\n    cld\n    ldx #$ff\n    txs\n\n"
           "; Add your code here. The example programs show how to use the LCD.\n"
           "    stp\n\nnmi:\nirq:\n    rti\n\n    .org $fffa\n"
           "    .word nmi\n    .word reset\n    .word irq\n";
}
AsmBuildResult assembleProgram(const fs::path& assembler,const fs::path& programDirectory,
                              const fs::path& buildDirectory,const std::string& filename,const std::string& source) {
    AsmBuildResult result;
    try {
        if(!validName(filename)) throw std::runtime_error("Invalid assembly filename");
        const auto directory=fs::absolute(buildDirectory)/("build-"+uniqueSuffix());
        fs::create_directories(directory);
        auto input=directory/filename;result.rom=directory/"program.bin";
        auto log=directory/"assembler.log";
        auto listing=directory/"program.lst";
        writeText(input,source);
        int status=runAssembler(fs::absolute(assembler),fs::absolute(programDirectory),input,result.rom,log,listing);
        result.output=IdeWorkspace::readText(log);
        std::smatch error;
        if(std::regex_search(result.output,error,std::regex("(?:error|fatal error)[^\\n]*line ([0-9]+)")))
            result.errorLine=std::stoi(error[1]);
        if(status==124) result.output+="\nBuild stopped: assembler exceeded the 10 second limit.\n";
        if(status!=0) {result.output+="\nBuild failed (exit "+std::to_string(status)+").\n";return result;}
        if(!fs::exists(result.rom) || fs::file_size(result.rom)!=32768)
            throw std::runtime_error("Expected a 32 KiB ROM. Use .org $8000 and vectors through $ffff.");
        auto rom=IdeWorkspace::readText(result.rom);
        unsigned reset=static_cast<unsigned char>(rom[0x7ffc])|(static_cast<unsigned char>(rom[0x7ffd])<<8);
        if(reset<0x8000) throw std::runtime_error("The reset vector must point into ROM ($8000-$ffff).");
        // Listing addresses come from the assembled snapshot, never from an
        // edited buffer. Included-source sections are deliberately excluded.
        std::ifstream lines(listing);std::string line;bool mainSource=false;
        const std::regex sourceHeader("^Source: \"(.*)\"");
        const std::regex encodedLine("^[0-9A-Fa-f]+:([0-9A-Fa-f]{4})[ \t]+[0-9A-Fa-f]+[ \t]+([0-9]+):");
        while(std::getline(lines,line)) {
            std::smatch match;
            if(std::regex_search(line,match,sourceHeader))
                mainSource=fs::path(match[1].str()).filename()==filename;
            else if(mainSource && std::regex_search(line,match,encodedLine))
                result.sourceLines[static_cast<uint16_t>(std::stoul(match[1],nullptr,16))]=std::stoul(match[2]);
        }
        result.success=true;result.output+="\nBuild succeeded. ROM: 32768 bytes.\n";
    } catch(const std::exception& e) {result.output+="\nBuild failed: "+std::string(e.what())+"\n";}
    return result;
}
