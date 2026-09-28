#ifndef _65C02_SYSTEM_EMULATION_W65C22_H
#define _65C02_SYSTEM_EMULATION_W65C22_H

#include "system_types.h"
#include <functional>

class PortBus;

// Digital W65C22 model. Register accesses occur between PHI2 ticks; tick()
// advances complete PHI2 periods. Pin setters describe external signal levels.
// Use the CPU state mutex when accessing a VIA connected to a running System.
class W65C22 {
public:
    enum Register : byte {
        ORB, ORA, DDRB, DDRA, T1CL, T1CH, T1LL, T1LH,
        T2CL, T2CH, SR, ACR, PCR, IFR, IER, ORA_NO_HANDSHAKE
    };
    enum Interrupt : byte {
        CA2_FLAG = 1, CA1_FLAG = 2, SR_FLAG = 4, CB2_FLAG = 8,
        CB1_FLAG = 16, T2_FLAG = 32, T1_FLAG = 64
    };
    enum class Variant { W65C22S, W65C22N };
    enum class Drive { Low, High, HighImpedance };
    struct Pins {
        byte pa, pb, paOutputMask, pbOutputMask;
        Drive ca2, cb1, cb2, irq;
        bool ca1Level, ca2Level, cb1Level, cb2Level;
    };
    struct BusRead { bool driving; byte data; };

    explicit W65C22(Variant variant = Variant::W65C22S);
    byte readFromRegisters(word address);
    void writeToRegisters(byte data, word address);
    void tick(uint64_t cycles = 1);
    // One selected/deselected PHI2 bus cycle. CS2B and RESB are active low.
    BusRead busCycle(bool cs1, bool cs2b, bool read, word address, byte data = 0);
    void reset();
    void setRESB(bool high);
    void connectPortBus(PortBus* bus);

    void setPortAInput(byte levels, byte drivenMask = 0xff);
    void setPortBInput(byte levels, byte drivenMask = 0xff);
    void releasePortA(byte mask = 0xff);
    void releasePortB(byte mask = 0xff);
    void setCA1(bool high);
    void setCA2(bool high);
    void setCB1(bool high);
    void setCB2(bool high);
    byte portARead() const;
    byte portBRead() const;
    void portAWrite(byte data) { writeToRegisters(data, ORA); }
    void portBWrite(byte data) { writeToRegisters(data, ORB); }
    bool ca2Level() const;
    bool cb1Level() const;
    bool cb2Level() const;
    bool irqAsserted() const { return !resetHeld && (ifr & ier & 0x7f) != 0; }
    Pins pins() const;
    // Called on pin/drive changes, on the calling emulation thread. The
    // callback may supply peripheral input levels, but must not tick the VIA.
    void setPinCallback(std::function<void(const Pins&)> callback);

private:
    unsigned ca2Mode() const { return (pcr >> 1) & 7; }
    unsigned cb2Mode() const { return (pcr >> 5) & 7; }
    unsigned shiftMode() const { return (acr >> 2) & 7; }
    bool serialClockOutput() const;
    byte portBOutput() const;
    void portAccess(bool portA, bool write);
    void updateControlOutputs();
    void publish();
    void startShift();
    void shiftEdge(bool rising);
    void finishShiftBit();
    void clockTimer2();
    void clockSerial();

    Variant variant;
    PortBus* bus{};
    std::function<void(const Pins&)> pinCallback;
    Pins lastPins{};
    bool pinsPublished{};
    byte lastA{}, lastB{}, lastDDRA{}, lastDDRB{};
    bool busPublished{};
    byte ora{}, orb{}, ddra{}, ddrb{}, ira{}, irb{}, acr{}, pcr{}, ifr{}, ier{}, sr{};
    bool latchedA{}, latchedB{};
    byte externalA{}, externalB{}, externalMaskA{}, externalMaskB{};
    byte heldA{}, heldB{};
    bool ca1{true}, ca2{true}, cb1{true}, cb2{true};
    bool ca2Output{true}, cb2Output{true}, serialClock{true}, serialData{true};
    bool ca2Pulse{}, cb2Pulse{}, previousPB6{};
    bool resetHeld{};
    word t1Counter{}, t1Latch{}, t2Counter{};
    byte t2Latch{};
    bool t2Reload{};
    bool t1Running{}, t2Running{}, t1Armed{}, t2Armed{}, t1Reload{}, timerPB7{true};
    bool shiftActive{}, serialInputPending{};
    unsigned shiftBits{}, serialDivider{};
};
#endif
