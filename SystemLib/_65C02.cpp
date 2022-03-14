#include "Bus.h"
#include "_65C02.h"


_65C02::_65C02(double Mhz) : cycles{Mhz} {
    reset();
    using CPU = _65C02;
    opCodeMatrix.reserve(256);

    opCodeMatrix =
            {
                    {&CPU::BRK, &CPU::stackB    }, {&CPU::ORA, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::TSB, &CPU::zeroPageB   }, {&CPU::ORA, &CPU::zeroPageA  }, {&CPU::ASL, &CPU::zeroPageB  }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::PHP, &CPU::stackE      }, {&CPU::ORA, &CPU::immediate  }, {&CPU::ASL, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::TSB, &CPU::absoluteB                  }, {&CPU::ORA, &CPU::absoluteA  }, {&CPU::ASL, &CPU::absoluteB  }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::BPL, &CPU::relativeA }, {&CPU::ORA, &CPU::zeroPageIndirectIndexed }, {&CPU::ORA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::TRB, &CPU::zeroPageB   }, {&CPU::ORA, &CPU::zeroPageXA }, {&CPU::ASL, &CPU::zeroPageXB }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::CLC, &CPU::immediate   }, {&CPU::ORA, &CPU::absoluteY  }, {&CPU::INC, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::TRB, &CPU::absoluteB                  }, {&CPU::ORA, &CPU::absoluteXA }, {&CPU::ASL, &CPU::absoluteXB }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::JSR, &CPU::absoluteD }, {&CPU::AND, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::BIT, &CPU::zeroPageB   }, {&CPU::AND, &CPU::zeroPageA  }, {&CPU::ROL, &CPU::zeroPageB  }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::PLP, &CPU::stackF      }, {&CPU::AND, &CPU::immediate  }, {&CPU::ROL, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::BIT, &CPU::absoluteA                  }, {&CPU::AND, &CPU::absoluteA  }, {&CPU::ROL, &CPU::absoluteB  }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::BMI, &CPU::relativeA }, {&CPU::AND, &CPU::zeroPageIndirectIndexed }, {&CPU::AND, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::BIT, &CPU::zeroPageXA  }, {&CPU::AND, &CPU::zeroPageXA }, {&CPU::ROL, &CPU::zeroPageXB }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::SEC, &CPU::immediate   }, {&CPU::AND, &CPU::absoluteY  }, {&CPU::DEC, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::BIT, &CPU::absoluteXA                 }, {&CPU::AND, &CPU::absoluteXA }, {&CPU::ROL, &CPU::absoluteXB }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::RTI, &CPU::stackC    }, {&CPU::EOR, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::EOR, &CPU::zeroPageA  }, {&CPU::LSR, &CPU::zeroPageB  }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::PHA, &CPU::stackE      }, {&CPU::EOR, &CPU::immediate  }, {&CPU::LSR, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteC                  }, {&CPU::EOR, &CPU::absoluteA  }, {&CPU::LSR, &CPU::absoluteB  }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::BVC, &CPU::relativeA }, {&CPU::EOR, &CPU::zeroPageIndirectIndexed }, {&CPU::EOR, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::EOR, &CPU::zeroPageXA }, {&CPU::LSR, &CPU::zeroPageXB }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::CLI, &CPU::immediate   }, {&CPU::EOR, &CPU::absoluteY  }, {&CPU::PHY, &CPU::stackE         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::EOR, &CPU::absoluteXA }, {&CPU::LSR, &CPU::absoluteXB }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::RTS, &CPU::stackD    }, {&CPU::ADC, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::STZ, &CPU::zeroPageA   }, {&CPU::ADC, &CPU::zeroPageA  }, {&CPU::ROR, &CPU::zeroPageB  }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::PLA, &CPU::stackF      }, {&CPU::ADC, &CPU::immediate  }, {&CPU::ROR, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteIndirect           }, {&CPU::ADC, &CPU::absoluteA  }, {&CPU::ROR, &CPU::absoluteB  }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::BVS, &CPU::relativeA }, {&CPU::ADC, &CPU::zeroPageIndirectIndexed }, {&CPU::ADC, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::STZ, &CPU::zeroPageB   }, {&CPU::ADC, &CPU::zeroPageXA }, {&CPU::ROR, &CPU::zeroPageXB }, {&CPU::RMB, &CPU::zeroPageC }, {&CPU::SEI, &CPU::immediate   }, {&CPU::ADC, &CPU::absoluteY  }, {&CPU::PLY, &CPU::stackF         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteIndexedIndirect    }, {&CPU::ADC, &CPU::absoluteXA }, {&CPU::ROR, &CPU::absoluteXB }, {&CPU::BBR, &CPU::immediate },
                    {&CPU::BRA, &CPU::relativeA }, {&CPU::STA, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::STY, &CPU::zeroPageA   }, {&CPU::STA, &CPU::zeroPageA  }, {&CPU::STX, &CPU::zeroPageA  }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::DEY, &CPU::immediate   }, {&CPU::BIT, &CPU::immediate  }, {&CPU::TXA, &CPU::immediate      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::STY, &CPU::absoluteA                  }, {&CPU::STA, &CPU::absoluteA  }, {&CPU::STX, &CPU::absoluteA  }, {&CPU::BBS, &CPU::immediate },
                    {&CPU::BCC, &CPU::relativeA }, {&CPU::STA, &CPU::zeroPageIndirectIndexed }, {&CPU::STA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::STY, &CPU::zeroPageXA  }, {&CPU::STA, &CPU::zeroPageXA }, {&CPU::STX, &CPU::zeroPageY  }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::TYA, &CPU::immediate   }, {&CPU::STA, &CPU::absoluteY  }, {&CPU::TXS, &CPU::immediate      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::STZ, &CPU::absoluteA                  }, {&CPU::STA, &CPU::absoluteXA }, {&CPU::STZ, &CPU::absoluteXA }, {&CPU::BBS, &CPU::immediate },
                    {&CPU::LDY, &CPU::immediate }, {&CPU::LDA, &CPU::zeroPageIndexedIndirect }, {&CPU::LDX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::LDY, &CPU::zeroPageA   }, {&CPU::LDA, &CPU::zeroPageA  }, {&CPU::LDX, &CPU::zeroPageA  }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::TAY, &CPU::immediate   }, {&CPU::LDA, &CPU::immediate  }, {&CPU::TAX, &CPU::immediate      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::LDY, &CPU::absoluteA                  }, {&CPU::LDA, &CPU::absoluteA  }, {&CPU::LDX, &CPU::absoluteA  }, {&CPU::BBS, &CPU::immediate },
                    {&CPU::BCS, &CPU::relativeA }, {&CPU::LDA, &CPU::zeroPageIndirectIndexed }, {&CPU::LDA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::LDY, &CPU::zeroPageXA  }, {&CPU::LDA, &CPU::zeroPageXA }, {&CPU::LDX, &CPU::zeroPageY  }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::CLV, &CPU::immediate   }, {&CPU::LDA, &CPU::absoluteY  }, {&CPU::TSX, &CPU::immediate      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::LDY, &CPU::absoluteXA                 }, {&CPU::LDA, &CPU::absoluteXA }, {&CPU::LDX, &CPU::absoluteY  }, {&CPU::BBS, &CPU::immediate },
                    {&CPU::CPY, &CPU::immediate }, {&CPU::CMP, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::CPY, &CPU::zeroPageA   }, {&CPU::CMP, &CPU::zeroPageA  }, {&CPU::DEC, &CPU::zeroPageB  }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::INY, &CPU::immediate   }, {&CPU::CMP, &CPU::immediate  }, {&CPU::DEX, &CPU::immediate      }, {&CPU::WAI, &CPU::immediate  }, {&CPU::CPY, &CPU::absoluteA                  }, {&CPU::CMP, &CPU::absoluteA  }, {&CPU::DEC, &CPU::absoluteB  }, {&CPU::BBS, &CPU::immediate },
                    {&CPU::BNE, &CPU::relativeA }, {&CPU::CMP, &CPU::zeroPageIndirectIndexed }, {&CPU::CMP, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::CMP, &CPU::zeroPageXA }, {&CPU::DEC, &CPU::zeroPageXB }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::CLD, &CPU::immediate   }, {&CPU::CMP, &CPU::absoluteY  }, {&CPU::PHX, &CPU::stackE         }, {&CPU::STP, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::CMP, &CPU::absoluteXA }, {&CPU::DEC, &CPU::absoluteXB }, {&CPU::BBS, &CPU::immediate },
                    {&CPU::CPX, &CPU::immediate }, {&CPU::SBC, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::CPX, &CPU::zeroPageA   }, {&CPU::SBC, &CPU::zeroPageA  }, {&CPU::INC, &CPU::zeroPageB  }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::INX, &CPU::immediate   }, {&CPU::SBC, &CPU::immediate  }, {&CPU::NOP, &CPU::immediate      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::CPX, &CPU::absoluteA                  }, {&CPU::SBC, &CPU::absoluteA  }, {&CPU::INC, &CPU::absoluteB  }, {&CPU::BBS, &CPU::immediate },
                    {&CPU::BEQ, &CPU::relativeA }, {&CPU::SBC, &CPU::zeroPageIndirectIndexed }, {&CPU::SBC, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::SBC, &CPU::zeroPageXA }, {&CPU::INC, &CPU::zeroPageXB }, {&CPU::SMB, &CPU::zeroPageC }, {&CPU::SED, &CPU::immediate   }, {&CPU::SBC, &CPU::absoluteY  }, {&CPU::PLX, &CPU::stackF         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::SBC, &CPU::absoluteXA }, {&CPU::INC, &CPU::absoluteXB }, {&CPU::BBS, &CPU::immediate },
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
    pushByteToStack((data & 0xFF00) >> 8);  //PCH
    pushByteToStack(data); //PCL
}

byte _65C02::pullByteFromStack(bool incSPBefore, bool incSPAfter) {
    if(incSPBefore) {
        readByte(SP);
        SP++;
    }
    return readByte(SPToAddress(incSPAfter));
}

//return the SP as a full 16 bit address in the first page even though SP is a byte
word _65C02::SPToAddress(bool incrementSP) {
    return incrementSP ? 0x100 | SP++ : 0x100 | SP;
}

word _65C02::rAddrZP() {
    return readByte(fetchByte());
}

word _65C02::wAddrZP() {
    return fetchByte();
}

word _65C02::accumulator() {
    return readByte(PC);
}

word _65C02::wAddrAcm() {
    return 0;
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

word _65C02::absoluteY() {
    word address = fetchWord();
    dword effectiveAddress = address + Y;
    byte data{readByte(effectiveAddress)};
    return (((address & 0xFF) + Y) > 0xFF) ? readByte(effectiveAddress - 0x100) : data;
}

word _65C02::absoluteY() {
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

word _65C02::zeroPageIndirect() {
    byte ZPAddr = fetchByte();
    return readByte(readByte(ZPAddr) | (readByte((byte)(ZPAddr + 1)) << 8));
}

word _65C02::immediate(word* data) {
    *data = fetchByte();
    return *data;
}

word _65C02::absoluteA(word* data) {
    return fetchWord();
}

word _65C02::absoluteB(word* data) {

}

word _65C02::absoluteC(word* data) {
    return fetchWord();
}

word _65C02::absoluteD(word* data) {
    return 0;
}

word _65C02::zeroPageA(word* data) {
    return 0;
}

word _65C02::zeroPageB(word* data) {
    return 0;
}

word _65C02::zeroPageC(word* data) {
    return 0;
}

word _65C02::impliedA(word* data) {
    return 0;
}

word _65C02::impliedB(word* data) {
    return 0;
}

word _65C02::impliedC(word* data) {
    return 0;
}

word _65C02::zeroPageIndirectIndexed(word* data) {
    return 0;
}

word _65C02::zeroPageIndexedIndirect(word* data) {
    return 0;
}

word _65C02::zeroPageXA(word* data) {
    return 0;
}

word _65C02::zeroPageXB(word* data) {
    return 0;
}

word _65C02::zeroPageY(word* data) {
    return 0;
}

word _65C02::absoluteXA(word* data) {
    return 0;
}

word _65C02::absoluteXB(word* data) {
    return 0;
}

word _65C02::relativeA(word* data) {
    return 0;
}

word _65C02::relativeB(word* data) {
    return 0;
}

word _65C02::absoluteIndirect(word* data) {
    return 0;
}

word _65C02::stackA(word* data) {
    return 0;
}

word _65C02::stackB(word* data) {
    return 0;
}

word _65C02::stackC(word* data) {
    return 0;
}

word _65C02::stackD(word* data) {
    return 0;
}

word _65C02::stackE(word* data) {
    return 0;
}

word _65C02::stackF(word* data) {
    return 0;
}

word _65C02::absoluteIndexedIndirect(word* data) {
    return 0;
}

word _65C02::zeroPageIndirect(word* data) {
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
    byte subAddrLow = fetchByte();
    readByte(SPToAddress());
    pushWordToStack(PC);
    PC = (fetchByte() << 8) | subAddrLow;
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
    readByte(PC);
    byte PCL = pullByteFromStack(true, true);
    byte PCH = pullByteFromStack();
    PC = (PCH << 8) | PCL;
    readByte(++PC);
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






