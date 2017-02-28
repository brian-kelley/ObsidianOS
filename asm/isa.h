#ifndef ISA_H
#define ISA_H

#define INVALID_REG 8

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned dword;

//All possible type/size combinations for operands
//Packed into 4 bits, can only have 16
//NOTE: Size must be encoded elsewhere!
//Also, MEM_ABSOLUTE looks the same as MEMORY, except that there is no base/index regs
//IMMEDIATE looks the same as CODE_RELATIVE (either label or number)
//so use context (jmp/jXX/call default to CODE_REL, all other mnemonics default to IMMEDIATE)
enum OperandType
{
  NO_OPERAND,
  REG_MEM_8,
  REG_MEM,        //16 or 32 bits depending on mode
  REG_8,
  REG,            //16 or 32 bits
  IMM_8,          //8-bit imm
  IMM,            //16 or 32 bits
  FPU_REG,
  SEGMENT_REG,
  CONTROL_REG,
  REG_DX,         //note: special regs also match REG and REG_MEM (for the correct size)
  REG_CL,         //in 32 bit mode, REG_AX means ax or eax, depending on size override
  REG_AL,
  REG_AX,
  MEM_ABSOLUTE,   //absolute mem (for mov only, called "moffs" in manual)
};

//Values for mod field in mod/reg/rm
enum MOD_GROUPS
{
  MOD_MEM     = 0,
  MOD_MEM_D8  = 1,
  MOD_MEM_D32 = 2,
  MOD_REG     = 3
};

enum RegType
{
  GP8, 
  GP16,
  GP32,
  X87,
  SEGMENT,
  CONTROL
};

enum RegID
{
  ID_AX,
  ID_CX,
  ID_DX,
  ID_BX,
  ID_SP,
  ID_BP,
  ID_SI,
  ID_DI
};

typedef struct
{
  char mnemonic[8];     //is null-terminated so max length is 7
  short opcodeOffset;   //index of first opcode in opcode table
  short numOpcodes;     //number of unique opcodes for this mnemonic
} Mnemonic;

//for minimum executable size, bit pack as many opcode attributes as possible
enum OpcodeFlags                    // i386 manual notation:
{
  HAS_DIGIT = 1 << 0,               // "/digit", if yes, digit is in Opcode.digit, OR it with main opcode byte
  REG_IN_OPCODE = 1 << 1,           // "+rb", "+rw", "+rd": register code in low 3 bits of opcode (NOT same as /r, meaning 2nd reg in modRM)
  HAS_CODE_OFFSET = 1 << 2,         // "+cb", "+cw", "+cd" - uses imm value as address relative to next instruction
  HAS_EXPANSION_PREFIX = 1 << 3,    // whether 0x0F (opcode expansion prefix) is before the single opcode byte
  DIGIT0 = 1 << 4,                  // 3-bit digit value (if HAS_DIGIT)
  DIGIT1 = 1 << 5,
  DIGIT2 = 1 << 6,
  HAS_MODRM = 1 << 7
};

typedef struct
{
  byte opcode;      //main opcode byte (prefix with 0x0F if flags & HAS_EXPANSION_PREFIX
  byte opTypes;     //low 4 bits are first operand, high 4 are second operand
  byte flags;      //OpcodeFlags1 bit field
} Opcode;

#define NUM_MNEMONICS 13
Mnemonic mneTable[] =
{
  {"aaa", 0, 1},
  {"aas", 1, 1},
  {"adc", 2, 9},
  {"add", 11, 9},
  {"and", 20, 9},
  {"arpl", 29, 1},
  {"bsf", 30, 1},
  {"bsr", 31, 1},
  {"bt", 32, 2},
  {"btc", 34, 2},
  {"btr", 36, 2},
  {"bts", 38, 2},
  {"call", 40, 2}
};

#define NUM_OPCODES 42
Opcode opcodeTable[] =
{
  //aaa
  {0x37, NO_OPERAND | NO_OPERAND << 4, 0},
  //aas
  {0x3F, NO_OPERAND | NO_OPERAND << 4, 0},
  //adc
  {0x14, REG_AL | IMM_8 << 4, 0},
  {0x15, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_DIGIT | HAS_MODRM | 2 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_DIGIT | HAS_MODRM | 2 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_DIGIT | HAS_MODRM | 2 << 4},
  {0x10, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x11, REG_MEM | REG << 4, HAS_MODRM},
  {0x12, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x13, REG | REG_MEM << 4, HAS_MODRM},
  //add
  {0x04, REG_AL | IMM_8 << 4, 0},
  {0x05, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_DIGIT | HAS_MODRM | 0 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_DIGIT | HAS_MODRM | 0 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_DIGIT | HAS_MODRM | 0 << 4},
  {0x00, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x01, REG_MEM | REG << 4, HAS_MODRM},
  {0x02, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x03, REG | REG_MEM << 4, HAS_MODRM},
  //and
  {0x24, REG_AL | IMM_8 << 4, 0},
  {0x25, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_DIGIT | HAS_MODRM | 4 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_DIGIT | HAS_MODRM | 4 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_DIGIT | HAS_MODRM | 4 << 4},
  {0x20, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x21, REG_MEM | REG << 4, HAS_MODRM},
  {0x22, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x23, REG | REG_MEM << 4, HAS_MODRM},
  //arpl
  {0x63, REG_MEM | REG << 4, HAS_MODRM},
  //bsf
  {0xBC, REG | REG_MEM << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //bsr
  {0xBD, REG | REG_MEM << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //bt
  {0xA3, REG_MEM | REG << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  {0xBA, REG_MEM | IMM_8, HAS_MODRM | HAS_EXPANSION_PREFIX | HAS_DIGIT | 4 << 4},
  //btc
  {0xBB, REG_MEM | REG << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  {0xBA, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_EXPANSION_PREFIX | HAS_DIGIT | 7 << 4},
  //btr
  {0xB3, REG_MEM | REG << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  {0xBA, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_EXPANSION_PREFIX | HAS_DIGIT | 6 << 4},
  //bts
  {0xAB, REG_MEM | REG << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  {0xBA, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_EXPANSION_PREFIX | HAS_DIGIT | 5 << 4},
  //call
  {0xE8, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0xFF, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 2 << 4}
};

#endif

