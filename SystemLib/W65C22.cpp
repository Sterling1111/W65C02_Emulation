#include "W65C22.h"
#include "PortBus.h"
#include <utility>

namespace {
bool activeEdge(bool oldLevel, bool newLevel, bool positive) {
    return oldLevel != newLevel && newLevel == positive;
}
W65C22::Drive drive(bool level) {
    return level ? W65C22::Drive::High : W65C22::Drive::Low;
}
}

W65C22::W65C22(Variant variant) : variant(variant) { reset(); }

void W65C22::reset() {
    // RESB deliberately preserves timer counters/latches and the shift register.
    heldA = portARead();
    heldB = portBRead();
    ora = orb = ddra = ddrb = ira = irb = acr = pcr = ifr = ier = 0;
    latchedA = latchedB = false;
    t1Running = t2Running = t1Armed = t2Armed = t1Reload = false;
    t2Reload = false;
    shiftActive = serialInputPending = false;
    shiftBits = serialDivider = 0;
    ca2Pulse = cb2Pulse = false;
    ca2Output = cb2Output = serialClock = serialData = timerPB7 = true;
    previousPB6 = (portBRead() & 0x40) != 0;
    publish();
}

void W65C22::setRESB(bool high) {
    resetHeld = !high;
    if (resetHeld) reset();
    else publish();
}

void W65C22::connectPortBus(PortBus* connectedBus) {
    bus = connectedBus;
    busPublished = false;
    publish();
}

byte W65C22::portBOutput() const {
    return (acr & 0x80) ? byte((orb & 0x7f) | (timerPB7 ? 0x80 : 0)) : orb;
}

byte W65C22::portARead() const {
    byte inputs = variant == Variant::W65C22S ? heldA : 0xff;
    byte levels = byte((ora & ddra) | (inputs & ~ddra));
    return byte((levels & ~externalMaskA) | (externalA & externalMaskA));
}

byte W65C22::portBRead() const {
    byte inputs = bus ? bus->portBRead() : (variant == Variant::W65C22S ? heldB : 0xff);
    byte levels = byte((portBOutput() & ddrb) | (inputs & ~ddrb));
    return byte((levels & ~externalMaskB) | (externalB & externalMaskB));
}

bool W65C22::serialClockOutput() const {
    const auto mode = shiftMode();
    return mode != 0 && mode != 3 && mode != 7;
}

bool W65C22::ca2Level() const { return ca2Mode() >= 4 ? ca2Output : ca2; }
bool W65C22::cb1Level() const { return serialClockOutput() ? serialClock : cb1; }
bool W65C22::cb2Level() const {
    if (shiftMode() >= 4) return serialData;
    if (shiftMode() != 0) return cb2;
    return cb2Mode() >= 4 ? cb2Output : cb2;
}

W65C22::Pins W65C22::pins() const {
    return {portARead(), portBRead(), ddra, ddrb,
            ca2Mode() >= 4 ? drive(ca2Output) : Drive::HighImpedance,
            serialClockOutput() ? drive(serialClock) : Drive::HighImpedance,
            shiftMode() >= 4 ? drive(serialData) :
                (shiftMode() == 0 && cb2Mode() >= 4 ? drive(cb2Output) : Drive::HighImpedance),
            irqAsserted() ? Drive::Low :
                (variant == Variant::W65C22S ? Drive::High : Drive::HighImpedance),
            ca1, ca2Level(), cb1Level(), cb2Level()};
}

void W65C22::publish() {
    if (bus) {
        const byte outputA = portARead(), outputB = portBOutput() & ddrb;
        if (!busPublished || outputB != lastB || ddrb != lastDDRB) bus->portBWrite(outputB, ddrb);
        if (!busPublished || outputA != lastA || ddra != lastDDRA) bus->portAWrite(outputA);
        lastA = outputA; lastB = outputB; lastDDRA = ddra; lastDDRB = ddrb;
        busPublished = true;
    }
    const auto now = pins();
    heldA = now.pa;
    heldB = now.pb;
    const bool changed = !pinsPublished || now.pa != lastPins.pa || now.pb != lastPins.pb ||
        now.paOutputMask != lastPins.paOutputMask || now.pbOutputMask != lastPins.pbOutputMask ||
        now.ca2 != lastPins.ca2 || now.cb1 != lastPins.cb1 || now.cb2 != lastPins.cb2 || now.irq != lastPins.irq ||
        now.ca1Level != lastPins.ca1Level || now.ca2Level != lastPins.ca2Level ||
        now.cb1Level != lastPins.cb1Level || now.cb2Level != lastPins.cb2Level;
    lastPins = now;
    pinsPublished = true;
    if (changed && pinCallback) pinCallback(now);
}

void W65C22::setPinCallback(std::function<void(const Pins&)> callback) {
    pinCallback = std::move(callback);
    pinsPublished = false;
    publish();
}

void W65C22::setPortAInput(byte levels, byte drivenMask) {
    externalA = levels; externalMaskA = drivenMask; publish();
}
void W65C22::setPortBInput(byte levels, byte drivenMask) {
    externalB = levels; externalMaskB = drivenMask; publish();
}
void W65C22::releasePortA(byte mask) { heldA = portARead(); externalMaskA &= ~mask; publish(); }
void W65C22::releasePortB(byte mask) { heldB = portBRead(); externalMaskB &= ~mask; publish(); }

void W65C22::setCA1(bool high) {
    const bool edge = activeEdge(ca1, high, (pcr & 1) != 0);
    ca1 = high;
    if (!resetHeld && edge) {
        if (acr & 1) { ira = portARead(); latchedA = true; }
        ifr |= CA1_FLAG;
        if (ca2Mode() == 4) ca2Output = true;
    }
    publish();
}
void W65C22::setCA2(bool high) {
    const bool edge = activeEdge(ca2, high, (ca2Mode() & 2) != 0);
    ca2 = high;
    if (!resetHeld && ca2Mode() < 4 && edge) ifr |= CA2_FLAG;
    publish();
}
void W65C22::setCB1(bool high) {
    const bool changed = cb1 != high;
    const bool edge = activeEdge(cb1, high, (pcr & 0x10) != 0);
    cb1 = high;
    if (!resetHeld && !serialClockOutput()) {
        if (edge) {
            if (acr & 2) { irb = portBRead(); latchedB = true; }
            ifr |= CB1_FLAG;
            if (shiftMode() == 0 && cb2Mode() == 4) cb2Output = true;
        }
        if (changed && (shiftMode() == 3 || shiftMode() == 7) && shiftActive)
            shiftEdge(high);
    }
    publish();
}
void W65C22::setCB2(bool high) {
    const bool edge = activeEdge(cb2, high, (cb2Mode() & 2) != 0);
    cb2 = high;
    if (!resetHeld && shiftMode() == 0 && cb2Mode() < 4 && edge) ifr |= CB2_FLAG;
    publish();
}

void W65C22::portAccess(bool portA, bool write) {
    const unsigned mode = portA ? ca2Mode() : cb2Mode();
    ifr &= ~(portA ? CA1_FLAG : CB1_FLAG);
    if (mode < 4 && !(mode & 1)) ifr &= ~(portA ? CA2_FLAG : CB2_FLAG);
    // Acknowledge releases the input latch until the next active control edge.
    (portA ? latchedA : latchedB) = false;
    if ((portA || write) && (portA || shiftMode() == 0)) {
        if (mode == 4 || mode == 5) {
            (portA ? ca2Output : cb2Output) = false;
            (portA ? ca2Pulse : cb2Pulse) = mode == 5;
        }
    }
}

byte W65C22::readFromRegisters(word address) {
    if (resetHeld) return 0;
    byte value = 0;
    switch (address & 15) {
    case ORB:
        value = byte(((latchedB && (acr & 2) ? irb : portBRead()) & ~ddrb) | (portBOutput() & ddrb));
        portAccess(false, false); break;
    case ORA:
    case ORA_NO_HANDSHAKE:
        value = latchedA && (acr & 1) ? ira : portARead();
        if ((address & 15) == ORA) portAccess(true, false);
        break;
    case DDRB: value = ddrb; break;
    case DDRA: value = ddra; break;
    case T1CL: value = byte(t1Counter); ifr &= ~T1_FLAG; break;
    case T1CH: value = byte(t1Counter >> 8); break;
    case T1LL: value = byte(t1Latch); break;
    case T1LH: value = byte(t1Latch >> 8); break;
    case T2CL: value = byte(t2Counter); ifr &= ~T2_FLAG; break;
    case T2CH: value = byte(t2Counter >> 8); break;
    case SR: value = sr; startShift(); break;
    case ACR: value = acr; break;
    case PCR: value = pcr; break;
    case IFR: value = byte(ifr | (irqAsserted() ? 0x80 : 0)); break;
    case IER: value = byte(ier | 0x80); break;
    }
    publish();
    return value;
}

void W65C22::updateControlOutputs() {
    ca2Pulse = cb2Pulse = false;
    ca2Output = ca2Mode() != 6;
    cb2Output = cb2Mode() != 6;
}

void W65C22::writeToRegisters(byte data, word address) {
    if (resetHeld) return;
    switch (address & 15) {
    case ORB: orb = data; portAccess(false, true); break;
    case ORA: ora = data; portAccess(true, true); break;
    case ORA_NO_HANDSHAKE: ora = data; break;
    case DDRB: ddrb = data; break;
    case DDRA: ddra = data; break;
    case T1CL:
    case T1LL: t1Latch = word((t1Latch & 0xff00) | data); break;
    case T1CH:
        t1Latch = word((data << 8) | (t1Latch & 0xff));
        t1Counter = t1Latch;
        t1Running = t1Armed = true; t1Reload = false;
        timerPB7 = false; ifr &= ~T1_FLAG; break;
    case T1LH: t1Latch = word((data << 8) | (t1Latch & 0xff)); ifr &= ~T1_FLAG; break;
    case T2CL: t2Latch = data; break;
    case T2CH:
        t2Counter = word((data << 8) | t2Latch);
        t2Running = t2Armed = true; t2Reload = false; ifr &= ~T2_FLAG;
        previousPB6 = (portBRead() & 0x40) != 0; break;
    case SR: sr = data; startShift(); break;
    case ACR: {
        const unsigned oldMode = shiftMode();
        acr = data;
        if (!(acr & 1)) latchedA = false;
        if (!(acr & 2)) latchedB = false;
        if (oldMode != shiftMode()) {
            shiftActive = serialInputPending = false;
            shiftBits = 0; serialClock = true;
        }
        if (shiftMode() == 0) ifr &= ~SR_FLAG;
        break;
    }
    case PCR: pcr = data; updateControlOutputs(); break;
    case IFR: ifr &= ~(data & 0x7f); break;
    case IER:
        if (data & 0x80) ier |= data & 0x7f;
        else ier &= ~(data & 0x7f);
        break;
    }
    publish();
}

void W65C22::startShift() {
    ifr &= ~SR_FLAG;
    shiftBits = 0;
    serialInputPending = false;
    shiftActive = shiftMode() != 0;
    serialClock = true;
    // Internally generated CB1 clocks have a full low and high half-period.
    serialDivider = 2;
    if (shiftMode() == 1 || shiftMode() == 4 || shiftMode() == 5) {
        t2Counter = word((t2Counter & 0xff00) | t2Latch);
        t2Running = true;
        t2Reload = false;
    }
}

void W65C22::finishShiftBit() {
    if (++shiftBits != 8) return;
    shiftBits = 0;
    if (shiftMode() == 4) return; // Free-running output never sets IFR2.
    ifr |= SR_FLAG;
    if (shiftMode() != 3 && shiftMode() != 7) shiftActive = false;
}

void W65C22::shiftEdge(bool rising) {
    if (shiftMode() < 4) {
        // Inputs are sampled on the PHI2 tick following CB1's rising edge.
        if (rising) serialInputPending = true;
    } else if (!rising) {
        serialData = (sr & 0x80) != 0;
        sr = byte((sr << 1) | (serialData ? 1 : 0));
        // Externally clocked output counts falling edges; internal output
        // completes after the eighth complete clock pulse (Figures 2-10..12).
        if (shiftMode() == 7) finishShiftBit();
    } else if (shiftMode() != 7) {
        finishShiftBit();
    }
}

void W65C22::clockSerial() {
    if (serialInputPending) {
        serialInputPending = false;
        sr = byte((sr << 1) | (cb2 ? 1 : 0));
        finishShiftBit();
    }
    if (!shiftActive || (shiftMode() != 2 && shiftMode() != 6)) return;
    if (--serialDivider == 0) {
        serialClock = !serialClock;
        serialDivider = 1;
        shiftEdge(serialClock);
    }
}

void W65C22::clockTimer2() {
    const bool serialTimer = shiftMode() == 1 || shiftMode() == 4 || shiftMode() == 5;
    if (serialTimer && t2Reload) {
        t2Counter = word((t2Counter & 0xff00) | t2Latch);
        t2Reload = false;
        if (shiftActive) {
            serialClock = !serialClock;
            shiftEdge(serialClock);
        }
        return;
    }
    const bool underflow = t2Counter == 0;
    const bool lowUnderflow = (t2Counter & 0xff) == 0;
    --t2Counter;
    if (underflow && t2Armed) { ifr |= T2_FLAG; t2Armed = false; }
    if (serialTimer && lowUnderflow) t2Reload = true;
}

void W65C22::tick(uint64_t cycles) {
    while (cycles--) {
        if (resetHeld) continue;
        if (!t1Running && !t2Running && !shiftActive && !serialInputPending &&
            !ca2Pulse && !cb2Pulse) continue;
        if (ca2Pulse) { ca2Pulse = false; ca2Output = true; }
        if (cb2Pulse) { cb2Pulse = false; cb2Output = true; }
        if (t1Running) {
            if (t1Reload) { t1Counter = t1Latch; t1Reload = false; }
            else {
                const bool underflow = t1Counter == 0;
                --t1Counter;
                if (underflow) {
                    t1Reload = true;
                    if (t1Armed || (acr & 0x40)) {
                        ifr |= T1_FLAG;
                        timerPB7 = (acr & 0x40) ? !timerPB7 : true;
                        t1Armed = false;
                    }
                }
            }
        }
        // Consume pending serial input before generating this cycle's edge.
        clockSerial();
        const bool pb6 = (portBRead() & 0x40) != 0;
        if (t2Running && (!(acr & 0x20) || (previousPB6 && !pb6))) clockTimer2();
        previousPB6 = pb6;
        publish();
    }
}

W65C22::BusRead W65C22::busCycle(bool cs1, bool cs2b, bool read, word address, byte data) {
    // Address/data are transferred at the end of this full PHI2 period.
    tick();
    if (resetHeld || !cs1 || cs2b) return {false, 0};
    if (read) return {true, readFromRegisters(address)};
    writeToRegisters(data, address);
    return {false, 0};
}
