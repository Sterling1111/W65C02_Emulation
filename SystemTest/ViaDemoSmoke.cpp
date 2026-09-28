// Integration check for the assembled VASM/via_demo.asm ROM.
// Run ViaDemoSmoke with the assembled binary path; no window or wall-clock waits.
#include "System.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <memory>
#include <string>
using Board = std::unique_ptr<System>;
void require(bool good, const char* message) { if(!good) throw std::runtime_error(message); }
unsigned count(System& s,unsigned addr) { return s.ram[addr] | s.ram[addr+1]<<8; }
Board board(const char* file,double mhz) {
    Board s(new System{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,mhz});
    s->loadProgram(file);
    s->cpu.reset(s->eeprom[0x7ffc] | s->eeprom[0x7ffd]<<8);
    return s;
}
void pixels(System& s,const std::string& row1,const std::string& row2) {
    auto r=vrEmuLcdNew(16,2,EmuLcdRomA00);
    vrEmuLcdSendCommand(r,0x38); vrEmuLcdSendCommand(r,0x0c); vrEmuLcdSendCommand(r,6);
    for(auto c:row1) vrEmuLcdWriteByte(r,c);
    vrEmuLcdSendCommand(r,0xc0);
    for(auto c:row2) vrEmuLcdWriteByte(r,c);
    vrEmuLcdUpdatePixels(r); s.lcd.updatePixels();
    int mismatches=0;
    for(int y=0;y<s.lcd.numPixelsY();++y)
        for(int x=0;x<s.lcd.numPixelsX();++x)
            mismatches += s.lcd.pixelState(x,y)!=vrEmuLcdPixelState(r,x,y);
    vrEmuLcdDestroy(r);
    require(mismatches==0,"LCD pixels differ from expected complete frame");
}
void completedFrame(System& s) {
    const auto frame=s.ram[14];
    const auto deadline=s.cpu.cycles.getCycles()+50000;
    while(s.ram[14]==frame && s.cpu.cycles.getCycles()<deadline) s.cpu.execute();
    require(s.ram[14]!=frame,"No completed LCD frame");
}
void checkFrame(System& s) {
    std::ostringstream row;
    row << "T1:" << std::uppercase << std::hex << std::setfill('0') << std::setw(4)
        << count(s,4) << " T2:" << std::setw(4) << count(s,6) << ' ';
    pixels(s,"VIA OK   SR:A5   ",row.str());
}
int main(int argc,char** argv) try {
    require(argc==2,"Expected ROM path");
    for(double mhz:{.001,1.0}) {
        auto s=board(argv[1],mhz);
        bool waited=false;
        while(s->cpu.cycles.getCycles()<100000) {
            s->cpu.execute(); waited |= s->cpu.WAIT;
        }
        require(s->ram[9]==0x55 && !s->ram[10] && s->ram[8]==0xa5,"Startup self-test failed");
        require(count(*s,0)>count(*s,2) && count(*s,2)>2,"Timer IRQ counters not progressing independently");
        require(waited,"Foreground did not use WAI");
        completedFrame(*s); checkFrame(*s);
        const auto limit=s->cpu.cycles.getCycles()+20000;
        while(!s->cpu.WAIT && s->cpu.cycles.getCycles()<limit) s->cpu.execute();
        require(s->cpu.WAIT,"Did not reach idle for wrap test");
        s->ram[0]=s->ram[1]=s->ram[2]=s->ram[3]=0xff;
        const auto until=s->cpu.cycles.getCycles()+25000;
        while(s->cpu.cycles.getCycles()<until) s->cpu.execute();
        require(count(*s,0)<10 && count(*s,2)<10,"16-bit counters did not wrap");
        completedFrame(*s); checkFrame(*s);
        // Repeat board reset with the LCD retaining its existing DDRAM.
        s->registers.reset();
        s->cpu.reset(s->eeprom[0x7ffc] | s->eeprom[0x7ffd]<<8);
        const auto end=s->cpu.cycles.getCycles()+100000;
        while(s->cpu.cycles.getCycles()<end) s->cpu.execute();
        require(s->ram[9]==0x55 && !s->ram[10],"Restart did not pass");
        completedFrame(*s); checkFrame(*s);
        std::cout << "PASS " << mhz << " MHz: self-tests, IRQs, WAI, LCD frames, counter wrap, reset\n";
    }
    for(int fault:{3,6}) {
        auto s=board(argv[1],1);
        s->cpu.cycles.setTickCallback([&] {
            s->lcd.tick(s->cpu.cycles.getFrequencyHz());
            if(s->ram[12]!=fault) s->bus.tick(); // Suppress timer/SR clocks at chosen test.
        });
        while(!s->cpu.STOP && s->cpu.cycles.getCycles()<100000) s->cpu.execute();
        require(s->cpu.STOP && s->ram[9]==0xee && s->ram[10]==fault,"Injected fault falsely passed");
        pixels(*s,std::string("VIA FAIL CODE:0")+char('0'+fault),"Press R to retry");
        std::cout << "PASS injected missing clock: failure code " << fault << ", bounded exit and LCD\n";
    }
    auto s=board(argv[1],1);
    while(s->cpu.cycles.getCycles()<100000) {
        s->cpu.execute();
        // Mask IRQ delivery whenever the runtime enables its two timers.
        if((s->registers.readFromRegisters(W65C22::IER)&0x60)==0x60 && s->ram[12]==6)
            s->registers.writeToRegisters(0x7f,W65C22::IER);
    }
    require(s->ram[9]==0 && count(*s,0)==0 && count(*s,2)==0,"Claims success without timer IRQ delivery");
    pixels(*s,"IRQ WAIT SR:--  ","T1:0000 T2:0000 ");
    std::cout << "PASS masked timer IRQs: stays in IRQ WAIT, never reports success\n";
}
catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
}
