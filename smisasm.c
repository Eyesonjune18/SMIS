#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


#define USAGE "Usage: ./smisasm <ASM file>\n"
#define MAX_INSTRUCTION_LEN 50
#define MAX_STRING_LEN 500
#define IMMEDIATE_MAX_VAL 65535
// 16-bit unsigned int limit

#define OP_SET 1
#define OP_COPY 2

#define OP_ADD 3
#define OP_ADD_IMM 4
#define OP_SUBTRACT 5
#define OP_SUBTRACT_IMM 6
#define OP_MULTIPLY 7
#define OP_MULTIPLY_IMM 8
#define OP_DIVIDE 9
#define OP_DIVIDE_IMM 10

#define OP_COMPARE 11
#define OP_COMPARE_IMM 12

#define OP_SHIFT_LEFT 13
#define OP_SHIFT_LEFT_IMM 14
#define OP_SHIFT_RIGHT 15
#define OP_SHIFT_RIGHT_IMM 16

#define OP_AND 17
#define OP_AND_IMM 18
#define OP_OR 19
#define OP_OR_IMM 20
#define OP_XOR 21
#define OP_XOR_IMM 22
#define OP_NAND 23
#define OP_NAND_IMM 24
#define OP_NOR 25
#define OP_NOR_IMM 26
#define OP_NOT 27

#define OP_LOAD 28
#define OP_STORE 29


int line = 1;
// Line number is stored in order to give more descriptive error messages


void readInstructions(char* filename);
unsigned int assembleInstruction(char* instruction);

unsigned int AType(char* instruction);
unsigned int IType(char* instruction);
unsigned int SType(char* instruction);

unsigned int getRegisterNum(char* str);
unsigned int getImmediateVal(char* str);
bool fitsRegisterSyntax(char* str);
bool fitsImmediateSyntax(char* str);

bool containsOnlyNums(char* str);
int countWords(char* str);
char* getFirstWord(char* str);
char* getWord(char* str, int w);
unsigned char* getBinary(unsigned int n, int length);
unsigned char binaryChar(unsigned int n);


int main(int argc, char** argv) {

    if(argc != 2) {

        printf(USAGE);
        exit(-1);

    }

    readInstructions(argv[1]);

}

void readInstructions(char* filename) {
    // Reads all instructions from the given file and interprets them

    FILE* asmFile;

    if(!(asmFile = fopen(filename, "r"))) {

        printf("File %s does not exist.\n", filename);
        printf(USAGE);
        exit(-1);

    }

    char* instruction = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    while(fgets(instruction, MAX_INSTRUCTION_LEN, asmFile)) {

        bool skipLine = false;

        if(!strncmp(instruction, "\n", 2) || !strncmp(instruction, "//", 2)) skipLine = true;
        // Skip line breaks and comments

        if(!skipLine) {
            
            int lineBreakIndex = strnlen(instruction, MAX_INSTRUCTION_LEN) - 1;
            if(instruction[lineBreakIndex] == '\n') instruction[lineBreakIndex] = '\0';
            // Remove any trailing line breaks from the instruction

            printf("Instruction at line %i: %s\n", line, getBinary(assembleInstruction(instruction), 32));

        }

        line++;

    }

    free(instruction);

}

unsigned int assembleInstruction(char* instruction) {
    // Assembles all instruction types into their respective numeric values

    unsigned int instructionNum = 0;

    if((instructionNum = AType(instruction))) return instructionNum;
    else if((instructionNum = IType(instruction))) return instructionNum;
    else if((instructionNum = SType(instruction))) return instructionNum;

    return 0;

}

unsigned int AType(char* instruction) {
    // Assembles all basic A-type (arithmetic) instructions, excluding COPY, COMPARE, and NOT
    // Returns 0 if the given string is not a valid A-type instruction

    unsigned int instructionNum = 0;

    char* opcodeStr = getFirstWord(instruction);
    unsigned int opcodeNum;

    if(!strncmp(opcodeStr, "ADD", 4)) opcodeNum = OP_ADD;
    else if(!strncmp(opcodeStr, "SUBTRACT", 9)) opcodeNum = OP_SUBTRACT;
    else if(!strncmp(opcodeStr, "MULTIPLY", 9)) opcodeNum = OP_MULTIPLY;
    else if(!strncmp(opcodeStr, "DIVIDE", 7)) opcodeNum = OP_DIVIDE;

    else if(!strncmp(opcodeStr, "SHIFT-LEFT", 11)) opcodeNum = OP_SHIFT_LEFT;
    else if(!strncmp(opcodeStr, "SHIFT-RIGHT", 12)) opcodeNum = OP_SHIFT_RIGHT;

    else if(!strncmp(opcodeStr, "AND", 4)) opcodeNum = OP_AND;
    else if(!strncmp(opcodeStr, "OR", 3)) opcodeNum = OP_OR;
    else if(!strncmp(opcodeStr, "XOR", 4)) opcodeNum = OP_XOR;
    else if(!strncmp(opcodeStr, "NAND", 5)) opcodeNum = OP_NAND;
    else if(!strncmp(opcodeStr, "NOR", 4)) opcodeNum = OP_NOR;

    else return 0;

    instructionNum += opcodeNum << 24;

    if(countWords(instruction) != 4) {

        printf("Incorrect number of arguments at line %i\n", line);
        printf("Instruction: %s\n", instruction);
        exit(-1);

    }

    for(int arg = 1; arg <= 3; arg++) {
        
        if(!fitsRegisterSyntax(getWord(instruction, arg))) {

            printf("Wrong format of argument %i at line %i\n", arg, line);
            printf("Instruction: %s\n", instruction);
            exit(-1);

        }

    }

    unsigned int rDest = getRegisterNum(getWord(instruction, 1));
    unsigned int rOp1 = getRegisterNum(getWord(instruction, 2));
    unsigned int rOp2 = getRegisterNum(getWord(instruction, 3));

    instructionNum += rDest << 20;
    instructionNum += rOp1 << 16;
    instructionNum += rOp2 << 12;

    return instructionNum;

}

unsigned int IType(char* instruction) {
    // Assembles all basic I-type (immediate) instructions, excluding SET and COMPARE-IMM
    // Returns 0 if the given string is not a valid I-type instruction

    unsigned int instructionNum = 0;

    char* opcodeStr = getFirstWord(instruction);
    unsigned int opcodeNum;

    if(!strncmp(opcodeStr, "ADD-IMM", 8)) opcodeNum = OP_ADD_IMM;
    else if(!strncmp(opcodeStr, "SUBTRACT-IMM", 13)) opcodeNum = OP_SUBTRACT_IMM;
    else if(!strncmp(opcodeStr, "MULTIPLY-IMM", 13)) opcodeNum = OP_MULTIPLY_IMM;
    else if(!strncmp(opcodeStr, "DIVIDE-IMM", 11)) opcodeNum = OP_DIVIDE_IMM;

    else if(!strncmp(opcodeStr, "SHIFT-LEFT-IMM", 15)) opcodeNum = OP_SHIFT_LEFT_IMM;
    else if(!strncmp(opcodeStr, "SHIFT-RIGHT-IMM", 16)) opcodeNum = OP_SHIFT_RIGHT_IMM;

    else if(!strncmp(opcodeStr, "AND-IMM", 8)) opcodeNum = OP_AND_IMM;
    else if(!strncmp(opcodeStr, "OR-IMM", 7)) opcodeNum = OP_OR_IMM;
    else if(!strncmp(opcodeStr, "XOR-IMM", 8)) opcodeNum = OP_XOR_IMM;
    else if(!strncmp(opcodeStr, "NAND-IMM", 9)) opcodeNum = OP_NAND_IMM;
    else if(!strncmp(opcodeStr, "NOR-IMM", 8)) opcodeNum = OP_NOR_IMM;
    else if(!strncmp(opcodeStr, "LOAD", 5)) opcodeNum = OP_LOAD;
    else if(!strncmp(opcodeStr, "STORE", 6)) opcodeNum = OP_STORE;

    else return 0;

    instructionNum += opcodeNum << 24;

    if(countWords(instruction) != 4) {

        printf("Incorrect number of arguments at line %i\n", line);
        printf("Instruction: %s\n", instruction);
        exit(-1);

    }

    for(int arg = 1; arg <= 3; arg++) {
        
        if((arg != 3 && !fitsRegisterSyntax(getWord(instruction, arg)))
            || (arg == 3 && !fitsImmediateSyntax(getWord(instruction, arg)))) {

            printf("Wrong format of argument %i at line %i\n", arg, line);
            printf("Instruction: %s\n", instruction);
            exit(-1);

        }

    }

    unsigned int rDest = getRegisterNum(getWord(instruction, 1));
    unsigned int rOp1 = getRegisterNum(getWord(instruction, 2));
    unsigned int iOp2 = getImmediateVal(getWord(instruction, 3));

    instructionNum += rDest << 20;
    instructionNum += rOp1 << 16;
    instructionNum += iOp2;

    return instructionNum;

}

unsigned int SType(char* instruction) {
    // Assembles all non-standard instructions
    // Returns 0 if the given string is not a valid special instruction

    unsigned int instructionNum = 0;

    char* opcodeStr = getFirstWord(instruction);
    unsigned int opcodeNum;

    bool immediateMode = false;
    bool compareMode = false;
    bool notMode = false;
    
    if(!strncmp(opcodeStr, "SET", 4)) { opcodeNum = OP_SET; immediateMode = true; }
    else if(!strncmp(opcodeStr, "COPY", 5)) opcodeNum = OP_COPY;
    else if(!strncmp(opcodeStr, "COMPARE", 8)) { opcodeNum = OP_COMPARE; compareMode = true; }
    else if(!strncmp(opcodeStr, "COMPARE-IMM", 12)) { opcodeNum = OP_COMPARE_IMM; immediateMode = true; compareMode = true;}
    else if(!strncmp(opcodeStr, "NOT", 4)) { opcodeNum = OP_NOT; notMode = true; }

    else return 0;

    instructionNum += opcodeNum << 24;

    if(countWords(instruction) != 3) {

        printf("Incorrect number of arguments at line %i\n", line);
        printf("Instruction: %s\n", instruction);
        exit(-1);

    }

    for(int arg = 1; arg <= 2; arg++) {
        
        if((arg == 1 && !fitsRegisterSyntax(getWord(instruction, arg)))
            || (arg == 2 && !immediateMode && !fitsRegisterSyntax(getWord(instruction, arg)))
            || (arg == 2 && immediateMode && !fitsImmediateSyntax(getWord(instruction, arg)))) {

            printf("Wrong format of argument %i at line %i\n", arg, line);
            printf("Instruction: %s\n", instruction);
            exit(-1);

        }

    }

    unsigned int reg = getRegisterNum(getWord(instruction, 1));
    unsigned int op = immediateMode ? getImmediateVal(getWord(instruction, 2)) : getRegisterNum(getWord(instruction, 2));

    if(compareMode) instructionNum += reg << 16;
    else instructionNum += reg << 20;
    if(immediateMode) instructionNum += op;
    else if(notMode) instructionNum += op << 16;
    else instructionNum += op << 12;

    return instructionNum;

}

unsigned int getRegisterNum(char* str) {
    // Gets the register address from a given string
    // Assumes that string has already been validated as a proper register address argument

    return strtol(str + 1, NULL, 10);

}

unsigned int getImmediateVal(char* str) {
    // Gets the immediate value from a given string
    // Assumes that string has already been validated as a proper immediate argument

    return strtol(str + 1, NULL, 10);

}

bool fitsRegisterSyntax(char* str) {
    // Checks if a given string fits the SMIS register standard syntax "R<4-bit unsigned register address>"

    if(*str != 'R') return false;

    if(!containsOnlyNums(str + 1)) return false;

    int regNum;
    if(!(regNum = strtol(str + 1, NULL, 10)) || regNum > 15) return false;

    return true;

}

bool fitsImmediateSyntax(char* str) {
    // Checks if a given string fits the SMIS immediate standard syntax "#<16-bit unsigned int>"

    if(*str != '#') return false;

    if(!containsOnlyNums(str + 1)) return false;

    unsigned int immVal;
    if(!(immVal = strtol(str + 1, NULL, 10)) || immVal > IMMEDIATE_MAX_VAL) return false;

    return true;

}

bool containsOnlyNums(char* str) {
    // Checks if a given string contains only numerical digit characters

    while(*str) {

        if(*str < '0' || *str > '9') return false;
        str++;

    }

    return true;

}

int countWords(char* str) {
    // Counts the number of space-separated words in a given string

    // TODO: Make this safer

    int count = 0;

    while(*str) {

        if(*str == ' ') count++;
        str++;

    }

    count++;

    return count;

}

char* getFirstWord(char* str) {
    // Gets the first word (all characters before first space or null terminator) from a given string

    int strlen = strnlen(str, MAX_STRING_LEN) + 1;

    char* word = malloc(strlen * sizeof(char));

    int i = 0;
    
    while(*str && *str != ' ') {

        word[i] = *str;
        str++;
        i++;

    }

    word[i] = '\0';

    int wordlen = strnlen(word, MAX_STRING_LEN) + 1;
    word = realloc(word, wordlen * sizeof(char));

    return word;

}

char* getWord(char* str, int w) {
    // Gets an indexed word from a given string

    if(w >= countWords(str)) {

        printf("Internal error: cannot get word %i (indexed) from string with %i words\n", w, countWords(str));
        exit(-2);

    }

    for(int i = 0; i < w; i++) {

        str += strnlen(getFirstWord(str), MAX_STRING_LEN) + 1;

    }

    return getFirstWord(str);

}

unsigned char* getBinary(unsigned int n, int length) {
    // Returns a given int in binary format

    unsigned char* binary = malloc(n * sizeof(unsigned char));

    for(int i = 0; i < length; i++) binary[length - (i + 1)] = binaryChar((n & 1 << i) >> i);

    return binary;

}

unsigned char binaryChar(unsigned int n) {

    if(n == 0) return '0';
    else if(n == 1) return '1';
    else {

        printf("Internal error: cannot get binary char equivalent for digit %i\n", n);
        exit(-2);

    }

}