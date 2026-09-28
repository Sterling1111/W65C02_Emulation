#include "gtest/gtest.h"
#include "AsmDocument.h"
#include "IdeWorkspace.h"
#include "System.h"
#include <atomic>
#include <chrono>
#include <fstream>
#include <filesystem>

namespace fs=std::filesystem;
namespace {
void writeFile(const fs::path& path,const std::string& text) {
    std::ofstream file(path,std::ios::binary);file.write(text.data(),text.size());
}
class IdeFiles : public testing::Test {
protected:
    fs::path root;
    void SetUp() override {
        static std::atomic<int> id{0};
        root=fs::temp_directory_path()/("w65c02 ide ' space-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(id++));
        fs::create_directories(root);
    }
    void TearDown() override {std::error_code ignored;fs::remove_all(root,ignored);}
    std::string program="    .org $8000\nreset:\n    lda #$42\n    sta $10\n    stp\n    .org $fffa\n    .word reset\n    .word reset\n    .word reset\n";
};
}
TEST(AsmEditor, ReplaceSelectionUndoRedoAndSavedBaseline) {
    AsmDocument d("lda #1\nsta $6000\n");
    d.setCursor(4);d.setCursor(6,true);d.insert("#$ff");
    EXPECT_EQ(d.text(),"lda #$ff\nsta $6000\n");EXPECT_TRUE(d.dirty());
    d.undo();EXPECT_FALSE(d.dirty());EXPECT_EQ(d.selectedText(),"#1");
    d.redo();d.markSaved();EXPECT_FALSE(d.dirty());
    d.undo();EXPECT_TRUE(d.dirty());d.redo();EXPECT_FALSE(d.dirty());
}
TEST(AsmEditor, NavigationDeletionAndAutoIndent) {
    AsmDocument d("    lda #1\n    sta $6000");
    d.end(false);d.newline();EXPECT_EQ(d.text(),"    lda #1\n    \n    sta $6000");
    EXPECT_EQ(d.location(d.cursor),std::make_pair(size_t(1),size_t(4)));
    d.insert("nop");d.moveVertical(-1,false);EXPECT_EQ(d.location(d.cursor).first,0u);
    d.home(false);EXPECT_EQ(d.cursor,0u);d.backspace();EXPECT_EQ(d.cursor,0u);
    d.end(false,true);d.eraseForward();EXPECT_EQ(d.cursor,d.text().size());
    d.moveHorizontal(-1,false,true);EXPECT_EQ(d.selectedText(),"");
    d.setCursor(0);d.setCursor(4,true);d.backspace();EXPECT_EQ(d.text().substr(0,3),"lda");
}
TEST(AsmEditor, MultiLineIndentExcludesUnselectedNextLine) {
    AsmDocument d("one\ntwo\nthree\n");d.setCursor(0);d.setCursor(8,true);
    d.indent(false);EXPECT_EQ(d.text(),"    one\n    two\nthree\n");
    d.indent(true);EXPECT_EQ(d.text(),"one\ntwo\nthree\n");
    d.undo();EXPECT_EQ(d.text(),"    one\n    two\nthree\n");
}
TEST(AsmEditor, NormalizesLineEndingsAndKeepsTrailingEmptyLine) {
    AsmDocument d("a\r\n\tb\rc\n");EXPECT_EQ(d.text(),"a\n    b\nc\n");
    EXPECT_EQ(d.lineCount(),4u);EXPECT_EQ(d.lineStart(100),d.text().size());
    d.setCursor(999);EXPECT_EQ(d.cursor,d.text().size());d.moveVertical(-100,false);EXPECT_EQ(d.cursor,0u);
}
TEST(AsmEditor, SourceLimitDoesNotDestroyBufferOrUndo) {
    AsmDocument d("original");
    EXPECT_THROW(d.insert(std::string(1024*1024,'x')),std::runtime_error);
    EXPECT_EQ(d.text(),"original");EXPECT_FALSE(d.dirty());
}
TEST_F(IdeFiles, ListsOnlyProgramsAndPreservesUnsavedBufferUntilSave) {
    writeFile(root/"b.asm","b");writeFile(root/"a.asm","a");writeFile(root/"notes.txt","text");
    IdeWorkspace ws(root);EXPECT_EQ(ws.programs(),(std::vector<std::string>{"a.asm","b.asm"}));
    auto d=ws.open("a.asm");d.end(false,true);d.insert("\n");
    EXPECT_EQ(IdeWorkspace::readText(root/"a.asm"),"a");
    ws.save("a.asm",d);EXPECT_EQ(IdeWorkspace::readText(root/"a.asm"),"a\n");EXPECT_FALSE(d.dirty());
}
TEST_F(IdeFiles, SaveRejectsOutsideChangesAndNewFileCollisions) {
    writeFile(root/"a.asm","first");IdeWorkspace ws(root);auto d=ws.open("a.asm");d.insert("edit");
    writeFile(root/"a.asm","outside");EXPECT_THROW(ws.save("a.asm",d),std::runtime_error);
    EXPECT_EQ(IdeWorkspace::readText(root/"a.asm"),"outside");EXPECT_TRUE(d.dirty());
    AsmDocument fresh("new");fresh.onDisk=false;
    EXPECT_THROW(ws.save("a.asm",fresh),std::runtime_error);
    ws.save("new.asm",fresh);EXPECT_EQ(IdeWorkspace::readText(root/"new.asm"),"new");EXPECT_FALSE(fresh.dirty());
}
TEST_F(IdeFiles, PathsCannotEscapeWorkspace) {
    IdeWorkspace ws(root);
    EXPECT_THROW(ws.validateNewName("../outside.asm"),std::runtime_error);
    EXPECT_THROW(ws.validateNewName("/tmp/outside.asm"),std::runtime_error);
    EXPECT_THROW(ws.validateNewName("x;touch bad.asm"),std::runtime_error);
#ifndef _WIN32
    writeFile(root/"target.asm","original");fs::create_symlink(root/"target.asm",root/"link.asm");
    EXPECT_THROW(ws.open("link.asm"),std::runtime_error);
#endif
}
TEST_F(IdeFiles, AssemblesAndExecutesRealRomFromDirectoryWithSpacesAndQuotes) {
    auto result=assembleProgram(TEST_VASM_PATH,root,root/"build files","test.asm",program);
    ASSERT_TRUE(result.success)<<result.output;ASSERT_EQ(fs::file_size(result.rom),32768u);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(result.rom.string());
    s.cpu.reset(s.eeprom[0x7ffc]|s.eeprom[0x7ffd]<<8);s.cpu.execute(10);
    EXPECT_EQ(s.ram[0x10],0x42);EXPECT_TRUE(s.cpu.STOP);
}
TEST_F(IdeFiles, BuildUsesSnapshotAndWorkspaceRelativeIncludes) {
    writeFile(root/"constants.inc","ANSWER = $42\n");
    auto source=program;source.replace(source.find("#$42"),4,"#ANSWER");
    source="    .include \"constants.inc\"\n"+source;
    auto result=assembleProgram(TEST_VASM_PATH,root,root/"build","test.asm",source);
    EXPECT_TRUE(result.success)<<result.output;
    EXPECT_FALSE(fs::exists(root/"test.asm"));
}
TEST_F(IdeFiles, ReportsAssemblerLineAndNeverReusesStaleRom) {
    auto good=assembleProgram(TEST_VASM_PATH,root,root/"build","test.asm",program);ASSERT_TRUE(good.success);
    auto bad=assembleProgram(TEST_VASM_PATH,root,root/"build","test.asm","    .org $8000\nreset:\n    broken_instruction\n");
    EXPECT_FALSE(bad.success);EXPECT_EQ(bad.errorLine,3)<<bad.output;EXPECT_NE(good.rom,bad.rom);
    EXPECT_TRUE(fs::exists(good.rom));EXPECT_FALSE(fs::exists(bad.rom));
}
TEST_F(IdeFiles, RejectsTruncatedRomAndBadResetVector) {
    auto shortRom=assembleProgram(TEST_VASM_PATH,root,root/"build","test.asm","    .org $8000\n    nop\n");
    EXPECT_FALSE(shortRom.success);EXPECT_NE(shortRom.output.find("32 KiB"),std::string::npos);
    auto source=program;auto pos=source.find(".word reset",source.find(".word reset")+1);
    source.replace(pos,11,".word $0000");
    auto invalid=assembleProgram(TEST_VASM_PATH,root,root/"build","test.asm",source);
    EXPECT_FALSE(invalid.success);EXPECT_NE(invalid.output.find("reset vector"),std::string::npos);
}
TEST_F(IdeFiles, NewProgramTemplateBuilds) {
    auto result=assembleProgram(TEST_VASM_PATH,root,root/"build","new.asm",IdeWorkspace::starterProgram());
    EXPECT_TRUE(result.success)<<result.output;
}
TEST_F(IdeFiles, MissingAssemblerProducesFailureInsteadOfRunningPreviousProgram) {
    auto result=assembleProgram(root/"no assembler",root,root/"build","test.asm",program);
    EXPECT_FALSE(result.success);EXPECT_FALSE(result.output.empty());
}
TEST_F(IdeFiles, RomLoaderRejectsInvalidFilesAndClearsTailOnShorterReplacement) {
    EEPROM rom;writeFile(root/"valid.bin",std::string(32768,char(0x42)));rom.loadProgram((root/"valid.bin").string());
    writeFile(root/"huge.bin",std::string(32769,'x'));writeFile(root/"empty.bin","");
    EXPECT_THROW(rom.loadProgram((root/"missing.bin").string()),std::runtime_error);
    EXPECT_THROW(rom.loadProgram((root/"huge.bin").string()),std::runtime_error);
    EXPECT_THROW(rom.loadProgram((root/"empty.bin").string()),std::runtime_error);
    EXPECT_EQ(rom[0],0x42);EXPECT_EQ(rom[32767],0x42);
    writeFile(root/"short.bin",std::string(1,char(0xdb)));rom.loadProgram((root/"short.bin").string());
    EXPECT_EQ(rom[0],0xdb);EXPECT_EQ(rom[1],0xea);EXPECT_EQ(rom[32767],0xea);
}

TEST_F(IdeFiles, DebugListingMapsInstructionAddressesToMainSourceLines) {
    auto result=assembleProgram(TEST_VASM_PATH,root,root/"build","step.asm",program);
    ASSERT_TRUE(result.success)<<result.output;
    EXPECT_EQ(result.sourceLines.at(0x8000),3u);
    EXPECT_EQ(result.sourceLines.at(0x8002),4u);
    EXPECT_EQ(result.sourceLines.at(0x8004),5u);
    EXPECT_EQ(result.sourceLines.count(0x8001),0u); // Operand, not an instruction boundary.
}
TEST_F(IdeFiles, DebugResetExecutesNothingAndStepRunsExactlyOneInstruction) {
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","step.asm",program);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());
    s.ram[0x10]=0;
    s.reset(false);
    EXPECT_EQ(s.cpu.PC,0x8000);EXPECT_EQ(s.cpu.A,0);EXPECT_EQ(s.ram[0x10],0);
    auto cycles=s.cpu.cycles.getCycles();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(s.cpu.cycles.getCycles(),cycles);
    auto step=s.cpu.step();
    EXPECT_EQ(step.kind,W65C02::StepResult::Kind::Instruction);
    EXPECT_EQ(step.address,0x8000);EXPECT_EQ(step.nextPC,0x8002);EXPECT_EQ(step.mnemonic,"LDA");
    EXPECT_EQ(step.cycles,2u);EXPECT_EQ(s.cpu.A,0x42);EXPECT_EQ(s.ram[0x10],0);
    cycles=s.cpu.cycles.getCycles();std::this_thread::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(s.cpu.PC,0x8002);EXPECT_EQ(s.cpu.cycles.getCycles(),cycles);
    step=s.cpu.step();EXPECT_EQ(step.address,0x8002);EXPECT_EQ(s.ram[0x10],0x42);EXPECT_FALSE(s.cpu.STOP);
    step=s.cpu.step();EXPECT_EQ(step.mnemonic,"STP");EXPECT_TRUE(s.cpu.STOP);
    cycles=s.cpu.cycles.getCycles();step=s.cpu.step();
    EXPECT_EQ(step.kind,W65C02::StepResult::Kind::Stopped);EXPECT_EQ(step.cycles,0u);
    EXPECT_EQ(s.cpu.cycles.getCycles(),cycles);
    s.reset(false);EXPECT_FALSE(s.cpu.STOP);EXPECT_EQ(s.cpu.PC,0x8000);EXPECT_EQ(s.cpu.A,0);
}
TEST_F(IdeFiles, StepEntersSubroutinesAndFollowsReturnsAndBranches) {
    auto source="    .org $8000\nreset:\n    jsr sub\n    bra done\nsub:\n    inx\n    rts\ndone:\n    stp\n    .org $fffa\n    .word reset,reset,reset\n";
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","step.asm",source);ASSERT_TRUE(rom.success)<<rom.output;
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    auto step=s.cpu.step();EXPECT_EQ(step.mnemonic,"JSR");EXPECT_EQ(s.cpu.PC,0x8005);EXPECT_EQ(s.cpu.X,0);
    EXPECT_EQ(s.cpu.SP,0xfd);
    step=s.cpu.step();EXPECT_EQ(step.mnemonic,"INX");EXPECT_EQ(s.cpu.X,1);
    step=s.cpu.step();EXPECT_EQ(step.mnemonic,"RTS");EXPECT_EQ(s.cpu.PC,0x8003);EXPECT_EQ(s.cpu.SP,0xff);
    step=s.cpu.step();EXPECT_EQ(step.mnemonic,"BRA");EXPECT_EQ(s.cpu.PC,0x8007);
}
TEST_F(IdeFiles, ContinuePreservesSteppedStateAndDoesNotResetProgram) {
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","step.asm",program);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    s.cpu.step();s.cpu.A=0x79;s.cpu.X=0x31;
    s.cpu.resume();
    bool halted=false;
    for(int i=0;i<100 && !halted;++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::lock_guard<std::mutex> lock(s.cpu.stateMutex());halted=s.cpu.STOP;
    }
    s.cpu.stop();ASSERT_TRUE(halted);
    EXPECT_EQ(s.ram[0x10],0x79);EXPECT_EQ(s.cpu.X,0x31);EXPECT_EQ(s.cpu.A,0x79);
}
TEST_F(IdeFiles, SteppingRunningCpuLeavesWorkerStopped) {
    auto source="    .org $8000\nreset:\n    inx\n    bra reset\n    .org $fffa\n    .word reset,reset,reset\n";
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","step.asm",source);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    auto result=s.cpu.step();EXPECT_EQ(result.kind,W65C02::StepResult::Kind::Instruction);
    auto pc=s.cpu.PC;auto x=s.cpu.X;auto cycles=s.cpu.cycles.getCycles();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(s.cpu.PC,pc);EXPECT_EQ(s.cpu.X,x);EXPECT_EQ(s.cpu.cycles.getCycles(),cycles);
}
TEST_F(IdeFiles, StepClocksViaAndLcdByTheInstructionCycleCount) {
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","step.asm",program);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,.001};s.loadProgram(rom.rom.string());s.reset(false);
    s.lcd.advanceTime(std::chrono::milliseconds(9));ASSERT_TRUE(s.lcd.isBusy());
    s.registers.writeToRegisters(100,W65C22::T1CL);s.registers.writeToRegisters(0,W65C22::T1CH);
    auto result=s.cpu.step();EXPECT_EQ(result.cycles,2u);EXPECT_FALSE(s.lcd.isBusy());
    EXPECT_EQ(s.registers.readFromRegisters(W65C22::T1CL),98);
}
TEST_F(IdeFiles, WaitingStepAdvancesOneClockAndInterruptStepExecutesOneHandlerInstruction) {
    auto source="    .org $8000\nreset:\n    cli\n    wai\n    lda #$33\n    stp\nirq:\n    lda $6004\n    inx\n    rti\n    .org $fffa\n    .word irq,reset,irq\n";
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","step.asm",source);ASSERT_TRUE(rom.success)<<rom.output;
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    s.cpu.step();s.cpu.step();ASSERT_TRUE(s.cpu.WAIT);
    s.registers.writeToRegisters(0xc0,W65C22::IER);
    s.registers.writeToRegisters(0,W65C22::T1CL);s.registers.writeToRegisters(0,W65C22::T1CH);
    auto result=s.cpu.step();EXPECT_EQ(result.kind,W65C02::StepResult::Kind::Waiting);EXPECT_EQ(result.cycles,1u);
    EXPECT_EQ(s.cpu.PC,0x8002);ASSERT_TRUE(s.registers.irqAsserted());
    result=s.cpu.step();EXPECT_EQ(result.kind,W65C02::StepResult::Kind::Instruction);
    EXPECT_EQ(result.address,0x8005);EXPECT_EQ(result.mnemonic,"LDA");EXPECT_EQ(s.cpu.X,0);
    EXPECT_FALSE(s.registers.irqAsserted());EXPECT_FALSE(s.cpu.WAIT);
    s.cpu.step();EXPECT_EQ(s.cpu.X,1);s.cpu.step();EXPECT_EQ(s.cpu.PC,0x8002);
    s.cpu.step();EXPECT_EQ(s.cpu.A,0x33);
}

namespace {
bool awaitBreakpoint(W65C02& cpu) {
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(std::chrono::steady_clock::now()<deadline) {
        if(cpu.breakpointState().hit)return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}
}
TEST_F(IdeFiles, BreakpointStopsBeforeStoreAndFreezesPeripheralClocks) {
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","break.asm",program);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    s.ram[0x10]=0;s.cpu.setBreakpoints({0x8002});s.cpu.resume();
    ASSERT_TRUE(awaitBreakpoint(s.cpu));
    uint64_t cycles;
    {std::lock_guard<std::mutex> lock(s.cpu.stateMutex());
        EXPECT_EQ(s.cpu.PC,0x8002);EXPECT_EQ(s.cpu.A,0x42);EXPECT_EQ(s.ram[0x10],0);
        cycles=s.cpu.cycles.getCycles();EXPECT_EQ(cycles,2u);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {std::lock_guard<std::mutex> lock(s.cpu.stateMutex());EXPECT_EQ(s.cpu.cycles.getCycles(),cycles);}
    s.cpu.step();EXPECT_EQ(s.ram[0x10],0x42);EXPECT_EQ(s.cpu.PC,0x8004);EXPECT_FALSE(s.cpu.breakpointState().hit);
}
TEST_F(IdeFiles, ContinuePassesCurrentBreakpointOnceAndStopsAgainOnNextIteration) {
    const std::string source="    .org $8000\nreset:\n    inx\n    bra reset\n    .org $fffa\n    .word reset,reset,reset\n";
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","break.asm",source);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    s.cpu.setBreakpoints({0x8000});s.cpu.resume();ASSERT_TRUE(awaitBreakpoint(s.cpu));
    {std::lock_guard<std::mutex> lock(s.cpu.stateMutex());EXPECT_EQ(s.cpu.X,0);EXPECT_EQ(s.cpu.cycles.getCycles(),0u);}
    for(int i=1;i<=3;++i) {
        s.cpu.resume();ASSERT_TRUE(awaitBreakpoint(s.cpu));
        std::lock_guard<std::mutex> lock(s.cpu.stateMutex());EXPECT_EQ(s.cpu.X,i);EXPECT_EQ(s.cpu.PC,0x8000);
    }
    s.reset(false);s.cpu.resume();ASSERT_TRUE(awaitBreakpoint(s.cpu));
    {std::lock_guard<std::mutex> lock(s.cpu.stateMutex());EXPECT_EQ(s.cpu.X,0);EXPECT_EQ(s.cpu.PC,0x8000);}
}
TEST_F(IdeFiles, StepBypassesCurrentBreakpointButContinueHonorsNextOne) {
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","break.asm",program);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    s.cpu.setBreakpoints({0x8000,0x8002,0x8004});s.cpu.resume();ASSERT_TRUE(awaitBreakpoint(s.cpu));
    auto result=s.cpu.step();EXPECT_EQ(result.address,0x8000);EXPECT_EQ(s.cpu.PC,0x8002);
    s.cpu.resume();ASSERT_TRUE(awaitBreakpoint(s.cpu));EXPECT_EQ(s.cpu.breakpointState().address,0x8002);
    s.cpu.setBreakpoints({0x8004});s.cpu.resume();ASSERT_TRUE(awaitBreakpoint(s.cpu));
    EXPECT_EQ(s.cpu.breakpointState().address,0x8004);
    {std::lock_guard<std::mutex> lock(s.cpu.stateMutex());EXPECT_EQ(s.ram[0x10],0x42);EXPECT_FALSE(s.cpu.STOP);}
    s.cpu.setBreakpoints({});s.cpu.resume();
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    bool stopped=false;
    while(std::chrono::steady_clock::now()<deadline) {
        {std::lock_guard<std::mutex> lock(s.cpu.stateMutex());stopped=s.cpu.STOP;}
        if(stopped)break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    s.cpu.stop();EXPECT_TRUE(stopped);EXPECT_FALSE(s.cpu.breakpointState().hit);
}
TEST_F(IdeFiles, BreakpointCatchesFirstInterruptHandlerInstructionAfterWai) {
    const std::string source="    .org $8000\nreset:\n    cli\n    wai\n    stp\nirq:\n    inx\n    rti\n    .org $fffa\n    .word irq,reset,irq\n";
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","break.asm",source);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    s.cpu.step();s.cpu.step();ASSERT_TRUE(s.cpu.WAIT);
    s.cpu.setBreakpoints({0x8003});s.cpu.interrupt(false,true);s.cpu.resume();ASSERT_TRUE(awaitBreakpoint(s.cpu));
    {std::lock_guard<std::mutex> lock(s.cpu.stateMutex());
        EXPECT_EQ(s.cpu.PC,0x8003);EXPECT_EQ(s.cpu.X,0);EXPECT_EQ(s.cpu.SP,0xfc);EXPECT_FALSE(s.cpu.WAIT);
    }
    auto result=s.cpu.step();EXPECT_EQ(result.address,0x8003);EXPECT_EQ(result.cycles,2u);EXPECT_EQ(s.cpu.X,1);
    s.cpu.interrupt(false,false);s.cpu.step();EXPECT_EQ(s.cpu.PC,0x8002);
}
TEST_F(IdeFiles, BreakpointsCanBeAddedWhileCpuRunsAndDoNotAffectSynchronousExecution) {
    const std::string source="    .org $8000\nreset:\n    inx\n    bra reset\n    .org $fffa\n    .word reset,reset,reset\n";
    auto rom=assembleProgram(TEST_VASM_PATH,root,root/"build","break.asm",source);ASSERT_TRUE(rom.success);
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(rom.rom.string());s.reset(false);
    s.cpu.resume();s.cpu.setBreakpoints({0x8000});ASSERT_TRUE(awaitBreakpoint(s.cpu));s.cpu.stop();
    s.cpu.reset(0x8000);s.cpu.execute(2);EXPECT_EQ(s.cpu.X,1);EXPECT_EQ(s.cpu.PC,0x8000);
    EXPECT_FALSE(s.cpu.breakpointState().hit);
}
