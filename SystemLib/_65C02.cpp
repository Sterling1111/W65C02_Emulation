#include "Bus.h"
#include "_65C02.h"


_65C02::_65C02(double Mhz) : cycles{Mhz} {
    reset();
    using CPU = _65C02;
    opCodeMatrix.reserve(256);

    opCodeMatrix =
            {
                    {&CPU::BRK, &CPU::stackB    }, {&CPU::ORA, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::TSB, &CPU::zeroPageB   }, {&CPU::ORA, &CPU::zeroPageA  }, {&CPU::ASL, &CPU::zeroPageB  }, {&CPU::RMB0, &CPU::zeroPageC }, {&CPU::PHP, &CPU::stackE      }, {&CPU::ORA, &CPU::immediate  }, {&CPU::ASL, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::TSB, &CPU::absoluteB                  }, {&CPU::ORA, &CPU::absoluteA  }, {&CPU::ASL, &CPU::absoluteB  }, {&CPU::BBR0, &CPU::immediate },
                    {&CPU::BPL, &CPU::relativeA }, {&CPU::ORA, &CPU::zeroPageIndirectIndexed }, {&CPU::ORA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::TRB, &CPU::zeroPageB   }, {&CPU::ORA, &CPU::zeroPageXA }, {&CPU::ASL, &CPU::zeroPageXB }, {&CPU::RMB1, &CPU::zeroPageC }, {&CPU::CLC, &CPU::impliedA    }, {&CPU::ORA, &CPU::absoluteY  }, {&CPU::INC, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::TRB, &CPU::absoluteB                  }, {&CPU::ORA, &CPU::absoluteXA }, {&CPU::ASL, &CPU::absoluteXB }, {&CPU::BBR1, &CPU::immediate },
                    {&CPU::JSR, &CPU::absoluteD }, {&CPU::AND, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::BIT, &CPU::zeroPageB   }, {&CPU::AND, &CPU::zeroPageA  }, {&CPU::ROL, &CPU::zeroPageB  }, {&CPU::RMB2, &CPU::zeroPageC }, {&CPU::PLP, &CPU::stackF      }, {&CPU::AND, &CPU::immediate  }, {&CPU::ROL, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::BIT, &CPU::absoluteA                  }, {&CPU::AND, &CPU::absoluteA  }, {&CPU::ROL, &CPU::absoluteB  }, {&CPU::BBR2, &CPU::immediate },
                    {&CPU::BMI, &CPU::relativeA }, {&CPU::AND, &CPU::zeroPageIndirectIndexed }, {&CPU::AND, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::BIT, &CPU::zeroPageXA  }, {&CPU::AND, &CPU::zeroPageXA }, {&CPU::ROL, &CPU::zeroPageXB }, {&CPU::RMB3, &CPU::zeroPageC }, {&CPU::SEC, &CPU::impliedA    }, {&CPU::AND, &CPU::absoluteY  }, {&CPU::DEC, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::BIT, &CPU::absoluteXA                 }, {&CPU::AND, &CPU::absoluteXA }, {&CPU::ROL, &CPU::absoluteXB }, {&CPU::BBR3, &CPU::immediate },
                    {&CPU::RTI, &CPU::stackC    }, {&CPU::EOR, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::EOR, &CPU::zeroPageA  }, {&CPU::LSR, &CPU::zeroPageB  }, {&CPU::RMB4, &CPU::zeroPageC }, {&CPU::PHA, &CPU::stackE      }, {&CPU::EOR, &CPU::immediate  }, {&CPU::LSR, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteC                  }, {&CPU::EOR, &CPU::absoluteA  }, {&CPU::LSR, &CPU::absoluteB  }, {&CPU::BBR4, &CPU::immediate },
                    {&CPU::BVC, &CPU::relativeA }, {&CPU::EOR, &CPU::zeroPageIndirectIndexed }, {&CPU::EOR, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::EOR, &CPU::zeroPageXA }, {&CPU::LSR, &CPU::zeroPageXB }, {&CPU::RMB5, &CPU::zeroPageC }, {&CPU::CLI, &CPU::impliedA    }, {&CPU::EOR, &CPU::absoluteY  }, {&CPU::PHY, &CPU::stackE         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::EOR, &CPU::absoluteXA }, {&CPU::LSR, &CPU::absoluteXB }, {&CPU::BBR5, &CPU::immediate },
                    {&CPU::RTS, &CPU::stackD    }, {&CPU::ADC, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::STZ, &CPU::zeroPageA   }, {&CPU::ADC, &CPU::zeroPageA  }, {&CPU::ROR, &CPU::zeroPageB  }, {&CPU::RMB6, &CPU::zeroPageC }, {&CPU::PLA, &CPU::stackF      }, {&CPU::ADC, &CPU::immediate  }, {&CPU::ROR, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteIndirect           }, {&CPU::ADC, &CPU::absoluteA  }, {&CPU::ROR, &CPU::absoluteB  }, {&CPU::BBR6, &CPU::immediate },
                    {&CPU::BVS, &CPU::relativeA }, {&CPU::ADC, &CPU::zeroPageIndirectIndexed }, {&CPU::ADC, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::STZ, &CPU::zeroPageB   }, {&CPU::ADC, &CPU::zeroPageXA }, {&CPU::ROR, &CPU::zeroPageXB }, {&CPU::RMB7, &CPU::zeroPageC }, {&CPU::SEI, &CPU::impliedA    }, {&CPU::ADC, &CPU::absoluteY  }, {&CPU::PLY, &CPU::stackF         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteIndexedIndirect    }, {&CPU::ADC, &CPU::absoluteXA }, {&CPU::ROR, &CPU::absoluteXB }, {&CPU::BBR7, &CPU::immediate },
                    {&CPU::BRA, &CPU::relativeA }, {&CPU::STA, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::STY, &CPU::zeroPageA   }, {&CPU::STA, &CPU::zeroPageA  }, {&CPU::STX, &CPU::zeroPageA  }, {&CPU::SMB0, &CPU::zeroPageC }, {&CPU::DEY, &CPU::impliedA    }, {&CPU::BIT, &CPU::immediate  }, {&CPU::TXA, &CPU::impliedA      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::STY, &CPU::absoluteA                  }, {&CPU::STA, &CPU::absoluteA  }, {&CPU::STX, &CPU::absoluteA  }, {&CPU::BBS0, &CPU::immediate },
                    {&CPU::BCC, &CPU::relativeA }, {&CPU::STA, &CPU::zeroPageIndirectIndexed }, {&CPU::STA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::STY, &CPU::zeroPageXA  }, {&CPU::STA, &CPU::zeroPageXA }, {&CPU::STX, &CPU::zeroPageY  }, {&CPU::SMB1, &CPU::zeroPageC }, {&CPU::TYA, &CPU::impliedA    }, {&CPU::STA, &CPU::absoluteY  }, {&CPU::TXS, &CPU::impliedA       }, {&CPU::XXX, &CPU::immediate  }, {&CPU::STZ, &CPU::absoluteA                  }, {&CPU::STA, &CPU::absoluteXA }, {&CPU::STZ, &CPU::absoluteXA }, {&CPU::BBS1, &CPU::immediate },
                    {&CPU::LDY, &CPU::immediate }, {&CPU::LDA, &CPU::zeroPageIndexedIndirect }, {&CPU::LDX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::LDY, &CPU::zeroPageA   }, {&CPU::LDA, &CPU::zeroPageA  }, {&CPU::LDX, &CPU::zeroPageA  }, {&CPU::SMB2, &CPU::zeroPageC }, {&CPU::TAY, &CPU::impliedA    }, {&CPU::LDA, &CPU::immediate  }, {&CPU::TAX, &CPU::immediate      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::LDY, &CPU::absoluteA                  }, {&CPU::LDA, &CPU::absoluteA  }, {&CPU::LDX, &CPU::absoluteA  }, {&CPU::BBS2, &CPU::immediate },
                    {&CPU::BCS, &CPU::relativeA }, {&CPU::LDA, &CPU::zeroPageIndirectIndexed }, {&CPU::LDA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::LDY, &CPU::zeroPageXA  }, {&CPU::LDA, &CPU::zeroPageXA }, {&CPU::LDX, &CPU::zeroPageY  }, {&CPU::SMB3, &CPU::zeroPageC }, {&CPU::CLV, &CPU::impliedA    }, {&CPU::LDA, &CPU::absoluteY  }, {&CPU::TSX, &CPU::impliedA       }, {&CPU::XXX, &CPU::immediate  }, {&CPU::LDY, &CPU::absoluteXA                 }, {&CPU::LDA, &CPU::absoluteXA }, {&CPU::LDX, &CPU::absoluteY  }, {&CPU::BBS3, &CPU::immediate },
                    {&CPU::CPY, &CPU::immediate }, {&CPU::CMP, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::CPY, &CPU::zeroPageA   }, {&CPU::CMP, &CPU::zeroPageA  }, {&CPU::DEC, &CPU::zeroPageB  }, {&CPU::SMB4, &CPU::zeroPageC }, {&CPU::INY, &CPU::impliedA    }, {&CPU::CMP, &CPU::immediate  }, {&CPU::DEX, &CPU::impliedA       }, {&CPU::WAI, &CPU::immediate  }, {&CPU::CPY, &CPU::absoluteA                  }, {&CPU::CMP, &CPU::absoluteA  }, {&CPU::DEC, &CPU::absoluteB  }, {&CPU::BBS4, &CPU::immediate },
                    {&CPU::BNE, &CPU::relativeA }, {&CPU::CMP, &CPU::zeroPageIndirectIndexed }, {&CPU::CMP, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::CMP, &CPU::zeroPageXA }, {&CPU::DEC, &CPU::zeroPageXB }, {&CPU::SMB5, &CPU::zeroPageC }, {&CPU::CLD, &CPU::impliedA    }, {&CPU::CMP, &CPU::absoluteY  }, {&CPU::PHX, &CPU::stackE         }, {&CPU::STP, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::CMP, &CPU::absoluteXA }, {&CPU::DEC, &CPU::absoluteXB }, {&CPU::BBS5, &CPU::immediate },
                    {&CPU::CPX, &CPU::immediate }, {&CPU::SBC, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::CPX, &CPU::zeroPageA   }, {&CPU::SBC, &CPU::zeroPageA  }, {&CPU::INC, &CPU::zeroPageB  }, {&CPU::SMB6, &CPU::zeroPageC }, {&CPU::INX, &CPU::impliedA    }, {&CPU::SBC, &CPU::immediate  }, {&CPU::NOP, &CPU::impliedA       }, {&CPU::XXX, &CPU::immediate  }, {&CPU::CPX, &CPU::absoluteA                  }, {&CPU::SBC, &CPU::absoluteA  }, {&CPU::INC, &CPU::absoluteB  }, {&CPU::BBS6, &CPU::immediate },
                    {&CPU::BEQ, &CPU::relativeA }, {&CPU::SBC, &CPU::zeroPageIndirectIndexed }, {&CPU::SBC, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::SBC, &CPU::zeroPageXA }, {&CPU::INC, &CPU::zeroPageXB }, {&CPU::SMB7, &CPU::zeroPageC }, {&CPU::SED, &CPU::impliedA    }, {&CPU::SBC, &CPU::absoluteY  }, {&CPU::PLX, &CPU::stackF         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::SBC, &CPU::absoluteXA }, {&CPU::INC, &CPU::absoluteXB }, {&CPU::BBS7, &CPU::immediate },
            };
}

/**
 * Connects the cpu to a bus by setting this->bus to the Bus* passed in
 * @param b Pointer to Bus which the cpu is to be linked to
 */
void _65C02::connectBus(Bus *b) {
    this->bus = b;
}

/**
 * resets the cpu by setting PC to 0xFFFE, SP to 0xFF, PS to 0x00, and A, X, and Y registers to 0x00
 * @param pc the value to set the PC. Default value of 0xFFFE.
 */
void _65C02::reset(word pc) {
    PC = pc;
    SP = 0xFF;
    PS.reset();
    PS.set(StatusFlags::U);
    PS.set(StatusFlags::B);
    PS.set(StatusFlags::I);
    A = X = Y = 0;
}

/**
 * Increments this->cycles and reads a byte from the bus
 * @param address The address to read from
 * @return The value at the given address
 */
byte _65C02::readByte(word address) {
    CyclesIncrementer cd(cycles);
    return bus->read(address);
}

/**
 * Calls {@link _65C02}
 * @param address
 * @return
 */
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

void _65C02::bitInstructionSetStatus(byte value) {
    NZVSetStatus(value);
}

void _65C02::pushByteToStack(byte data) {
    writeByte(data, SPToAddress());
    --SP;
}

void _65C02::pushWordToStack(word data) {
    pushByteToStack((data & 0xFF00) >> 8);  //PCH
    return pushByteToStack(data); //PCL
}

byte _65C02::pullByteFromStack() {
    byte data = readByte(SPToAddress());
    ++SP;
    return data;
}

byte _65C02::readByteFromStack() {
    return readByte(SPToAddress());
}

//return the SP as a full 16 bit address in the first page even though SP is a byte
word _65C02::SPToAddress() {
    return 0x100 | SP;
}

/**
 * Computes the immediate address for the following instructions:
 * LDY, CPY, CPX, LDX, ORA, AND, EOR, ADC, BIT, LDA, CMP, SBC
 * @return The immediate address
 */
word _65C02::immediate(byte _65C02::*, const std::function<byte(byte)>&) {
    return fetchByte();
}

/**
 * Computes the absolute address for the following instructions:
 * BIT, STY, STZ, LSY, CPY, CPX, STX, LDX, ORA, AND, EOR, ADC, STA, LDA, CMP, SBC
 * @param Register The register that the data at computed address will be writen to
 * @return The data at computed address if instruction is a read instruction or 0
 * which is a filler value if the instruction is a write instruction
 */
word _65C02::absoluteA(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    word address{fetchWord()};
    if(Register) {
        writeByte(this->*Register, address);
        return 0;
    } return readByte(address);
}

word _65C02::absoluteB(byte _65C02::*, const std::function<byte(byte)>& op) {
    word address{fetchWord()};
    byte data{readByte(address)};
    readByte(address);
    writeByte(op(data), address);
    return 0;
}

/**
 * Computes the absolute address for the following instruction:
 * JMP
 * @return The absolute address which will be the new PC
 */
word _65C02::absoluteC(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return fetchWord();
}

/**
 * Computes the absolute address for the following instruction:
 * JSR
 * @return The absolute address which will be the new PC
 */
word _65C02::absoluteD(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte subAddrLow = fetchByte();
    readByte(SPToAddress());
    pushWordToStack(PC);
    return (fetchByte() << 8) | subAddrLow;
}

/**
 * Computes the zero page address for the following instructions:
 * BIT, STZ, STY, LDY, CPY, CPX, STX, LDX, ORA, AND, EOR, ADC, STA, LDA, CMP, SBC
 * @param Register The register that the data at computed address will be writen to
 * @return The data at computed address if instruction is a read instruction or 0
 * which is a filler value if the instruction is a write instruction
 */
word _65C02::zeroPageA(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte zeroPage{fetchByte()};
    if(Register) {
        writeByte(this->*Register, zeroPage);
        return 0;
    } return readByte(zeroPage);
}

word _65C02::zeroPageB(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte zeroPage{fetchByte()};
    byte data{readByte(zeroPage)};
    readByte(zeroPage + 1);
    writeByte(op(data), zeroPage);
    return 0;
}

word _65C02::zeroPageC(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte zeroPage{fetchByte()};
    byte data{readByte(zeroPage)};
    readByte(zeroPage);
    writeByte(op(data), zeroPage);
    return 0;
}

word _65C02::accumulator(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    this->*Register = op(this->*Register);
    return readByte(PC);
}

/**
 * Computes the implied address for the following instructions:
 * DEY, INY, INX, DEX, NOP, TYA, TAY, TXA, TXS, TAX, CLC, SEC, CLI, SEI, CLV, CLD, SED
 * @return
 */
word _65C02::impliedA(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return readByte(PC);
}

word _65C02::impliedB(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return 0;
}

word _65C02::impliedC(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return 0;
}

word _65C02::zeroPageIndirectIndexed(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte zpAddress = fetchByte();
    word address = readByte(zpAddress) | (readByte(static_cast<byte>(zpAddress + 1)) << 8);
    if(((address & 0xFF) + Y) > 0xFF)
        readByte(static_cast<byte>(zpAddress + 1));
    if(Register) {
        writeByte(this->*Register, address + Y);
        return 0;
    } return readByte(address + Y);
}

word _65C02::zeroPageIndexedIndirect(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte zeroPage{readByte(PC)};
    fetchByte();
    word effectiveAddress = readByte(static_cast<byte>(zeroPage + X)) | (readByte(static_cast<byte>(zeroPage + X + 1)) << 8);
    if(Register) {
        writeByte(this->*Register, effectiveAddress);
        return 0;
    } return readByte(effectiveAddress);
}

word _65C02::zeroPageXA(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte address{fetchByte()};
    byte effectiveAddress = address + X;
    readByte(address);
    if(Register) {
        writeByte(this->*Register, effectiveAddress);
        return 0;
    } return readByte(effectiveAddress);
}

word _65C02::zeroPageXB(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return 0;
}

word _65C02::zeroPageY(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte address{fetchByte()};
    byte effectiveAddress = address + Y;
    readByte(address);
    cycles += (address + Y > 0xFF);
    if(Register) {
        writeByte(this->*Register, effectiveAddress);
        return 0;
    } else return readByte(effectiveAddress);
}

word _65C02::absoluteXA(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    word address = fetchWord();
    dword effectiveAddress = address + X;
    byte data{readByte(effectiveAddress)};
    if(Register) {
        if(((address & 0xFF) + X) > 0xFF)
            writeByte(this->*Register, effectiveAddress - 0x100);
        else
            writeByte(this->*Register, effectiveAddress);
    }
    else if(((address & 0xFF) + X) > 0xFF)
        readByte(effectiveAddress - 0x100);
    else return data;
    return 0;
}

word _65C02::absoluteXB(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return 0;
}

word _65C02::absoluteY(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    word address = fetchWord();
    dword effectiveAddress = address + Y;
    byte data{readByte(effectiveAddress)};
    if(Register) {
        if(((address & 0xFF) + Y) > 0xFF)
            writeByte(this->*Register, effectiveAddress - 0x100);
        else
            writeByte(this->*Register, effectiveAddress);
    }
    else if(((address & 0xFF) + Y) > 0xFF)
        readByte(effectiveAddress - 0x100);
    else return data;
    return 0;
}

word _65C02::relativeA(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte value{fetchByte()};
    if(op(0)) {     //Then we should branch. the value passed to op is garbage. It isn't needed.
        readByte(PC);
        auto svalue = (sbyte)value;
        if((PC & 0xFF) + svalue < 0 || (PC & 0xFF) + svalue > 0xFF)
            readByte(PC);
        return PC + svalue;
    }
    return PC;
}

word _65C02::relativeB(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return 0;
}

word _65C02::absoluteIndirect(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    word address{fetchWord()};
    readByte(--PC);
    return readByte(address) | (readByte(address + 1) << 8);
}

word _65C02::stackA(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    readByte(PC);
    readByte(PC);
    pushWordToStack(SPToAddress());
    return 0;
}

word _65C02::stackB(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return 0;
}

word _65C02::stackC(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    return 0;
}

word _65C02::stackD(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    readByte(PC);
    pullByteFromStack();
    byte PCL = pullByteFromStack();
    byte PCH = readByteFromStack();
    PC = (PCH << 8) | PCL;
    readByte(PC);
    return PC + 1;
}

word _65C02::stackE(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    readByte(PC);
    pushByteToStack(this->*Register);
    return 0;
}

word _65C02::stackF(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    readByte(PC);
    pullByteFromStack();
    return readByte(SPToAddress());
}

word _65C02::absoluteIndexedIndirect(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    word address{fetchWord()};
    readByte(--PC);
    return readByte(address + X) | (readByte(address + X + 1) << 8);
}

word _65C02::zeroPageIndirect(byte _65C02::* Register, const std::function<byte(byte)>& op) {
    byte ZPAddr = fetchByte();
    word effectiveAddress = readByte(ZPAddr) | (readByte((byte)(ZPAddr + 1)) << 8);
    if(Register) {
        writeByte(this->*Register, effectiveAddress);
        return 0;
    } else return readByte(effectiveAddress);
}

void _65C02::execute(uint64_t instructionsToExecute) {
    Opcode opcode{};
    while(instructionsToExecute--) {
        opcode = opCodeMatrix[fetchByte()];
        (this->*(opcode.instruction))(opcode.addrMode);
    }
}

void _65C02::ADC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    byte value = (this->*addrMode)(nullptr, nullptr);
    word result = (word)A + (word)value + (word)PS.test(StatusFlags::C);
    bool SB1 = A >> SIGN_BIT_POS, SB2 = value >> SIGN_BIT_POS, SBR = result >> SIGN_BIT_POS;
    PS.set(StatusFlags::V, !(SB1 ^ SB2) && (SB2 ^ SBR));
    PS.set(StatusFlags::C, result >> CARRY_BIT_POS);
    loadRegister(A, (byte)(result & MAX_BYTE));
}

void _65C02::AND(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(A, A & (this->*addrMode)(nullptr, nullptr));
}

void _65C02::ASL(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        PS.set(StatusFlags::C, a & 0X80);
        NZSetStatus(a << 1);
        return a << 1;
    });
}

void _65C02::BBR0(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBR1(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBR2(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBR3(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBR4(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBR5(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBR6(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBR7(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS0(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS1(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS2(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS3(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS4(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS5(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS6(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BBS7(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BCC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::C) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

void _65C02::BCS(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::C) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

void _65C02::BEQ(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::Z) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

void _65C02::BIT(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    bitInstructionSetStatus(A & (this->*addrMode)(nullptr, nullptr));
}

void _65C02::BMI(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::N) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

//I dont dare to say that this is clean code. This is dirty code but sometimes if you try to find the ultimate
//solution to every problem you spin your wheels for ages. It works. The end.
void _65C02::BNE(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::Z) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

void _65C02::BPL(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::N) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

void _65C02::BRA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return 1;    //branch always
    });
}

void _65C02::BRK(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::BVC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::V) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

void _65C02::BVS(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::V) ? 1 : 0;    //return 1 for should branch and 2 for should not
    });
}

void _65C02::CLC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::C);
}

void _65C02::CLD(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::D);
}

void _65C02::CLI(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::I);
}

void _65C02::CLV(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::V);
}

void _65C02::CMP(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::CPX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::CPY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::DEC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        NZSetStatus(a - 1);
        return a - 1;
    });
}

void _65C02::DEX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, X - 1);
}

void _65C02::DEY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(Y, Y - 1);
}

void _65C02::EOR(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(A, A ^ (this->*addrMode)(nullptr, nullptr));
}

void _65C02::INC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        NZSetStatus(a + 1);
        return a + 1;
    });
}

void _65C02::INX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, X + 1);
}

void _65C02::INY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(Y, Y + 1);
}

void _65C02::JMP(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void _65C02::JSR(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void _65C02::LDA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(A, (this->*addrMode)(nullptr, nullptr));
}

void _65C02::LDX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(X, (this->*addrMode)(nullptr, nullptr));
}

void _65C02::LDY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(Y, (this->*addrMode)(nullptr, nullptr));
}

void _65C02::LSR(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        PS.set(StatusFlags::C, a & 0x01);
        NZSetStatus(a >> 1);
        return a >> 1;
    });
}

void _65C02::NOP(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    readByte(PC);
}

void _65C02::ORA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(A, A | (this->*addrMode)(nullptr, nullptr));
}

void _65C02::PHA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, nullptr);
}

void _65C02::PHP(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PS.set(StatusFlags::B);
    PS_byte = PS.to_ulong();
    (this->*addrMode)(&_65C02::PS_byte, nullptr);
}

void _65C02::PHX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::X, nullptr);
}

void _65C02::PHY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::Y, nullptr);
}

void _65C02::PLA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(A, (this->*addrMode)(nullptr, nullptr));
}

void _65C02::PLP(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PS = (this->*addrMode)(nullptr, nullptr);
}

void _65C02::PLX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(X, (this->*addrMode)(nullptr, nullptr));
}

void _65C02::PLY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    loadRegister(Y, (this->*addrMode)(nullptr, nullptr));
}

void _65C02::RMB0(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b11111110;
    });
}

void _65C02::RMB1(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b11111101;
    });
}

void _65C02::RMB2(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b11111011;
    });
}

void _65C02::RMB3(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b11110111;
    });
}

void _65C02::RMB4(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b11101111;
    });
}

void _65C02::RMB5(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b11011111;
    });
}

void _65C02::RMB6(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b10111111;
    });
}

void _65C02::RMB7(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a & 0b01111111;
    });
}

void _65C02::ROL(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        byte result = (a << 1) | PS.test(StatusFlags::C);
        PS.set(StatusFlags::C, a & 0X80);
        NZSetStatus(result);
        return result;
    });
}

void _65C02::ROR(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        byte result = (a >> 1) | (PS.test(StatusFlags::C) << SIGN_BIT_POS);
        PS.set(StatusFlags::C, a & 0x01);
        NZSetStatus(result);
        return result;
    });
}

void _65C02::RTI(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::RTS(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void _65C02::SBC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    byte value = (this->*addrMode)(nullptr, nullptr);
    word result = (word)A + (word)~value + (word)(PS.test(StatusFlags::C));
    bool SB1 = A >> SIGN_BIT_POS, SB2 = value >> SIGN_BIT_POS, SBR = (result & 0xFF) >> SIGN_BIT_POS;
    PS.set(StatusFlags::V, (SB1 ^ SB2) && !(SB2 ^ SBR));
    PS.set(StatusFlags::C, !(result >> CARRY_BIT_POS));
    loadRegister(A, (byte)(result & MAX_BYTE));
}

void _65C02::SEC(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::C);
}

void _65C02::SED(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::D);
}

void _65C02::SEI(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::I);
}

void _65C02::SMB0(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b00000001;
    });
}

void _65C02::SMB1(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b00000010;
    });
}

void _65C02::SMB2(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b00000100;
    });
}

void _65C02::SMB3(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b00001000;
    });
}

void _65C02::SMB4(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b00010000;
    });
}

void _65C02::SMB5(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b00100000;
    });
}

void _65C02::SMB6(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b01000000;
    });
}

void _65C02::SMB7(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, [this](byte a) {
        return a | 0b10000000;
    });
}

void _65C02::STA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, nullptr);
}

void _65C02::STP(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::STX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::X, nullptr);
}

void _65C02::STY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::Y, nullptr);
}

void _65C02::STZ(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::ZERO, nullptr);
}

void _65C02::TAX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, A);
}

void _65C02::TAY(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(Y, A);
}

void _65C02::TRB(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        PS.set(StatusFlags::Z, (A & a) == 0);
        return ~A & a;
    });
}

void _65C02::TSB(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(&_65C02::A, [this](byte a) {
        PS.set(StatusFlags::Z, (A & a) == 0);
        return A | a;
    });
}

void _65C02::TSX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, SP);
}

void _65C02::TXA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(A, X);
}

void _65C02::TXS(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    SP = X;
}

void _65C02::TYA(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(A, Y);
}

void _65C02::WAI(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {

}

void _65C02::XXX(word (_65C02::* addrMode)(byte _65C02::*, const std::function<byte(byte)>&)) {
    //NOP(addrMode);
    exit(-1);
}

void _65C02::NZSetStatus(byte value) {
    PS.set(StatusFlags::Z, value == 0);
    PS.set(StatusFlags::N , value >> SIGN_BIT_POS);
}

void _65C02::NZCSetStatus(byte value) {
    NZSetStatus(value);
    PS.set(StatusFlags::C, value >> CARRY_BIT_POS);
}

void _65C02::NZVSetStatus(byte value) {
    PS.set(StatusFlags::Z, value == 0);
    PS.set(StatusFlags::N , value >> SIGN_BIT_POS);
    PS.set(StatusFlags::V, (value & OVERFLOW_BIT_MASK) >> OVERFLOW_BIT_POS);
}

void _65C02::loadRegister(byte& Register, byte value) {
    Register = value;
    NZSetStatus(value);
}

void _65C02::setCycleDuration(double Mhz) {
    cycles.setCycleDuration(Mhz);
}






