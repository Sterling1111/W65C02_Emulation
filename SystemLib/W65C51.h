#ifndef W65C02_W65C51_H
#define W65C02_W65C51_H
#include "system_types.h"
#include <deque>
#include <string>

// Byte-level W65C51N model with serial frame timing in emulated PHI2 time.
// Access under the owning CPU's stateMutex while its worker is running.
class W65C51 {
public:
    enum Register { Data, Status, Command, Control };
    void reset();
    byte read(word reg);
    void write(byte data,word reg);
    byte status() const;
    bool irqAsserted() const { return irq && (command&1) && !(command&2); }
    bool readyToReceive() const { return (command&1) && ((command&0x0c)!=0 || (command&0x10)); }
    double baudRate() const;
    void tick(double cpuHz,bool hostMaySend=true);
    // External serial peer. The queue belongs to the terminal, not the ACIA RDR.
    void queueInput(const std::string& text);
    size_t pendingInput() const { return input.size()+(rxRemaining>0); }
    void cancelInput() { input.clear();rxRemaining=0; }
    std::string takeOutput();
    // Deliver a completed external frame, including optional framing-error bit.
    void receive(byte data,bool framingError=false);
    void setModemInputs(bool clearToSend,bool dataSetReady,bool carrierDetected);
    byte commandRegister() const { return command; }
    byte controlRegister() const { return control; }
    uint64_t transmitted() const { return txCount; }
    uint64_t received() const { return rxCount; }
private:
    byte command{},control{},rx{},tx{},rxFrame{};
    bool full{},overrun{},framing{},irq{},cts{true},dsr{true},dcd{true};
    double txRemaining{},rxRemaining{};
    uint64_t txCount{},rxCount{};
    std::deque<byte> input;
    std::string output;
    byte mask() const { return static_cast<byte>((1u<<(8-((control>>5)&3)))-1); }
    double frameSeconds(bool receiving=false) const;
};
#endif
