#include <iostream>
#include "W65C02.h"

W65C02::W65C02(double Mhz) {
    reset();
    initializeOpcodeMatrix();
    cycles.setCycleDuration(Mhz);
}

void W65C02::initializeOpcodeMatrix() {
    using CPU = W65C02;
    opCodeMatrix =
            {
                    {&CPU::BRK, &CPU::stackB    }, {&CPU::ORA, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::TSB, &CPU::zeroPageB   }, {&CPU::ORA, &CPU::zeroPageA  }, {&CPU::ASL, &CPU::zeroPageB  }, {&CPU::RMB0, &CPU::zeroPageC }, {&CPU::PHP, &CPU::stackE      }, {&CPU::ORA, &CPU::immediate  }, {&CPU::ASL, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::TSB, &CPU::absoluteB                  }, {&CPU::ORA, &CPU::absoluteA  }, {&CPU::ASL, &CPU::absoluteB  }, {&CPU::BBR0, &CPU::relativeB },
                    {&CPU::BPL, &CPU::relativeA }, {&CPU::ORA, &CPU::zeroPageIndirectIndexed }, {&CPU::ORA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::TRB, &CPU::zeroPageB   }, {&CPU::ORA, &CPU::zeroPageXA }, {&CPU::ASL, &CPU::zeroPageXB }, {&CPU::RMB1, &CPU::zeroPageC }, {&CPU::CLC, &CPU::impliedA    }, {&CPU::ORA, &CPU::absoluteY  }, {&CPU::INC, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::TRB, &CPU::absoluteB                  }, {&CPU::ORA, &CPU::absoluteXA }, {&CPU::ASL, &CPU::absoluteXB }, {&CPU::BBR1, &CPU::relativeB },
                    {&CPU::JSR, &CPU::absoluteD }, {&CPU::AND, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::BIT, &CPU::zeroPageA   }, {&CPU::AND, &CPU::zeroPageA  }, {&CPU::ROL, &CPU::zeroPageB  }, {&CPU::RMB2, &CPU::zeroPageC }, {&CPU::PLP, &CPU::stackF      }, {&CPU::AND, &CPU::immediate  }, {&CPU::ROL, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::BIT, &CPU::absoluteA                  }, {&CPU::AND, &CPU::absoluteA  }, {&CPU::ROL, &CPU::absoluteB  }, {&CPU::BBR2, &CPU::relativeB },
                    {&CPU::BMI, &CPU::relativeA }, {&CPU::AND, &CPU::zeroPageIndirectIndexed }, {&CPU::AND, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::BIT, &CPU::zeroPageXA  }, {&CPU::AND, &CPU::zeroPageXA }, {&CPU::ROL, &CPU::zeroPageXB }, {&CPU::RMB3, &CPU::zeroPageC }, {&CPU::SEC, &CPU::impliedA    }, {&CPU::AND, &CPU::absoluteY  }, {&CPU::DEC, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::BIT, &CPU::absoluteXA                 }, {&CPU::AND, &CPU::absoluteXA }, {&CPU::ROL, &CPU::absoluteXB }, {&CPU::BBR3, &CPU::relativeB },
                    {&CPU::RTI, &CPU::stackC    }, {&CPU::EOR, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::EOR, &CPU::zeroPageA  }, {&CPU::LSR, &CPU::zeroPageB  }, {&CPU::RMB4, &CPU::zeroPageC }, {&CPU::PHA, &CPU::stackE      }, {&CPU::EOR, &CPU::immediate  }, {&CPU::LSR, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteC                  }, {&CPU::EOR, &CPU::absoluteA  }, {&CPU::LSR, &CPU::absoluteB  }, {&CPU::BBR4, &CPU::relativeB },
                    {&CPU::BVC, &CPU::relativeA }, {&CPU::EOR, &CPU::zeroPageIndirectIndexed }, {&CPU::EOR, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::EOR, &CPU::zeroPageXA }, {&CPU::LSR, &CPU::zeroPageXB }, {&CPU::RMB5, &CPU::zeroPageC }, {&CPU::CLI, &CPU::impliedA    }, {&CPU::EOR, &CPU::absoluteY  }, {&CPU::PHY, &CPU::stackE         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::EOR, &CPU::absoluteXA }, {&CPU::LSR, &CPU::absoluteXB }, {&CPU::BBR5, &CPU::relativeB },
                    {&CPU::RTS, &CPU::stackD    }, {&CPU::ADC, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::STZ, &CPU::zeroPageA   }, {&CPU::ADC, &CPU::zeroPageA  }, {&CPU::ROR, &CPU::zeroPageB  }, {&CPU::RMB6, &CPU::zeroPageC }, {&CPU::PLA, &CPU::stackF      }, {&CPU::ADC, &CPU::immediate  }, {&CPU::ROR, &CPU::accumulator    }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteIndirect           }, {&CPU::ADC, &CPU::absoluteA  }, {&CPU::ROR, &CPU::absoluteB  }, {&CPU::BBR6, &CPU::relativeB },
                    {&CPU::BVS, &CPU::relativeA }, {&CPU::ADC, &CPU::zeroPageIndirectIndexed }, {&CPU::ADC, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::STZ, &CPU::zeroPageXA  }, {&CPU::ADC, &CPU::zeroPageXA }, {&CPU::ROR, &CPU::zeroPageXB }, {&CPU::RMB7, &CPU::zeroPageC }, {&CPU::SEI, &CPU::impliedA    }, {&CPU::ADC, &CPU::absoluteY  }, {&CPU::PLY, &CPU::stackF         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::JMP, &CPU::absoluteIndexedIndirect    }, {&CPU::ADC, &CPU::absoluteXA }, {&CPU::ROR, &CPU::absoluteXB }, {&CPU::BBR7, &CPU::relativeB },
                    {&CPU::BRA, &CPU::relativeA }, {&CPU::STA, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::STY, &CPU::zeroPageA   }, {&CPU::STA, &CPU::zeroPageA  }, {&CPU::STX, &CPU::zeroPageA  }, {&CPU::SMB0, &CPU::zeroPageC }, {&CPU::DEY, &CPU::impliedA    }, {&CPU::BIT, &CPU::immediate  }, {&CPU::TXA, &CPU::impliedA      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::STY, &CPU::absoluteA                  }, {&CPU::STA, &CPU::absoluteA  }, {&CPU::STX, &CPU::absoluteA  }, {&CPU::BBS0, &CPU::relativeB},
                    {&CPU::BCC, &CPU::relativeA }, {&CPU::STA, &CPU::zeroPageIndirectIndexed }, {&CPU::STA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::STY, &CPU::zeroPageXA  }, {&CPU::STA, &CPU::zeroPageXA }, {&CPU::STX, &CPU::zeroPageY  }, {&CPU::SMB1, &CPU::zeroPageC }, {&CPU::TYA, &CPU::impliedA    }, {&CPU::STA, &CPU::absoluteY  }, {&CPU::TXS, &CPU::impliedA       }, {&CPU::XXX, &CPU::immediate  }, {&CPU::STZ, &CPU::absoluteA                  }, {&CPU::STA, &CPU::absoluteXA }, {&CPU::STZ, &CPU::absoluteXA }, {&CPU::BBS1, &CPU::relativeB },
                    {&CPU::LDY, &CPU::immediate }, {&CPU::LDA, &CPU::zeroPageIndexedIndirect }, {&CPU::LDX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::LDY, &CPU::zeroPageA   }, {&CPU::LDA, &CPU::zeroPageA  }, {&CPU::LDX, &CPU::zeroPageA  }, {&CPU::SMB2, &CPU::zeroPageC }, {&CPU::TAY, &CPU::impliedA    }, {&CPU::LDA, &CPU::immediate  }, {&CPU::TAX, &CPU::impliedA      }, {&CPU::XXX, &CPU::immediate  }, {&CPU::LDY, &CPU::absoluteA                  }, {&CPU::LDA, &CPU::absoluteA  }, {&CPU::LDX, &CPU::absoluteA  }, {&CPU::BBS2, &CPU::relativeB },
                    {&CPU::BCS, &CPU::relativeA }, {&CPU::LDA, &CPU::zeroPageIndirectIndexed }, {&CPU::LDA, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::LDY, &CPU::zeroPageXA  }, {&CPU::LDA, &CPU::zeroPageXA }, {&CPU::LDX, &CPU::zeroPageY  }, {&CPU::SMB3, &CPU::zeroPageC }, {&CPU::CLV, &CPU::impliedA    }, {&CPU::LDA, &CPU::absoluteY  }, {&CPU::TSX, &CPU::impliedA       }, {&CPU::XXX, &CPU::immediate  }, {&CPU::LDY, &CPU::absoluteXA                 }, {&CPU::LDA, &CPU::absoluteXA }, {&CPU::LDX, &CPU::absoluteY  }, {&CPU::BBS3, &CPU::relativeB },
                    {&CPU::CPY, &CPU::immediate }, {&CPU::CMP, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::CPY, &CPU::zeroPageA   }, {&CPU::CMP, &CPU::zeroPageA  }, {&CPU::DEC, &CPU::zeroPageB  }, {&CPU::SMB4, &CPU::zeroPageC }, {&CPU::INY, &CPU::impliedA    }, {&CPU::CMP, &CPU::immediate  }, {&CPU::DEX, &CPU::impliedA       }, {&CPU::WAI, &CPU::impliedB  }, {&CPU::CPY, &CPU::absoluteA                  }, {&CPU::CMP, &CPU::absoluteA  }, {&CPU::DEC, &CPU::absoluteB  }, {&CPU::BBS4, &CPU::relativeB },
                    {&CPU::BNE, &CPU::relativeA }, {&CPU::CMP, &CPU::zeroPageIndirectIndexed }, {&CPU::CMP, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::CMP, &CPU::zeroPageXA }, {&CPU::DEC, &CPU::zeroPageXB }, {&CPU::SMB5, &CPU::zeroPageC }, {&CPU::CLD, &CPU::impliedA    }, {&CPU::CMP, &CPU::absoluteY  }, {&CPU::PHX, &CPU::stackE         }, {&CPU::STP, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::CMP, &CPU::absoluteXA }, {&CPU::DEC, &CPU::absoluteXB }, {&CPU::BBS5, &CPU::relativeB },
                    {&CPU::CPX, &CPU::immediate }, {&CPU::SBC, &CPU::zeroPageIndexedIndirect }, {&CPU::XXX, &CPU::immediate         }, {&CPU::XXX, &CPU::immediate }, {&CPU::CPX, &CPU::zeroPageA   }, {&CPU::SBC, &CPU::zeroPageA  }, {&CPU::INC, &CPU::zeroPageB  }, {&CPU::SMB6, &CPU::zeroPageC }, {&CPU::INX, &CPU::impliedA    }, {&CPU::SBC, &CPU::immediate  }, {&CPU::NOP, &CPU::impliedA       }, {&CPU::XXX, &CPU::immediate  }, {&CPU::CPX, &CPU::absoluteA                  }, {&CPU::SBC, &CPU::absoluteA  }, {&CPU::INC, &CPU::absoluteB  }, {&CPU::BBS6, &CPU::relativeB },
                    {&CPU::BEQ, &CPU::relativeA }, {&CPU::SBC, &CPU::zeroPageIndirectIndexed }, {&CPU::SBC, &CPU::zeroPageIndirect  }, {&CPU::XXX, &CPU::immediate }, {&CPU::XXX, &CPU::immediate   }, {&CPU::SBC, &CPU::zeroPageXA }, {&CPU::INC, &CPU::zeroPageXB }, {&CPU::SMB7, &CPU::zeroPageC }, {&CPU::SED, &CPU::impliedA    }, {&CPU::SBC, &CPU::absoluteY  }, {&CPU::PLX, &CPU::stackF         }, {&CPU::XXX, &CPU::immediate  }, {&CPU::XXX, &CPU::immediate                  }, {&CPU::SBC, &CPU::absoluteXA }, {&CPU::INC, &CPU::absoluteXB }, {&CPU::BBS7, &CPU::relativeB }
            };

    opCodeStringMatrix =
            {
                    {"BRK", "stackB"    }, {"ORA", "zeroPageIndexedIndirect" }, {"XXX", "immediate"         }, {"XXX", "immediate" }, {"TSB", "zeroPageB"   }, {"ORA", "zeroPageA"  }, {"ASL", "zeroPageB"  }, {"RMB0", "zeroPageC" }, {"PHP", "stackE"      }, {"ORA", "immediate"  }, {"ASL", "accumulator"    }, {"XXX", "immediate"  }, {"TSB", "absoluteB"                  }, {"ORA", "absoluteA"  }, {"ASL", "absoluteB"  }, {"BBR0", "relativeB" },
                    {"BPL", "relativeA" }, {"ORA", "zeroPageIndirectIndexed" }, {"ORA", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"TRB", "zeroPageB"   }, {"ORA", "zeroPageXA" }, {"ASL", "zeroPageXB" }, {"RMB1", "zeroPageC" }, {"CLC", "impliedA"    }, {"ORA", "absoluteY"  }, {"INC", "accumulator"    }, {"XXX", "immediate"  }, {"TRB", "absoluteB"                  }, {"ORA", "absoluteXA" }, {"ASL", "absoluteXB" }, {"BBR1", "relativeB" },
                    {"JSR", "absoluteD" }, {"AND", "zeroPageIndexedIndirect" }, {"XXX", "immediate"         }, {"XXX", "immediate" }, {"BIT", "zeroPageA"   }, {"AND", "zeroPageA"  }, {"ROL", "zeroPageB"  }, {"RMB2", "zeroPageC" }, {"PLP", "stackF"      }, {"AND", "immediate"  }, {"ROL", "accumulator"    }, {"XXX", "immediate"  }, {"BIT", "absoluteA"                  }, {"AND", "absoluteA"  }, {"ROL", "absoluteB"  }, {"BBR2", "relativeB" },
                    {"BMI", "relativeA" }, {"AND", "zeroPageIndirectIndexed" }, {"AND", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"BIT", "zeroPageXA"  }, {"AND", "zeroPageXA" }, {"ROL", "zeroPageXB" }, {"RMB3", "zeroPageC" }, {"SEC", "impliedA"    }, {"AND", "absoluteY"  }, {"DEC", "accumulator"    }, {"XXX", "immediate"  }, {"BIT", "absoluteXA"                 }, {"AND", "absoluteXA" }, {"ROL", "absoluteXB" }, {"BBR3", "relativeB" },
                    {"RTI", "stackC"    }, {"EOR", "zeroPageIndexedIndirect" }, {"XXX", "immediate"         }, {"XXX", "immediate" }, {"XXX", "immediate"   }, {"EOR", "zeroPageA"  }, {"LSR", "zeroPageB"  }, {"RMB4", "zeroPageC" }, {"PHA", "stackE"      }, {"EOR", "immediate"  }, {"LSR", "accumulator"    }, {"XXX", "immediate"  }, {"JMP", "absoluteC"                  }, {"EOR", "absoluteA"  }, {"LSR", "absoluteB"  }, {"BBR4", "relativeB" },
                    {"BVC", "relativeA" }, {"EOR", "zeroPageIndirectIndexed" }, {"EOR", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"XXX", "immediate"   }, {"EOR", "zeroPageXA" }, {"LSR", "zeroPageXB" }, {"RMB5", "zeroPageC" }, {"CLI", "impliedA"    }, {"EOR", "absoluteY"  }, {"PHY", "stackE"         }, {"XXX", "immediate"  }, {"XXX", "immediate"                  }, {"EOR", "absoluteXA" }, {"LSR", "absoluteXB" }, {"BBR5", "relativeB" },
                    {"RTS", "stackD"    }, {"ADC", "zeroPageIndexedIndirect" }, {"XXX", "immediate"         }, {"XXX", "immediate" }, {"STZ", "zeroPageA"   }, {"ADC", "zeroPageA"  }, {"ROR", "zeroPageB"  }, {"RMB6", "zeroPageC" }, {"PLA", "stackF"      }, {"ADC", "immediate"  }, {"ROR", "accumulator"    }, {"XXX", "immediate"  }, {"JMP", "absoluteIndirect"           }, {"ADC", "absoluteA"  }, {"ROR", "absoluteB"  }, {"BBR6", "relativeB" },
                    {"BVS", "relativeA" }, {"ADC", "zeroPageIndirectIndexed" }, {"ADC", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"STZ", "zeroPageXA"  }, {"ADC", "zeroPageXA" }, {"ROR", "zeroPageXB" }, {"RMB7", "zeroPageC" }, {"SEI", "impliedA"    }, {"ADC", "absoluteY"  }, {"PLY", "stackF"         }, {"XXX", "immediate"  }, {"JMP", "absoluteIndexedIndirect"    }, {"ADC", "absoluteXA" }, {"ROR", "absoluteXB" }, {"BBR7", "relativeB" },
                    {"BRA", "relativeA" }, {"STA", "zeroPageIndexedIndirect" }, {"XXX", "immediate"         }, {"XXX", "immediate" }, {"STY", "zeroPageA"   }, {"STA", "zeroPageA"  }, {"STX", "zeroPageA"  }, {"SMB0", "zeroPageC" }, {"DEY", "impliedA"    }, {"BIT", "immediate"  }, {"TXA", "impliedA"      }, {"XXX", "immediate"  }, {"STY", "absoluteA"                  }, {"STA", "absoluteA"  }, {"STX", "absoluteA"  }, {"BBS0", "relativeB"},
                    {"BCC", "relativeA" }, {"STA", "zeroPageIndirectIndexed" }, {"STA", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"STY", "zeroPageXA"  }, {"STA", "zeroPageXA" }, {"STX", "zeroPageY"  }, {"SMB1", "zeroPageC" }, {"TYA", "impliedA"    }, {"STA", "absoluteY"  }, {"TXS", "impliedA"       }, {"XXX", "immediate"  }, {"STZ", "absoluteA"                  }, {"STA", "absoluteXA" }, {"STZ", "absoluteXA" }, {"BBS1", "relativeB" },
                    {"LDY", "immediate" }, {"LDA", "zeroPageIndexedIndirect" }, {"LDX", "immediate"         }, {"XXX", "immediate" }, {"LDY", "zeroPageA"   }, {"LDA", "zeroPageA"  }, {"LDX", "zeroPageA"  }, {"SMB2", "zeroPageC" }, {"TAY", "impliedA"    }, {"LDA", "immediate"  }, {"TAX", "impliedA"      }, {"XXX", "immediate"  }, {"LDY", "absoluteA"                  }, {"LDA", "absoluteA"  }, {"LDX", "absoluteA"  }, {"BBS2", "relativeB" },
                    {"BCS", "relativeA" }, {"LDA", "zeroPageIndirectIndexed" }, {"LDA", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"LDY", "zeroPageXA"  }, {"LDA", "zeroPageXA" }, {"LDX", "zeroPageY"  }, {"SMB3", "zeroPageC" }, {"CLV", "impliedA"    }, {"LDA", "absoluteY"  }, {"TSX", "impliedA"       }, {"XXX", "immediate"  }, {"LDY", "absoluteXA"                 }, {"LDA", "absoluteXA" }, {"LDX", "absoluteY"  }, {"BBS3", "relativeB" },
                    {"CPY", "immediate" }, {"CMP", "zeroPageIndexedIndirect" }, {"XXX", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"CPY", "zeroPageA"   }, {"CMP", "zeroPageA"  }, {"DEC", "zeroPageB"  }, {"SMB4", "zeroPageC" }, {"INY", "impliedA"    }, {"CMP", "immediate"  }, {"DEX", "impliedA"       }, {"WAI", "impliedB"  }, {"CPY", "absoluteA"                  }, {"CMP", "absoluteA"  }, {"DEC", "absoluteB"  }, {"BBS4", "relativeB" },
                    {"BNE", "relativeA" }, {"CMP", "zeroPageIndirectIndexed" }, {"CMP", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"XXX", "immediate"   }, {"CMP", "zeroPageXA" }, {"DEC", "zeroPageXB" }, {"SMB5", "zeroPageC" }, {"CLD", "impliedA"    }, {"CMP", "absoluteY"  }, {"PHX", "stackE"         }, {"STP", "immediate"  }, {"XXX", "immediate"                  }, {"CMP", "absoluteXA" }, {"DEC", "absoluteXB" }, {"BBS5", "relativeB" },
                    {"CPX", "immediate" }, {"SBC", "zeroPageIndexedIndirect" }, {"XXX", "immediate"         }, {"XXX", "immediate" }, {"CPX", "zeroPageA"   }, {"SBC", "zeroPageA"  }, {"INC", "zeroPageB"  }, {"SMB6", "zeroPageC" }, {"INX", "impliedA"    }, {"SBC", "immediate"  }, {"NOP", "impliedA"       }, {"XXX", "immediate"  }, {"CPX", "absoluteA"                  }, {"SBC", "absoluteA"  }, {"INC", "absoluteB"  }, {"BBS6", "relativeB" },
                    {"BEQ", "relativeA" }, {"SBC", "zeroPageIndirectIndexed" }, {"SBC", "zeroPageIndirect"  }, {"XXX", "immediate" }, {"XXX", "immediate"   }, {"SBC", "zeroPageXA" }, {"INC", "zeroPageXB" }, {"SMB7", "zeroPageC" }, {"SED", "impliedA"    }, {"SBC", "absoluteY"  }, {"PLX", "stackF"         }, {"XXX", "immediate"  }, {"XXX", "immediate"                  }, {"SBC", "absoluteXA" }, {"INC", "absoluteXB" }, {"BBS7", "relativeB" }
            };

    
}

void W65C02::connectBus(Bus* bus) {
    this->bus = bus;
}

/**
 * resets the cpu by setting PC to 0xFFFE, SP to 0xFF, PS to 0x00, and A, X, and Y registers to 0x00
 * @param pc the value to set the PC. Default value of 0xFFFE.
 */
void W65C02::reset(word pc) {
    PC = pc;
    SP = 0xFF;
    PS.reset();
    PS.set(StatusFlags::U);
    PS.set(StatusFlags::B);
    PS.set(StatusFlags::I);
    A = X = Y = 0;
    STOP = WAIT = false;
}

void W65C02::interruptRequest() {
    WAIT = false;
    if(PS.test(StatusFlags::I)) return;
    PS.reset(StatusFlags::B);
    readByte(PC);
    readByte(PC);
    pushWordToStack(PC);
    pushByteToStack(PS.to_ulong());
    PC = readByte(0xFFFE) | readByte(0xFFFF) << 8;
    PS.set(StatusFlags::I, true);
    PS.reset(StatusFlags::D);
}

void W65C02::nonMaskableInterrupt() {
    WAIT = false;
    readByte(PC);
    readByte(PC);
    pushWordToStack(PC);
    pushByteToStack(PS.to_ulong());
    PC = readByte(0xFFFA) | readByte(0xFFFB) << 8;
    PS.set(StatusFlags::I, true);
    PS.reset(StatusFlags::D);
}

/**
 * Increments this->cycles and reads a byte from the bus
 * @param address The address to read from
 * @return The value at the given address
 */
byte W65C02::readByte(word address) {
    CyclesIncrementer cd(cycles);
    return bus->read(address);
}

/**
 * Calls {@link W65C02}
 * @param address
 * @return
 */
word W65C02::readWord(word address) {
    word data = readByte(address);
    return data | (readByte(address + 1) << 8);
}

byte W65C02::fetchByte() {
    return readByte(PC++);
}

word W65C02::fetchWord() {
    word data = fetchByte();
    return data | (fetchByte() << 8);
}

void W65C02::writeWord(word data, word address) {
    writeByte(data & 0xFF, address);
    writeByte(data >> 8, address + 1);
}

void W65C02::writeByte(byte data, word address) {
    bus->write(data, address);
    ++cycles;
}

void W65C02::bitInstructionSetStatus(byte value, bool immediateMode) {
    if(immediateMode) {
        PS.set(StatusFlags::Z, value == 0);
    }
    NZVSetStatus(value);
}

void W65C02::pushByteToStack(byte data) {
    writeByte(data, SPToAddress());
    --SP;
}

void W65C02::pushWordToStack(word data) {
    pushByteToStack((data & 0xFF00) >> 8);  //PCH
    return pushByteToStack(data); //PCL
}

byte W65C02::pullByteFromStack() {
    byte data = readByte(SPToAddress());
    ++SP;
    return data;
}

byte W65C02::readByteFromStack() {
    return readByte(SPToAddress());
}

word W65C02::SPToAddress() const {
    return 0x100 | SP;
}

/**
 * Computes the immediate address for the following instructions:
 * LDY, CPY, CPX, LDX, ORA, AND, EOR, ADC, BIT, LDA, CMP, SBC
 * @return The immediate address
 */
word W65C02::immediate(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    return fetchByte();
}

/**
 * Computes the absolute address for the following instructions:
 * BIT, STY, STZ, LSY, CPY, CPX, STX, LDX, ORA, AND, EOR, ADC, STA, LDA, CMP, SBC
 * @param Register The register that the data at computed address will be writen to
 * @return The data at computed address if Instruction is a read Instruction or 0
 * which is a filler value if the Instruction is a write Instruction
 */
word W65C02::absoluteA(byte W65C02::* reg, Operation op) {
    word address{fetchWord()};
    if(reg) {
        writeByte(this->*reg, address);
        return 0;
    } return readByte(address);
}

word W65C02::absoluteB(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    word address{fetchWord()};
    byte data{readByte(address)};
    readByte(address);
    writeByte(op(data), address);
    return 0;
}

/**
 * Computes the absolute address for the following Instruction:
 * JMP
 * @return The absolute address which will be the new PC
 */
word W65C02::absoluteC(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    return fetchWord();
}

/**
 * Computes the absolute address for the following Instruction:
 * JSR
 * @return The absolute address which will be the new PC
 */
word W65C02::absoluteD(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    byte subAddrLow = fetchByte();
    readByte(SPToAddress());
    pushWordToStack(PC);
    return (fetchByte() << 8) | subAddrLow;
}

/**
 * Computes the zero page address for the following instructions:
 * BIT, STZ, STY, LDY, CPY, CPX, STX, LDX, ORA, AND, EOR, ADC, STA, LDA, CMP, SBC
 * @param Register The register that the data at computed address will be writen to
 * @return The data at computed address if Instruction is a read Instruction or 0
 * which is a filler value if the Instruction is a write Instruction
 */
word W65C02::zeroPageA(byte W65C02::* reg, Operation op) {
    byte zeroPage{fetchByte()};
    if(reg) {
        writeByte(this->*reg, zeroPage);
        return 0;
    } return readByte(zeroPage);
}

word W65C02::zeroPageB(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    byte zeroPage{fetchByte()};
    byte data{readByte(zeroPage)};
    readByte(zeroPage);
    writeByte(op(data), zeroPage);
    return 0;
}

word W65C02::zeroPageC(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    byte zeroPage{fetchByte()};
    byte data{readByte(zeroPage)};
    readByte(zeroPage);
    writeByte(op(data), zeroPage);
    return 0;
}

word W65C02::accumulator(byte W65C02::* reg, Operation op) {
    this->*reg = op(this->*reg);
    return readByte(PC);
}

/**
 * Computes the implied address for the following instructions:
 * DEY, INY, INX, DEX, NOP, TYA, TAY, TXA, TXS, TAX, CLC, SEC, CLI, SEI, CLV, CLD, SED
 * @return
 */
word W65C02::impliedA(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    return readByte(PC);
}

//TODO - wait for interupt
__attribute__((unused)) word W65C02::impliedB(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    readByte(PC);
    readByte(PC);
    return 0;
}

//TODO - stop the clock
__attribute__((unused)) word W65C02::impliedC(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    readByte(PC);
    readByte(PC);
    return 0;
}

word W65C02::zeroPageIndirectIndexed(byte W65C02::* reg, Operation op) {
    byte zpAddress = fetchByte();
    word address = readByte(zpAddress) | (readByte(static_cast<byte>(zpAddress + 1)) << 8);
    if(((address & 0xFF) + Y) > 0xFF and !reg)
        readByte(static_cast<byte>(zpAddress + 1));
    if(reg) {
        readByte(static_cast<byte>(zpAddress + 1));
        writeByte(this->*reg, address + Y);
        return 0;
    } return readByte(address + Y);
}

word W65C02::zeroPageIndexedIndirect(byte W65C02::* reg, Operation op) {
    byte zeroPage{readByte(PC)};
    fetchByte();
    word effectiveAddress = readByte(static_cast<byte>(zeroPage + X)) | (readByte(static_cast<byte>(zeroPage + X + 1)) << 8);
    if(reg) {
        writeByte(this->*reg, effectiveAddress);
        return 0;
    } return readByte(effectiveAddress);
}

word W65C02::zeroPageXA(byte W65C02::* reg, Operation op) {
    byte address{fetchByte()};
    byte effectiveAddress = address + X;
    readByte(PC - 1);
    if(reg) {
        writeByte(this->*reg, effectiveAddress);
        return 0;
    } return readByte(effectiveAddress);
}

word W65C02::zeroPageXB(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    byte zeroPage{readByte(PC)};
    byte effectiveAddress = zeroPage + X;
    fetchByte();
    byte data{readByte(effectiveAddress)};
    readByte(effectiveAddress);
    writeByte(op(data), effectiveAddress);
    return 0;
}

word W65C02::zeroPageY(byte W65C02::* reg, Operation op) {
    byte address{fetchByte()};
    byte effectiveAddress = address + Y;
    readByte(PC - 1);
    cycles += (address + Y > 0xFF);
    if(reg) {
        writeByte(this->*reg, effectiveAddress);
        return 0;
    } else return readByte(effectiveAddress);
}

word W65C02::absoluteXA(byte W65C02::* reg, Operation op) {
    word address = fetchWord();
    dword effectiveAddress = address + X;
    if(reg) {
        if(((address & 0xFF) + X) > 0xFF) {
            readByte(PC - 1);
            writeByte(this->*reg, effectiveAddress);
        } else {
            readByte(effectiveAddress);
            writeByte(this->*reg, effectiveAddress);
        }
    }
    else if(((address & 0xFF) + X) > 0xFF) {
        readByte(PC - 1);
        return readByte(effectiveAddress);
    } else return readByte(effectiveAddress);
    return 0;
}

word W65C02::absoluteXB(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    auto ins = opcode.instruction;
    word address{fetchWord()};
    word effectiveAddress = address + X;
    if(((address & 0xFF) + X) > 0xFF)
        readByte(PC - 1);
    else if(ins != &W65C02::ASL && ins != &W65C02::LSR && ins != &W65C02::ROL && ins != &W65C02::ROR)
        readByte(effectiveAddress);
    readByte(effectiveAddress);
    byte data{readByte(effectiveAddress)};
    writeByte(op(data), effectiveAddress);
    return 0;
}

word W65C02::absoluteY(byte W65C02::* reg, Operation op) {
    word address = fetchWord();
    dword effectiveAddress = address + Y;
    if(reg) {
        if(((address & 0xFF) + Y) > 0xFF) {
            readByte(PC - 1);
            writeByte(this->*reg, effectiveAddress);
        } else {
            readByte(effectiveAddress);
            writeByte(this->*reg, effectiveAddress);
        }
    }
    else if(((address & 0xFF) + Y) > 0xFF) {
        readByte(PC - 1);
        return readByte(effectiveAddress);
    } else return readByte(effectiveAddress);
    return 0;
}

word W65C02::relativeA(__attribute((unused)) byte W65C02::* reg, Operation op) {
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

//TODO - test the correctness of this function it is gautenteed to be wrong.
word W65C02::relativeB(__attribute__((unused)) byte W65C02::* reg, Operation op) {
    byte value{fetchByte()};
    byte zeroPage{fetchByte()};
    readByte(zeroPage);
    readByte(zeroPage);
    readByte(zeroPage);
    if(op(0)) {     //Then we should branch. the value passed to op is garbage. It isn't needed.
        readByte(PC);
        auto svalue = (sbyte)value;
        if((PC & 0xFF) + svalue < 0 || (PC & 0xFF) + svalue > 0xFF)
            readByte(PC);
        return PC + svalue;
    }
    return PC;
}

word W65C02::absoluteIndirect(byte W65C02::* reg, Operation op) {
    word address{fetchWord()};
    readByte(--PC);
    return readByte(address) | (readByte(address + 1) << 8);
}

word W65C02::stackA(byte W65C02::* reg, Operation op) {
    readByte(PC);
    readByte(PC);
    pushWordToStack(SPToAddress());
    return 0;
}

word W65C02::stackB(byte W65C02::* reg, Operation op) {
    fetchByte();
    pushWordToStack(PC);
    PS.set(StatusFlags::B, true);
    pushByteToStack(PS.to_ulong());
    PS.set(StatusFlags::I, true);
    return readByte(0xFFFE) | readByte(0xFFFF) << 8;
}

word W65C02::stackC(byte W65C02::* reg, Operation op) {
    pullByteFromStack();
    PS = pullByteFromStack();
    byte PCL = pullByteFromStack();
    byte PCH = readByteFromStack();
    return (PCH << 8) | PCL;
}

word W65C02::stackD(byte W65C02::* reg, Operation op) {
    readByte(PC);
    pullByteFromStack();
    byte PCL = pullByteFromStack();
    byte PCH = readByteFromStack();
    PC = (PCH << 8) | PCL;
    readByte(PC);
    return PC + 1;
}

word W65C02::stackE(byte W65C02::* reg, Operation op) {
    readByte(PC);
    pushByteToStack(this->*reg);
    return 0;
}

word W65C02::stackF(byte W65C02::* reg, Operation op) {
    readByte(PC);
    pullByteFromStack();
    return readByte(SPToAddress());
}

word W65C02::absoluteIndexedIndirect(byte W65C02::* reg, Operation op) {
    word address{fetchWord()};
    readByte(--PC);
    return readByte(address + X) | (readByte(address + X + 1) << 8);
}

word W65C02::zeroPageIndirect(byte W65C02::* reg, Operation op) {
    byte ZPAddr = fetchByte();
    word effectiveAddress = readByte(ZPAddr) | (readByte((byte)(ZPAddr + 1)) << 8);
    if(reg) {
        writeByte(this->*reg, effectiveAddress);
        return 0;
    } else return readByte(effectiveAddress);
}

void W65C02::execute(uint64_t numInstructionsToExecute) {
    while(numInstructionsToExecute--) {
        if(STOP) continue;
        if(IRQB) interruptRequest();
        if(NMIB) {
            nonMaskableInterrupt();
            NMIB = false;
        }
        if(WAIT) continue;
        int pcTemp = PC;
        byte opcodeNum = fetchByte();
        opcode = opCodeMatrix[opcodeNum];
        opcodeString = opCodeStringMatrix[opcodeNum];

        (this->*(opcode.instruction))(opcode.addressMode);
        /*std::cout << opcodeString.instructionString << "\t" << std::left << std::setw(25) << opcodeString.addressModeString <<
                  "PC: " << std::setw(10) << PC << "A: " << std::setw(10) << (int)A << "X: "  << std::setw(10) << (int)X << "Y: " << std::setw(10) << (int)Y <<
                  "SP: " << std::setw(10) << (int)SP << std::endl;*/
    }
}

void W65C02::ADC(AddressMode addrMode) {
    byte value = (this->*addrMode)(nullptr, nullptr);
    word result = (word)A + (word)value + (word)PS.test(StatusFlags::C);
    bool SB1 = A >> SIGN_BIT_POS, SB2 = value >> SIGN_BIT_POS, SBR = (result & SIGN_BIT_MASK) >> SIGN_BIT_POS;
    PS.set(StatusFlags::V, !(SB1 ^ SB2) && (SB2 ^ SBR));
    PS.set(StatusFlags::C, result >> CARRY_BIT_POS);
    loadRegister(A, (byte)(result & MAX_BYTE));
}

void W65C02::AND(AddressMode addrMode) {
    loadRegister(A, A & (this->*addrMode)(nullptr, nullptr));
}

void W65C02::ASL(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        PS.set(StatusFlags::C, a & 0X80);
        NZSetStatus(a << 1);
        return a << 1;
    });
}

void W65C02::BBR0(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b00000001);
    });
}

void W65C02::BBR1(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b00000010);
    });
}

void W65C02::BBR2(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b00000100);
    });
}

void W65C02::BBR3(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b00001000);
    });
}

void W65C02::BBR4(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b00010000);
    });
}

void W65C02::BBR5(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b00100000);
    });
}

void W65C02::BBR6(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b01000000);
    });
}

void W65C02::BBR7(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return !(data & 0b10000000);
    });

}

void W65C02::BBS0(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b00000001);
    });
}

void W65C02::BBS1(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b00000010);
    });
}

void W65C02::BBS2(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b00000100);
    });
}

void W65C02::BBS3(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b00001000);
    });
}

void W65C02::BBS4(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b00010000);
    });
}

void W65C02::BBS5(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b00100000);
    });
}

void W65C02::BBS6(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b01000000);
    });
}

void W65C02::BBS7(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte data) {
        return (data & 0b10000000);
    });
}

void W65C02::BCC(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::C);    //return true for should branch false otherwise
    });
}

void W65C02::BCS(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::C) ? 1 : 0;    //return true for should branch false otherwise
    });
}

void W65C02::BEQ(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::Z);    //return true for should branch false otherwise
    });
}

void W65C02::BIT(AddressMode addrMode) {
    byte memVal = (this->*addrMode)(nullptr, nullptr);
    byte result = A & memVal;
    PS.set(StatusFlags::Z, result == 0);
    if(addrMode != &W65C02::immediate) {
        PS.set(StatusFlags::N, (bool)(memVal & 0x80));
        PS.set(StatusFlags::V, (bool)(memVal & 0x40));
    }
}

void W65C02::BMI(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::N);    //return true for should branch false otherwise
    });
}

void W65C02::BNE(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::Z);    //return true for should branch false otherwise
    });
}

void W65C02::BPL(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::N);    //return true for should branch false otherwise
    });
}

void W65C02::BRA(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [](byte) {
        return 1;    //branch always
    });
}

void W65C02::BRK(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void W65C02::BVC(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return !PS.test(StatusFlags::V);    //return true for should branch false otherwise
    });
}

void W65C02::BVS(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, [this](byte) {
        return PS.test(StatusFlags::V);    //return true for should branch false otherwise
    });
}

void W65C02::CLC(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::C);
}

void W65C02::CLD(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::D);
}

void W65C02::CLI(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::I);
}

void W65C02::CLV(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    PS.reset(StatusFlags::V);
}

void W65C02::CMP(AddressMode addrMode) {
    byte value = (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::C, A >= value);
    PS.set(StatusFlags::Z, A == value);
    PS.set(StatusFlags::N, ((byte)(A - value)) >> 7);
}

void W65C02::CPX(AddressMode addrMode) {
    byte value = (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::C, X >= value);
    PS.set(StatusFlags::Z, X == value);
    PS.set(StatusFlags::N, ((byte)(X - value)) >> 7);
}

void W65C02::CPY(AddressMode addrMode) {
    byte value = (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::C, Y >= value);
    PS.set(StatusFlags::Z, Y == value);
    PS.set(StatusFlags::N, ((byte)(Y - value)) >> 7);
}

void W65C02::DEC(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        NZSetStatus(a - 1);
        return a - 1;
    });
}

void W65C02::DEX(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, X - 1);
}

void W65C02::DEY(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(Y, Y - 1);
}

void W65C02::EOR(AddressMode addrMode) {
    loadRegister(A, A ^ (this->*addrMode)(nullptr, nullptr));
}

void W65C02::INC(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        NZSetStatus(a + 1);
        return a + 1;
    });
}

void W65C02::INX(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, X + 1);
}

void W65C02::INY(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(Y, Y + 1);
}

void W65C02::JMP(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void W65C02::JSR(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void W65C02::LDA(AddressMode addrMode) {
    loadRegister(A, (this->*addrMode)(nullptr, nullptr));
}

void W65C02::LDX(AddressMode addrMode) {
    loadRegister(X, (this->*addrMode)(nullptr, nullptr));
}

void W65C02::LDY(AddressMode addrMode) {
    loadRegister(Y, (this->*addrMode)(nullptr, nullptr));
}

void W65C02::LSR(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        PS.set(StatusFlags::C, a & 0x01);
        NZSetStatus(a >> 1);
        return a >> 1;
    });
}

void W65C02::NOP(AddressMode addrMode) {
    readByte(PC);
}

void W65C02::ORA(AddressMode addrMode) {
    loadRegister(A, A | (this->*addrMode)(nullptr, nullptr));
}

void W65C02::PHA(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, nullptr);
}

void W65C02::PHP(AddressMode addrMode) {
    PS.set(StatusFlags::B);
    PS_byte = PS.to_ulong();
    (this->*addrMode)(&W65C02::PS_byte, nullptr);
}

void W65C02::PHX(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::X, nullptr);
}

void W65C02::PHY(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::Y, nullptr);
}

void W65C02::PLA(AddressMode addrMode) {
    loadRegister(A, (this->*addrMode)(nullptr, nullptr));
}

void W65C02::PLP(AddressMode addrMode) {
    PS = (this->*addrMode)(nullptr, nullptr);
}

void W65C02::PLX(AddressMode addrMode) {
    loadRegister(X, (this->*addrMode)(nullptr, nullptr));
}

void W65C02::PLY(AddressMode addrMode) {
    loadRegister(Y, (this->*addrMode)(nullptr, nullptr));
}

void W65C02::RMB0(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b11111110;
    });
}

void W65C02::RMB1(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b11111101;
    });
}

void W65C02::RMB2(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b11111011;
    });
}

void W65C02::RMB3(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b11110111;
    });
}

void W65C02::RMB4(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b11101111;
    });
}

void W65C02::RMB5(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b11011111;
    });
}

void W65C02::RMB6(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b10111111;
    });
}

void W65C02::RMB7(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a & 0b01111111;
    });
}

void W65C02::ROL(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        byte result = (a << 1) | PS.test(StatusFlags::C);
        PS.set(StatusFlags::C, a & 0X80);
        NZSetStatus(result);
        return result;
    });
}

void W65C02::ROR(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        byte result = (a >> 1) | (PS.test(StatusFlags::C) << SIGN_BIT_POS);
        PS.set(StatusFlags::C, a & 0x01);
        NZSetStatus(result);
        return result;
    });
}

void W65C02::RTI(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void W65C02::RTS(AddressMode addrMode) {
    PC = (this->*addrMode)(nullptr, nullptr);
}

void W65C02::SBC(AddressMode addrMode) {
    byte value = (this->*addrMode)(nullptr, nullptr);
    word result = (word)A + (word)~value + (word)(PS.test(StatusFlags::C));
    bool SB1 = A >> SIGN_BIT_POS, SB2 = value >> SIGN_BIT_POS, SBR = (result & SIGN_BIT_MASK) >> SIGN_BIT_POS;
    PS.set(StatusFlags::V, (SB1 ^ SB2) && !(SB2 ^ SBR));
    PS.set(StatusFlags::C, !(result >> CARRY_BIT_POS));
    loadRegister(A, (byte)(result & MAX_BYTE));
}

void W65C02::SEC(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::C);
}

void W65C02::SED(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::D);
}

void W65C02::SEI(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    PS.set(StatusFlags::I);
}

void W65C02::SMB0(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b00000001;
    });
}

void W65C02::SMB1(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b00000010;
    });
}

void W65C02::SMB2(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b00000100;
    });
}

void W65C02::SMB3(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b00001000;
    });
}

void W65C02::SMB4(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b00010000;
    });
}

void W65C02::SMB5(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b00100000;
    });
}

void W65C02::SMB6(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b01000000;
    });
}

void W65C02::SMB7(AddressMode addrMode) {
    (this->*addrMode)(nullptr, [](byte a) {
        return a | 0b10000000;
    });
}

void W65C02::STA(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, nullptr);
}

void W65C02::STP(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    STOP = true;
}

void W65C02::STX(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::X, nullptr);
}

void W65C02::STY(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::Y, nullptr);
}

void W65C02::STZ(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::ZERO, nullptr);
}

void W65C02::TAX(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, A);
}

void W65C02::TAY(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(Y, A);
}

void W65C02::TRB(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        PS.set(StatusFlags::Z, (A & a) == 0);
        return ~A & a;
    });
}

void W65C02::TSB(AddressMode addrMode) {
    (this->*addrMode)(&W65C02::A, [this](byte a) {
        PS.set(StatusFlags::Z, (A & a) == 0);
        return A | a;
    });
}

void W65C02::TSX(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(X, SP);
}

void W65C02::TXA(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(A, X);
}

void W65C02::TXS(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    SP = X;
}

void W65C02::TYA(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    loadRegister(A, Y);
}

void W65C02::WAI(AddressMode addrMode) {
    (this->*addrMode)(nullptr, nullptr);
    WAIT = true;
}

void W65C02::XXX(AddressMode addrMode) {
    NOP(addrMode);
}

void W65C02::NZSetStatus(byte value) {
    PS.set(StatusFlags::Z, value == 0);
    PS.set(StatusFlags::N , value >> SIGN_BIT_POS);
}

void W65C02::NZCSetStatus(byte value) {
    NZSetStatus(value);
    PS.set(StatusFlags::C, value >> CARRY_BIT_POS);
}

void W65C02::NZVSetStatus(byte value) {
    PS.set(StatusFlags::Z, value == 0);
    PS.set(StatusFlags::N , value >> SIGN_BIT_POS);
    PS.set(StatusFlags::V, (value & OVERFLOW_BIT_MASK) >> OVERFLOW_BIT_POS);
}

void W65C02::loadRegister(byte& Register, byte value) {
    Register = value;
    NZSetStatus(value);
}

void W65C02::setCycleDuration(double Mhz) {
    cycles.setCycleDuration(Mhz);
}
