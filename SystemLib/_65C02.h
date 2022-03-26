#ifndef INC_65C02_EMULATION__65C02_H
#define INC_65C02_EMULATION__65C02_H

#include "system_types.h"
#include "Cycles.h"
#include <bitset>
#include <vector>

#define SIGN_BIT_POS 7
#define CARRY_BIT_POS 8
#define OVERFLOW_BIT_POS 6
#define OVERFLOW_BIT_MASK 0x40
#define MAX_BYTE 0XFF

class Bus;

class _65C02 {
public:
    word PC;    //program counter
    byte SP;    //stack pointer
    byte A, X, Y;   //registers
    //this will be set equal to PS when PS needs to be pushed onto stack
    byte PS_byte;

    /*carry, zero, interrupt disable, decimal mode, break command,
     * unused-always 1, overflow, negative*/
    //enum StatusFlags {C, Z, I, D, B, U, V, N, numFlags};
    enum StatusFlags {N = 7, V = 6, U = 5, B = 4, D = 3, I = 2, Z = 1, C = 0, numFlags = 8};
    std::bitset<StatusFlags::numFlags> PS;

    Bus* bus = nullptr;

    Cycles cycles;
    static dword cycleDuration;
    struct CyclesIncrementer {
        Cycles& cycles;
        explicit CyclesIncrementer(Cycles& cycles) : cycles{cycles} {};
        ~CyclesIncrementer() { ++cycles; }
    };

    explicit _65C02(double Mhz = 1);
    ~_65C02()= default;

    void connectBus(Bus* b);
    void setCycleDuration(double Mhz);
    void reset(word pc=0xFFFE);
    word readWord(word address);
    byte readByte(word address);
    byte fetchByte();
    word fetchWord();
    void writeWord(word data, word address);
    void writeByte(byte data, word address);
    void bitInstructionSetStatus(byte result);
    void loadRegister(byte& Register, byte value);
    void NZSetStatus(byte value);
    void NZCSetStatus(byte value);
    void NZVSetStatus(byte value);
    void execute(uint64_t instructionsToExecute = 1);
    void pushByteToStack(byte data);
    void pushWordToStack(word data);
    word SPToAddress();
    byte pullByteFromStack();

    void immediate(byte _65C02::* Register = nullptr, byte (*op)(byte, byte)=nullptr);

    word absoluteA(byte _65C02::* Register = nullptr);
    word absoluteB(byte _65C02::* Register = nullptr);
    word absoluteC(byte _65C02::* Register = nullptr);
    word absoluteD(byte _65C02::* Register = nullptr);

    word zeroPageA(byte _65C02::* Register = nullptr);
    word zeroPageB(byte _65C02::* Register = nullptr);
    word zeroPageC(byte _65C02::* Register = nullptr);

    word accumulator(byte _65C02::* Register = nullptr);

    word impliedA(byte _65C02::* Register = nullptr);
    word impliedB(byte _65C02::* Register = nullptr);
    word impliedC(byte _65C02::* Register = nullptr);

    word zeroPageIndirectIndexed(byte _65C02::* Register = nullptr);

    word zeroPageIndexedIndirect(byte _65C02::* Register = nullptr);

    word zeroPageXA(byte _65C02::* Register = nullptr);
    word zeroPageXB(byte _65C02::* Register = nullptr);

    word zeroPageY(byte _65C02::* Register = nullptr);

    word absoluteXA(byte _65C02::* Register = nullptr);
    word absoluteXB(byte _65C02::* Register = nullptr);

    word absoluteY(byte _65C02::* Register = nullptr);

    word relativeA(byte _65C02::* Register = nullptr);
    word relativeB(byte _65C02::* Register = nullptr);

    word absoluteIndirect(byte _65C02::* Register = nullptr);

    word stackA(byte _65C02::* Register = nullptr);
    word stackB(byte _65C02::* Register = nullptr);
    word stackC(byte _65C02::* Register = nullptr);
    word stackD(byte _65C02::* Register = nullptr);
    word stackE(byte _65C02::* Register = nullptr);
    word stackF(byte _65C02::* Register = nullptr);

    word absoluteIndexedIndirect(byte _65C02::* Register = nullptr);

    word zeroPageIndirect(byte _65C02::* Register = nullptr);

    void ADC(word (_65C02::* addrMode)(byte _65C02::*));    void AND(word (_65C02::* addrMode)(byte _65C02::*));	void ASL(word (_65C02::* addrMode)(byte _65C02::*));	void BBR(word (_65C02::* addrMode)(byte _65C02::*));
    void BBS(word (_65C02::* addrMode)(byte _65C02::*));    void BCC(word (_65C02::* addrMode)(byte _65C02::*));    void BCS(word (_65C02::* addrMode)(byte _65C02::*));	void BEQ(word (_65C02::* addrMode)(byte _65C02::*));
    void BIT(word (_65C02::* addrMode)(byte _65C02::*));	void BMI(word (_65C02::* addrMode)(byte _65C02::*));    void BNE(word (_65C02::* addrMode)(byte _65C02::*));	void BPL(word (_65C02::* addrMode)(byte _65C02::*));
    void BRA(word (_65C02::* addrMode)(byte _65C02::*));    void BRK(word (_65C02::* addrMode)(byte _65C02::*));	void BVC(word (_65C02::* addrMode)(byte _65C02::*));    void BVS(word (_65C02::* addrMode)(byte _65C02::*));
    void CLC(word (_65C02::* addrMode)(byte _65C02::*));	void CLD(word (_65C02::* addrMode)(byte _65C02::*));	void CLI(word (_65C02::* addrMode)(byte _65C02::*));    void CLV(word (_65C02::* addrMode)(byte _65C02::*));
    void CMP(word (_65C02::* addrMode)(byte _65C02::*));	void CPX(word (_65C02::* addrMode)(byte _65C02::*));	void CPY(word (_65C02::* addrMode)(byte _65C02::*));    void DEC(word (_65C02::* addrMode)(byte _65C02::*));
    void DEX(word (_65C02::* addrMode)(byte _65C02::*));	void DEY(word (_65C02::* addrMode)(byte _65C02::*));	void EOR(word (_65C02::* addrMode)(byte _65C02::*));    void INC(word (_65C02::* addrMode)(byte _65C02::*));
    void INX(word (_65C02::* addrMode)(byte _65C02::*));	void INY(word (_65C02::* addrMode)(byte _65C02::*));	void JMP(word (_65C02::* addrMode)(byte _65C02::*));    void JSR(word (_65C02::* addrMode)(byte _65C02::*));
    void LDA(word (_65C02::* addrMode)(byte _65C02::*));	void LDX(word (_65C02::* addrMode)(byte _65C02::*));	void LDY(word (_65C02::* addrMode)(byte _65C02::*));    void LSR(word (_65C02::* addrMode)(byte _65C02::*));
    void NOP(word (_65C02::* addrMode)(byte _65C02::*));	void ORA(word (_65C02::* addrMode)(byte _65C02::*));	void PHA(word (_65C02::* addrMode)(byte _65C02::*));    void PHP(word (_65C02::* addrMode)(byte _65C02::*));
    void PHX(word (_65C02::* addrMode)(byte _65C02::*));    void PHY(word (_65C02::* addrMode)(byte _65C02::*));    void PLA(word (_65C02::* addrMode)(byte _65C02::*));	void PLP(word (_65C02::* addrMode)(byte _65C02::*));
    void PLX(word (_65C02::* addrMode)(byte _65C02::*));    void PLY(word (_65C02::* addrMode)(byte _65C02::*));    void RMB(word (_65C02::* addrMode)(byte _65C02::*));    void ROL(word (_65C02::* addrMode)(byte _65C02::*));
    void ROR(word (_65C02::* addrMode)(byte _65C02::*));	void RTI(word (_65C02::* addrMode)(byte _65C02::*));	void RTS(word (_65C02::* addrMode)(byte _65C02::*));	void SBC(word (_65C02::* addrMode)(byte _65C02::*));
    void SEC(word (_65C02::* addrMode)(byte _65C02::*));	void SED(word (_65C02::* addrMode)(byte _65C02::*));	void SEI(word (_65C02::* addrMode)(byte _65C02::*));	void SMB(word (_65C02::* addrMode)(byte _65C02::*));
    void STA(word (_65C02::* addrMode)(byte _65C02::*));    void STP(word (_65C02::* addrMode)(byte _65C02::*));    void STX(word (_65C02::* addrMode)(byte _65C02::*));	void STY(word (_65C02::* addrMode)(byte _65C02::*));
    void STZ(word (_65C02::* addrMode)(byte _65C02::*));    void TAX(word (_65C02::* addrMode)(byte _65C02::*));	void TAY(word (_65C02::* addrMode)(byte _65C02::*));    void TRB(word (_65C02::* addrMode)(byte _65C02::*));
    void TSB(word (_65C02::* addrMode)(byte _65C02::*));    void TSX(word (_65C02::* addrMode)(byte _65C02::*));	void TXA(word (_65C02::* addrMode)(byte _65C02::*));	void TXS(word (_65C02::* addrMode)(byte _65C02::*));
    void TYA(word (_65C02::* addrMode)(byte _65C02::*));    void WAI(word (_65C02::* addrMode)(byte _65C02::*));    void XXX(word (_65C02::* addrMode)(byte _65C02::*));

    struct Opcode {
        void (_65C02::* instruction)(word (_65C02::* addrMode)(byte _65C02::*));
        word (_65C02::* addrMode)(byte _65C02::*);
    };

    std::vector<Opcode> opCodeMatrix;

    //instruction opcodes
    static constexpr byte
    //Load/Store Operations
    INS_LDA_IM = 0xA9,  INS_LDA_ZP = 0xA5, INS_LDA_ZPX = 0xB5, INS_LDA_ABS = 0xAD, INS_LDA_ABSX = 0xBD, INS_LDA_ABSY = 0xB9, INS_LDA_XIND = 0xA1, INS_LDA_INDY = 0xB1, INS_LDA_ZPIND = 0xB2,
    INS_LDX_IM = 0xA2,  INS_LDX_ZP = 0xA6, INS_LDX_ZPY = 0xB6, INS_LDX_ABS = 0xAE, INS_LDX_ABSY = 0xBE,
    INS_LDY_IM = 0xA0,  INS_LDY_ZP = 0xA4, INS_LDY_ZPX = 0xB4, INS_LDY_ABS = 0xAC, INS_LDY_ABSX = 0xBC,
    INS_STA_ZP = 0x85, INS_STA_ZPX = 0x95, INS_STA_ABS = 0x8D, INS_STA_ABSX = 0x9D, INS_STA_ABSY = 0x99, INS_STA_XIND = 0x81, INS_STA_INDY = 0x91,
    INS_STX_ZP = 0x86, INS_STX_ZPY = 0x96, INS_STX_ABS = 0x8E,
    INS_STY_ZP = 0x84, INS_STY_ZPX = 0x94, INS_STY_ABS = 0x8C,
    //Logical Operations
    INS_AND_IM = 0x29, INS_AND_ZP = 0x25, INS_AND_ZPX = 0x35, INS_AND_ABS = 0x2D, INS_AND_ABSX = 0x3D, INS_AND_ABSY = 0x39, INS_AND_XIND = 0x21, INS_AND_INDY = 0x31,
    INS_ORA_IM = 0x09, INS_ORA_ZP = 0x05, INS_ORA_ZPX = 0x15, INS_ORA_ABS = 0x0D, INS_ORA_ABSX = 0x1D, INS_ORA_ABSY = 0x19, INS_ORA_XIND = 0x01, INS_ORA_INDY = 0x11,
    INS_EOR_IM = 0x49, INS_EOR_ZP = 0x45, INS_EOR_ZPX = 0x55, INS_EOR_ABS = 0x4D, INS_EOR_ABSX = 0x5D, INS_EOR_ABSY = 0x59, INS_EOR_XIND = 0x41, INS_EOR_INDY = 0x51,
    INS_BIT_ZP = 0x24, INS_BIT_ABS = 0x2C,
    //JUmps and Calls
    INS_RTS = 0x60,
    INS_JMP_ABS = 0x4C, INS_JMP_IND = 0x6C, INS_JMP_ABS_IND = 0x7C,
    INS_JSR = 0x20,
    //Stack Operations
    INS_PHA_IMP = 0x48,
    INS_PHP_IMP = 0x08,
    INS_PLA_IMP = 0x68,
    INS_PLP_IMP = 0x28,
    INS_TSX_IMP = 0xBA,
    INS_TXS_IMP = 0x9A,
    //Register Transfers
    INS_TXA_IMP = 0x8A,
    INS_TAX_IMP = 0xAA,
    INS_TAY_IMP = 0xA8;
};



#endif //INC_65C02_EMULATION__65C02_H
