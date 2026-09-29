#include "gtest/gtest.h"
#include "System.h"
#include "SerialTerminal.h"
#include <string>

namespace {
void clocks(W65C51& acia,unsigned count,bool ready=true) {
    while(count--)acia.tick(1000000,ready);
}
void setup(W65C51& acia) { acia.write(0x1f,W65C51::Control);acia.write(0x09,W65C51::Command); }
}
TEST(Acia, RegistersResetAndProgramResetPreservesControlAndCommandHighBits) {
    W65C51 acia;EXPECT_EQ(acia.status(),0x10);EXPECT_EQ(acia.read(2),0);EXPECT_EQ(acia.read(3),0);
    acia.write(0xe9,2);acia.write(0x1f,3);acia.receive(0x42);acia.receive(0x43);
    EXPECT_TRUE(acia.status()&4);acia.write(0,1);
    EXPECT_EQ(acia.read(2),0xe0);EXPECT_EQ(acia.read(3),0x1f);EXPECT_FALSE(acia.status()&4);
    EXPECT_FALSE(acia.irqAsserted());acia.reset();EXPECT_EQ(acia.status(),0x10);
}
TEST(Acia, ReceiveIrqAcknowledgementAndOverrunAreSeparateFromDataRead) {
    W65C51 acia;setup(acia);acia.receive(0x42);EXPECT_TRUE(acia.irqAsserted());
    EXPECT_EQ(acia.read(1),0x98);EXPECT_FALSE(acia.irqAsserted());EXPECT_EQ(acia.status(),0x18);
    acia.receive(0x43);EXPECT_TRUE(acia.status()&4);EXPECT_TRUE(acia.irqAsserted());
    EXPECT_EQ(acia.read(0),0x42);EXPECT_FALSE(acia.status()&0x0e);
    acia.read(1);EXPECT_FALSE(acia.irqAsserted());
    acia.receive(0x44,true);EXPECT_TRUE(acia.status()&2);acia.read(0);EXPECT_FALSE(acia.status()&2);
}
TEST(Acia, ReceiveInterruptCanBeDisabledWithoutDisablingPolling) {
    W65C51 acia;setup(acia);acia.write(0x0b,2);acia.receive(0x42);
    EXPECT_FALSE(acia.irqAsserted());EXPECT_TRUE(acia.status()&8);EXPECT_EQ(acia.read(0),0x42);
}
TEST(Acia, WdcTransmitFlagStaysSetAndEarlyWriteReplacesUnbufferedFrame) {
    W65C51 acia;setup(acia);acia.write('A',0);clocks(acia,500);
    EXPECT_TRUE(acia.status()&0x10);EXPECT_TRUE(acia.takeOutput().empty());
    acia.write('B',0);clocks(acia,521);EXPECT_EQ(acia.takeOutput(),"B");
    EXPECT_EQ(acia.transmitted(),1u);
}
TEST(Acia, SerialFramesHonorBaudRateWordLengthAndCts) {
    W65C51 acia;setup(acia);acia.setModemInputs(false,true,true);acia.write('A',0);
    clocks(acia,2000);EXPECT_TRUE(acia.takeOutput().empty());
    acia.setModemInputs(true,true,true);clocks(acia,521);EXPECT_EQ(acia.takeOutput(),"A");
    acia.write(0x3e,3); // 9600 baud, seven data bits.
    acia.write(0xc2,0);clocks(acia,938);EXPECT_EQ(acia.takeOutput(),"B");
}
TEST(Acia, TerminalQueueWaitsForRtsAndExternalFlowControl) {
    W65C51 acia;acia.queueInput("AB");clocks(acia,2000);EXPECT_EQ(acia.received(),0u);
    setup(acia);clocks(acia,2000,false);EXPECT_EQ(acia.received(),0u);
    clocks(acia,522);EXPECT_EQ(acia.read(0),'A');clocks(acia,522);EXPECT_EQ(acia.read(0),'B');
    EXPECT_EQ(acia.pendingInput(),0u);
}
TEST(Acia, SystemMapsRegistersAndViaPa0StopsTerminalSender) {
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};
    s.bus.write(0x1f,0x5003);s.bus.write(0x09,0x5002);
    EXPECT_EQ(s.bus.read(0x5003),0x1f);
    s.registers.writeToRegisters(1,W65C22::DDRA);s.registers.portAWrite(1);s.acia.queueInput("Z");
    for(int i=0;i<1000;++i)s.bus.tick();EXPECT_EQ(s.acia.received(),0u);
    s.registers.portAWrite(0);for(int i=0;i<1000;++i)s.bus.tick();
    EXPECT_TRUE(s.bus.irqAsserted());EXPECT_EQ(s.bus.read(0x5001),0x98);
    EXPECT_EQ(s.bus.read(0x5000),'Z');EXPECT_FALSE(s.bus.irqAsserted());
    s.reset(false);EXPECT_EQ(s.acia.status(),0x10);EXPECT_EQ(s.acia.pendingInput(),0u);
}
TEST(SerialTerminal, PasteNormalizesLineEndingsAndRejectsNonAsciiAtomically) {
    EXPECT_EQ(SerialTerminal::input("10 PRINT 1\r\n20 END\nRUN\r"),"10 PRINT 1\r20 END\rRUN\r");
    EXPECT_THROW(SerialTerminal::input("bad\xc3\xa9"),std::runtime_error);
}
TEST(SerialTerminal, HandlesCarriageReturnBackspaceAndBoundedScrollback) {
    SerialTerminal terminal;terminal.append("ABC\rX\n123\bX");EXPECT_EQ(terminal.text(),"XBC\n12X");
    for(int i=0;i<3000;++i)terminal.append("line\r\n");EXPECT_EQ(terminal.lines().size(),2000u);
    terminal.clear();EXPECT_EQ(terminal.text(),"");
}
class SerialFirmware : public testing::Test {
protected:
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};
    std::string output;
    void SetUp() override {s.loadProgram(TEST_SERIAL_ROM);s.ram.initialize();s.reset(false);}
    bool until(const std::string& needle) {
        for(unsigned i=0;i<40000;++i) {
            s.cpu.execute(128);output+=s.acia.takeOutput();
            if(output.find(needle)!=std::string::npos)return true;
        }
        return false;
    }
    void send(const std::string& command) {output.clear();s.acia.queueInput(command);}
    void basic() {
        ASSERT_TRUE(until("\\\r\n"));send("8000R\r");
        ASSERT_TRUE(until("MEMORY SIZE?"))<<output;send("\r");
        ASSERT_TRUE(until("TERMINAL WIDTH?"))<<output;send("80\r");
        ASSERT_TRUE(until("\r\nOK\r\n"))<<output;
        ASSERT_NE(output.find("15359 BYTES FREE"),std::string::npos);
    }
};
TEST_F(SerialFirmware, WozmonExaminesStoresAndRunsMachineCodeThroughSerial) {
    ASSERT_TRUE(until("\\\r\n"));send("0400: A9 5A 85 10 4C 00 FE\r");
    ASSERT_TRUE(until("0400: EA\r\n"))<<output;
    send("0400.0406\r");ASSERT_TRUE(until("0400: A9 5A 85 10 4C 00 FE"))<<output;
    send("0400R\r");ASSERT_TRUE(until("\\\r\n"))<<output;
    EXPECT_EQ(s.ram[0x10],0x5a);EXPECT_FALSE(s.cpu.STOP);
}
TEST_F(SerialFirmware, BasicRunsArithmeticLoopsStringsAndMemoryAccess) {
    basic();ASSERT_FALSE(HasFatalFailure());
    send("PRINT 2+2;1.5*2\r");ASSERT_TRUE(until("\r\nOK\r\n"))<<output;
    EXPECT_NE(output.find(" 4  3 "),std::string::npos)<<output;
    send("10 FOR I=1 TO 3\r20 PRINT I\r30 NEXT I\rRUN\r");
    ASSERT_TRUE(until("\r\nOK\r\n"))<<output;
    EXPECT_NE(output.find(" 1 \r\n 2 \r\n 3 "),std::string::npos)<<output;
    send("PRINT \"SERIAL OK\"\r");ASSERT_TRUE(until("\r\nOK\r\n"))<<output;
    EXPECT_NE(output.find("\r\nSERIAL OK\r\n"),std::string::npos)<<output;
    send("POKE 12288,42:PRINT PEEK(12288)\r");ASSERT_TRUE(until("\r\nOK\r\n"))<<output;
    EXPECT_EQ(s.ram[12288],42);EXPECT_NE(output.find(" 42 "),std::string::npos)<<output;
}
TEST_F(SerialFirmware, LongPasteUsesBiosFlowControlAndBreakReturnsToPrompt) {
    basic();ASSERT_FALSE(HasFatalFailure());
    std::string program;
    for(int i=1;i<=60;++i)program+=std::to_string(i*10)+" REM FLOW CONTROL TEST LINE\r";
    program+="610 PRINT 12345\r620 END\rRUN\r";
    send(program);ASSERT_TRUE(until("\r\nOK\r\n"))<<output;
    EXPECT_NE(output.find(" 12345 "),std::string::npos)<<output;
    EXPECT_FALSE(s.acia.status()&4);EXPECT_EQ(s.acia.pendingInput(),0u);
    send("NEW\r");ASSERT_TRUE(until("\r\nOK\r\n"));
    send("10 GOTO 10\rRUN\r");s.cpu.execute(100000);s.acia.takeOutput();
    send(std::string(1,3));ASSERT_TRUE(until("\r\nOK\r\n"))<<output;
    EXPECT_NE(output.find("BREAK"),std::string::npos)<<output;
}
