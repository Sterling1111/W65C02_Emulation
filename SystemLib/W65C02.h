#ifndef INC_65C02_EMULATION__65C02_H
#define INC_65C02_EMULATION__65C02_H

#include <bitset>
#include <functional>
#include "Cycles.h"
#include "Bus.h"
#include <vector>

#define SIGN_BIT_POS 7
#define CARRY_BIT_POS 8
#define OVERFLOW_BIT_POS 6
#define OVERFLOW_BIT_MASK 0x40
#define SIGN_BIT_MASK 0x80
#define MAX_BYTE 0XFF


class W65C02 {
public:
    word PC{};    //program counter
    byte SP{};    //stack pointer
    byte A{}, X{}, Y{};   //registers
    bool RESB{}, IRQB{}, NMIB{};
    //this will be set equal to PS when PS needs to be pushed onto stack
    byte PS_byte{};
    Bus* bus;
private:
    byte ZERO = 0x00; //This is meant to make the STZ instruction work

public:
    /*carry, zero, interrupt disable, decimal mode, break command,
     * unused-always 1, overflow, negative*/
    //enum StatusFlags {C, Z, I, D, B, U, V, N, numFlags};
    enum StatusFlags {N = 7, V = 6, U = 5, B = 4, D = 3, I = 2, Z = 1, C = 0, numFlags = 8};
    std::bitset<StatusFlags::numFlags> PS;

    Cycles cycles{};
    static dword cycleDuration;
    struct CyclesIncrementer {
        Cycles& cycles;
        explicit CyclesIncrementer(Cycles& cycles) : cycles{cycles} {};
        ~CyclesIncrementer() { ++cycles; }
    };

    explicit W65C02(double Mhz = 1);
    void connectBus(Bus* bus);
    void initializeOpcodeMatrix();
    void setCycleDuration(double Mhz);
    void reset(word pc=0xFFFE);
    void interruptRequest();
    void nonMaskableInterrupt();
    word readWord(word address);
    byte readByte(word address);
    byte fetchByte();
    word fetchWord();
    void writeWord(word data, word address);
    void writeByte(byte data, word address);
    void bitInstructionSetStatus(byte result, bool immediateMode);
    void loadRegister(byte& Register, byte value);
    void NZSetStatus(byte value);
    void NZCSetStatus(byte value);
    void NZVSetStatus(byte value);
    void execute(uint64_t numInstructionsToExecute = 1);
    void pushByteToStack(byte data);
    void pushWordToStack(word data);
    word SPToAddress() const;
    byte pullByteFromStack();
    byte readByteFromStack();

    word immediate(byte W65C02::* Register = nullptr, const std::function<byte(byte)>& op = nullptr);

    word absoluteA(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word absoluteB(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word absoluteC(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word absoluteD(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word zeroPageA(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word zeroPageB(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word zeroPageC(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word accumulator(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word impliedA(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word impliedB(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word impliedC(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word zeroPageIndirectIndexed(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word zeroPageIndexedIndirect(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word zeroPageXA(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word zeroPageXB(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word zeroPageY(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word absoluteXA(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word absoluteXB(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word absoluteY(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word relativeA(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word relativeB(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word absoluteIndirect(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word stackA(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word stackB(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word stackC(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word stackD(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word stackE(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);
    word stackF(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word absoluteIndexedIndirect(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    word zeroPageIndirect(byte W65C02::* Register  = nullptr, const std::function<byte(byte)>& op = nullptr);

    void ADC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void AND(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void ASL(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void BBR0(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void BBR1(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBR2(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBR3(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBR4(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void BBR5(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBR6(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBR7(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBS0(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void BBS1(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBS2(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBS3(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBS4(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void BBS5(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBS6(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BBS7(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void BCC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void BCS(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void BEQ(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void BIT(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void BMI(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void BNE(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void BPL(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void BRA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void BRK(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void BVC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void BVS(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void CLC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void CLD(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void CLI(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void CLV(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void CMP(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void CPX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void CPY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void DEC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void DEX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void DEY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void EOR(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void INC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void INX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void INY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void JMP(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void JSR(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void LDA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void LDX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void LDY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void LSR(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void NOP(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void ORA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void PHA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void PHP(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void PHX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void PHY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void PLA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void PLP(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void PLX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void PLY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void RMB0(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void RMB1(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void RMB2(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void RMB3(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void RMB4(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void RMB5(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void RMB6(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void RMB7(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void ROL(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void ROR(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void RTI(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void RTS(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void SBC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void SEC(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void SED(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void SEI(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void SMB0(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void SMB1(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void SMB2(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void SMB3(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void SMB4(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void SMB5(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void SMB6(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&)); void SMB7(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void STA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void STP(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void STX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void STY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void STZ(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void TAX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void TAY(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void TRB(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void TSB(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void TSX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void TXA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));	void TXS(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void TYA(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));  void WAI(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
    void XXX(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));

    struct Opcode {
        void (W65C02::* instruction)(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
        word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&);
    };

    std::vector<Opcode> opCodeMatrix;
    Opcode opcode{};

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
    INS_AND_IM = 0x29, INS_AND_ZP = 0x25, INS_AND_ZPX = 0x35, INS_AND_ABS = 0x2D, INS_AND_ABSX = 0x3D, INS_AND_ABSY = 0x39, INS_AND_XIND = 0x21, INS_AND_INDY = 0x31, INS_AND_ZPIND = 0x32,
            INS_ORA_IM = 0x09, INS_ORA_ZP = 0x05, INS_ORA_ZPX = 0x15, INS_ORA_ABS = 0x0D, INS_ORA_ABSX = 0x1D, INS_ORA_ABSY = 0x19, INS_ORA_XIND = 0x01, INS_ORA_INDY = 0x11, INS_ORA_ZPIND = 0x12,
            INS_EOR_IM = 0x49, INS_EOR_ZP = 0x45, INS_EOR_ZPX = 0x55, INS_EOR_ABS = 0x4D, INS_EOR_ABSX = 0x5D, INS_EOR_ABSY = 0x59, INS_EOR_XIND = 0x41, INS_EOR_INDY = 0x51, INS_EOR_ZPIND = 0x52,
            INS_BIT_IM = 0x89, INS_BIT_ZP = 0x24, INS_BIT_ABS = 0x2C, INS_BIT_ZPX = 0x34, INS_BIT_ABSX = 0x3C,
    //JUmps and Calls
    INS_RTS = 0x60,
            INS_JMP_ABS = 0x4C, INS_JMP_IND = 0x6C, INS_JMP_ABS_IND = 0x7C,
            INS_JSR = 0x20,
    //Stack Operations
    INS_PHA_IMP = 0x48,
            INS_PHP_IMP = 0x08,
            INS_PHX_IMP = 0xDA,
            INS_PHY_IMP = 0x5A,
            INS_PLA_IMP = 0x68,
            INS_PLP_IMP = 0x28,
            INS_PLX_IMP = 0xFA,
            INS_PLY_IMP = 0x7A,
            INS_TSX_IMP = 0xBA,
            INS_TXS_IMP = 0x9A,
    //Register Transfers
    INS_TXA_IMP = 0x8A,
            INS_TAX_IMP = 0xAA,
            INS_TAY_IMP = 0xA8,
            INS_TYA_IMP = 0x98,
    //shift instructions
    INS_ASL_ABSX = 0x1E,
            INS_ROL_ABSX = 0x3E,
            INS_LSR_ABSX = 0x5E,
            INS_ROR_ABSX = 0x7E;
};

#endif //INC_65C02_EMULATION__65C02_H
