#include "asm.h"
#include "isa.h"

//Global labels
LabelTree globalLabels;

//Local labels: names start with '.'
//labels are resolved and then tree is cleared every time a global label is encountered
LabelTree localLabels;  

char* code;
size_t iter;
size_t len;
size_t location;  //current location in output (in bytes from start, can be changed with org)
size_t lineno;
FILE* output;
int formatMode;     //FLAT or ELF
int bitsMode;       //BITS_16 or BITS_32

LabelTree initTree()
{
  LabelTree ltree;
  ltree.nodes = malloc(10 * sizeof(LabelNode));
  ltree.nnodes = 0;
  ltree.ncap = 10;
  return ltree;
}

//free every allocation associated with tree
void destroyTree(LabelTree* lt)
{
  for(int i = 0; i < lt->nnodes; i++)
  {
    free(lt->nodes[i].name);
    free(lt->nodes[i].refs);
  }
  free(lt->nodes);
}

LabelNode* treeSearch(LabelTree* lt, char* name)
{
  int search = 0;
  while(true)
  {
    LabelNode* sn = &lt->nodes[search];
    int cmp = strcmp(name, sn->name);
    if(cmp == 0)
    {
      //label already exists
      //note: caller should check before for this
      return &lt->nodes[search];
    }
    else if(cmp < 0)
    {
      //use left subtree
      if(sn->l == -1)
        return NULL;
      else
        search = sn->l;
    }
    else if(cmp > 0)
    {
      //use right subtree
      if(sn->r == -1)
        return NULL;
      else
        search = sn->r;
    }
  }
}

LabelNode* treeInsert(LabelTree* lt, char* name)
{
  //no matter what, need space for an insertion 
  if(lt->nnodes == lt->ncap)
  {
    puts("***** Reallocating lt nodes!");
    //multiply capacity by 1.5
    int newCap = lt->ncap + (lt->ncap >> 1);
    lt->nodes = realloc(lt->nodes, newCap * sizeof(LabelNode));
    lt->ncap = newCap;
  }
  //initialize the node
  LabelNode n;
  n.name = name;
  n.refs = malloc(10 * sizeof(int));
  n.nrefs = 0;
  n.ncap = 10;
  n.loc = -1;
  //new node has no children
  n.l = -1;
  n.r = -1;
  //if tree is empty, create first node
  if(lt->nnodes != 0)
  {
    //search tree for node (get nearest name)
    int search = 0;
    while(true)
    {
      LabelNode* sn = &lt->nodes[search];
      int cmp = strcmp(name, sn->name);
      if(cmp == 0)
      {
        //label already exists
        //don't create a new node
        free(n.refs);
        return sn;
      }
      else if(cmp < 0)
      {
        //use left subtree
        if(sn->l == -1)
        {
          sn->l = lt->nnodes;
          break;
        }
        else
          search = sn->l;
      }
      else if(cmp > 0)
      {
        //use right subtree
        if(sn->r == -1)
        {
          sn->r = lt->nnodes;
          break;
        }
        else
          search = sn->r;
      }
    }
    //add node at end of node list
  }
  lt->nodes[lt->nnodes] = n;
  return lt->nodes + lt->nnodes++;
}

bool validLabelFirstChar(char c)
{
  return isalpha(c) || (c && strchr("._?", c));
}

bool validLabelChar(char c)
{
  return isalpha(c) || isdigit(c) || (c && strchr("._?$#@-", c));
}

//note: name is not null-terminated
//name is taken as longest sequence of allowed characters
//this means error checking must be done when labels are actually resolved
LabelNode* insertLabel(char* name)
{
  int labelLen = 0;
  if(!validLabelFirstChar(*name))
  {
    err("invalid label first character");
  }
  bool local = *name == '.';
  labelLen++;
  while(validLabelChar(name[labelLen]))
  {
    labelLen++;
    if(labelLen >= 64)
    {
      err("label must be shorter than 64 characters");
    }
  }
  //note: label not leaked, ownership transferred to localLabels or globalLabels
  char* label = malloc(1 + labelLen);
  memcpy(label, name, labelLen);
  label[labelLen] = 0;
  if(local)
    return treeInsert(&localLabels, label);
  else
    return treeInsert(&globalLabels, label);
}

void labelAddReference(LabelNode* ln)
{
  if(ln->nrefs == ln->ncap)
  {
    int newCap = ln->ncap + (ln->ncap >> 1);
    ln->refs = realloc(ln->refs, newCap * sizeof(int));
    ln->ncap = newCap;
  }
  ln->refs[ln->nrefs] = ftell(output);
  ln->nrefs++;
}

int getGPRegID(char firstLetter)
{
  switch(firstLetter)
  {
    case 'a':
      return 0;
    case 'b':
      return 3;
    case 'c':
      return 1;
    case 'd':
      return 2;
  }
  return INVALID_REG;
}

//Get 3-bit register ID
int getRegID(char* name, OUT int* regType)
{
  //note: return 0xAF for invalid
  //caller can check for anything >= INVALID_REG as invalid
  //first get number of chars in reg name
  size_t nameLen = 0;
  for(char* iter = name; isalpha(*iter); iter++)
  {
    nameLen++;
  }
  if(nameLen < 2 || nameLen > 3)
  {
    //all regs have names that are 2 or 3 chars long
    return INVALID_REG;
  }
  if(nameLen == 2 && name[1] == 'h')
  {
    *regType = GP8;
    int baseID = getGPRegID(name[0]);
    return baseID == INVALID_REG ? INVALID_REG : baseID + 4;
  }
  if(nameLen == 2 && name[1] == 'l')
  {
    *regType = GP8;
    return getGPRegID(name[0]);
  }
  if(name[2] == 'x')
  {
    *regType = GP32;
    return getGPRegID(name[1]);
  }
  if(nameLen == 3 && name[0] == 's' && name[1] == 't')
  {
    //floating-point reg st0-st7
    if(name[2] < '0' && name[2] > '7')
    {
      return INVALID_REG;
    }
    *regType = X87;
    return name[2] - '0';
  }
  if(nameLen == 2 && name[1] == 's')
  {
    *regType = SEGMENT;
    switch(name[0])
    {
      case 'e':
        return 0;
      case 'c':
        return 1;
      case 's':
        return 2;
      case 'd':
        return 3;
      case 'f':
        return 4;
      case 'g':
        return 5;
      default:
        return INVALID_REG;
    }
  }
  if(name[0] == 'e')
  {
    //name is same as the 16-bit version except for 'e' prefix
    name++;
    *regType = GP32;
  }
  else
  {
    *regType = GP16;
  }
  if(name[1] == 'x')
  {
    int gp = getGPRegID(name[0]);
    if(gp < 4)
    {
      return gp;
    }
    else
    {
      return INVALID_REG;
    }
  }
  else if(name[1] == 'p')
  {
    if(name[0] == 's')
      return 4;
    else if(name[0] == 'b')
      return 5;
  }
  else if(name[1] == 'i')
  {
    if(name[0] == 's')
      return 6;
    else if(name[0] == 'd')
      return 7;
  }
  return INVALID_REG;
}

//precondition: name must be valid
//note: non-FPU and non-segment regs only
int getRegSize(char* name)
{
  if(name[0] == 'e')
    return 4;
  else if(name[1] == 'h' || name[1] == 'l')
    return 1;
  else
    return 2;
}

void writeData(void* ptr, int size)
{
  fwrite(ptr, size, 1, output);
  location += size;
}

void writeChar(char c)
{
  fputc(c, output);
  location++;
}

void writeByte(byte b)
{
  fwrite(&b, 1, 1, output);
  location++;
}

bool fitsU8(int val)
{
  return val >= -(1 << 7) && val < (1 << 8);
}

bool fitsU16(int val)
{
  return val >= -(1 << 15) && val < (1 << 16);
}

//if val could fit in signed type only (disp, rel)
bool fitsI8(int val)
{
  return val >= -(1 << 7) && val < (1 << 7);
}

bool fitsI16(int val)
{
  return val >- -(1 << 15) && val < (1 << 15);
}

void eatWhitespace()
{
  while(iter < len && (code[iter] == ' ' || code[iter] == '\t'))
  {
    iter++;
  }
}

void eatLineEnd()
{
  //eat a comment and/or whitespace up to and including newline
  if(iter == len)
    return;
  eatWhitespace();
  if(iter == len)
    return;
  if(code[iter] == ';')
  {
    while(iter < len && code[iter] != '\n')
      iter++;
  }
  //if not EOF, move to char after the newline
  if(iter < len && code[iter] == '\n')
  {
    iter++;
    lineno++;
  }
}

int parseInt()
{
  //parse int, preceded by whitespace, and update iter
  //strtol will provide end of number in numEnd
  eatWhitespace();
  int val = 1;
  if(code[iter] == '-')
  {
    iter++;
    val = -1;
  }
  if(!isdigit(code[iter]))
  {
    err("invalid integer");
  }
  val *= atoi(code + iter);
  while(isdigit(code[iter]))
  {
    iter++;
  }
  return val;
}

void parseDB()
{
  iter += 2;
  //can be a string, or sequence of space-separated hex or dec numbers
  eatWhitespace();
  if(code[iter] == '"')
  {
    //C-style string literal
    iter++;
    char toWrite;
    while(iter < len && code[iter] != '"')
    {
      if(code[iter] == '\n' || code[iter] == 0)
      {
        break;
      }
      if(code[iter] == '\\')
      {
        //escaped char, print next one verbatim
        iter++;
        if(code[iter] == 'x')
        {
          //read in 1 or 2 digit hex literal
          unsigned val;
          //avoid read past end of input buffer
          if(iter + 1 == len)
          {
            err("unterminated string constant ending in hex literal");
          }
          char hexnum[3] = {code[iter + 1], code[iter + 2], 0};
          if(1 != sscanf(hexnum, "%x", &val))
          {
            err("\\x escape code not followed by valid hex number");
          }
          writeByte(val);
          if(isxdigit(code[iter + 2]))
          {
            iter += 2;
          }
          else
          {
            iter++;
          }
        }
        else
        {
          if(code[iter] == 'n')
            toWrite = '\n';
          else if(code[iter] == 't')
            toWrite = '\t';
          else if(code[iter] == '0')
            toWrite = '\0';
          else if(code[iter] == '\\')
            toWrite = '\\';
          else if(code[iter] == 'r')
            toWrite = '\r';
          else
            err("unrecognized escape code");
          writeData(&toWrite, 1);
        }
      }
      else
      {
        //write the same char from input stream
        writeData(code + iter, 1);
      }
      iter++;
    }
    if(code[iter] != '"')
    {
      err("unterminated string constant");
    }
    //move iter to next char after closing '"'
    iter++;
  }
  else
  {
    //read numbers, each one can either be hex or dec
    //do bounds checking, value > 255 or < -128 is an error
    while(code[iter] != '\n' && code[iter] != ';' && code[iter] != 0)
    {
      if(code[iter + 1]  == 'x' || code[iter + 1] == 'X')
      {
        unsigned val;
        iter += 2;
        if(1 != sscanf(code + iter, "%x", &val))
        {
          err("invalid byte hex literal");
        }
        {
          err("value too big for 8 bits");
        }
        writeData(&val, 1);
        iter += 2;
        while(isxdigit(code[iter]))
          iter++;
      }
      else
      {
        //decimal number
        int val = 1;
        if(code[iter] == '-')
        {
          iter++;
          val = -1;
        }
        if(!isdigit(code[iter])) 
        {
          err("invalid byte literal");
        }
        val *= atoi(code + iter);
        if(!fitsU8(val))
        {
          err("byte value doesn't fit in 8 bits");
        }
        writeData(&val, 1);
        while(isdigit(code[iter]))
        {
          iter++;
        }
      }
      eatWhitespace();
      if(code[iter] == ',')
      {
        iter++;
        eatWhitespace();
      }
    }
  }
}

void parseDW()
{
  iter += 2;
  eatWhitespace();
  while(code[iter] != '\n' && code[iter] != ';' && code[iter] != 0)
  {
    if(code[iter + 1]  == 'x' || code[iter + 1] == 'X')
    {
      unsigned val;
      if(1 != sscanf(code + iter + 2, "%x", &val))
      {
        err("invalid hex word literal");
      }
      if(!fitsU16(val))
      {
        err("word value bigger than u16 max");
      }
      writeData(&val, 2);
      while(isxdigit(code[iter]))
        iter++;
      eatWhitespace();
    }
    else
    {
      if(code[iter] == '-')
      {
        iter++;
        if(!isdigit(code[iter]))
        {
          err("invalid word literal");
        }
        int val = -atoi(code + iter);
        if(!fitsU16(val))
        {
          err("word literal smaller than i16 min");
        }
        writeData(&val, 2);
      }
      else if(isdigit(code[iter]))
      {
        int val = atoi(code + iter);
        if(!fitsU16(val))
        {
          err("word literal bigger than u16 max");
        }
        writeData(&val, 2);
      }
      else
      {
        err("invalid word literal");
      }
    }
    while(isdigit(code[iter]))
    {
      iter++;
    }
    eatWhitespace();
    if(code[iter] == ',')
    {
      iter++;
      eatWhitespace();
    }
  }
}

void parseDD()
{
  //unlike DB and DD, no size checks are performed - what is read is used
  iter += 2;
  eatWhitespace();
  while(code[iter] != '\n' && code[iter] != ';' && code[iter] != 0)
  {
    if(code[iter + 1]  == 'x' || code[iter + 1] == 'X')
    {
      iter += 2;
      unsigned val;
      if(1 != sscanf(code + iter, "%x", &val))
      {
        err("invalid dword hex literal");
      }
      writeData(&val, 4);
      while(isxdigit(code[iter]) != ' ')
        iter++;
    }
    else
    {
      int val;
      if(1 != sscanf(code + iter, "%i", &val))
      {
        err("invalid dword literal");
      }
      writeData(&val, 4);
      if(code[iter] == '-')
        iter++;
      while(isdigit(code[iter]))
        iter++;
    }
    eatWhitespace();
    if(code[iter] == ',')
    {
      iter++;
      eatWhitespace();
    }
  }
}

OperandSet getEmptyOperandSet()
{
  OperandSet os;
  os.dispLabel = NULL;
  os.immLabel = NULL;
  os.op1Type = NO_OPERAND;          //an OperandType enum value
  os.op2Type = NO_OPERAND;
  os.reg1 = INVALID_REG;             //1st reg operand
  os.reg2 = INVALID_REG;             //2nd reg operand
  os.baseReg = INVALID_REG;
  os.indexReg = INVALID_REG;
  os.scale = 1;
  os.disp = 0;
  os.hasMem = false;
  os.sizeOverride = false;
  return os;
}

OperandSet parseOperands()
{
  OperandSet os = getEmptyOperandSet();
  //precondition: iter is anywhere after the mnemonic
  //"byte", "word" or "dword" before mem (set to 8, 16 or 32 when encountered)
  //if have mem and this is still 0 at end of operands, get from context, otherwise error
  int opSizeHint = 0;
  //If the user provides a hex imm val, treat it as unsigned (NASM's behavior)
  //do not condense to 8 bits even if signed val fits
  bool assumeSignedImm = true;
  //(32-bit mode only): whether operation is on 16-bit reg
  //parse operands until end of line, EOF or comment
  while(code[iter] != '\n' && code[iter] != 0 && code[iter] != ';')
  {
    eatWhitespace();
    int* nextOp = NULL;
    if(os.op1Type == NO_OPERAND)
      nextOp = &os.op1Type;
    else if(os.op2Type == NO_OPERAND)
      nextOp = &os.op2Type;
    else
    {
      printf("%i, %i\n", os.op1Type, os.op2Type);
      err("more than 3 operands provided");
    }
    //allow (optional) commas between operands
    if(code[iter] == ',')
    {
      iter++;
      eatWhitespace();
    }
    if(code[iter] == 0)
    {
      break;
    }
    if(code[iter] == '[')
    {
      //parse memory
      parseMem(&os.baseReg, &os.indexReg, &os.scale, &os.disp, &os.dispLabel);
      if(os.baseReg == INVALID_REG && os.indexReg == INVALID_REG)
      {
        *nextOp = MEM_ABSOLUTE;
      }
      else
      {
        *nextOp = REG_MEM;
      }
    }
    else if(code[iter] == '0' && code[iter + 1] == 'x')
    {
      if(1 != sscanf(code + iter, "%x", (unsigned*) &os.imm))
      {
        err("invalid hex literal");
      }
      assumeSignedImm = false;
      *nextOp = IMM;
      iter += 2;
      while(isxdigit(code[iter]))
      {
        iter++;
      }
    }
    else if(isdigit(code[iter]) || code[iter] == '-')
    {
      if(1 != sscanf(code + iter, "%i", &os.imm))
      {
        err("invalid imm val");
      }
      if(os.imm >= (1 << 7) && os.imm < (1 << 8))
      {
        assumeSignedImm = false;
      }
      if(code[iter] == '-')
      {
        iter++;
      }
      while(isdigit(code[iter]))
      {
        iter++;
      }
      //imm is stored as i32, but can be encoded as i8, i16 or i32 (handle size later)
      *nextOp = IMM;
    }
    else
    {
      //some other string, must be a reg name, label or mem size hint
      int newOpSizeHint = 0;
      if(strncmp(&code[iter], "byte ", 5) == 0)
        newOpSizeHint = 8;
      else if(strncmp(&code[iter], "word ", 5) == 0)
        newOpSizeHint = 16;
      else if(strncmp(&code[iter], "dword ", 6) == 0)
        newOpSizeHint = 32;
      if(newOpSizeHint)
      {
        if(opSizeHint && opSizeHint != newOpSizeHint)
        {
          err("operand size hint redefined with different value");
        }
        opSizeHint = newOpSizeHint;
        while(isalpha(code[iter]))
          iter++;
      }
      else
      {
        //reg or label
        int regType;
        int regID = getRegID(&code[iter], &regType);
        if(regID != INVALID_REG)
        {
          while(isalpha(code[iter]))
            iter++;
          if(regType == GP8 || regType == GP16 || regType == GP32)
          {
            if(regType == GP8)
            {
              if(regID == ID_AX)
                *nextOp = REG_AL;
              else if(regID == ID_CX)
                *nextOp = REG_CL;
              else
                *nextOp = REG_8;
            }
            else
            {
              if(bitsMode == BITS_32 && regType == GP16)
              {
                os.sizeOverride = true;
              }
              else if(bitsMode == BITS_16 && regType == GP32)
              {
                err("can't use 32-bit regs in 16 bit mode");
              }
              if(regID == ID_AX)
                *nextOp = REG_AX;
              else if(regID == ID_DX)
                *nextOp = REG_DX;
              else
                *nextOp = REG;
            }
            if(os.reg1 == INVALID_REG)
              os.reg1 = regID;
            else if(os.reg2 == INVALID_REG)
              os.reg2 = regID;
            else
              err("more than two register operands provided");
          }
          else if(regType == SEGMENT)
          {
            if(os.reg1 == INVALID_REG)
              os.reg1 = regID;
            else if(os.reg2 == INVALID_REG)
              os.reg2 = regID;
            else
              err("more than two register operands provided");
            *nextOp = SEGMENT_REG;
          }
          else if(regType == CONTROL)
          {
            *nextOp = CONTROL_REG;
          }
          else if(regType == X87)
          {
            *nextOp = FPU_REG;
          }
          else
          {
            err("register not allowed in this mode");
          }
        }
        else
        {
          //label providing immediate value
          LabelNode* ln = insertLabel(code + iter);
          if(ln->loc >= 0)
          {
            //already have imm label loc, don't need to use a reference
            //can possibly save 3 bytes this way, by using IMM_8
            os.imm += ln->loc;
          }
          else
          {
            //will add ref later
            os.immLabel = ln->name;
          }
          iter += strlen(ln->name);
          *nextOp = IMM;
        }
      }
    }
    eatWhitespace();
  }
  if(opSizeHint == 8)
  {
    if(os.op1Type == REG_MEM)
      os.op1Type = REG_MEM_8;
    if(os.op2Type == REG_MEM)
      os.op2Type = REG_MEM_8;
  }
  if(!os.immLabel)
  {
    if((assumeSignedImm && -(1 << 7) <= os.imm && os.imm < (1 << 7)) ||
        (!assumeSignedImm && ((unsigned) os.imm) < (1 << 8)))
    {
      //imm can fit in 8 bits
      if(os.op1Type == IMM)
        os.op1Type = IMM_8;
      else if(os.op2Type == IMM)
        os.op2Type = IMM_8;
    }
  }
  //all imm label references must be 32-bits, even if value could fit
  return os;
}

//true if expected OpType can always be "implicitly converted" to parsed
//"expected" comes from the opcode, and "parsed" comes from parseOperands()
//both "parsed" and "expected" should always be the most specific way to describe the type
bool opTypesEquivalent(int expected, int parsed)
{
  if(expected == parsed)
  {
    return true;
  }
  if(expected == IMM && parsed == IMM_8)
  {
    //parsed an immediate that could fit in a byte but could be stored as 16 or 32
    return true;
  }
  if(expected == REG_MEM && parsed == MEM_ABSOLUTE)
  {
    //can write a "moffs" memory as REG_MEM by using disp with no base
    return true;
  }
  if(expected == REG_MEM && parsed == REG)
  {
    return true;
  }
  if(expected == REG_MEM_8 && parsed == REG_8)
  {
    return true;
  }
  if(expected == REG_MEM || expected == REG)
  {
    if(parsed == REG_AX || parsed == REG_DX)
    {
      return true;
    }
  }
  if(expected == REG_MEM_8 || expected == REG_8)
  {
    if(parsed == REG_AL || parsed == REG_CL)
    {
      return true;
    }
  }
  return false;
}

bool matchOperands(Opcode* opc, OperandSet* os)
{
  int expectedType1, expectedType2;
  getOpTypes(opc, &expectedType1, &expectedType2);
  return opTypesEquivalent(expectedType1, os->op1Type) &&
         opTypesEquivalent(expectedType2, os->op2Type);
}

int getInstructionSize(Opcode* opc, OperandSet* operands)
{
  int size = 1;
  if(opc->flags & HAS_EXPANSION_PREFIX)
  {
    size++;
  }
  if(operands->sizeOverride)
  {
    size++;
  }
  if(opc->flags & HAS_MODRM)
  {
    size++;
    if(operands->indexReg != INVALID_REG)
    {
      //also need SIB
      size++;
    }
  }
  int op1Type, op2Type;
  getOpTypes(opc, &op1Type, &op2Type);
  if(op1Type == MEM_ABSOLUTE || op2Type == MEM_ABSOLUTE)
  {
    //note: oasm won't support address-size override
    size += 4;
  }
  if(op1Type == REG_MEM || op1Type == REG_MEM_8 || op2Type == REG_MEM || op2Type == REG_MEM_8)
  {
    if(operands->disp || operands->dispLabel)
    {
      size += 4;
    }
  }
  if(op1Type == IMM || op2Type == IMM)
  {
    if(operands->sizeOverride)
      size += 2;
    else
      size += 4;
  }
  if(op1Type == IMM_8 || op2Type == IMM_8)
  {
    size++;
  }
  return size;
}

void parseMem(OUT int* base, OUT int* index, OUT int* scale, OUT int* disp, OUT char** dispLabel)
{
  //parse memory operand
  //code[iter] must be '['
  //iter will be advanced to index after closing ']'
  //*dispLabel is label whose address must be added to displacement, or NULL if none
  //dispConstant is sum of constant displacements provided
  *base = INVALID_REG;
  *index = INVALID_REG;
  *scale = 1;
  *disp = 0;
  *dispLabel = NULL;
  //Memory operand has 4 components: base reg, displacement, index reg, scale
  iter++;
  //Flow chart for mem parsing
  //Until hit ']':
  //  number?
  //    if followed by *, parse reg immediately after, have scaled index
  //    otherwise add value to displacement
  //  other string?
  //    if valid reg name
  //      if reg name followed by *
  //        scaled index
  //      else
  //        base register OR index scaled by 1 (only need SIB if there is a different base register)
  //    else
  //      look up the string in symbol table and add symbol location to displacement
  //  eat a +
  while(code[iter] != ']')
  {
    int regID;
    int regType;
    eatWhitespace();
    if(isdigit(code[iter]) || code[iter] == '-')
    {
      int num = 0;
      //signed numerical constant displacement
      if(code[iter] == '-')
      {
        iter++;
        eatWhitespace();
        num -= atoi(&code[iter]);
      }
      else
      {
        num += atoi(&code[iter]);
      }
      while(isdigit(code[iter]))
        iter++;
      eatWhitespace();
      if(code[iter] != '*')
      {
        *disp += num;
      }
      else
      {
        //move past '*'
        iter++;
        eatWhitespace();
        //expect a scaled reg next
        regID = getRegID(&code[iter], &regType);
        while(isalpha(code[iter]))
          iter++;
        if(regID == INVALID_REG)
        {
          err("invalid register name");
        }
        if(regType != GP32)
        {
          err("only 32-bit GP registers can be used for scaled index");
        }
        if(*index != INVALID_REG)
        {
          err("can only have one index register in address expression");
        }
        *index = regID;
        *scale = num;
        if(*scale != 1 && *scale != 2 && *scale != 4 && *scale != 8)
        {
          err("invalid scale, must be 1, 2, 4 or 8");
        }
      }
    }
    else if(INVALID_REG != (regID = getRegID(&code[iter], &regType)))
    {
      //base or index reg (may be scaled)
      while(isalpha(code[iter]))
        iter++;
      eatWhitespace();
      //if a '*', have scaled index reg
      //otherwise, a base or index
      if(code[iter] == '*')
      {
        //read scale
        iter++;
        eatWhitespace();
        *scale = 0;
        if(isdigit(code[iter]))
          *scale = atoi(&code[iter]);
        if(*scale != 1 && *scale != 2 && *scale != 4 && *scale != 8)
          err("invalid scale, must be 1, 2, 4 or 8");
        while(isdigit(code[iter]))
          iter++;
        *index = regID;
      }
      else
      {
        //base or non-scaled index reg
        if(*base == INVALID_REG)
        {
          *base = regID;
        }
        else if(*index == INVALID_REG)
        {
          *index = regID;
          *scale = 1;
        }
        else
        {
          err("address expression can have at most one base and one index reg");
        }
      }
    }
    else
    {
      //displacement term from label
      if(*dispLabel)
        err("address expression can only use one label");
      LabelNode* ln = insertLabel(code + iter);
      iter += strlen(ln->name);
      if(ln->loc == -1)
      {
        //must resolve later
        *dispLabel = ln->name;
      }
      else
      {
        //know displacement so can just add it immediately
        *disp += ln->loc;
      }
    }
    eatWhitespace();
    if(code[iter] == '+')
    {
      iter++;
    }
  }
  //move past ']'
  iter++;
}

//Attempt to arrange base/index/scale in a valid way, and throw error if impossible
void arrangeMemRegs(int mod, int* base, int* index, int* scale)
{
  //for MOD_REG (mem with no disp), can't have esp or ebp as modrm base
  //for MOD_MEM_D32 (mem with disp), can't have esp as modrm base
  //in both cases, must move the base to the SIB index
  if((mod == MOD_REG && (*base == ID_SP || *base == ID_BP)) ||
      (mod == MOD_MEM_D32 && *base == ID_SP) ||
      (*index == ID_SP))
  {
    //can't have that base reg, substitude into index if possible
    if(*scale != 1 && *index != INVALID_REG)
    {
      err("can't have esp/ebp + scaled index");
    }
    else
    {
      //can safely swap base and index
      int temp = *base;
      *base = *index;
      *index = temp;
      *scale = 1;
    }
    //check if still invalid, possible for e.g. [esp + esp]
    if(*index == ID_SP)
    {
      err("invalid addressing regs");
    }
  }
}

//TODO: use 8-bit disp (mod = 1) when possible
//sib is set to -1 if not used
//
//opType1 and opType2 are the parsed operand types
void getModSIB(Opcode* opc, OperandSet* os, OUT int* modrm, OUT int* sib)
{
  int op1Type, op2Type;
  getOpTypes(opc, &op1Type, &op2Type);
  //prepare each field separately, in the low bits of these ints (SHL into place + OR at the end)
  int mod = 0;        // 2 bits, shift left 6
  int reg = 0;        // 3 bits, shift left 3
  int rm = 0;         // 3 bits
  //can easily get mod (currently only 3 cases: 11b = 2nd reg arg, 10b = mem with disp 32, 00b = other
  bool haveMem = os->op1Type == REG_MEM || os->op1Type == REG_MEM_8 || os->op2Type == REG_MEM || os->op2Type == REG_MEM_8;
  if(!haveMem)
  {
    //rm field contains a second reg
    mod = MOD_REG;
  }
  else if((os->disp || os->dispLabel) && os->baseReg != INVALID_REG)
  {
    mod = MOD_MEM_D32;
  }
  if(os->baseReg != INVALID_REG || os->indexReg != INVALID_REG)
  {
    arrangeMemRegs(mod, &os->baseReg, &os->indexReg, &os->scale);
  }
  //sib required if:
  //  -have an index reg (with any scale)
  //  -base reg can't be a r/m base (applies to esp, and ebp for mod = 0)
  bool haveSIB = haveMem && (os->indexReg != INVALID_REG || os->baseReg == ID_SP || (mod == MOD_REG && os->baseReg == ID_BP));
  //note: r/m is ALWAYS the first operand, unless there are 2 operands (then the 2nd reg is in /reg)
  //set reg field
  //is either a digit, REG/REG_8 id, or left undetermined
  if(!haveMem && (op1Type == REG || op1Type == REG_8 || op1Type == SEGMENT_REG))
  {
    //2 reg operands, 2nd always goes in reg field
    reg = os->reg1;
  }
  else if(!haveMem && (op2Type == REG || op2Type == REG_8 || op2Type == SEGMENT_REG))
  {
    if(os->reg2 == INVALID_REG)
    {
      //2 regs, 2nd one is REG
      reg = os->reg1;
    }
    else
    {
      //just 1 reg
      reg = os->reg2;
    }
  }
  else if(opc->flags & HAS_DIGIT)
  {
    //opcode requires a digit, get as bits 4-6 of opcode flags
    reg = (opc->flags >> 4) & 0b111;
  }
  //get rm field
  if(haveSIB)
  {
    rm = 0b100;
  }
  else if(mod == MOD_REG)
  {
    if(op1Type == REG_MEM || op1Type == REG_MEM_8)
    {
      rm = os->reg1;
    }
    else if(op2Type == REG_MEM || op2Type == REG_MEM_8)
    {
      if(os->reg2 == INVALID_REG)
      {
        rm = os->reg1;
      }
      else
      {
        rm = os->reg2;
      }
    }
  }
  else if(mod == MOD_MEM && os->baseReg == INVALID_REG)
  {
    //just [disp]
    rm = 0b101;
  }
  else if(mod == MOD_MEM)
  { 
    rm = os->baseReg;
  }
  else if(mod == MOD_MEM_D32)
  {
    rm = os->baseReg;
  }
  else
  {
    errInternal(__LINE__);
  }
  *modrm = (mod << 6) | (reg << 3) | rm;
  if(haveSIB)
  {
    int scaleBits = 0;
    switch(os->scale)
    {
      case 1:
        scaleBits = 0;
        break;
      case 2:
        scaleBits = 1;
        break;
      case 4:
        scaleBits = 2;
        break;
      case 8:
        scaleBits = 3;
        break;
      default:
        errInternal(__LINE__);
    }
    if(os->indexReg == INVALID_REG)
    {
      *sib = (scaleBits << 6) | (0b100 << 3) | os->baseReg;
    }
    else
    {
      *sib = (scaleBits << 6) | (os->indexReg << 3) | os->baseReg;
    }
  }
  else
  {
    *sib = -1;
  }
}

void parseInstruction(char* mneSource, size_t mneLen)
{
  /* All operand types:
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
  REG_DX,
  REG_CL,         //also matches REG_MEM_8 and REG_8
  MEM_ABSOLUTE    //absolute mem (for mov and jmp)
  */
  if(mneLen > 7)
  {
    err("label not terminated by :");
  }
  char mne[8];
  memcpy(mne, mneSource, mneLen);
  mne[mneLen] = 0;
  Mnemonic* m = NULL;
  for(int i = 0; i < NUM_MNEMONICS; i++)
  {
    if(strcmp(mne, mneTable[i].mnemonic) == 0)
    {
      m = &mneTable[i];
      break;
    }
  }
  if(!m)
  {
    err("unknown mnemonic or label without :");
  }
  iter += strlen(mne);
  OperandSet os = parseOperands();
  //need to check for code offset (rel8/rel32) values in place of imm
  //now use the operand types and sizes to look up opcode
  Opcode* opc = NULL;
  int bestSize = 100;
  for(int i = m->opcodeOffset; i < m->opcodeOffset + m->numOpcodes; i++)
  {
    Opcode* iter = &opcodeTable[i];
    if(matchOperands(iter, &os))
    {
      int iterSize = getInstructionSize(iter, &os);
      if(iterSize < bestSize)
      {
        opc = iter;
        bestSize = iterSize;
      }
    }
  }
  if(opc == NULL)
  {
    err("no opcode for given operands");
  }
  int op1Type, op2Type;
  getOpTypes(opc, &op1Type, &op2Type);
  //get modrm and sib (if needed BEFORE emitting anything, to catch errors earlier)
  bool haveMem = os.op1Type == REG_MEM || os.op1Type == REG_MEM_8 || os.op2Type == REG_MEM || os.op2Type == REG_MEM_8 || os.disp || os.dispLabel;
  int modrm = -1;
  int sib = -1;
  if(opc->flags & HAS_MODRM)
  {
    getModSIB(opc, &os, &modrm, &sib);
  }
  //emit instruction
  //first, opcode prefixes
  if(opc->flags & HAS_EXPANSION_PREFIX)
  {
    writeByte(0x0F);
  }
  if(os.sizeOverride)
  {
    writeByte(0x66);
  }
  //opcode, possibly OR'd with a reg id
  byte opcode = opc->opcode;
  if(opc->flags & REG_IN_OPCODE)
  {
    if(os.reg1 != INVALID_REG)
    {
      printf("or'ing opcode with reg1: %i\n", os.reg1);
      opcode |= os.reg1;
    }
    else if(os.reg2 != INVALID_REG)
    {
      printf("or'ing opcode with reg2: %i\n", os.reg2);
      opcode |= os.reg2;
    }
  }
  writeByte(opcode);
  //modrm/sib
  int digit = getDigit(opc);
  if(opc->flags & HAS_MODRM)
  {
    writeByte(modrm);
    if(sib != -1)
    {
      writeByte(sib);
    }
  }
  //disp, if exists (currently always 32 bit)
  if(haveMem)
  {
    if(os.disp || os.dispLabel)
    {
      //add ref to label if used
      if(os.dispLabel)
      {
        labelAddReference(insertLabel(os.dispLabel));
      }
      //have a disp, even if given constant is 0
      //note: this works fine for mov with moffs (absolute mem without modrm)
      writeData(&os.disp, 4);
    }
  }
  //immediate, if used (currently can be 1 or 4 bytes, 2 is for op-size override and (TODO) 16-bit mode)
  if(os.imm || os.immLabel)
  {
    if(op1Type == IMM || op2Type == IMM)
    {
      if(os.immLabel)
      {
        //add label reference at current location in output file
        labelAddReference(insertLabel(os.immLabel));
      }
      if(opc->flags & HAS_CODE_OFFSET)
      {
        puts("asdf");
        //know that end of instruction is loc + 4
        os.imm -= (location + 4);
      }
      if(!os.immLabel && os.sizeOverride)
      {
        writeData(&os.imm, 2);
      }
      else if(!os.sizeOverride)
      {
        writeData(&os.imm, 4);
      }
      else
      {
        err("16-bit imm values with labels not supported");
      }
    }
    else if(op1Type == IMM_8 || op2Type == IMM_8)
    {
      if(os.immLabel)
      {
        err("8-bit imm values with labels not supported");
      }
      writeData(&os.imm, 1);
    }
  }
}

void parseLine()
{
  const unsigned char zero = 0;
  eatWhitespace();
  //handle empty/comment lines
  if(code[iter] == ';' || code[iter] == '\n')
  {
    eatLineEnd();
    return;
  }
  else if(!strncmp(iter + code, "global ", 7))
  {
    //global declaration
    iter += 6;
    eatWhitespace();
    //TODO: in ELFO mode, make this label an externally visible symbol
    //in FLAT mode, is a no-op
  }
  else if(!strncmp(iter + code, "align ", 6))
  {
    iter += 6;
    //alignment: output 0 bytes until aligned to given boundary
    //read alignment amount
    int alignment = parseInt();
    int pad = alignment - (location % alignment);
    for(int i = 0; i < pad; i++)
    {
      fwrite(&zero, 1, 1, output);
    }
    location += pad;
  }
  else if(!strncmp(iter + code, "org ", 4))
  {
    iter += 7;
    //read in new location
    location = parseInt();
  }
  else if(!strncmp(iter + code, "bits ", 5))
  {
    iter += 5;
    int nbits = 0;
    if(isdigit(iter[code]))
    {
      nbits = atoi(iter + code);
    }
    if(nbits == 16)
    {
      bitsMode = BITS_16;
    }
    else if(nbits == 32)
    {
      bitsMode = BITS_32;
    }
    else
    {
      err("invalid bits directive: 16 or 32 only");
    }
    while(isdigit(code[iter]))
    {
      iter++;
    }
  }
  else if(!strncmp(iter + code, "db ", 3))
  {
    parseDB();
  }
  else if(!strncmp(iter + code, "dw ", 3))
  {
    parseDW();
  }
  else if(!strncmp(iter + code, "dd ", 3))
  {
    parseDD();
  }
  else
  {
    //either label or mnemonic followed by operands
    //check for label format (string w/ no spaces followed immediately by ':')
    char* idStart = code + iter;
    //scan ahead to next space or newline
    size_t idLen = 0;
    while(iter + idLen < len && idStart[idLen] != ' ' && idStart[idLen] != '\n' && idStart[idLen] != ':')
    {
      idLen++;
    }
    if(idStart[idLen] == ':')
    {
      if(*idStart != '.')
      {
        //a global label is being defined, purge local labels
        resolveLabels(true);
        destroyTree(&localLabels);
        localLabels = initTree();
      }
      //is a label; validate it and insert in correct tree if not already done
      LabelNode* label = insertLabel(idStart);
      bool local = label->name[0] == '.';
      //if the label already has a given location, error
      if(label->loc >= 0)
        err("label redefined");
      //set the label's position to here
      label->loc = location;
      //continue on same line (past the label and the ':')
      iter += idLen + 1;
    }
    else
    {
      parseInstruction(idStart, idLen);
    }
  }
  eatLineEnd();
}

void parse()
{
  globalLabels = initTree();
  localLabels = initTree();
  while(iter < len)
  {
    parseLine();
  }
  //resolve global lables throughout program
  resolveLabels(false);
}

bool hasDigit(Opcode* opc)
{
  return opc->flags & HAS_DIGIT ? 1 : 0;
}

void getOpTypes(Opcode* opc, OUT int* op1, OUT int* op2)
{
  *op1 = opc->opTypes & 0xF;
  *op2 = (opc->opTypes & 0xF0) >> 4;
}

int getDigit(Opcode* opc)
{
  return opc->flags & HAS_DIGIT ? (opc->flags >> 4) & 0b111 : INVALID_REG;
}

void resolveLabels(bool local)
{
  LabelTree* tree = local ? &localLabels : &globalLabels;
  //add the label's location as a 32-bit number to the value already there in file
  for(int i = 0; i < tree->nnodes; i++)
  {
    LabelNode* lnode = &tree->nodes[i];
    //label MUST be resolved at this point, is error if not
    if(lnode->loc == -1)
    {
      err("label undefined");
    }
    //iterate over all references and add their location to the existing 4-byte value there
    if(lnode->nrefs > 0)
    {
      printf("Resolving references to label \"%s\", with loc %#x\n", lnode->name, lnode->loc);
    }
    for(int j = 0; j < lnode->nrefs; j++)
    {
      int refLoc = lnode->refs[j];
      fseek(output, refLoc, SEEK_SET);
      int totalValue = 0;
      fread(&totalValue, 1, 4, output);
      fseek(output, refLoc, SEEK_SET);
      totalValue += lnode->loc;
      fwrite(&totalValue, 1, 4, output);
    }
  }
  //return to end of output file to continue writing
  fseek(output, 0, SEEK_END);
}

void err(const char* str)
{
  printf("Error on line %lu: %s\n", lineno, str);
  exit(1);
}

void errInternal(int asmLineNo)
{
  printf("Internal error: assembler line %i\n", asmLineNo);
  exit(2);
}

int main(int argc, const char** argv)
{
  //const char* ifn = NULL;
  const char* ifn = "test.asm";
  const char* ofn = "test.bin";
  formatMode = FLAT;
  bitsMode = BITS_16;
  for(int i = 1; i < argc; i++)
  {
    if(!strcmp(argv[i], "-o"))
    {
      if(i + 1 < argc)
      {
        //output file specified
        ofn = argv[++i];
      }
      else
      {
        puts("Error: -o not followed by output file name");
        exit(1);
      }
    }
    else if(!strcmp(argv[i], "-e") || !strcmp(argv[i], "--elf"))
    {
      formatMode = ELF;
    }
    else if(!strcmp(argv[i], "-16") || !strcmp(argv[i], "--bits16"))
    {
      bitsMode = BITS_16;
    }
    else if(!strcmp(argv[i], "-32") || !strcmp(argv[i], "--bits32"))
    {
      bitsMode = BITS_32;
    }
    else
    {
      //input file
      if(!ifn)
      {
        ifn = argv[i];
      }
      else
      {
        puts("Error: cannot specify more than 1 input file");
        exit(1);
      }
    }
  }
  if(!ifn)
  {
    puts("Error: no input file given");
    exit(1);
  }
  //get file length
  len = 0;
  FILE* input = fopen(ifn, "r");
  if(!input)
  {
    printf("Error: could not open input file \"%s\"\n", ifn);
  }
  fseek(input, 0, SEEK_END);
  len = ftell(input);
  rewind(input);
  code = malloc(len + 1);
  fread(code, 1, len, input);
  code[len] = 0;
  fclose(input);
  //process file & write output
  output = fopen(ofn, "w+");
  //input lines start at 1, to follow Vim convention
  lineno = 1;
  location = 0;
  iter = 0;
  parse();
  fclose(output);
  return 0;
}

