#ifndef ASM_H
#define ASM_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "stdbool.h"

#include "isa.h"

#define OUT

typedef struct 
{
  char* name;   //allocated once when label is first defined or used
  int* refs;    //expanded as needed to store all mentions throughout file
  int nrefs;    //actual number of references (uses) of this label
  int ncap;     //allocated capacity
  int loc;      //address of label, or -1 if not yet defined
  int l;        //index of left subtree, or -1
  int r;        //right subtree, or -1
} LabelNode;

//Binary search tree for labels
typedef struct
{
  LabelNode* nodes;
  int nnodes;
  int ncap;
} LabelTree;

typedef struct
{
  char* dispLabel;
  char* immLabel;
  int op1Type;          //an OperandType enum value
  int op2Type;
  int reg1;             //1st operand reg ID, or INVALID_REG
  int reg2;             //2nd operand reg ID
  int baseReg;
  int indexReg;
  int scale;
  int disp;
  int imm;         //immediate value (always sign-extended if underlying is signed)
  bool hasMem;
  bool sizeOverride;    //whether operation is on 16-bit regs in 32-bit mode
} OperandSet;

enum FormatMode
{
  FLAT,
  ELF
};

enum BitsMode
{
  BITS_16,
  BITS_32
};

//Values for mod field in mod/reg/rm
enum MOD_GROUPS
{
  MOD_MEM     = 0,
  MOD_MEM_D8  = 1,
  MOD_MEM_D32 = 2,
  MOD_REG     = 3
};

//Opcode matching utilities
//Operand parsing/matching functions

bool hasDigit(Opcode* opc);
void getOpTypes(Opcode* opc, OUT int* op1, OUT int* op2);
int getDigit(Opcode* opc);

OperandSet getEmptyOperandSet();                            //get initialized OperandSet
OperandSet parseOperands();                                 //from end of mnemonic, parse operands into OperandSet
bool opTypesEquivalent(int expected, int parsed);           //whether "parsed" OpType can be used in place of "expected"
bool matchOperands(Opcode* opc, OperandSet* operands);      //return true if given operands match opcode
int getInstructionSize(Opcode* opc, OperandSet* operands);  //get number of bytes to encode the instruction

//Binary search tree implementation for labels
LabelTree initTree();
void destroyTree(LabelTree* lt);
LabelNode* treeSearch(LabelTree* lt, char* name);
LabelNode* treeInsert(LabelTree* lt, char* name);
//look up or create labelNode in correct tree (name doesn't have to be \0-terminated)
LabelNode* insertLabel(char* name);
void labelAddReference(LabelNode* ln);

bool validLabelFirstChar(char c);
bool validLabelChar(char c);

//get the 3-bit register ID (can be GP, FPU, segment or control)
int getRegID(char* name, OUT int* regType);

//Input/output stream functions (can pass signed versions)
void writeData(void* ptr, int size);
void writeChar(char c);
void writeByte(byte b);

//Size validation utilities - prepare possibly signed values for serialization
//after this validation, can pun value's bytes for writing
//if val could fit in signed OR unsigned type (imm, db/dw/dd)
bool fitsU8(int val);
bool fitsU16(int val);

//if val could fit in signed type only (disp, rel)
bool fitsI8(int val);
bool fitsI16(int val);

void eatWhitespace();
void eatLineEnd();
//read in an integer literal
int parseInt();
//parse a line
void parseLine();
void parseDB();
void parseDW();
void parseDD();
//parse a memory expression
void parseMem(OUT int* base, OUT int* index, OUT int* scale, OUT int* disp, OUT char** dispLabel);
//costruct modR/M byte (16-bit mode)
byte getModRM(Opcode* opc, OperandSet* os);
//construct modR/M byte and SIB (32-bit mode)
void getModSIB(Opcode* opc, OperandSet* os, OUT int* modrm, OUT int* sib);
//validate and rearrange base/index if necessary, e.g. [esp] or [ebp]
void arrangeMemRegs(int mod, int* base, int* index, int* scale);
//parse a memory instruction
void parseInstruction();
//parse whole input stream
void parse();
//resolve all memory 
//call at each global label for locals, and at end of file for globals
void resolveLabels(bool local);
//print "Error on line <lineno>: <errString>" and exit
void err(const char* errString);
//print "Internal error on line <lineno>" and exit, if assembler is correct, impossible to get here
void errInternal(int asmLineNo);
#endif

