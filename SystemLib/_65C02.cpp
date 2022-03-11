#include "Bus.h"
#include "_65C02.h"


_65C02::_65C02(double Mhz) : cycles{Mhz} {
    reset();
    using CPU = _65C02;
    opCodeMatrix.reserve(256);

    opCodeMatrix =
            {
                    {&CPU::BRK, &CPU::rAddrS     }, {&CPU::ORA, &CPU::rAddrXInd }, {&CPU::XXX, &CPU::rAddrIM       }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::TSB, &CPU::rAddrZP     }, {&CPU::ORA, &CPU::rAddrZP     }, {&CPU::ASL, &CPU::rAddrZP     }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::PHP, &CPU::rAddrS      }, {&CPU::ORA, &CPU::rAddrIM     }, {&CPU::ASL, &CPU::rAddrAcm    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::TSB, &CPU::rAddrAbs    }, {&CPU::ORA, &CPU::rAddrAbs    }, {&CPU::ASL, &CPU::rAddrAbs    }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::BPL, &CPU::rAddrPcr   }, {&CPU::ORA, &CPU::rAddrIndY }, {&CPU::ORA, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::TRB, &CPU::rAddrZP     }, {&CPU::ORA, &CPU::rAddrZPX    }, {&CPU::ASL, &CPU::rAddrZPX    }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::CLC, &CPU::rAddrImp    }, {&CPU::ORA, &CPU::rAddrAbsY   }, {&CPU::INC, &CPU::rAddrAcm    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::TRB, &CPU::rAddrAbs    }, {&CPU::ORA, &CPU::rAddrAbsX   }, {&CPU::ASL, &CPU::rAddrAbsX   }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::JSR, &CPU::rAddrAbs   }, {&CPU::AND, &CPU::rAddrXInd }, {&CPU::XXX, &CPU::rAddrIM       }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::BIT, &CPU::rAddrZP     }, {&CPU::AND, &CPU::rAddrZP     }, {&CPU::ROL, &CPU::rAddrZP     }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::PLP, &CPU::rAddrS      }, {&CPU::AND, &CPU::rAddrIM     }, {&CPU::ROL, &CPU::rAddrAcm    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::BIT, &CPU::rAddrAbs    }, {&CPU::AND, &CPU::rAddrAbs    }, {&CPU::ROL, &CPU::rAddrAbs    }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::BMI, &CPU::rAddrPcr   }, {&CPU::AND, &CPU::rAddrIndY }, {&CPU::AND, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::BIT, &CPU::rAddrZPX    }, {&CPU::AND, &CPU::rAddrZPX    }, {&CPU::ROL, &CPU::rAddrZPX    }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::SEC, &CPU::rAddrImp    }, {&CPU::AND, &CPU::rAddrAbsY   }, {&CPU::DEC, &CPU::rAddrAcm    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::BIT, &CPU::rAddrAbsX   }, {&CPU::AND, &CPU::rAddrAbsX   }, {&CPU::ROL, &CPU::rAddrAbsX   }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::RTI, &CPU::rAddrS     }, {&CPU::EOR, &CPU::rAddrXInd }, {&CPU::XXX, &CPU::rAddrIM       }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::EOR, &CPU::rAddrZP     }, {&CPU::LSR, &CPU::rAddrZP     }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::PHA, &CPU::rAddrS      }, {&CPU::EOR, &CPU::rAddrIM     }, {&CPU::LSR, &CPU::rAddrAcm    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::JMP, &CPU::rAddrAbs    }, {&CPU::EOR, &CPU::rAddrAbs    }, {&CPU::LSR, &CPU::rAddrAbs    }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::BVC, &CPU::rAddrPcr   }, {&CPU::EOR, &CPU::rAddrIndY }, {&CPU::EOR, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::EOR, &CPU::rAddrZPX    }, {&CPU::LSR, &CPU::rAddrZPX    }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::CLI, &CPU::rAddrImp    }, {&CPU::EOR, &CPU::rAddrAbsY   }, {&CPU::PHY, &CPU::rAddrS      }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::EOR, &CPU::rAddrAbsX   }, {&CPU::LSR, &CPU::rAddrAbsX   }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::RTS, &CPU::rAddrS     }, {&CPU::ADC, &CPU::rAddrXInd }, {&CPU::XXX, &CPU::rAddrIM       }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::STZ, &CPU::rAddrZP     }, {&CPU::ADC, &CPU::rAddrZP     }, {&CPU::ROR, &CPU::rAddrZP     }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::PLA, &CPU::rAddrS      }, {&CPU::ADC, &CPU::rAddrIM     }, {&CPU::ROR, &CPU::rAddrAcm    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::JMP, &CPU::rAddrAInd   }, {&CPU::ADC, &CPU::rAddrAbs    }, {&CPU::ROR, &CPU::rAddrAbs    }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::BVS, &CPU::rAddrPcr   }, {&CPU::ADC, &CPU::rAddrIndY }, {&CPU::ADC, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::STZ, &CPU::rAddrZPX    }, {&CPU::ADC, &CPU::rAddrZPX    }, {&CPU::ROR, &CPU::rAddrZPX    }, {&CPU::RMB, &CPU::rAddrZP }, {&CPU::SEI, &CPU::rAddrImp    }, {&CPU::ADC, &CPU::rAddrAbsY   }, {&CPU::PLY, &CPU::rAddrS      }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::JMP, &CPU::rAddrAIndX  }, {&CPU::ADC, &CPU::rAddrAbsX   }, {&CPU::ROR, &CPU::rAddrAbsX   }, {&CPU::BBR, &CPU::rAddrIM },
                    {&CPU::BRA, &CPU::rAddrPcr   }, {&CPU::STA, &CPU::wAddrXInd }, {&CPU::XXX, &CPU::rAddrIM       }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::STY, &CPU::wAddrZP     }, {&CPU::STA, &CPU::wAddrZP     }, {&CPU::STX, &CPU::wAddrZP     }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::DEY, &CPU::rAddrImp    }, {&CPU::BIT, &CPU::rAddrIM     }, {&CPU::TXA, &CPU::rAddrImp    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::STY, &CPU::wAddrAbs    }, {&CPU::STA, &CPU::wAddrAbs    }, {&CPU::STX, &CPU::wAddrAbs    }, {&CPU::BBS, &CPU::rAddrIM },
                    {&CPU::BCC, &CPU::rAddrPcr   }, {&CPU::STA, &CPU::wAddrIndY }, {&CPU::STA, &CPU::wAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::STY, &CPU::wAddrZPX    }, {&CPU::STA, &CPU::wAddrZPX    }, {&CPU::STX, &CPU::wAddrZPY    }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::TYA, &CPU::rAddrImp    }, {&CPU::STA, &CPU::wAddrAbsY   }, {&CPU::TXS, &CPU::rAddrImp    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::STZ, &CPU::rAddrAbs    }, {&CPU::STA, &CPU::wAddrAbsX   }, {&CPU::STZ, &CPU::rAddrAbsX   }, {&CPU::BBS, &CPU::rAddrIM },
                    {&CPU::LDY, &CPU::rAddrIM    }, {&CPU::LDA, &CPU::rAddrXInd }, {&CPU::LDX, &CPU::rAddrIM       }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::LDY, &CPU::rAddrZP     }, {&CPU::LDA, &CPU::rAddrZP     }, {&CPU::LDX, &CPU::rAddrZP     }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::TAY, &CPU::rAddrImp    }, {&CPU::LDA, &CPU::rAddrIM     }, {&CPU::TAX, &CPU::rAddrImp    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::LDY, &CPU::rAddrAbs    }, {&CPU::LDA, &CPU::rAddrAbs    }, {&CPU::LDX, &CPU::rAddrAbs    }, {&CPU::BBS, &CPU::rAddrIM },
                    {&CPU::BCS, &CPU::rAddrPcr   }, {&CPU::LDA, &CPU::rAddrIndY }, {&CPU::LDA, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::LDY, &CPU::rAddrZPX    }, {&CPU::LDA, &CPU::rAddrZPX    }, {&CPU::LDX, &CPU::rAddrZPY    }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::CLV, &CPU::rAddrImp    }, {&CPU::LDA, &CPU::rAddrAbsY   }, {&CPU::TSX, &CPU::rAddrImp    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::LDY, &CPU::rAddrAbsX   }, {&CPU::LDA, &CPU::rAddrAbsX   }, {&CPU::LDX, &CPU::rAddrAbsY   }, {&CPU::BBS, &CPU::rAddrIM },
                    {&CPU::CPY, &CPU::rAddrIM    }, {&CPU::CMP, &CPU::rAddrXInd }, {&CPU::XXX, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::CPY, &CPU::rAddrZP     }, {&CPU::CMP, &CPU::rAddrZP     }, {&CPU::DEC, &CPU::rAddrZP     }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::INY, &CPU::rAddrImp    }, {&CPU::CMP, &CPU::rAddrIM     }, {&CPU::DEX, &CPU::rAddrImp    }, {&CPU::WAI, &CPU::rAddrImp    }, {&CPU::CPY, &CPU::rAddrAbs    }, {&CPU::CMP, &CPU::rAddrAbs    }, {&CPU::DEC, &CPU::rAddrAbs    }, {&CPU::BBS, &CPU::rAddrIM },
                    {&CPU::BNE, &CPU::rAddrPcr   }, {&CPU::CMP, &CPU::rAddrIndY }, {&CPU::CMP, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::CMP, &CPU::rAddrZPX    }, {&CPU::DEC, &CPU::rAddrZPX    }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::CLD, &CPU::rAddrImp    }, {&CPU::CMP, &CPU::rAddrAbsY   }, {&CPU::PHX, &CPU::rAddrS      }, {&CPU::STP, &CPU::rAddrImp    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::CMP, &CPU::rAddrAbsX   }, {&CPU::DEC, &CPU::rAddrAbsX   }, {&CPU::BBS, &CPU::rAddrIM },
                    {&CPU::CPX, &CPU::rAddrIM    }, {&CPU::SBC, &CPU::rAddrXInd }, {&CPU::XXX, &CPU::rAddrIM       }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::CPX, &CPU::rAddrZP     }, {&CPU::SBC, &CPU::rAddrZP     }, {&CPU::INC, &CPU::rAddrZP     }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::INX, &CPU::rAddrImp    }, {&CPU::SBC, &CPU::rAddrIM     }, {&CPU::NOP, &CPU::rAddrImp    }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::CPX, &CPU::rAddrAbs    }, {&CPU::SBC, &CPU::rAddrAbs    }, {&CPU::INC, &CPU::rAddrAbs    }, {&CPU::BBS, &CPU::rAddrIM },
                    {&CPU::BEQ, &CPU::rAddrPcr   }, {&CPU::SBC, &CPU::rAddrIndY }, {&CPU::SBC, &CPU::rAddrZPInd    }, {&CPU::XXX, &CPU::rAddrIM }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::SBC, &CPU::rAddrZPX    }, {&CPU::INC, &CPU::rAddrZPX    }, {&CPU::SMB, &CPU::rAddrZP }, {&CPU::SED, &CPU::rAddrImp    }, {&CPU::SBC, &CPU::rAddrAbsY   }, {&CPU::PLX, &CPU::rAddrS      }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::XXX, &CPU::rAddrIM     }, {&CPU::SBC, &CPU::rAddrAbsX   }, {&CPU::INC, &CPU::rAddrAbsX   }, {&CPU::BBS, &CPU::rAddrIM },
            };
}


void _65C02::connectBus(Bus *b) {
    bus = b;
}

void _65C02::reset(word pc) {
    PC = pc;
    SP = 0xFF;
    PS.reset();
    A = X = Y = 0;
}

byte _65C02::readByte(word address) {
    CyclesIncrementer cd(cycles);
    return bus->read(address);
}

word _65C02::readWord(word address) {
    word data = readByte(address);
    return data | (readByte(address + 1) << 8);
}

byte _65C02::fetchByte() {
    return readByte(PC++);
}

/*    6502 is little Endian which means that the first byte read
    * will be the least significant byte. That is if you want to
    * store 0x1234 in memory then it will be stored as 34 12. What
    * the code below for our example will first read 34 from memory
    * as a word so it will be 0034. Then it reads the next byte from
    * memory which is 12 shifts it to the left by 8 bits = 1200 and
    * then does 0034 | 1200 = 1234 which is what we wanted.*/
word _65C02::fetchWord() {
    word data = fetchByte();
    return data | (fetchByte() << 8);
}

/*
     * 0x1234 will be stored as 34 12 so write LSB bytes at low address
     * and MSB bytes at high address.
     */
void _65C02::writeWord(word data, word address) {
    writeByte(data & 0xFF, address);
    writeByte(data >> 8, address + 1);
}

void _65C02::writeByte(byte data, word address) {
    bus->write(data, address);
    ++cycles;
}

void _65C02::bitInstructionSetStatus(byte result) {
    PS.set(StatusFlags::Z, result == 0);
    PS.set(StatusFlags::N , (result & 0b10000000) >> 7);
    PS.set(StatusFlags::V, (result & 0x40) >> 6);
}

void _65C02::pushByteToStack(byte data) {
    writeByte(data, SPToAddress());
    SP--;
}

void _65C02::pushWordToStack(word data) {
    pushByteToStack((data & 0xFF00) >> 8);
    pushByteToStack(data);
}

byte _65C02::pullByteFromStack(bool incSPBefore, bool incSPAfter) {
    if(incSPBefore) {
        SP++;
        ++cycles;
    }
    return readByte(SPToAddress(incSPAfter));
}

//return the SP as a full 16 bit address in the first page even though SP is a byte
word _65C02::SPToAddress(bool incrementSP) {
    return incrementSP ? 0x100 | SP++ : 0x100 | SP;
}

word _65C02::rAddrIM() {
    return fetchByte();
}

word _65C02::wAddrIM() {
    return 0;
}

word _65C02::rAddrAbs() {
    return readByte(fetchWord());
}

word _65C02::wAddrAbs() {
    return fetchWord();
}

word _65C02::rAddrZP() {
    return readByte(fetchByte());
}

word _65C02::wAddrZP() {
    return fetchByte();
}

word _65C02::rAddrAcm() {
    return readByte(PC);
}

word _65C02::wAddrAcm() {
    return 0;
}

word _65C02::rAddrImp() {
    return readByte(PC);
}

word _65C02::wAddrImp() {
    return 0;
}

word _65C02::rAddrIndY() {
    byte zpAddress = fetchByte();
    word address = readByte(zpAddress) | (readByte(static_cast<byte>(zpAddress + 1)) << 8);
    if(((address & 0xFF) + Y) > 0xFF)
        readByte(static_cast<byte>(zpAddress + 1));
    return readByte(address + Y);
}

word _65C02::wAddrIndY() {
    byte zpAddress = fetchByte();
    word address = readWord(zpAddress);
    ++cycles;
    return address + Y;
}

word _65C02::rAddrXInd() {
    byte zeroPage{readByte(PC)};
    fetchByte();
    word effectiveAddress = readByte(static_cast<byte>(zeroPage + X)) | (readByte(static_cast<byte>(zeroPage + X + 1)) << 8);
    return readByte(effectiveAddress);
}

word _65C02::wAddrXInd() {
    byte startAddress = (fetchByte() + X) & 0xFF;
    ++cycles;
    return readByte(startAddress) | (readByte((startAddress + 0x01) & 0xFF)) << 8;
}

word _65C02::rAddrZPX() {
    byte address{fetchByte()};
    byte effectiveAddress = address + X;
    readByte(address);
    return readByte(effectiveAddress);
}

word _65C02::wAddrZPX() {
    ++cycles;
    return static_cast<byte>(fetchByte() + X);
}

word _65C02::rAddrZPY() {
    byte address{fetchByte()};
    byte effectiveAddress = address + Y;
    readByte(address);
    cycles += (address + Y > 0xFF);
    return readByte(effectiveAddress);
}

word _65C02::wAddrZPY() {
    ++cycles;
    return static_cast<byte>(fetchByte() + Y);
}

word _65C02::rAddrAbsX() {
    word address = fetchWord();
    dword effectiveAddress = address + X;
    byte data{readByte(effectiveAddress)};
    return (((address & 0xFF) + X) > 0xFF) ? readByte(effectiveAddress - 0x100) : data;
}

word _65C02::wAddrAbsX() {
    word address = fetchWord();
    dword effectiveAddress = address + X;
    readByte(effectiveAddress);
    return (((address & 0xFF) + X) > 0xFF) ? effectiveAddress - 0x100 : effectiveAddress;
}

word _65C02::rAddrAbsY() {
    word address = fetchWord();
    dword effectiveAddress = address + Y;
    byte data{readByte(effectiveAddress)};
    return (((address & 0xFF) + Y) > 0xFF) ? readByte(effectiveAddress - 0x100) : data;
}

word _65C02::wAddrAbsY() {
    word address = fetchWord();
    dword effectiveAddress = address + Y;
    readByte(effectiveAddress);
    return (((address & 0xFF) + Y) > 0xFF) ? effectiveAddress - 0x100 : effectiveAddress;
}

word _65C02::rAddrPcr() {
    return 0;
}

word _65C02::wAddrPcr() {
    return 0;
}

word _65C02::rAddrAInd() {
    word pointer{fetchWord()};
    readByte(PC);
    return readByte(readByte(pointer) | (readByte(pointer + 1) << 8));
}

word _65C02::wAddrAInd() {
    return 0;
}

word _65C02::rAddrS() {
    return 0;
}

word _65C02::wAddrS() {
    return 0;
}

word _65C02::rAddrAIndX() {
    //TODO This is all wrong we need to fix this
    word address = fetchWord();
    dword pointer = address + X;
    readByte(PC);
    byte latch{readByte(pointer + X)};
    return latch;
}

word _65C02::wAddrAIndX() {
    return 0;
}

word _65C02::rAddrZPInd() {
    byte ZPAddr = fetchByte();
    return readByte(readByte(ZPAddr) | (readByte((byte)(ZPAddr + 1)) << 8));
}

word _65C02::wAddrZPInd() {
    return 0;
}

void _65C02::execute(uint64_t instructionsToExecute) {
    cycles.reset();
    Opcode opcode{};
    while(instructionsToExecute--) {
        opcode = opCodeMatrix[fetchByte()];
        (this->*(opcode.instruction))(opcode.addrMode);
    }
}

void _65C02::ADC(word (_65C02::* addrMode)()) {

}

void _65C02::AND(word (_65C02::* addrMode)()) {

}

void _65C02::ASL(word (_65C02::* addrMode)()) {

}

void _65C02::BBR(word (_65C02::* addrMode)()) {

}

void _65C02::BBS(word (_65C02::* addrMode)()) {

}

void _65C02::BCC(word (_65C02::* addrMode)()) {

}

void _65C02::BCS(word (_65C02::* addrMode)()) {

}

void _65C02::BEQ(word (_65C02::* addrMode)()) {

}

void _65C02::BIT(word (_65C02::* addrMode)()) {

}

void _65C02::BMI(word (_65C02::* addrMode)()) {

}

void _65C02::BNE(word (_65C02::* addrMode)()) {

}

void _65C02::BPL(word (_65C02::* addrMode)()) {

}

void _65C02::BRA(word (_65C02::* addrMode)()) {

}

void _65C02::BRK(word (_65C02::* addrMode)()) {

}

void _65C02::BVC(word (_65C02::* addrMode)()) {

}

void _65C02::BVS(word (_65C02::* addrMode)()) {

}

void _65C02::CLC(word (_65C02::* addrMode)()) {

}

void _65C02::CLD(word (_65C02::* addrMode)()) {

}

void _65C02::CLI(word (_65C02::* addrMode)()) {

}

void _65C02::CLV(word (_65C02::* addrMode)()) {

}

void _65C02::CMP(word (_65C02::* addrMode)()) {

}

void _65C02::CPX(word (_65C02::* addrMode)()) {

}

void _65C02::CPY(word (_65C02::* addrMode)()) {

}

void _65C02::DEC(word (_65C02::* addrMode)()) {

}

void _65C02::DEX(word (_65C02::* addrMode)()) {

}

void _65C02::DEY(word (_65C02::* addrMode)()) {

}

void _65C02::EOR(word (_65C02::* addrMode)()) {

}

void _65C02::INC(word (_65C02::* addrMode)()) {

}

void _65C02::INX(word (_65C02::* addrMode)()) {

}

void _65C02::INY(word (_65C02::* addrMode)()) {

}

void _65C02::JMP(word (_65C02::* addrMode)()) {
    PC = (this->*addrMode)();
}

void _65C02::JSR(word (_65C02::* addrMode)()) {

}

void _65C02::LDA(word (_65C02::* addrMode)()) {
    loadRegister(A, (this->*addrMode)());
}

void _65C02::LDX(word (_65C02::* addrMode)()) {
    loadRegister(X, (this->*addrMode)());
}

void _65C02::LDY(word (_65C02::* addrMode)()) {
    loadRegister(Y, (this->*addrMode)());
}

void _65C02::LSR(word (_65C02::* addrMode)()) {

}

void _65C02::NOP(word (_65C02::* addrMode)()) {
    readByte(PC);
}

void _65C02::ORA(word (_65C02::* addrMode)()) {

}

void _65C02::PHA(word (_65C02::* addrMode)()) {

}

void _65C02::PHP(word (_65C02::* addrMode)()) {

}

void _65C02::PHX(word (_65C02::* addrMode)()) {

}

void _65C02::PHY(word (_65C02::* addrMode)()) {

}

void _65C02::PLA(word (_65C02::* addrMode)()) {

}

void _65C02::PLP(word (_65C02::* addrMode)()) {

}

void _65C02::PLX(word (_65C02::* addrMode)()) {

}

void _65C02::PLY(word (_65C02::* addrMode)()) {

}

void _65C02::RMB(word (_65C02::* addrMode)()) {

}

void _65C02::ROL(word (_65C02::* addrMode)()) {

}

void _65C02::ROR(word (_65C02::* addrMode)()) {

}

void _65C02::RTI(word (_65C02::* addrMode)()) {

}

void _65C02::RTS(word (_65C02::* addrMode)()) {

}

void _65C02::SBC(word (_65C02::* addrMode)()) {

}

void _65C02::SEC(word (_65C02::* addrMode)()) {

}

void _65C02::SED(word (_65C02::* addrMode)()) {

}

void _65C02::SEI(word (_65C02::* addrMode)()) {

}

void _65C02::SMB(word (_65C02::* addrMode)()) {

}

void _65C02::STA(word (_65C02::* addrMode)()) {
    writeByte(A, (this->*addrMode)());
}

void _65C02::STP(word (_65C02::* addrMode)()) {

}

void _65C02::STX(word (_65C02::* addrMode)()) {
    writeByte(X, (this->*addrMode)());
}

void _65C02::STY(word (_65C02::* addrMode)()) {
    writeByte(Y, (this->*addrMode)());
}

void _65C02::STZ(word (_65C02::* addrMode)()) {

}

void _65C02::TAX(word (_65C02::* addrMode)()) {

}

void _65C02::TAY(word (_65C02::* addrMode)()) {

}

void _65C02::TRB(word (_65C02::* addrMode)()) {

}

void _65C02::TSB(word (_65C02::* addrMode)()) {

}

void _65C02::TSX(word (_65C02::* addrMode)()) {

}

void _65C02::TXA(word (_65C02::* addrMode)()) {

}

void _65C02::TXS(word (_65C02::* addrMode)()) {

}

void _65C02::TYA(word (_65C02::* addrMode)()) {

}

void _65C02::WAI(word (_65C02::* addrMode)()) {

}

void _65C02::XXX(word (_65C02::* addrMode)()) {
    NOP(addrMode);
}

void _65C02::loadRegister(byte& Register, byte value) {
    Register = value;
    PS.set(StatusFlags::Z, Register == 0);
    PS.set(StatusFlags::N , (Register & 0b10000000) >> 7);
}

void _65C02::setCycleDuration(double Mhz) {
    cycles.setCycleDuration(Mhz);
}






