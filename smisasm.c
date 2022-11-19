#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


#define USAGE "Usage: ./smisasm <ASM file>\n"
#define MAX_INSTRUCTION_LEN 50
#define MAX_STRING_LEN 500
#define IMMEDIATE_MAX_VAL 65535

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
#define OP_SHIFT_RIGHT 14

#define OP_AND 15
#define OP_OR 16
#define OP_XOR 17
#define OP_NAND 18
#define OP_NOR 19
#define OP_NOT 20

#define OP_LOAD 21
#define OP_STORE 22


int line = 1;


void readInstructions(char* filename);
unsigned int assembleInstruction(char* instruction);

unsigned int aType(char* instruction);
int aType_c(char* instruction);
int mType(char* instruction);
int iType(char* instruction);

unsigned int getRegisterNum(char* str);
unsigned int getImmediateVal(char* str);
bool fitsRegisterSyntax(char* str);
bool fitsImmediateSyntax(char* str);

bool containsOnlyNums(char* str);
int countWords(char* str);
char* getFirstWord(char* str);
char* getWord(char* str, int w);
void printBinary(unsigned int n, int length);


int main(int argc, char** argv) {

    if(argc != 2) {

        printf(USAGE);
        exit(-1);

    }

    readInstructions(argv[1]);

}

void readInstructions(char* filename) {
    // Reads all instructions from the given file and interprets them

    FILE* f;

    if(!(f = fopen(filename, "r"))) {

        printf("File %s does not exist.\n", filename);
        printf(USAGE);
        exit(-1);

    }

    char* instruction = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    while(fgets(instruction, MAX_INSTRUCTION_LEN, f)) {

        if(strncmp(instruction, "\n", 2)) {
            
            int lineBreakIndex = strnlen(instruction, MAX_INSTRUCTION_LEN) - 1;
            if(instruction[lineBreakIndex] == '\n') instruction[lineBreakIndex] = '\0';

            printBinary(assembleInstruction(instruction), 32);

        }

        line++;

    }

    free(instruction);

}

unsigned int assembleInstruction(char* instruction) {
    // Returns an int representing the numeric representation of the given instruction

    return aType(instruction);

}

unsigned int aType(char* instruction) {

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
    else if(!strncmp(opcodeStr, "NOT", 4)) opcodeNum = OP_NOT;

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

unsigned int getRegisterNum(char* str) {

    return strtol(str + 1, NULL, 10);

}

unsigned int getImmediateVal(char* str) {

    return strtol(str + 1, NULL, 10);

}

bool fitsRegisterSyntax(char* str) {

    if(*str != 'R') return false;

    if(!containsOnlyNums(str + 1)) return false;

    int regNum;
    if(!(regNum = strtol(str + 1, NULL, 10)) || regNum > 15) return false;

    return true;

}

bool fitsImmediateSyntax(char* str) {

    if(*str != '#') return false;

    if(!containsOnlyNums(str + 1)) return false;

    unsigned int immVal;
    if(!(immVal = strtol(str + 1, NULL, 10)) || immVal > IMMEDIATE_MAX_VAL) return false;

    return true;

}

bool containsOnlyNums(char* str) {

    while(*str) {

        if(*str < '0' || *str > '9') return false;
        str++;

    }

    return true;

}

int countWords(char* str) {

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

    if(w >= countWords(str)) {

        printf("Internal error: cannot get word %i (indexed) from string with %i words\n", w, countWords(str));
        exit(-2);

    }

    for(int i = 0; i < w; i++) {

        str += strnlen(getFirstWord(str), MAX_STRING_LEN) + 1;

    }

    return getFirstWord(str);

}

void printBinary(unsigned int n, int length) {
    // Prints a given int in binary format

    for(int i = length - 1; i >= 0; i--) printf("%i", (n & 1 << i) >> i);
    printf("\n");

}