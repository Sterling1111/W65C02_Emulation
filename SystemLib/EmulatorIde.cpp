#include "EmulatorIde.h"
#include <SFML/Window/Clipboard.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace {
using namespace sf;
constexpr float Width=1280,Height=900,Side=240;
constexpr float CodeX=308,CodeY=153,LineHeight=21;
constexpr unsigned CodeSize=15;
constexpr int VisibleLines=24;
const Color bg(20,25,29),panel(26,32,37),edge(47,57,63),muted(132,151,158);
const Color white(218,230,232),accent(104,211,182),cyan(124,190,229),amber(231,190,116);
const FloatRect saveButton(650,17,76,32),buildButton(736,17,78,32),runButton(824,17,144,32);
const FloatRect stopButton(978,17,68,32),clockButton(1058,17,204,32);
const FloatRect debugButton(643,70,152,32),pauseButton(805,70,137,32);
const FloatRect stepButton(952,70,112,32),restartButton(1074,70,188,32);
const FloatRect editorArea(252,143,1012,523),consoleArea(252,719,1012,149);
const FloatRect boardArea(262,142,998,716);
void box(RenderTarget& t,FloatRect r,Color color) {
    RectangleShape s({r.width,r.height});s.setPosition(r.left,r.top);s.setFillColor(color);t.draw(s);
}
void label(RenderTarget& t,const Font& f,const std::string& value,float x,float y,unsigned size,Color color) {
    Text text(value,f,size);text.setPosition(x,y);text.setFillColor(color);t.draw(text);
}
void button(RenderTarget& t,const Font& f,FloatRect bounds,const std::string& title,bool primary=false,bool enabled=true) {
    box(t,bounds,enabled?(primary?Color(61,128,111):Color(42,52,59)):Color(31,38,43));
    label(t,f,title,bounds.left+12,bounds.top+7,13,enabled?white:muted);
}
void fitWindow(RenderWindow& window) {
    auto size=window.getSize();
    if(!size.x || !size.y)return;
    float scale=std::min(size.x/Width,size.y/Height);
    float w=Width*scale/size.x,h=Height*scale/size.y;
    View view(FloatRect(0,0,Width,Height));
    view.setViewport({(1-w)/2,(1-h)/2,w,h});window.setView(view);
}
std::string shorten(const std::string& value,size_t length) {
    if(value.size()<=length)return value;
    return value.substr(0,length-3)+"...";
}
std::string hexValue(unsigned value,int width) {
    std::ostringstream s;s<<std::uppercase<<std::hex<<std::setfill('0')<<std::setw(width)<<value;return s.str();
}
std::string clockText(double hz) {
    std::ostringstream s;s<<std::setprecision(4)<<(hz>=1e6?hz/1e6:hz/1e3)<<(hz>=1e6?" MHz":" kHz");return s.str();
}
std::string asciiClipboard() {
    std::string text;
    for(auto c:Clipboard::getString()) {
        if(c=='\n'||c=='\r'||c=='\t'||(c>=32 && c<127))text+=static_cast<char>(c);
        else throw std::runtime_error("The assembly editor accepts ASCII text. Clipboard contains other characters.");
    }
    return text;
}
FloatRect fittedBoard() {
    float scale=std::min(boardArea.width/BreadboardView::Width,boardArea.height/BreadboardView::Height);
    float w=BreadboardView::Width*scale,h=BreadboardView::Height*scale;
    return {boardArea.left+(boardArea.width-w)/2,boardArea.top+(boardArea.height-h)/2,w,h};
}
std::vector<Color> syntax(const std::string& line) {
    std::vector<Color> colors(line.size(),white);
    static const std::unordered_set<std::string> instructions={
        "adc","and","asl","bcc","bcs","beq","bit","bmi","bne","bpl","bra","brk","bvc","bvs",
        "clc","cld","cli","clv","cmp","cpx","cpy","dec","dex","dey","eor","inc","inx","iny",
        "jmp","jsr","lda","ldx","ldy","lsr","nop","ora","pha","php","phx","phy","pla","plp",
        "plx","ply","rol","ror","rti","rts","sbc","sec","sed","sei","sta","stp","stx","sty",
        "stz","tax","tay","trb","tsb","tsx","txa","txs","tya","wai"};
    for(size_t i=0;i<line.size();) {
        size_t start=i;Color color=white;
        if(line[i]==';') {std::fill(colors.begin()+i,colors.end(),Color(114,142,128));break;}
        if(line[i]=='"'||line[i]=='\'') {
            char quote=line[i++];while(i<line.size()) {if(line[i++] == quote)break;}
            color=Color(188,208,139);
        } else if(std::isalnum(static_cast<unsigned char>(line[i]))||line[i]=='_'||line[i]=='.'||line[i]=='$'||line[i]=='%') {
            ++i;while(i<line.size() && (std::isalnum(static_cast<unsigned char>(line[i]))||line[i]=='_'||line[i]=='.'))++i;
            auto word=line.substr(start,i-start),lower=word;
            std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return std::tolower(c);});
            if(instructions.count(lower))color=cyan;
            else if(word[0]=='.')color=Color(198,165,229);
            else if(word[0]=='$'||word[0]=='%'||std::isdigit(static_cast<unsigned char>(word[0])))color=amber;
            else if(i<line.size() && line[i]==':')color=accent;
            else if(word=="A"||word=="X"||word=="Y"||word=="a"||word=="x"||word=="y")color=Color(217,163,173);
        } else ++i;
        std::fill(colors.begin()+start,colors.begin()+i,color);
    }
    return colors;
}
}

EmulatorIde::EmulatorIde(System& boardSystem,std::filesystem::path programs,
                         std::filesystem::path assemblerPath,std::filesystem::path buildPath)
    :system(boardSystem),workspace(std::move(programs)),assembler(std::move(assemblerPath)),builds(std::move(buildPath)) {
    const auto desktop=VideoMode::getDesktopMode();
    float scale=std::min(1.f,std::min(std::max(320.f,float(desktop.width)-40)/Width,
                                    std::max(240.f,float(desktop.height)-100)/Height));
    window.create(VideoMode(unsigned(Width*scale),unsigned(Height*scale)),"W65C02 Studio",Style::Default);
    fitWindow(window);
    window.setFramerateLimit(75);
    window.setKeyRepeatEnabled(true);
    if(!uiFont.loadFromFile(BREADBOARD_FONT_PATH) || !codeFont.loadFromFile(IDE_CODE_FONT_PATH))
        throw std::runtime_error("Cannot load IDE fonts from the build assets directory");
    board.reset(new BreadboardView);
    snapshot.lcdWidth=system.lcd.numPixelsX();snapshot.lcdHeight=system.lcd.numPixelsY();
    snapshot.pixels.resize(snapshot.lcdWidth*snapshot.lcdHeight);
    refreshPrograms();
    if(std::find(names.begin(),names.end(),"hello_world.asm")!=names.end())selectProgram("hello_world.asm");
    else if(!names.empty())selectProgram(names.front());
    setConsole("Ready. Select a program on the left.\nF5: build & run.  F6: build.  F7: build & debug.  F10: step one instruction.\nCtrl+S: save   Ctrl+F: find   Ctrl+G: go to line   Ctrl+Tab: switch tabs.\n");
    // Retain the command-line workflow: an existing a.out can still be reset
    // from the Board tab. Sources are only saved by an explicit save/build.
    if(std::filesystem::exists("a.out")) {
        try {system.loadProgram("a.out");loaded=true;loadedName="a.out";}
        catch(const std::exception& e) {setConsole(std::string("Could not load a.out: ")+e.what());}
    }
}
AsmDocument* EmulatorIde::document() {
    auto it=documents.find(selected);return it==documents.end()?nullptr:&it->second;
}
void EmulatorIde::refreshPrograms() {
    names=workspace.programs();
    for(const auto& entry:documents)if(std::find(names.begin(),names.end(),entry.first)==names.end())names.push_back(entry.first);
    std::sort(names.begin(),names.end());
    listTop=std::min(listTop,names.empty()?size_t(0):names.size()-1);
}
void EmulatorIde::selectProgram(const std::string& name) {
    if(!documents.count(name))documents.emplace(name,workspace.open(name));
    selected=name;setTab(Tab::Editor);findOpen=false;status="Editing "+name;keepCursorVisible();
}
void EmulatorIde::saveCurrent() {
    if(auto* doc=document()) {workspace.save(selected,*doc);status="Saved "+selected;refreshPrograms();}
}
void EmulatorIde::setConsole(const std::string& output) {
    console.clear();std::istringstream input(output);std::string line;
    while(std::getline(input,line)) {
        if(line.empty())console.push_back("");
        else for(size_t pos=0;pos<line.size();pos+=123)console.push_back(line.substr(pos,123));
    }
    consoleTop=console.size()>8?console.size()-8:0;
}
void EmulatorIde::build(bool runProgram,bool debug) {
    if(building || !document())return;
    saveCurrent();
    buildName=selected;buildSource=document()->text();runAfterBuild=runProgram;debugAfterBuild=debug;
    errorLine=0;errorFile.clear();building=true;
    status="Building "+buildName+"...";
    setConsole("Assembling "+buildName+" with vasm (W65C02).\n");
    auto executable=assembler,root=workspace.directory(),out=builds;
    auto name=buildName,source=buildSource;
    buildJob=std::async(std::launch::async,[executable,root,out,name,source]{return assembleProgram(executable,root,out,name,source);});
}
void EmulatorIde::loadAndRun(const AsmBuildResult& result) {
    releaseInputs();system.cpu.stop();
    system.loadProgram(result.rom.string());
    system.ram.initialize();system.lcd.powerOn();
    system.reset(false);loaded=true;paused=debugAfterBuild;loadedName=buildName;
    loadedSource=buildSource;loadedLines=result.sourceLines;
    syncBreakpoints();
    if(!debugAfterBuild)system.cpu.resume();
    setTab(debugAfterBuild?Tab::Editor:Tab::Board);
    if(debugAfterBuild) {
        // Debug the just-built buffer, not a different file selected mid-build.
        selected=buildName;
        status="Paused at reset. F10 steps one instruction; F8 continues.";
        followInstruction();
    } else status="Running "+buildName;
    auto it=documents.find(buildName);
    if(it!=documents.end() && it->second.text()!=buildSource)status+=" (editor has newer changes)";
}
void EmulatorIde::finishBuild() {
    if(!building || buildJob.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return;
    auto result=buildJob.get();building=false;setConsole(result.output);
    if(result.success) {
        status="Built "+buildName;
        if((runAfterBuild || debugAfterBuild) && !closingAfterBuild)loadAndRun(result);
    } else {
        status="Build failed. See assembler output.";errorLine=result.errorLine;errorFile=buildName;
        if(!closingAfterBuild) {
            selectProgram(buildName);status="Build failed. See assembler output.";
            if(errorLine>0 && document()) {
                document()->setCursor(document()->lineStart(size_t(errorLine-1)));keepCursorVisible();
            }
        }
    }
    if(closingAfterBuild)window.close();
}
void EmulatorIde::keepCursorVisible() {
    if(auto* doc=document()) {
        auto p=doc->location(doc->cursor);
        if(p.first<doc->topLine)doc->topLine=p.first;
        if(p.first>=doc->topLine+VisibleLines)doc->topLine=p.first-VisibleLines+1;
        const size_t cols=size_t((1255-CodeX)/codeFont.getGlyph('M',CodeSize,false).advance)-1;
        if(p.second<doc->leftColumn)doc->leftColumn=p.second;
        if(p.second>=doc->leftColumn+cols)doc->leftColumn=p.second-cols+1;
    }
    caretClock.restart();
}
void EmulatorIde::findNext() {
    if(!document() || findText.empty())return;
    auto* d=document();size_t p=d->text().find(findText,d->selection().second);
    if(p==std::string::npos)p=d->text().find(findText);
    if(p==std::string::npos) {status="No match for "+findText;return;}
    d->setCursor(p);d->setCursor(p+findText.size(),true);keepCursorVisible();status="Found "+findText;
}
void EmulatorIde::closeRequested() {
    releaseInputs();
    for(const auto& item:documents)if(item.second.dirty()) {dialog=Dialog::Close;dialogError.clear();return;}
    if(building) {closingAfterBuild=true;status="Closing when the current build finishes...";}
    else window.close();
}
void EmulatorIde::acceptDialog() {
    if(dialog==Dialog::NewProgram) {
        auto name=dialogText;if(name.size()<4 || name.substr(name.size()-4)!=".asm")name+=".asm";
        if(name.size()>64)throw std::runtime_error("Use a filename no longer than 64 characters");
        workspace.validateNewName(name);
        if(documents.count(name))throw std::runtime_error("That program is already open");
        AsmDocument doc(IdeWorkspace::starterProgram());doc.onDisk=false;
        documents.emplace(name,std::move(doc));refreshPrograms();selectProgram(name);
        status="New program. Ctrl+S saves it in VASM.";dialog=Dialog::Closed;
    } else if(dialog==Dialog::Reload) {
        if(document())*document()=workspace.open(selected);
        dialog=Dialog::Closed;status="Reloaded "+selected;keepCursorVisible();
    } else if(dialog==Dialog::GoToLine) {
        size_t line=std::stoul(dialogText);
        if(!line || !document() || line>document()->lineCount())throw std::runtime_error("Enter an existing line number");
        document()->setCursor(document()->lineStart(line-1));keepCursorVisible();dialog=Dialog::Closed;
    }
}
void EmulatorIde::setTab(Tab next) {
    releaseInputs();tab=next;dragging=false;
}
void EmulatorIde::resetBoard(bool stayPaused) {
    if(!loaded) {status="Build & Debug (F7) or Build & Run (F5) to load a program first.";return;}
    system.reset(false);paused=stayPaused;
    syncBreakpoints();
    if(!stayPaused)system.cpu.resume();
    status=stayPaused?"Restarted at reset, paused. F10 steps; F8 continues.":"Reset "+loadedName;
    if(paused)followInstruction();
}
void EmulatorIde::pauseOrContinue() {
    if(!loaded || building)return;
    checkBreakpoint();syncBreakpoints();
    if(!system.firstReset) {resetBoard();return;}
    if(paused) {
        bool halted;
        {std::lock_guard<std::mutex> lock(system.cpu.stateMutex());halted=system.cpu.STOP;}
        if(halted) {status="STP halted the CPU. Restart paused (F9) or reset (R on the board).";return;}
        system.cpu.resume();paused=false;status="Continuing "+loadedName;
    } else {
        system.cpu.stop();paused=true;status="Paused "+loadedName+". F10 steps; F8 continues.";followInstruction();
    }
}
void EmulatorIde::stepInstruction() {
    if(building)return;
    if(!loaded) {status="Build & Debug (F7) loads your program before its first instruction.";return;}
    if(!system.firstReset)system.reset(false);
    auto result=system.cpu.step();paused=true;
    if(result.kind==W65C02::StepResult::Kind::Stopped)
        status="STP halted the CPU. Restart paused (F9) to step from the beginning.";
    else if(result.kind==W65C02::StepResult::Kind::Waiting)
        status="WAI: waiting for an interrupt. Advanced one idle clock. F8 runs the peripheral clocks.";
    else
        status="Stepped "+loadedName+"  $"+hexValue(result.address,4)+"  "+result.mnemonic+
               "  -> PC $"+hexValue(result.nextPC,4)+"  ("+std::to_string(result.cycles)+" cycles). Paused.";
    followInstruction();
}
void EmulatorIde::toggleBreakpoint(size_t line) {
    if(!document() || !line || line>document()->lineCount())return;
    auto& lines=breakpointLines[selected];
    if(lines.erase(line))status="Removed breakpoint on line "+std::to_string(line)+".";
    else {
        lines.insert(line);
        status="Breakpoint on line "+std::to_string(line)+
            (breakpointBound(line)?". F8 continues to it.":" pending: build/load matching source; choose a line that emits code.");
    }
    syncBreakpoints();
}
bool EmulatorIde::breakpointBound(size_t line) {
    if(!loaded || selected!=loadedName || !document() || document()->text()!=loadedSource)return false;
    for(const auto& entry:loadedLines)if(entry.second==line)return true;
    return false;
}
void EmulatorIde::syncBreakpoints() {
    std::vector<word> addresses;
    auto doc=documents.find(loadedName);
    auto points=breakpointLines.find(loadedName);
    if(loaded && doc!=documents.end() && doc->second.text()==loadedSource && points!=breakpointLines.end())
        for(const auto& entry:loadedLines)if(points->second.count(entry.second))addresses.push_back(entry.first);
    if(addresses!=activeBreakpoints) {
        system.cpu.setBreakpoints(addresses);activeBreakpoints=std::move(addresses);
    }
}
void EmulatorIde::checkBreakpoint() {
    // Keep modal actions tied to the file for which they were opened.
    if(!loaded || paused || dialog!=Dialog::Closed || closingAfterBuild)return;
    auto point=system.cpu.breakpointState();
    if(!point.hit)return;
    paused=true;setTab(Tab::Editor);selected=loadedName;followInstruction();
    status="Breakpoint at $"+hexValue(point.address,4)+" in "+loadedName+
        ". Paused before instruction. F10 steps; F8 continues.";
}
size_t EmulatorIde::nextSourceLine() {
    if(!loaded || !paused || selected!=loadedName || !document() || document()->text()!=loadedSource)return 0;
    word pc;
    {std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
        if(system.cpu.STOP)return 0;
        pc=system.cpu.PC;
    }
    auto found=loadedLines.find(pc);return found==loadedLines.end()?0:found->second;
}
void EmulatorIde::followInstruction() {
    size_t line=nextSourceLine();
    if(line && document()) {document()->setCursor(document()->lineStart(line-1));keepCursorVisible();}
}
std::string EmulatorIde::registerText() {
    std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
    return std::string(system.cpu.STOP?"STP":paused?"PAUSED":system.cpu.WAIT?"WAI":"RUN")+
        "  PC $"+hexValue(system.cpu.PC,4)+"  A $"+hexValue(system.cpu.A,2)+
        "  X $"+hexValue(system.cpu.X,2)+"  Y $"+hexValue(system.cpu.Y,2)+
        "  SP $"+hexValue(system.cpu.SP,2)+"  P $"+hexValue(system.cpu.PS.to_ulong(),2);
}
void EmulatorIde::updateInputs() {
    bool irq=keyIrq||mouseButton==BreadboardView::Button::Irq;
    bool nmi=keyNmi||mouseButton==BreadboardView::Button::Nmi;
    if(irq!=irqInput)system.cpu.interrupt(false,irq);
    if(nmi!=nmiInput)system.cpu.interrupt(true,nmi);
    irqInput=irq;nmiInput=nmi;
}
void EmulatorIde::releaseInputs() {
    keyIrq=keyNmi=keyReset=false;mouseButton=BreadboardView::Button::Released;updateInputs();
}
void EmulatorIde::changeClock() {
    double hz;
    {std::lock_guard<std::mutex> lock(system.cpu.stateMutex());hz=system.cpu.cycles.getFrequencyHz();}
    const double choices[]={1000,10000,100000,500000,1000000,2000000,10000000,50000000};
    double next=choices[0];for(double value:choices)if(value>hz+1) {next=value;break;}
    system.cpu.setCycleDuration(next/1e6);status="Clock set to "+clockText(next)+". Click CLOCK to cycle speeds.";
}
void EmulatorIde::editorKey(const sf::Event::KeyEvent& key) {
    auto* d=document();if(!d)return;
    const bool ctrl=key.control||key.system;
    if(ctrl) {
        switch(key.code) {
        case Keyboard::B:
            if(breakpointKeyHeld)return;
            breakpointKeyHeld=true;
            if(key.shift) {breakpointLines[selected].clear();syncBreakpoints();status="Cleared breakpoints in "+selected;}
            else toggleBreakpoint(d->location(d->cursor).first+1);
            return;
        case Keyboard::A:d->anchor=0;d->cursor=d->text().size();break;
        case Keyboard::C:Clipboard::setString(d->selectedText());return;
        case Keyboard::X:Clipboard::setString(d->selectedText());if(d->cursor!=d->anchor)d->insert("");break;
        case Keyboard::V:d->insert(asciiClipboard());break;
        case Keyboard::Z:if(key.shift)d->redo();else d->undo();break;
        case Keyboard::Y:d->redo();break;
        case Keyboard::F:findOpen=true;findText.clear();return;
        case Keyboard::G:dialog=Dialog::GoToLine;dialogText.clear();dialogError.clear();return;
        default:break;
        }
    }
    switch(key.code) {
    case Keyboard::Left:d->moveHorizontal(-1,key.shift,ctrl);break;
    case Keyboard::Right:d->moveHorizontal(1,key.shift,ctrl);break;
    case Keyboard::Up:d->moveVertical(-1,key.shift);break;
    case Keyboard::Down:d->moveVertical(1,key.shift);break;
    case Keyboard::PageUp:d->moveVertical(-VisibleLines,key.shift);break;
    case Keyboard::PageDown:d->moveVertical(VisibleLines,key.shift);break;
    case Keyboard::Home:d->home(key.shift,ctrl);break;
    case Keyboard::End:d->end(key.shift,ctrl);break;
    case Keyboard::Backspace:d->backspace();break;
    case Keyboard::Delete:d->eraseForward();break;
    case Keyboard::Return:d->newline();break;
    case Keyboard::Tab:if(!ctrl)d->indent(key.shift);break;
    default:break;
    }
    keepCursorVisible();
}
void EmulatorIde::positionCursor(Vector2f p,bool extend) {
    if(auto* d=document()) {
        auto line=d->topLine+size_t(std::max(0.f,std::floor((p.y-CodeY)/LineHeight)));
        line=std::min(line,d->lineCount()-1);
        const float cw=codeFont.getGlyph('M',CodeSize,false).advance;
        size_t col=d->leftColumn+size_t(std::max(0.f,std::floor((p.x-CodeX)/cw+.5f)));
        d->setCursor(std::min(d->lineStart(line)+col,d->lineEnd(line)),extend);keepCursorVisible();
    }
}
void EmulatorIde::mouseDown(Vector2f p) {
    if(dialog!=Dialog::Closed) {
        if(FloatRect(814,340,30,30).contains(p)) {dialog=Dialog::Closed;return;}
        if(dialog==Dialog::Close) {
            if(FloatRect(448,497,158,36).contains(p)) {
                for(auto& item:documents)if(item.second.dirty())workspace.save(item.first,item.second);
                dialog=Dialog::Closed;closeRequested();
            } else if(FloatRect(620,497,105,36).contains(p)) {
                dialog=Dialog::Closed;
                if(building)closingAfterBuild=true;else window.close();
            } else if(FloatRect(739,497,93,36).contains(p))dialog=Dialog::Closed;
        } else {
            if(FloatRect(620,497,108,36).contains(p))acceptDialog();
            else if(FloatRect(742,497,90,36).contains(p))dialog=Dialog::Closed;
        }
        return;
    }
    if(debugButton.contains(p)) {build(false,true);return;}
    if(pauseButton.contains(p)) {pauseOrContinue();return;}
    if(stepButton.contains(p)) {stepInstruction();return;}
    if(restartButton.contains(p)) {if(!building)resetBoard(true);return;}
    if(saveButton.contains(p)) {saveCurrent();return;}
    if(buildButton.contains(p)) {build(false);return;}
    if(runButton.contains(p)) {build(true);return;}
    if(stopButton.contains(p)) {releaseInputs();system.cpu.stop();paused=true;status="Paused. F10 steps; F8 continues.";followInstruction();return;}
    if(clockButton.contains(p)) {changeClock();return;}
    if(FloatRect(254,70,115,32).contains(p)) {setTab(Tab::Editor);return;}
    if(FloatRect(377,70,135,32).contains(p)) {setTab(Tab::Board);return;}
    if(FloatRect(186,87,36,30).contains(p)) {dialog=Dialog::NewProgram;dialogText.clear();dialogError.clear();return;}
    if(FloatRect(16,823,94,30).contains(p)) {refreshPrograms();status="Program list refreshed.";return;}
    if(FloatRect(122,823,100,30).contains(p)) {
        if(document()) {dialog=Dialog::Reload;dialogError.clear();}return;
    }
    if(p.x<Side && p.y>=133 && p.y<805) {
        size_t i=listTop+size_t((p.y-133)/34);if(i<names.size())selectProgram(names[i]);return;
    }
    if(tab==Tab::Editor) {
        if(editorArea.contains(p) && p.x<299) {
            if(p.y>=CodeY)toggleBreakpoint(document()?document()->topLine+size_t((p.y-CodeY)/LineHeight)+1:0);
            dragging=false;return;
        }
        if(editorArea.contains(p)) {findOpen=false;positionCursor(p,Keyboard::isKeyPressed(Keyboard::LShift)||Keyboard::isKeyPressed(Keyboard::RShift));dragging=true;}
        if(FloatRect(1091,108,162,25).contains(p)) {findOpen=true;findText.clear();}
    } else {
        auto r=fittedBoard();if(!r.contains(p))return;
        auto point=Vector2f((p.x-r.left)*BreadboardView::Width/r.width,(p.y-r.top)*BreadboardView::Height/r.height);
        mouseButton=BreadboardView::hitTest(point);
        if(mouseButton==BreadboardView::Button::Reset)resetBoard();
        updateInputs();
    }
}
void EmulatorIde::handle(const Event& event) {
    if(event.type==Event::KeyReleased && event.key.code==Keyboard::B)breakpointKeyHeld=false;
    if(closingAfterBuild)return;
    if(event.type==Event::Closed) {closeRequested();return;}
    if(event.type==Event::LostFocus) {breakpointKeyHeld=false;releaseInputs();dragging=false;for(auto& key:debugKeyHeld)key=false;return;}
    if(event.type==Event::Resized) {fitWindow(window);return;}
    if(event.type==Event::MouseButtonReleased && event.mouseButton.button==Mouse::Left) {
        dragging=false;mouseButton=BreadboardView::Button::Released;updateInputs();return;
    }
    if(event.type==Event::MouseButtonPressed && event.mouseButton.button==Mouse::Left) {
        mouseDown(window.mapPixelToCoords({event.mouseButton.x,event.mouseButton.y}));return;
    }
    if(event.type==Event::MouseMoved && dragging && tab==Tab::Editor && dialog==Dialog::Closed) {
        auto p=window.mapPixelToCoords({event.mouseMove.x,event.mouseMove.y});
        p.x=std::max(CodeX,std::min(1254.f,p.x));p.y=std::max(CodeY,std::min(CodeY+(VisibleLines-1)*LineHeight,p.y));
        positionCursor(p,true);return;
    }
    if(event.type==Event::MouseWheelScrolled && dialog==Dialog::Closed) {
        auto p=window.mapPixelToCoords({event.mouseWheelScroll.x,event.mouseWheelScroll.y});
        int amount=int(-event.mouseWheelScroll.delta*3);
        auto scroll=[&](size_t& value,size_t maximum){value=size_t(std::max<long long>(0,std::min<long long>(maximum,static_cast<long long>(value)+amount)));};
        if(p.x<Side)scroll(listTop,names.size()>19?names.size()-19:0);
        else if(tab==Tab::Editor && consoleArea.contains(p))scroll(consoleTop,console.size()>8?console.size()-8:0);
        else if(tab==Tab::Editor && document()) {
            if(event.mouseWheelScroll.wheel==Mouse::HorizontalWheel || Keyboard::isKeyPressed(Keyboard::LShift))
                scroll(document()->leftColumn,1024);
            else scroll(document()->topLine,document()->lineCount()>VisibleLines?document()->lineCount()-VisibleLines:0);
        }
        return;
    }
    if(event.type==Event::TextEntered) {
        auto c=event.text.unicode;
        if(c>=32 && c<127 && !Keyboard::isKeyPressed(Keyboard::LControl) && !Keyboard::isKeyPressed(Keyboard::RControl)) {
            if(dialog==Dialog::NewProgram || dialog==Dialog::GoToLine) {
                if(dialogText.size()<64)dialogText+=char(c);
                dialogError.clear();
            } else if(dialog==Dialog::Closed && tab==Tab::Editor) {
                if(findOpen) {if(findText.size()<80)findText+=char(c);}
                else if(document()) {document()->insert(std::string(1,char(c)));keepCursorVisible();}
            }
        }
        return;
    }
    if(event.type==Event::KeyReleased) {
        if(event.key.code>=Keyboard::F7 && event.key.code<=Keyboard::F10)
            debugKeyHeld[int(event.key.code)-int(Keyboard::F7)]=false;
        if(event.key.code==Keyboard::I)keyIrq=false;
        if(event.key.code==Keyboard::N)keyNmi=false;
        if(event.key.code==Keyboard::R)keyReset=false;
        updateInputs();return;
    }
    if(event.type!=Event::KeyPressed)return;
    const auto& k=event.key;const bool ctrl=k.control||k.system;
    if(dialog!=Dialog::Closed) {
        if(k.code==Keyboard::Escape)dialog=Dialog::Closed;
        else if(k.code==Keyboard::Return && dialog!=Dialog::Close)acceptDialog();
        else if(k.code==Keyboard::Backspace && !dialogText.empty())dialogText.pop_back();
        return;
    }
    if(k.code>=Keyboard::F7 && k.code<=Keyboard::F10) {
        auto& held=debugKeyHeld[int(k.code)-int(Keyboard::F7)];
        if(held)return;
        held=true;
        if(k.code==Keyboard::F7)build(false,true);
        else if(k.code==Keyboard::F8)pauseOrContinue();
        else if(k.code==Keyboard::F9) {if(!building)resetBoard(true);}
        else stepInstruction();
        return;
    }
    if(ctrl && k.code==Keyboard::S) {saveCurrent();return;}
    if(ctrl && k.code==Keyboard::N) {dialog=Dialog::NewProgram;dialogText.clear();dialogError.clear();releaseInputs();return;}
    if(ctrl && k.code==Keyboard::Tab) {setTab(tab==Tab::Editor?Tab::Board:Tab::Editor);return;}
    if(k.code==Keyboard::F5) {build(true);return;}
    if(k.code==Keyboard::F6) {build(false);return;}
    if(tab==Tab::Editor) {
        if(findOpen) {
            if(k.code==Keyboard::Escape)findOpen=false;
            else if(k.code==Keyboard::Return)findNext();
            else if(k.code==Keyboard::Backspace && !findText.empty())findText.pop_back();
            return;
        }
        editorKey(k);
    } else if(!ctrl && !k.alt) {
        if(k.code==Keyboard::I)keyIrq=true;
        if(k.code==Keyboard::N)keyNmi=true;
        if(k.code==Keyboard::R && !keyReset) {keyReset=true;resetBoard();}
        updateInputs();
    }
}
void EmulatorIde::drawEditor() {
    auto* d=document();
    label(window,uiFont,selected.empty()?"No program selected":selected+(d&&d->dirty()?"  /  unsaved":"  /  saved"),258,112,14,muted);
    if(findOpen) {
        box(window,{733,105,520,29},Color(42,54,62));
        label(window,codeFont,"Find: "+shorten(findText,44)+"_",746,112,13,white);
    } else {
        label(window,uiFont,"Click gutter / Ctrl+B: breakpoint   |   hollow: pending",710,112,12,muted);
        label(window,uiFont,"Find  Ctrl+F",1136,112,13,muted);
    }
    box(window,editorArea,Color(23,29,34));box(window,{252,143,46,523},Color(21,27,31));
    if(!d) {label(window,uiFont,"Choose a program or click + to create one.",310,190,18,muted);return;}
    const float cw=codeFont.getGlyph('M',CodeSize,false).advance;
    const size_t visibleColumns=size_t((1255-CodeX)/cw);
    auto selection=d->selection();auto location=d->location(d->cursor);
    const auto executionLine=nextSourceLine();
    for(size_t row=0;row<VisibleLines;++row) {
        size_t line=d->topLine+row;if(line>=d->lineCount())break;
        auto begin=d->lineStart(line),end=d->lineEnd(line);
        float y=CodeY+row*LineHeight;
        if(line==location.first)box(window,{299,y-1,962,LineHeight},Color(30,39,44));
        if(selected==errorFile && int(line+1)==errorLine)box(window,{252,y-1,1009,LineHeight},Color(95,43,47,125));
        if(line+1==executionLine) {
            box(window,{252,y-1,1009,LineHeight},Color(44,91,78,160));
            box(window,{250,y+3,3,13},accent);
        }
        auto from=std::max(begin,selection.first),to=std::min(end+1,selection.second);
        if(to>from) {
            auto left=std::max(from-begin,d->leftColumn),right=std::min(to-begin,d->leftColumn+visibleColumns);
            if(right>left)box(window,{CodeX+(left-d->leftColumn)*cw,y-1,(right-left)*cw,LineHeight},Color(51,86,103));
        }
        std::ostringstream number;number<<std::setw(4)<<line+1;
        label(window,codeFont,number.str(),256,y,12,line==location.first?accent:Color(97,118,127));
        if(breakpointLines[selected].count(line+1)) {
            CircleShape dot(4);dot.setPosition(289,y+4);
            dot.setOutlineColor(Color(246,108,115));dot.setOutlineThickness(1);
            dot.setFillColor(breakpointBound(line+1)?Color(246,108,115):Color(23,29,34));
            window.draw(dot);
        }
        auto content=d->text().substr(begin,end-begin);auto colors=syntax(content);
        for(size_t i=d->leftColumn;i<content.size() && i<d->leftColumn+visibleColumns;) {
            size_t j=i+1;
            while(j<content.size() && j<d->leftColumn+visibleColumns && colors[j]==colors[i])++j;
            label(window,codeFont,content.substr(i,j-i),CodeX+(i-d->leftColumn)*cw,y,CodeSize,colors[i]);i=j;
        }
    }
    if(!findOpen && window.hasFocus() && int(caretClock.getElapsedTime().asSeconds()*2)%2==0 &&
        location.first>=d->topLine && location.first<d->topLine+VisibleLines &&
        location.second>=d->leftColumn && location.second<d->leftColumn+visibleColumns)
        box(window,{CodeX+(location.second-d->leftColumn)*cw,CodeY+(location.first-d->topLine)*LineHeight,1.5f,18},accent);
    if(d->lineCount()>VisibleLines) {
        float h=std::max(16.f,editorArea.height*VisibleLines/d->lineCount());
        float y=editorArea.top+(editorArea.height-h)*std::min(1.f,float(d->topLine)/float(d->lineCount()-VisibleLines));
        box(window,{1259,y,3,h},Color(76,94,101));
    }
    label(window,codeFont,"Ln "+std::to_string(location.first+1)+", Col "+std::to_string(location.second+1),259,674,12,muted);
    if(loaded)label(window,codeFont,registerText(),542,674,12,paused?accent:muted);
    else label(window,uiFont,"F7: build paused at reset    |    F10: step one instruction",890,674,12,muted);
    label(window,uiFont,building?"ASSEMBLING...":"BUILD OUTPUT",258,703,11,building?accent:muted);
    label(window,uiFont,"F5  BUILD & RUN     F6  BUILD",1042,703,11,muted);
    box(window,consoleArea,Color(16,22,26));
    for(size_t i=0;i<8 && consoleTop+i<console.size();++i) {
        auto& line=console[consoleTop+i];bool error=line.find("error")!=std::string::npos || line.find("failed")!=std::string::npos;
        label(window,codeFont,line,264,727+i*17,12,error?Color(239,149,146):Color(166,187,188));
    }
}
void EmulatorIde::drawBoard() {
    {
        std::lock_guard<std::mutex> lock(system.cpu.stateMutex());
        system.lcd.updatePixels();
        for(int y=0;y<snapshot.lcdHeight;++y)for(int x=0;x<snapshot.lcdWidth;++x)
            snapshot.pixels[y*snapshot.lcdWidth+x]=system.lcd.pixelState(x,y);
        snapshot.pc=system.cpu.PC;snapshot.a=system.cpu.A;snapshot.x=system.cpu.X;snapshot.y=system.cpu.Y;
        snapshot.pa=system.registers.portARead();snapshot.pb=system.registers.portBRead();
        snapshot.frequencyHz=system.cpu.cycles.getFrequencyHz();snapshot.started=system.firstReset;
        snapshot.stopped=system.cpu.STOP;snapshot.paused=paused;snapshot.waiting=system.cpu.WAIT;
        snapshot.irq=system.cpu.IRQB||system.registers.irqAsserted();
    }
    label(window,uiFont,loaded?"LOADED  /  "+loadedName:"No ROM loaded. Choose a program and Build & Run.",263,113,14,muted);
    label(window,uiFont,"R  RESET     I  IRQ     N  NMI",1030,113,12,muted);
    auto savedView=window.getView();auto r=fittedBoard();
    View view(FloatRect(0,0,BreadboardView::Width,BreadboardView::Height));
    const auto vp=savedView.getViewport();
    view.setViewport({vp.left+r.left/Width*vp.width,vp.top+r.top/Height*vp.height,
                      r.width/Width*vp.width,r.height/Height*vp.height});window.setView(view);
    auto pressed=mouseButton!=BreadboardView::Button::Released?mouseButton:keyReset?BreadboardView::Button::Reset:
        keyIrq?BreadboardView::Button::Irq:keyNmi?BreadboardView::Button::Nmi:BreadboardView::Button::Released;
    board->draw(window,snapshot,pressed);window.setView(savedView);
    if(loaded)label(window,codeFont,registerText(),266,857,12,paused?accent:muted);
}
void EmulatorIde::drawDialog() {
    box(window,{0,0,Width,Height},Color(0,0,0,170));
    box(window,{425,327,430,228},Color(38,48,55));
    std::string title=dialog==Dialog::NewProgram?"New assembly program":dialog==Dialog::Close?"Save your changes?":
        dialog==Dialog::Reload?"Reload from disk?":"Go to line";
    label(window,uiFont,title,447,348,21,white);label(window,uiFont,"x",824,347,17,muted);
    if(dialog==Dialog::NewProgram || dialog==Dialog::GoToLine) {
        label(window,uiFont,dialog==Dialog::NewProgram?"Filename in VASM (for example: my_program.asm)":"Line number in the current program",448,391,13,muted);
        box(window,{448,419,383,33},Color(22,29,34));
        label(window,codeFont,shorten(dialogText,36)+"_",458,426,15,white);
    } else label(window,uiFont,dialog==Dialog::Close?"Unsaved edits are still open in the editor.":"This replaces this editor's changes with the saved file.",448,397,14,muted);
    label(window,uiFont,shorten(dialogError,58),448,466,12,Color(239,149,146));
    if(dialog==Dialog::Close) {
        button(window,uiFont,{448,497,158,36},"Save all & close",true);
        button(window,uiFont,{620,497,105,36},"Discard");button(window,uiFont,{739,497,93,36},"Cancel");
    } else {
        button(window,uiFont,{620,497,108,36},dialog==Dialog::NewProgram?"Create":dialog==Dialog::Reload?"Reload":"Go",true);
        button(window,uiFont,{742,497,90,36},"Cancel");
    }
}
void EmulatorIde::draw() {
    window.clear(bg);box(window,{0,0,Width,64},panel);box(window,{0,64,Side,810},Color(24,30,35));
    box(window,{Side,64,1,810},edge);box(window,{0,63,Width,1},edge);
    box(window,{18,19,26,26},Color(60,122,109));label(window,codeFont,"02",21,22,15,white);
    label(window,uiFont,"W65C02 STUDIO",57,18,21,white);
    label(window,uiFont,"ASSEMBLY + BREADBOARD",285,24,11,muted);
    button(window,uiFont,saveButton,"Save",false,document()!=nullptr);
    button(window,uiFont,buildButton,"Build",false,!building&&document());
    button(window,uiFont,runButton,building?"Building...":"Build & Run",true,!building&&document());
    button(window,uiFont,stopButton,"Stop",false,loaded);
    double hz;{std::lock_guard<std::mutex> lock(system.cpu.stateMutex());hz=system.cpu.cycles.getFrequencyHz();}
    button(window,uiFont,clockButton,"CLOCK   "+clockText(hz)+"   >");
    label(window,uiFont,"PROGRAMS",18,97,12,muted);button(window,uiFont,{186,87,36,30},"+");
    for(size_t row=0;row<19 && listTop+row<names.size();++row) {
        auto& name=names[listTop+row];float y=133+row*34;
        if(name==selected) {box(window,{8,y,224,31},Color(37,62,62));box(window,{8,y,3,31},accent);}
        bool dirty=documents.count(name)&&documents.at(name).dirty();
        label(window,codeFont,shorten(name,25),21,y+7,12,name==selected?white:muted);
        if(dirty)box(window,{220,y+13,4,4},amber);
    }
    label(window,uiFont,"VASM / .asm files",18,788,12,muted);
    button(window,uiFont,{16,823,94,30},"Refresh");button(window,uiFont,{122,823,100,30},"Reload");
    box(window,{254,70,115,32},tab==Tab::Editor?Color(45,62,66):panel);
    box(window,{377,70,135,32},tab==Tab::Board?Color(45,62,66):panel);
    label(window,uiFont,"Editor",281,76,15,tab==Tab::Editor?accent:muted);
    label(window,uiFont,"Breadboard",393,76,15,tab==Tab::Board?accent:muted);
    label(window,uiFont,"Ctrl+Tab",534,80,11,muted);
    button(window,uiFont,debugButton,"Build & Debug  F7",false,!building&&document());
    button(window,uiFont,pauseButton,paused?"Continue  F8":"Pause  F8",false,loaded&&!building);
    button(window,uiFont,stepButton,"Step  F10",true,loaded&&!building);
    button(window,uiFont,restartButton,"Restart paused  F9",false,loaded&&!building);
    if(tab==Tab::Editor)drawEditor();else drawBoard();
    box(window,{0,874,Width,26},Color(32,46,49));
    label(window,uiFont,shorten(status,146),15,880,12,white);
    if(dialog!=Dialog::Closed)drawDialog();
    window.display();
}
int EmulatorIde::run() {
    while(window.isOpen()) {
        checkBreakpoint();
        Event event{};
        while(window.pollEvent(event)) {
            try {handle(event);syncBreakpoints();}catch(const std::exception& e) {
                if(dialog!=Dialog::Closed)dialogError=e.what();else status=e.what();
            }
        }
        try {finishBuild();}catch(const std::exception& e) {building=false;status=e.what();setConsole(status);}
        checkBreakpoint();
        if(window.isOpen())draw();
    }
    releaseInputs();system.cpu.stop();return 0;
}
