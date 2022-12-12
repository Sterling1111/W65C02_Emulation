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
    byte ZERO = 0x00; //This is meant to make the STZ Instruction work

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

    typedef const std::function<byte(byte)>& Operation;
    typedef word (W65C02::* AddressMode)(byte W65C02::*, Operation);
    typedef void (W65C02::* Instruction)(AddressMode);

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

    word immediate(__attribute__((unused)) byte W65C02::* Register = nullptr, Operation op = nullptr);

    word absoluteA(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word absoluteB(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word absoluteC(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word absoluteD(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word zeroPageA(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word zeroPageB(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word zeroPageC(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word accumulator(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word impliedA(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word impliedB(__attribute__((unused)) __attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word impliedC(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word zeroPageIndirectIndexed(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word zeroPageIndexedIndirect(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word zeroPageXA(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word zeroPageXB(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word zeroPageY(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word absoluteXA(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word absoluteXB(__attribute__((unused)) byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word absoluteY(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word relativeA(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word relativeB(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word absoluteIndirect(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word stackA(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word stackB(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word stackC(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word stackD(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word stackE(byte W65C02::* Register  = nullptr, Operation op = nullptr);
    word stackF(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word absoluteIndexedIndirect(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    word zeroPageIndirect(byte W65C02::* Register  = nullptr, Operation op = nullptr);

    void ADC(AddressMode);  void AND(AddressMode);	void ASL(AddressMode);	void BBR0(AddressMode);
    void BBR1(AddressMode); void BBR2(AddressMode); void BBR3(AddressMode); void BBR4(AddressMode);
    void BBR5(AddressMode); void BBR6(AddressMode); void BBR7(AddressMode); void BBS0(AddressMode);
    void BBS1(AddressMode); void BBS2(AddressMode); void BBS3(AddressMode); void BBS4(AddressMode);
    void BBS5(AddressMode); void BBS6(AddressMode); void BBS7(AddressMode); void BCC(AddressMode);
    void BCS(AddressMode);	void BEQ(AddressMode);  void BIT(AddressMode);  void BMI(AddressMode);
    void BNE(AddressMode);	void BPL(AddressMode);  void BRA(AddressMode);  void BRK(AddressMode);
    void BVC(AddressMode);  void BVS(AddressMode);  void CLC(AddressMode);	void CLD(AddressMode);
    void CLI(AddressMode);  void CLV(AddressMode);  void CMP(AddressMode);	void CPX(AddressMode);
    void CPY(AddressMode);  void DEC(AddressMode);  void DEX(AddressMode);	void DEY(AddressMode);
    void EOR(AddressMode);  void INC(AddressMode);  void INX(AddressMode);	void INY(AddressMode);
    void JMP(AddressMode);  void JSR(AddressMode);  void LDA(AddressMode);	void LDX(AddressMode);
    void LDY(AddressMode);  void LSR(AddressMode);  void NOP(AddressMode);	void ORA(AddressMode);
    void PHA(AddressMode);  void PHP(AddressMode);  void PHX(AddressMode);  void PHY(AddressMode);
    void PLA(AddressMode);	void PLP(AddressMode);  void PLX(AddressMode);  void PLY(AddressMode);
    void RMB0(AddressMode); void RMB1(AddressMode); void RMB2(AddressMode); void RMB3(AddressMode);
    void RMB4(AddressMode); void RMB5(AddressMode); void RMB6(AddressMode); void RMB7(AddressMode);
    void ROL(AddressMode);  void ROR(AddressMode);  void RTI(AddressMode);	void RTS(AddressMode);
    void SBC(AddressMode);  void SEC(AddressMode);  void SED(AddressMode);	void SEI(AddressMode);
    void SMB0(AddressMode); void SMB1(AddressMode); void SMB2(AddressMode); void SMB3(AddressMode);
    void SMB4(AddressMode); void SMB5(AddressMode); void SMB6(AddressMode); void SMB7(AddressMode);
    void STA(AddressMode);  void STP(AddressMode);  void STX(AddressMode);	void STY(AddressMode);
    void STZ(AddressMode);  void TAX(AddressMode);  void TAY(AddressMode);  void TRB(AddressMode);
    void TSB(AddressMode);  void TSX(AddressMode);  void TXA(AddressMode);	void TXS(AddressMode);
    void TYA(AddressMode);  void WAI(AddressMode);
    void XXX(AddressMode);

    struct Opcode {
        //void (W65C02::* Instruction)(word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&));
        //word (W65C02::* addrMode)(byte W65C02::*, const std::function<byte(byte)>&);
        Instruction instruction;
        AddressMode addressMode;
    };

    struct OpcodeString {
        std::string instructionString;
        std::string addressModeString;
    };

    std::vector<Opcode> opCodeMatrix;
    std::vector<OpcodeString> opCodeStringMatrix;

    Opcode opcode{};
    OpcodeString opcodeString{};

    //Instruction opcodes
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
