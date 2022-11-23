#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


#define USAGE "Usage: ./smisasm <ASM file> <output BIN file>\n"
#define MAX_INSTRUCTION_LEN 50
#define MAX_STRING_LEN 500
#define INT_LIMIT 65535

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


typedef struct Label {

    char* labelName;
    unsigned int PCAddress;

} Label;


Label* SYMBOL_TABLE;
// Stores all labels in the assembled file
unsigned int SYMBOL_COUNT = 0;
// Stores the amount of symbols to avoid iterating over unallocated pointers

int INSTRUCTION_ADDR = 0;
// Instruction address is stored for symbol table usage
int LINE_NUMBER = 1;
// Line number is stored in order to give more descriptive error messages


void readLabels(char* readfile);
void readInstructions(char* readfile, char* writefile);
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
char* getBinary(unsigned int n, int length);
unsigned char binaryChar(unsigned int n);
bool isBlankLineOrComment(char* str);
bool isLabel(char* str);
void trimLineBreak(char* str);
void trimLabelColon(char* str);
void trimChar(char* str, char c);


int main(int argc, char** argv) {

    if(argc != 3) {

        printf(USAGE);
        exit(-1);

    }

    SYMBOL_TABLE = NULL;

    readLabels(argv[1]);
    readInstructions(argv[1], argv[2]);


    free(SYMBOL_TABLE);

}

void readLabels(char* readfile) {
    // Reads all jump labels into the symbol table for use in assembling jump instructions

    FILE* asmFile;

    if(!(asmFile = fopen(readfile, "r"))) {

        printf("File %s does not exist.\n", readfile);
        printf(USAGE);
        exit(-1);

    }

    char* line = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    while(fgets(line, MAX_INSTRUCTION_LEN, asmFile)) {

        if(isBlankLineOrComment(line)) continue;

        if(isLabel(line)) {

            trimLabelColon(line);

            Label l;
            l.labelName = strndup(line, MAX_INSTRUCTION_LEN);
            l.PCAddress = INSTRUCTION_ADDR + 2;

            SYMBOL_TABLE = realloc(SYMBOL_TABLE, (SYMBOL_COUNT + 1) * sizeof(Label));

            SYMBOL_TABLE[SYMBOL_COUNT] = l;
            
            SYMBOL_COUNT++;

        } else INSTRUCTION_ADDR += 2;

    }

    fclose(asmFile);
    free(line);

}

void readInstructions(char* readfile, char* writefile) {
    // Reads all instructions from the given file and interprets them

    FILE* asmFile;
    FILE* binFile;

    if(!(asmFile = fopen(readfile, "r"))) {

        printf("File %s does not exist.\n", readfile);
        printf(USAGE);
        exit(-1);

    }

    if(!(binFile = fopen(writefile, "wb"))) {

        printf("Cannot output to file %s.\n", writefile);
        printf(USAGE);
        exit(-1);

    }

    char* instruction = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    while(fgets(instruction, MAX_INSTRUCTION_LEN, asmFile)) {

        bool skipLine = false;

        if(isBlankLineOrComment(instruction) || isLabel(instruction)) skipLine = true;
        // Skip line breaks and comments

        if(!skipLine) {
            
            int lineBreakIndex = strnlen(instruction, MAX_INSTRUCTION_LEN) - 1;
            if(instruction[lineBreakIndex] == '\n') instruction[lineBreakIndex] = '\0';
            // Remove any trailing line breaks from the instruction

            unsigned int buffer = assembleInstruction(instruction);
            // unsigned char* instructionToPrint = getBinary(buffer, 32);

            printf("%.8X\n", buffer);

            fwrite(&buffer, sizeof(unsigned int), 1, binFile);

        }

        LINE_NUMBER++;

    }

    fclose(asmFile);
    fclose(binFile);
    free(instruction);

}

unsigned int assembleInstruction(char* instruction) {
    // Assembles all instruction types into their respective numeric values

    unsigned int instructionNum = 0;

    if((instructionNum = AType(instruction))) return instructionNum;
    else if((instructionNum = IType(instruction))) return instructionNum;
    else if((instructionNum = SType(instruction))) return instructionNum;

    else {

        printf("Invalid instruction at line %i\n", LINE_NUMBER);
        printf("Instruction: %s\n", instruction);

        exit(-1);

    }

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

        printf("Incorrect number of arguments at line %i\n", LINE_NUMBER);
        printf("Instruction: %s\n", instruction);
        exit(-1);

    }

    for(int arg = 1; arg <= 3; arg++) {
        
        if(!fitsRegisterSyntax(getWord(instruction, arg))) {

            printf("Wrong format of argument %i at line %i\n", arg, LINE_NUMBER);
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

        printf("Incorrect number of arguments at line %i\n", LINE_NUMBER);
        printf("Instruction: %s\n", instruction);
        exit(-1);

    }

    for(int arg = 1; arg <= 3; arg++) {
        
        if((arg != 3 && !fitsRegisterSyntax(getWord(instruction, arg)))
            || (arg == 3 && !fitsImmediateSyntax(getWord(instruction, arg)))) {

            printf("Wrong format of argument %i at line %i\n", arg, LINE_NUMBER);
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

        printf("Incorrect number of arguments at line %i\n", LINE_NUMBER);
        printf("Instruction: %s\n", instruction);
        exit(-1);

    }

    for(int arg = 1; arg <= 2; arg++) {
        
        if((arg == 1 && !fitsRegisterSyntax(getWord(instruction, arg)))
            || (arg == 2 && !immediateMode && !fitsRegisterSyntax(getWord(instruction, arg)))
            || (arg == 2 && immediateMode && !fitsImmediateSyntax(getWord(instruction, arg)))) {

            printf("Wrong format of argument %i at line %i\n", arg, LINE_NUMBER);
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

    unsigned int regNum = strtol(str + 1, NULL, 10);
    if(regNum > 15) return false;

    return true;

}

bool fitsImmediateSyntax(char* str) {
    // Checks if a given string fits the SMIS immediate standard syntax "#<16-bit unsigned int>"

    if(*str != '#') return false;

    if(!containsOnlyNums(str + 1)) return false;

    unsigned int immVal = strtol(str + 1, NULL, 10);
    if(immVal > INT_LIMIT) return false;

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

    // TODO: Make this function safer

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

char* getBinary(unsigned int n, int length) {
    // Returns a given int in binary format

    char* binary = malloc(n * sizeof(char));

    for(int i = 0; i < length; i++) binary[length - (i + 1)] = binaryChar((n & 1 << i) >> i);

    return binary;

}

unsigned char binaryChar(unsigned int n) {
    // Converts a 0 or 1 into '0' or '1' respectively

    if(n == 0) return '0';
    else if(n == 1) return '1';
    else {

        printf("Internal error: cannot get binary char equivalent for digit %i\n", n);
        exit(-2);

    }

}

bool isBlankLineOrComment(char* str) {
    // Checks a line of the ASM file to see if it should be skipped

    if(!strncmp(str, "\n", 2) || !strncmp(str, "//", 2)) return true;

    return false;

}

bool isLabel(char* str) {
    // Checks if a given line ends with a ':', denoting that it is a jump label

    trimLineBreak(str);
    int len = strnlen(str, MAX_STRING_LEN) - 1;

    return str[len] == ':';

}

void trimLineBreak(char* str) {
    // Trims a trailing line break from a string

    trimChar(str, '\n');

}

void trimLabelColon(char* str) {
    // Trims a trailing colon from a string

    trimChar(str, ':');

}

void trimChar(char* str, char c) {
    // Trims the first instance of a given character from the end of a string
    // If the string does not contain the character, it remains unchanged

    int len = strnlen(str, MAX_STRING_LEN);

    for(int i = len; i >= 0; i--) {
        
        if(str[i] == c) {
            
            str[i] = '\0';
            return;

        }

    }

}