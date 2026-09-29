#include "W65C51.h"
#include <stdexcept>

void W65C51::reset() {
    command=control=rx=tx=rxFrame=0;
    full=overrun=framing=irq=false;txRemaining=rxRemaining=0;
    input.clear();output.clear();txCount=rxCount=0;
}
byte W65C51::status() const {
    // W65C51N's TDRE is always set, including during transmission.
    return (irqAsserted()?0x80:0)|(!dsr?0x40:0)|(!dcd?0x20:0)|0x10|
           (full?8:0)|(overrun?4:0)|(framing?2:0);
}
byte W65C51::read(word reg) {
    switch(reg&3) {
    case Data: { const auto value=rx;full=overrun=framing=false;return value; }
    case Status: { const auto value=status();irq=false;return value; }
    case Command:return command;
    default:return control;
    }
}
void W65C51::write(byte data,word reg) {
    switch(reg&3) {
    case Data:
        // There is no transmit holding buffer: an early write replaces the frame.
        tx=data&mask();txRemaining=frameSeconds();break;
    case Status:
        command&=0xe0;overrun=false;irq=false;break;
    case Command:
        command=data;
        if(!(command&1) || (command&2))irq=false;
        else if(full)irq=true;
        break;
    case Control:control=data;break;
    }
}
double W65C51::baudRate() const {
    static constexpr double rates[]={115200,50,75,109.92,134.58,150,300,600,1200,1800,2400,3600,4800,7200,9600,19200};
    return rates[control&15];
}
double W65C51::frameSeconds(bool receiving) const {
    const unsigned bits=8-((control>>5)&3);
    const double stops=(control&0x80)?(bits==5?1.5:2):1;
    // External RxC defaults to a connected 1.8432 MHz clock / 16.
    return (1+bits+stops)/(receiving && !(control&0x10)?115200:baudRate());
}
void W65C51::receive(byte data,bool framingError) {
    if(!(command&1))return;
    ++rxCount;
    if(full)overrun=true; // Preserve the unread RDR on overrun.
    else {rx=data&mask();full=true;framing=framingError;}
    if(!(command&2))irq=true;
    if(command&0x10)write(data,Data);
}
void W65C51::tick(double cpuHz,bool hostMaySend) {
    const double elapsed=1/cpuHz;
    if(txRemaining>0 && cts && (command&1) && (command&0x0c)!=0x0c) {
        txRemaining-=elapsed;
        if(txRemaining<=0) {
            ++txCount;
            if(output.size()<65536)output+=static_cast<char>(tx);
        }
    }
    if(rxRemaining>0) {
        rxRemaining-=elapsed;
        if(rxRemaining<=0)receive(rxFrame);
    } else if(!input.empty() && hostMaySend && readyToReceive()) {
        rxFrame=input.front();input.pop_front();rxRemaining=frameSeconds(true);
    }
}
void W65C51::queueInput(const std::string& text) {
    if(text.size()+input.size()>65536)throw std::runtime_error("Serial input queue is full; wait for the program to read it.");
    for(unsigned char c:text)input.push_back(c);
}
std::string W65C51::takeOutput() { std::string result;result.swap(output);return result; }
void W65C51::setModemInputs(bool clearToSend,bool dataSetReady,bool carrierDetected) {
    if((dsr!=dataSetReady || dcd!=carrierDetected) && (command&1) && !(command&2))irq=true;
    cts=clearToSend;dsr=dataSetReady;dcd=carrierDetected;
}
