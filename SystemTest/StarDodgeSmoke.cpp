// Real assembled firmware check: no GUI, host sleeps, or replacement game logic.
#include "System.h"
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool value,const char* why) {if(!value)throw std::runtime_error(why);}
void until(System& s,const std::function<bool()>& done,uint64_t budget=600000) {
    const auto end=s.cpu.cycles.getCycles()+budget;
    do {s.cpu.execute();if(done())return;}while(s.cpu.cycles.getCycles()<end);
    throw std::runtime_error("Firmware did not reach expected state within cycle budget");
}
bool idle(System& s) {
    return s.cpu.WAIT && !s.cpu.NMIB && s.ram[8]==s.ram[9] && s.ram[10]==s.ram[11];
}
void press(System& s) {
    s.cpu.interrupt(true,true);
    until(s,[&]{return idle(s);});
    s.cpu.interrupt(true,false);
}
void nextFrame(System& s) {
    auto frame=s.ram[0x11];until(s,[&]{return s.ram[0x11]!=frame && idle(s);});
}
int number(System& s,int address) {return s.ram[address]+10*s.ram[address+1]+100*s.ram[address+2];}
std::string digits(int n) {return {char('0'+n/100),char('0'+n/10%10),char('0'+n%10)};}
const unsigned char sprites[]={
    16,24,30,15,30,24,16,0, 14,31,27,31,23,31,14,0,
    4,21,14,31,14,21,4,0, 17,10,4,27,4,10,17,0
};
void pixels(System& s,const std::string& first,const std::string& second) {
    require(first.size()==16 && second.size()==16,"Expected complete LCD rows");
    auto lcd=vrEmuLcdNew(16,2,EmuLcdRomA00);
    vrEmuLcdSendCommand(lcd,0x38);vrEmuLcdSendCommand(lcd,0x0c);vrEmuLcdSendCommand(lcd,6);
    vrEmuLcdSendCommand(lcd,0x40);for(auto c:sprites)vrEmuLcdWriteByte(lcd,c);
    vrEmuLcdSendCommand(lcd,0x80);for(auto c:first)vrEmuLcdWriteByte(lcd,c);
    vrEmuLcdSendCommand(lcd,0xc0);for(auto c:second)vrEmuLcdWriteByte(lcd,c);
    vrEmuLcdUpdatePixels(lcd);s.lcd.updatePixels();int mismatch=0;
    for(int y=0;y<s.lcd.numPixelsY();++y)for(int x=0;x<s.lcd.numPixelsX();++x)
        mismatch+=s.lcd.pixelState(x,y)!=vrEmuLcdPixelState(lcd,x,y);
    vrEmuLcdDestroy(lcd);
    if(mismatch)throw std::runtime_error("LCD mismatch: mode="+std::to_string(s.ram[0])+" frame="+std::to_string(s.ram[0x11])+" score="+std::to_string(number(s,2))+" pixels="+std::to_string(mismatch));
}
void coursePixels(System& s) {
    std::string rows[2];
    for(int row=0;row<2;++row) {
        for(int x=0;x<12;++x) {
            int c=s.ram[(row?0x30:0x20)+x];rows[row]+=char(c?c:' ');
        }
        if(s.ram[1]==row)rows[row][0]=0;
        rows[row]+='|';rows[row]+=digits(number(s,row?5:2));
    }
    pixels(s,rows[0],rows[1]);
}
void dodgeNext(System& s) {
    if(s.ram[0x21]==1 && s.ram[1]!=1)press(s);
    else if(s.ram[0x31]==1 && s.ram[1]!=0)press(s);
    nextFrame(s);require(s.ram[0]==1,"Autopilot collided despite choosing the clear lane");
    coursePixels(s);
}
}
int main(int argc,char** argv) try {
    require(argc==2,"Expected star_dodge.bin ROM path");
    System s{0,0x3fff,0x6000,0x7fff,0x8000,0xffff,1};s.loadProgram(argv[1]);s.reset(false);
    until(s,[&]{return s.ram[0x12]==1 && idle(s);});
    require(s.ram[0]==0,"Expected title mode");pixels(s,"   STAR DODGE   ","1MHz N:PLAY/SWAP");
    press(s);require(s.ram[0]==1 && s.ram[1]==0,"NMI did not start a new game");coursePixels(s);
    press(s);require(s.ram[1]==1,"NMI did not switch lanes");coursePixels(s);
    press(s);require(s.ram[1]==0,"Second NMI did not switch back");
    bool sawUpper=false,sawLower=false;
    for(int frames=0;number(s,2)<20 && frames<120;++frames) {
        dodgeNext(s);sawUpper|=s.ram[0x2b]==1;sawLower|=s.ram[0x3b]==1;
        require(number(s,2)==number(s,5),"Best score did not track a new record");
    }
    require(number(s,2)==20 && s.ram[0x0d]==3,"Scoring/carry/difficulty progression failed");
    require(sawUpper && sawLower,"Course did not spawn hazards in both lanes");
    for(int frames=0;frames<5 && !s.ram[0x21] && !s.ram[0x31];++frames)nextFrame(s);
    require(s.ram[0x21]==1 || s.ram[0x31]==1,"No incoming rock for collision check");
    int rockLane=s.ram[0x21]==1?0:1;if(s.ram[1]!=rockLane)press(s);
    nextFrame(s);require(s.ram[0]==2,"Hitting a rock did not end the round");
    pixels(s,std::string(1,char(3))+"BOOM! SCORE 020","BEST 020 N:RETRY");
    press(s);require(s.ram[0]==1 && number(s,2)==0 && number(s,5)==20,"Retry did not retain best/reset score");
    coursePixels(s);
    // Counter rollover and score saturation boundaries, with real input/render paths.
    s.ram[8]=s.ram[9]=255;press(s);require(s.ram[8]==0 && s.ram[9]==0,"NMI sequence wrap lost input");
    s.ram[2]=s.ram[5]=8;s.ram[3]=s.ram[4]=s.ram[6]=s.ram[7]=9;
    for(int frames=0;number(s,2)<999 && frames<20;++frames)dodgeNext(s);
    require(number(s,2)==999 && number(s,5)==999,"Score did not carry to 999");
    for(int frames=0;frames<5;++frames)dodgeNext(s);
    require(number(s,2)==999,"Score overflowed instead of saturating");
    s.reset(false);until(s,[&]{return s.ram[0x12]==1 && idle(s);});
    require(number(s,5)==0 && s.ram[0]==0,"Reset did not return to a fresh title");
    pixels(s,"   STAR DODGE   ","1MHz N:PLAY/SWAP");
    std::cout<<"PASS: title/CGRAM pixels, NMI controls, both lanes, 20 stars, decimal carry, speed progression, collision, retry/best, counter wrap, score saturation, reset\n";
} catch(const std::exception& error) {
    std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;
}
