#ifndef INC_65C02_EMULATION__65C02_H
#define INC_65C02_EMULATION__65C02_H

#include "system_types.h"
#include "Cycles.h"
#include <bitset>
#include <vector>

class Bus;

class _65C02 {
public:
    word PC;    //program counter
    byte SP;    //stack pointer
    byte A, X, Y;   //registers

    /*carry, zero, interrupt disable, decimal mode, break command,
     * unused-always 1, overflow, negative*/
    enum StatusFlags {C, Z, I, D, B, U, V, N, numFlags};
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
    void execute(uint64_t instructionsToExecute = 1);
    void pushByteToStack(byte data);
    void pushWordToStack(word data);
    word SPToAddress(bool incrementSP=false);
    byte pullByteFromStack(bool incSPBefore = false, bool incSPAfter = false);

    word immediate(word* = nullptr);

    word absoluteA(word* = nullptr);
    word absoluteB(word* = nullptr);
    word absoluteC(word* = nullptr);
    word absoluteD(word* = nullptr);

    word zeroPageA(word* = nullptr);
    word zeroPageB(word* = nullptr);
    word zeroPageC(word* = nullptr);

    word accumulator(word* = nullptr);

    word impliedA(word* = nullptr);
    word impliedB(word* = nullptr);
    word impliedC(word* = nullptr);

    word zeroPageIndirectIndexed(word* = nullptr);

    word zeroPageIndexedIndirect(word* = nullptr);

    word zeroPageXA(word* = nullptr);
    word zeroPageXB(word* = nullptr);

    word zeroPageY(word* = nullptr);

    word absoluteXA(word* = nullptr);
    word absoluteXB(word* = nullptr);

    word absoluteY(word* = nullptr);

    word relativeA(word* = nullptr);
    word relativeB(word* = nullptr);

    word absoluteIndirect(word* = nullptr);

    word stackA(word* = nullptr);
    word stackB(word* = nullptr);
    word stackC(word* = nullptr);
    word stackD(word* = nullptr);
    word stackE(word* = nullptr);
    word stackF(word* = nullptr);

    word absoluteIndexedIndirect(word* = nullptr);

    word zeroPageIndirect(word* = nullptr);

    void ADC(word (_65C02::* addrMode)());	void AND(word (_65C02::* addrMode)());	void ASL(word (_65C02::* addrMode)());	void BBR(word (_65C02::* addrMode)());
    void BBS(word (_65C02::* addrMode)());  void BCC(word (_65C02::* addrMode)());  void BCS(word (_65C02::* addrMode)());	void BEQ(word (_65C02::* addrMode)());
    void BIT(word (_65C02::* addrMode)());	void BMI(word (_65C02::* addrMode)());  void BNE(word (_65C02::* addrMode)());	void BPL(word (_65C02::* addrMode)());
    void BRA(word (_65C02::* addrMode)());  void BRK(word (_65C02::* addrMode)());	void BVC(word (_65C02::* addrMode)());  void BVS(word (_65C02::* addrMode)());
    void CLC(word (_65C02::* addrMode)());	void CLD(word (_65C02::* addrMode)());	void CLI(word (_65C02::* addrMode)());  void CLV(word (_65C02::* addrMode)());
    void CMP(word (_65C02::* addrMode)());	void CPX(word (_65C02::* addrMode)());	void CPY(word (_65C02::* addrMode)());  void DEC(word (_65C02::* addrMode)());
    void DEX(word (_65C02::* addrMode)());	void DEY(word (_65C02::* addrMode)());	void EOR(word (_65C02::* addrMode)());  void INC(word (_65C02::* addrMode)());
    void INX(word (_65C02::* addrMode)());	void INY(word (_65C02::* addrMode)());	void JMP(word (_65C02::* addrMode)());  void JSR(word (_65C02::* addrMode)());
    void LDA(word (_65C02::* addrMode)());	void LDX(word (_65C02::* addrMode)());	void LDY(word (_65C02::* addrMode)());  void LSR(word (_65C02::* addrMode)());
    void NOP(word (_65C02::* addrMode)());	void ORA(word (_65C02::* addrMode)());	void PHA(word (_65C02::* addrMode)());  void PHP(word (_65C02::* addrMode)());
    void PHX(word (_65C02::* addrMode)());  void PHY(word (_65C02::* addrMode)());  void PLA(word (_65C02::* addrMode)());	void PLP(word (_65C02::* addrMode)());
    void PLX(word (_65C02::* addrMode)());  void PLY(word (_65C02::* addrMode)());  void RMB(word (_65C02::* addrMode)());  void ROL(word (_65C02::* addrMode)());
    void ROR(word (_65C02::* addrMode)());	void RTI(word (_65C02::* addrMode)());	void RTS(word (_65C02::* addrMode)());	void SBC(word (_65C02::* addrMode)());
    void SEC(word (_65C02::* addrMode)());	void SED(word (_65C02::* addrMode)());	void SEI(word (_65C02::* addrMode)());	void SMB(word (_65C02::* addrMode)());
    void STA(word (_65C02::* addrMode)());  void STP(word (_65C02::* addrMode)());  void STX(word (_65C02::* addrMode)());	void STY(word (_65C02::* addrMode)());
    void STZ(word (_65C02::* addrMode)());  void TAX(word (_65C02::* addrMode)());	void TAY(word (_65C02::* addrMode)());  void TRB(word (_65C02::* addrMode)());
    void TSB(word (_65C02::* addrMode)());  void TSX(word (_65C02::* addrMode)());	void TXA(word (_65C02::* addrMode)());	void TXS(word (_65C02::* addrMode)());
    void TYA(word (_65C02::* addrMode)());  void WAI(word (_65C02::* addrMode)());  void XXX(word (_65C02::* addrMode)());

    struct Opcode {
        void (_65C02::* instruction)(word (_65C02::* addrMode)(word*));
        word (_65C02::* addrMode)(word* data);
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
    INS_JMP_ABS = 0x4C, INS_JMP_IND = 0x6C,
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
