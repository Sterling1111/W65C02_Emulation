#include "gtest/gtest.h"
#include "System.h"

class CpuConformance : public testing::Test {
protected:
    System system{0,0xffff,-1,-1,-1,-1,1};
    W65C02& cpu=system.cpu;
    RAM& ram=system.ram;
    void program(std::initializer_list<byte> bytes,word pc=0x800) {
        cpu.reset(pc);cpu.cycles.reset();cpu.IRQB=cpu.NMIB=false;
        for(auto b:bytes)ram[pc++]=b;
    }
};
TEST_F(CpuConformance, EveryBitBranchTestsMemoryAndUsesSignedDisplacement) {
    for(int bit=0;bit<8;++bit)for(bool setBranch:{false,true})for(bool set:{false,true})
    for(word pc:{word(0x800),word(0x8fc)})for(byte offset:{byte(5),byte(0xf8)}) {
        const byte opcode=(setBranch?0x8f:0x0f)+16*bit;
        program({opcode,0x10,offset},pc);ram[0x10]=set?(1<<bit):0;
        const auto ps=cpu.PS;const word next=pc+3;
        const bool taken=set==setBranch;
        const word target=next+static_cast<sbyte>(offset);
        cpu.execute();
        EXPECT_EQ(cpu.PC,taken?target:next)<<"opcode "<<unsigned(opcode);
        EXPECT_EQ(cpu.cycles.getCycles(),5u+taken+(taken&&((target&0xff00)!=(next&0xff00))));
        EXPECT_EQ(cpu.PS,ps);
    }
}
TEST_F(CpuConformance, ZeroPageYWrapHasNoPageCrossingPenalty) {
    program({0xb6,0xff});cpu.Y=1;ram[0]=0x42;ram[0x100]=0x99;cpu.execute();
    EXPECT_EQ(cpu.X,0x42);EXPECT_EQ(cpu.cycles.getCycles(),4u);
    program({0x96,0xff});cpu.Y=1;cpu.X=0x71;cpu.execute();
    EXPECT_EQ(ram[0],0x71);EXPECT_EQ(ram[0x100],0x99);EXPECT_EQ(cpu.cycles.getCycles(),4u);
}
TEST_F(CpuConformance, EveryReservedNopPreservesStateAndConsumesDocumentedBytesAndCycles) {
    for(unsigned code=0;code<256;++code) {
        if(cpu.opCodeMatrix[code].instruction!=&W65C02::XXX)continue;
        unsigned bytes=1,cycles=1;
        switch(code) {
        case 0x02:case 0x22:case 0x42:case 0x62:case 0x82:case 0xc2:case 0xe2:bytes=2;cycles=2;break;
        case 0x44:bytes=2;cycles=3;break;
        case 0x54:case 0xd4:case 0xf4:bytes=2;cycles=4;break;
        case 0x5c:bytes=3;cycles=8;break;
        case 0xdc:case 0xfc:bytes=3;cycles=4;break;
        }
        program({static_cast<byte>(code),0x34,0x12});cpu.A=0x42;cpu.X=1;cpu.Y=7;
        const auto ps=cpu.PS;ram[0x1234]=0x99;cpu.execute();
        EXPECT_EQ(cpu.PC,0x800+bytes)<<code;EXPECT_EQ(cpu.cycles.getCycles(),cycles)<<code;
        EXPECT_EQ(cpu.A,0x42);EXPECT_EQ(cpu.X,1);EXPECT_EQ(cpu.Y,7);EXPECT_EQ(cpu.PS,ps);
        EXPECT_EQ(ram[0x1234],0x99);
    }
}
TEST_F(CpuConformance, StpIsOneBytePreservesCapturedCyclesAndRequiresReset) {
    program({0xdb,0xe8});cpu.execute();
    EXPECT_TRUE(cpu.STOP);EXPECT_EQ(cpu.PC,0x801);EXPECT_EQ(cpu.cycles.getCycles(),2u);
    cpu.IRQB=cpu.NMIB=true;cpu.execute();EXPECT_EQ(cpu.PC,0x801);EXPECT_EQ(cpu.X,0);
    cpu.reset(0x801);cpu.IRQB=cpu.NMIB=false;cpu.execute();EXPECT_EQ(cpu.X,1);
}
TEST_F(CpuConformance, SimultaneousNmiAndIrqTakeOnlyNmiAndPushHardwareStatus) {
    program({0xea});cpu.PS.reset(W65C02::I);cpu.PS.set(W65C02::B);
    ram[0xfffa]=0;ram[0xfffb]=0x10;ram[0xfffe]=0;ram[0xffff]=0x20;ram[0x1000]=0xea;
    cpu.IRQB=cpu.NMIB=true;cpu.execute();
    EXPECT_EQ(cpu.PC,0x1001);EXPECT_EQ(cpu.SP,0xfc);EXPECT_FALSE(ram[0x1fd]&0x10);
    EXPECT_TRUE(ram[0x1fd]&0x20);EXPECT_EQ(ram[0x1ff],0x08);EXPECT_EQ(ram[0x1fe],0x00);
    EXPECT_TRUE(cpu.IRQB);EXPECT_FALSE(cpu.NMIB);EXPECT_TRUE(cpu.PS.test(W65C02::I));
}
TEST_F(CpuConformance, BinaryAdcAndSbcExhaustiveResultsAndFlags) {
    for(byte opcode:{byte(0x69),byte(0xe9)}) {
        program({opcode,0});
        for(int a=0;a<256;++a)for(int b=0;b<256;++b)for(int carry=0;carry<2;++carry) {
            cpu.PC=0x800;cpu.A=a;cpu.PS=0x20|carry;ram[0x801]=b;
            const int total=opcode==0x69?a+b+carry:a-b-(1-carry),result=total&255;
            const bool overflow=opcode==0x69?(~(a^b)&(a^result)&128):((a^b)&(a^result)&128);
            const bool cf=opcode==0x69?total>255:total>=0;
            const unsigned flags=(result&128)|(overflow?64:0)|(result==0?2:0)|cf;
            cpu.execute();
            ASSERT_EQ(cpu.A,result)<<unsigned(opcode)<<" "<<a<<" "<<b<<" "<<carry;
            ASSERT_EQ(cpu.PS.to_ulong()&0xc3,flags)<<unsigned(opcode)<<" "<<a<<" "<<b<<" "<<carry;
        }
    }
}
