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

#define NUM_MNEMONICS (sizeof(mneTable) / sizeof(Mnemonic))
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
  {"call", 40, 2},
  {"cbw", 42, 1},
  {"clc", 43, 1},
  {"cld", 44, 1},
  {"cli", 45, 1},
  {"clts", 46, 1},
  {"cmc", 47, 1},
  {"cmp", 48, 9},
  {"cmpsb", 57, 1},
  {"cmpsd", 58, 1},
  {"cdq", 59, 1},
  {"daa", 60, 1},
  {"das", 61, 1},
  {"dec", 62, 2},
  {"div", 64, 2},
  {"hlt", 66, 1},
  {"idiv", 67, 2},
  {"imul", 69, 5},
  {"in", 74, 4},
  {"inc", 78, 3},
  {"insb", 81, 1},
  {"insd", 82, 1},
  {"int", 83, 1},
  {"into", 84, 1},
  {"iret", 85, 1},
  {"ja", 86, 2},
  {"jnbe", 86, 2},
  {"jae", 88, 2},
  {"jnb", 88, 2},
  {"jnc", 88, 2},
  {"jb", 90, 2},
  {"jc", 90, 2},
  {"jnae", 90, 2},
  {"jbe", 92, 2},
  {"jna", 92, 2},
  {"jcxz", 94, 1},
  {"je", 95, 2},
  {"jz", 95, 2},
  {"jg", 97, 2},
  {"jnle", 97, 2},
  {"jge", 99, 2},
  {"jnl", 99, 2},
  {"jl", 101, 2},
  {"jnge", 101, 2},
  {"jle", 103, 2},
  {"jng", 103, 2},
  {"jne", 105, 2},
  {"jnz", 105, 2},
  {"jno", 107, 2},
  {"jnp", 109, 2},
  {"jpo", 109, 2},
  {"jns", 111, 2},
  {"jo", 113, 2},
  {"jp", 115, 2},
  {"jpe", 115, 2},
  {"js", 117, 2},
  {"jmp", 119, 3},
  {"lahf", 122, 1},
  {"lar", 123, 1},
  {"lea", 124, 1},
  {"lgdt", 125, 1},
  {"lidt", 126, 1},
  {"lldt", 127, 1},
  {"lmsw", 128, 1},
  {"lodsb", 129, 1},
  {"lodsd", 130, 1},
  {"loop", 131, 1},
  {"loope", 132, 1},
  {"loopz", 132, 1},
  {"loopnz", 133, 1},
  {"loopne", 133, 1},
  {"lsl", 134, 1},
  {"ltr", 135, 1},
  {"mov", 136, 14},
  {"movsb", 150, 1},
  {"movsd", 151, 1},
  {"movsx", 152, 1},
  {"movzx", 153, 1},
  {"mul", 154, 2},
  {"neg", 156, 2},
  {"nop", 158, 1},
  {"not", 159, 2},
  {"or", 161, 9},
  {"out", 170, 4},
  {"outsb", 174, 1},
  {"outsd", 175, 1},
  {"pop", 176, 2},
  {"popds", 178, 1},
  {"popes", 179, 1},
  {"popss", 180, 1},
  {"popfs", 181, 1},
  {"popgs", 182, 1},
  {"popa", 183, 1},
  {"popf", 184, 1},
  {"push", 185, 4},
  {"pushcs", 189, 1},
  {"pushss", 190, 1},
  {"pushds", 191, 1},
  {"pushes", 192, 1},
  {"pushfs", 193, 1},
  {"pushgs", 194, 1},
  {"pusha", 195, 1},
  {"pushf", 196, 1},
  {"rcl", 197, 6},
  {"rcr", 203, 6},
  {"rol", 209, 6},
  {"ror", 215, 6},
  {"ret", 221, 1},
  {"sahf", 222, 1},
  {"sal", 223, 6},
  {"shl", 223, 6},
  {"sar", 229, 6},
  {"shr", 235, 6},
  {"sbb", 241, 9},
  {"scasb", 250, 1},
  {"scasd", 251, 1},
  {"seta", 252, 1},
  {"setnbe", 252, 1},
  {"setae", 253, 1},
  {"setnb", 253, 1},
  {"setnc", 253, 1},
  {"setb", 254, 1},
  {"setc", 254, 1},
  {"setnae", 254, 1},
  {"setbe", 255, 1},
  {"setna", 255, 1},
  {"sete", 256, 1},
  {"setz", 256, 1},
  {"setg", 257, 1},
  {"setnle", 257, 1},
  {"setge", 258, 1},
  {"setnl", 258, 1},
  {"setl", 259, 1},
  {"setnge", 259, 1},
  {"setle", 260, 1},
  {"setng", 260, 1},
  {"setne", 261, 1},
  {"setnz", 261, 1},
  {"setno", 262, 1},
  {"setns", 263, 1},
  {"setp", 264, 1},
  {"setpe", 264, 1},
  {"setpo", 265, 1},
  {"setnp", 265, 1},
  {"sgdt", 266, 1},
  {"sidt", 267, 1},
  {"sldt", 268, 1},
  {"smsw", 269, 1},
  {"stc", 270, 1},
  {"std", 271, 1},
  {"sti", 272, 1},
  {"stosb", 273, 1},
  {"stosd", 274, 1},
  {"str", 275, 1},
  {"sub", 276, 9},
  {"test", 285, 6},
  {"verr", 291, 1},
  {"verw", 292, 1},
  {"wait", 293, 1},
  {"xchg", 294, 4},
  {"xlatb", 298, 1},
  {"xor", 299, 9}
};


#define NUM_OPCODES (sizeof(opcodeTable) / sizeof(Opcode))
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
  {0xFF, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  //cbw
  {0x98, NO_OPERAND | NO_OPERAND << 4, 0},
  //clc
  {0xF8, NO_OPERAND | NO_OPERAND << 4, 0},
  //cld
  {0xFC, NO_OPERAND | NO_OPERAND << 4, 0},
  //cli
  {0xFA, NO_OPERAND | NO_OPERAND << 4, 0},
  //clts
  {0x06, NO_OPERAND | NO_OPERAND << 4, HAS_EXPANSION_PREFIX},
  //cmc
  {0xF5, NO_OPERAND | NO_OPERAND << 4, 0},
  //cmp
  {0x3C, REG_AL | IMM_8 << 4, 0},
  {0x3D, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0x38, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x39, REG_MEM | REG << 4, HAS_MODRM},
  {0x3A, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x3B, REG | REG_MEM << 4, HAS_MODRM},
  //cmpsb
  {0xA6, NO_OPERAND | NO_OPERAND << 4, 0},
  //cmpsd
  {0xA7, NO_OPERAND | NO_OPERAND << 4, 0},
  //cdq
  {0x99, NO_OPERAND | NO_OPERAND << 4, 0},
  //daa
  {0x27, NO_OPERAND | NO_OPERAND << 4, 0},
  //das
  {0x2F, NO_OPERAND | NO_OPERAND << 4, 0},
  //dec
  {0xFE, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0xFF, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  //div
  {0xF6, REG_AL | REG_MEM_8 << 4, HAS_MODRM | HAS_DIGIT | 6 << 4},
  {0xF7, REG_AX | REG_MEM << 4, HAS_MODRM | HAS_DIGIT | 6 << 4},
  //hlt
  {0xF4, NO_OPERAND | NO_OPERAND << 4, 0},
  //idiv
  {0xF6, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0xF7, REG_AX | REG_MEM << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  //imul
  {0xF6, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0xF7, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0xAF, REG | REG_MEM << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  {0x6B, REG | IMM_8 << 4, HAS_MODRM},
  {0x69, REG | IMM << 4, HAS_MODRM},
  //in
  {0xE4, REG_AL | IMM_8 << 4, 0},
  {0xE5, REG_AX | IMM_8 << 4, 0},
  {0xEC, REG_AL | REG_DX << 4, 0},
  {0xED, REG_AX | REG_DX << 4, 0},
  //inc
  {0xFE, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0xFF, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 6 << 4},
  {0x40, REG | NO_OPERAND << 4, REG_IN_OPCODE},
  //insb
  {0x6C, NO_OPERAND | NO_OPERAND << 4, 0},
  //insd
  {0x6D, NO_OPERAND | NO_OPERAND << 4, 0},
  //int
  {0xCD, IMM_8 | NO_OPERAND << 4, 0},
  //into
  {0xCE, NO_OPERAND | NO_OPERAND << 4, 0},
  //iret
  {0xCF, NO_OPERAND | NO_OPERAND << 4, 0},
  //ja, jnbe
  {0x77, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x87, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jae, jnb, jnc
  {0x73, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x83, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jb, jc, jnae
  {0x72, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x82, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jbe, jna
  {0x76, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x86, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jcxz
  {0xE3, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  //je, jz
  {0x74, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x84, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jg, jnle
  {0x7F, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x8F, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jge, jnl
  {0x7D, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x8D, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jl, jnge
  {0x7C, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x8C, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jle, jng
  {0x7E, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x8E, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jne, jnz
  {0x75, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x85, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jno
  {0x71, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x81, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jnp, jpo
  {0x7B, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x8B, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jns
  {0x79, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x89, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jo
  {0x70, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x80, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jp, jpe
  {0x7A, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x8A, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //js
  {0x78, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0x88, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET | HAS_EXPANSION_PREFIX},
  //jmp
  {0xEB, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0xE9, IMM | NO_OPERAND << 4, HAS_CODE_OFFSET},
  {0xFF, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  //lahf
  {0x9F, NO_OPERAND | NO_OPERAND << 4, 0},
  //lar
  {0x02, REG | REG_MEM << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //lea
  {0x8D, REG | REG_MEM << 4, HAS_MODRM},
  //lgdt
  {0x01, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 2 << 4},
  //lidt
  {0x01, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 3 << 4},
  //lldt
  {0x00, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 2 << 4},
  //lmsw
  {0x01, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 6 << 4},
  //lodsb
  {0xAC, NO_OPERAND | NO_OPERAND << 4, 0},
  //lodsd
  {0xAD, NO_OPERAND | NO_OPERAND << 4, 0},
  //loop
  {0xE2, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  //loope, loopz
  {0xE1, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  //loopne, loopnz
  {0xE0, IMM_8 | NO_OPERAND << 4, HAS_CODE_OFFSET},
  //lsl
  {0x03, REG | REG_MEM << 4, HAS_EXPANSION_PREFIX | HAS_MODRM},
  //ltr
  {0x00, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 3 << 4},
  //mov
  {0x88, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x89, REG_MEM | REG << 4, HAS_MODRM},
  {0x8A, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x8B, REG | REG_MEM << 4, HAS_MODRM},
  {0x8C, REG_MEM | SEGMENT_REG << 4, HAS_MODRM},
  {0x8D, SEGMENT_REG | REG_MEM << 4, HAS_MODRM},
  {0xA0, REG_AL | MEM_ABSOLUTE << 4, 0},
  {0xA1, REG_AX | MEM_ABSOLUTE << 4, 0},
  {0xB0, REG_8 | IMM_8 << 4, REG_IN_OPCODE},
  {0xB8, REG | IMM << 4, REG_IN_OPCODE},
  {0xC6, REG_MEM_8 | IMM_8 << 4, HAS_MODRM},
  {0xC7, REG_MEM | IMM << 4, HAS_MODRM},
  {0x20, REG | CONTROL_REG << 4, HAS_EXPANSION_PREFIX},
  {0x22, CONTROL_REG | REG << 4, HAS_EXPANSION_PREFIX},
  //movsb
  {0xA4, NO_OPERAND | NO_OPERAND << 4, 0},
  //movsd
  {0xA5, NO_OPERAND | NO_OPERAND << 4, 0},
  //movsx
  {0xBE, REG | REG_MEM_8 << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //movzx
  {0xB6, REG | REG_MEM_8 << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //mul
  {0xF6, REG_AL | REG_MEM_8 << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  {0xF7, REG_AX | REG_MEM << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  //neg
  {0xF6, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0xF7, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  //nop
  {0x90, NO_OPERAND | NO_OPERAND << 4, 0},
  //not
  {0xF6, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  {0xF7, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  //or
  {0x0C, REG_AL | IMM_8 << 4, 0},
  {0x0D, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0x08, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x09, REG_MEM | REG << 4, HAS_MODRM},
  {0x0A, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x0B, REG | REG_MEM << 4, HAS_MODRM},
  //out
  {0xE6, IMM_8 | REG_AL << 4, 0},
  {0xE7, IMM_8 | REG_AX << 4, 0},
  {0xEE, REG_DX | REG_AL << 4, 0},
  {0xEF, REG_DX | REG_AX << 4, 0},
  //outsb
  {0x6E, NO_OPERAND | NO_OPERAND << 4, 0},
  //outsd
  {0x6F, NO_OPERAND | NO_OPERAND << 4, 0},
  //pop
  {0x8F, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0x58, REG | NO_OPERAND << 4, REG_IN_OPCODE},
  //popds
  {0x1F, NO_OPERAND | NO_OPERAND << 4, 0},
  //popes
  {0x07, NO_OPERAND | NO_OPERAND << 4, 0},
  //popss
  {0x17, NO_OPERAND | NO_OPERAND << 4, 0},
  //popfs
  {0xA1, NO_OPERAND | NO_OPERAND << 4, HAS_EXPANSION_PREFIX},
  //popgs
  {0xA9, NO_OPERAND | NO_OPERAND << 4, HAS_EXPANSION_PREFIX},
  //popa
  {0x61, NO_OPERAND | NO_OPERAND << 4, 0},
  //popf
  {0x9D, NO_OPERAND | NO_OPERAND << 4, 0},
  //push
  {0xFF, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 6 << 4},
  {0x50, REG | NO_OPERAND << 4, REG_IN_OPCODE},
  {0x6A, IMM_8 | NO_OPERAND << 4, 0},
  {0x68, IMM | NO_OPERAND << 4, 0},
  //pushcs
  {0x0E, NO_OPERAND | NO_OPERAND << 4, 0},
  //pushss
  {0x16, NO_OPERAND | NO_OPERAND << 4, 0},
  //pushds
  {0x1E, NO_OPERAND | NO_OPERAND << 4, 0},
  //pushes
  {0x06, NO_OPERAND | NO_OPERAND << 4, 0},
  //pushfs
  {0xA0, NO_OPERAND | NO_OPERAND << 4, HAS_EXPANSION_PREFIX},
  //pushgs
  {0xA8, NO_OPERAND | NO_OPERAND << 4, HAS_EXPANSION_PREFIX},
  //pusha
  {0x60, NO_OPERAND | NO_OPERAND << 4, 0},
  //pushf
  {0x9C, NO_OPERAND | NO_OPERAND << 4, 0},
  //rcl
  {0xD0, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  {0xD2, REG_MEM_8 | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  {0xC0, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  {0xD1, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  {0xD3, REG_MEM | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  {0xC1, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 2 << 4},
  //rcr
  {0xD0, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0xD2, REG_MEM_8 | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0xC0, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0xD1, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0xD3, REG_MEM | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0xC1, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  //rol
  {0xD0, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0xD2, REG_MEM_8 | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0xC0, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0xD1, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0xD3, REG_MEM | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0xC1, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  //ror
  {0xD0, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0xD2, REG_MEM_8 | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0xC0, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0xD1, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0xD3, REG_MEM | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  {0xC1, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 1 << 4},
  //ret
  {0xC3, NO_OPERAND | NO_OPERAND << 4, 0},
  //sahf
  {0x9E, NO_OPERAND | NO_OPERAND << 4, 0},
  //sal, shl
  {0xD0, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  {0xD2, REG_MEM_8 | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  {0xC0, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  {0xD1, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  {0xD3, REG_MEM | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  {0xC1, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 4 << 4},
  //sar
  {0xD0, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0xD2, REG_MEM_8 | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0xC0, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0xD1, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0xD3, REG_MEM | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  {0xC1, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 7 << 4},
  //shr
  {0xD0, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0xD2, REG_MEM_8 | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0xC0, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0xD1, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0xD3, REG_MEM | REG_CL << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0xC1, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  //sbb
  {0x1C, REG_AL | IMM_8 << 4, 0},
  {0x1D, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 3 << 4},
  {0x18, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x19, REG_MEM | REG << 4, HAS_MODRM},
  {0x1A, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x1B, REG | REG_MEM << 4, HAS_MODRM},
  //scasb
  {0xAE, NO_OPERAND | NO_OPERAND << 4, 0},
  //scasd
  {0xAF, NO_OPERAND | NO_OPERAND << 4, 0},
  //seta, setnbe
  {0x97, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setae, setnb, setnc
  {0x93, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setb, setc, setnae
  {0x92, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setbe, setna
  {0x96, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //sete, setz
  {0x94, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setg, setnle
  {0x9F, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setge, setnl
  {0x9D, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setl, setnge
  {0x9C, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setle, setng
  {0x9E, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setne, setnz
  {0x95, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setno
  {0x91, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setns
  {0x99, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setp, setpe
  {0x9A, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //setpo, setnp
  {0x9B, REG_MEM_8 | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX},
  //sgdt
  {0x01, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 0 << 4},
  //sidt
  {0x01, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 1 << 4},
  //sldt
  {0x00, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 0 << 4},
  //smsw
  {0x01, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 4 << 4},
  //stc
  {0xF9, NO_OPERAND | NO_OPERAND << 4, 0},
  //std
  {0xFD, NO_OPERAND | NO_OPERAND << 4, 0},
  //sti
  {0xFB, NO_OPERAND | NO_OPERAND << 4, 0},
  //stosb
  {0xAA, NO_OPERAND | NO_OPERAND << 4, 0},
  //stosd
  {0xAB, NO_OPERAND | NO_OPERAND << 4, 0},
  //str
  {0x00, REG_MEM | NO_OPERAND << 4, HAS_MODRM | HAS_EXPANSION_PREFIX | HAS_DIGIT | 1 << 4},
  //sub
  {0x2C, REG_AL | IMM_8 << 4, 0},
  {0x2D, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 5 << 4},
  {0x28, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x29, REG_MEM | REG << 4, HAS_MODRM},
  {0x2A, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x2B, REG | REG_MEM << 4, HAS_MODRM},
  //test
  {0xA8, REG_AL | IMM_8 << 4, 0},
  {0xA9, REG_AX | IMM << 4, 0},
  {0xF6, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0xF7, REG_MEM | IMM << 4, HAS_MODRM | HAS_DIGIT | 0 << 4},
  {0x84, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x85, REG_MEM | REG << 4, HAS_MODRM},
  //verr
  {0x00, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 4 << 4},
  //verw
  {0x00, REG_MEM | NO_OPERAND << 4, HAS_EXPANSION_PREFIX | HAS_MODRM | HAS_DIGIT | 5 << 4},
  //wait
  {0x9B, NO_OPERAND | NO_OPERAND << 4, 0},
  //xchg
  {0x90, REG_AX | REG << 4, REG_IN_OPCODE},
  {0x86, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x87, REG_MEM | REG << 4, HAS_MODRM},
  {0x87, REG | REG_MEM << 4, HAS_MODRM},
  //xlatb
  {0xD7, NO_OPERAND | NO_OPERAND << 4, 0},
  //xor
  {0x34, REG_AL | IMM_8 << 4, 0},
  {0x35, REG_AX | IMM << 4, 0},
  {0x80, REG_MEM_8 | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 6 << 4},
  {0x81, REG_MEM | IMM << 4, HAS_MODRM | HAS_DIGIT | 6 << 4},
  {0x83, REG_MEM | IMM_8 << 4, HAS_MODRM | HAS_DIGIT | 6 << 4},
  {0x30, REG_MEM_8 | REG_8 << 4, HAS_MODRM},
  {0x31, REG_MEM | REG << 4, HAS_MODRM},
  {0x32, REG_8 | REG_MEM_8 << 4, HAS_MODRM},
  {0x33, REG | REG_MEM << 4, HAS_MODRM}
};

#endif

