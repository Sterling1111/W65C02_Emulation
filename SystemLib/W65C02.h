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
    bool IRQB{}, NMIB{}, STOP{}, WAIT{};
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

    typedef const std::function<byte(byte)>& operation;
    typedef word (W65C02::* addressMode)(byte W65C02::*, operation);
    typedef void (W65C02::* instruction)(addressMode);

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

    word immediate(byte W65C02::* Register = nullptr, operation op = nullptr);

    word absoluteA(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word absoluteB(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word absoluteC(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word absoluteD(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word zeroPageA(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word zeroPageB(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word zeroPageC(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word accumulator(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word impliedA(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word impliedB(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word impliedC(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word zeroPageIndirectIndexed(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word zeroPageIndexedIndirect(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word zeroPageXA(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word zeroPageXB(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word zeroPageY(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word absoluteXA(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word absoluteXB(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word absoluteY(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word relativeA(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word relativeB(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word absoluteIndirect(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word stackA(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word stackB(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word stackC(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word stackD(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word stackE(byte W65C02::* Register  = nullptr, operation op = nullptr);
    word stackF(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word absoluteIndexedIndirect(byte W65C02::* Register  = nullptr, operation op = nullptr);

    word zeroPageIndirect(byte W65C02::* Register  = nullptr, operation op = nullptr);

    void ADC(addressMode);  void AND(addressMode);	void ASL(addressMode);	void BBR0(addressMode);
    void BBR1(addressMode); void BBR2(addressMode); void BBR3(addressMode); void BBR4(addressMode);
    void BBR5(addressMode); void BBR6(addressMode); void BBR7(addressMode); void BBS0(addressMode);
    void BBS1(addressMode); void BBS2(addressMode); void BBS3(addressMode); void BBS4(addressMode);
    void BBS5(addressMode); void BBS6(addressMode); void BBS7(addressMode); void BCC(addressMode);
    void BCS(addressMode);	void BEQ(addressMode);  void BIT(addressMode);  void BMI(addressMode);
    void BNE(addressMode);	void BPL(addressMode);  void BRA(addressMode);  void BRK(addressMode);
    void BVC(addressMode);  void BVS(addressMode);  void CLC(addressMode);	void CLD(addressMode);
    void CLI(addressMode);  void CLV(addressMode);  void CMP(addressMode);	void CPX(addressMode);
    void CPY(addressMode);  void DEC(addressMode);  void DEX(addressMode);	void DEY(addressMode);
    void EOR(addressMode);  void INC(addressMode);  void INX(addressMode);	void INY(addressMode);
    void JMP(addressMode);  void JSR(addressMode);  void LDA(addressMode);	void LDX(addressMode);
    void LDY(addressMode);  void LSR(addressMode);  void NOP(addressMode);	void ORA(addressMode);
    void PHA(addressMode);  void PHP(addressMode);  void PHX(addressMode);  void PHY(addressMode);
    void PLA(addressMode);	void PLP(addressMode);  void PLX(addressMode);  void PLY(addressMode);
    void RMB0(addressMode); void RMB1(addressMode); void RMB2(addressMode); void RMB3(addressMode);
    void RMB4(addressMode); void RMB5(addressMode); void RMB6(addressMode); void RMB7(addressMode);
    void ROL(addressMode);  void ROR(addressMode);  void RTI(addressMode);	void RTS(addressMode);
    void SBC(addressMode);  void SEC(addressMode);  void SED(addressMode);	void SEI(addressMode);
    void SMB0(addressMode); void SMB1(addressMode); void SMB2(addressMode); void SMB3(addressMode);
    void SMB4(addressMode); void SMB5(addressMode); void SMB6(addressMode); void SMB7(addressMode);
    void STA(addressMode);  void STP(addressMode);  void STX(addressMode);	void STY(addressMode);
    void STZ(addressMode);  void TAX(addressMode);  void TAY(addressMode);  void TRB(addressMode);
    void TSB(addressMode);  void TSX(addressMode);  void TXA(addressMode);	void TXS(addressMode);
    void TYA(addressMode);  void WAI(addressMode);
    void XXX(addressMode);

    struct Opcode {
        //void (W65C02::* instruction)(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
        //word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&);
        instruction instruction;
        addressMode addressMode;
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
