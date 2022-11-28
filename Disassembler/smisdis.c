#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>


#define USAGE "Usage: ./smisdis <input .bin executable file> <output .txt ASM file>\n"
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

#define OP_JUMP 30
#define OP_JUMP_IF_ZERO 31
#define OP_JUMP_IF_NOTZERO 32
#define OP_JUMP_LINK 33

#define OP_HALT 34


typedef struct Label {

    char* labelName;
    unsigned short int PCAddress;

} Label;


Label* SYMBOL_TABLE;
// Stores all labels in the assembled file
unsigned int SYMBOL_COUNT = 0;
// Stores the amount of symbols to avoid iterating over unallocated pointers

unsigned short int INSTRUCTION_ADDR = 0;
// Instruction address is stored for symbol table usage


void createLabels(char* readfile);
void readInstructions(char* readfile, char* writefile);
// char* disassembleInstruction(unsigned int instruction);
char* RType(unsigned int instruction);
char* IType(unsigned int instruction);
char* JType(unsigned int instruction);

bool labelExists(unsigned short int addr);
unsigned short int getOpcode(unsigned int instruction);
char* formatRegNum(unsigned short int regNum);
char* formatImmediateVal(unsigned short int immVal);
unsigned short int getRegOperand(unsigned int instruction, unsigned short int opNum);
unsigned short int getDestOrImmVal(unsigned int instruction);
char* getLabelName(unsigned short int addr);
char* generateLabelName(unsigned short int labelNum);
bool isJump(unsigned int instruction);
bool endsWith(char* str, char* substr);
void trimLabelColon(char* str);
void trimChar(char* str, char c);


int main(int argc, char** argv) {

    if(argc != 3) {

        printf("Incorrect number of arguments supplied.\n");
        printf(USAGE);
        exit(-1);

    }

    if(!endsWith(argv[1], ".bin") || !endsWith(argv[2], ".txt")) {

        printf("One or both of the supplied files have incorrect extensions.\n");
        printf(USAGE);
        exit(-1);

    }

    SYMBOL_TABLE = NULL;

    createLabels(argv[1]);
    readInstructions(argv[1], argv[2]);

    free(SYMBOL_TABLE);
    
}

void createLabels(char* readfile) {

    FILE* binFile;

    if(!(binFile = fopen(readfile, "rb"))) {

        printf("File %s does not exist.\n", readfile);
        printf(USAGE);
        exit(-1);

    }

    unsigned int instruction;
    
    while(fread(&instruction, 4, 1, binFile)) {

        instruction = ntohl(instruction);
        printf("%.8X", instruction);
        
        unsigned short int addr = getDestOrImmVal(instruction);

        if(isJump(instruction)) {
            
            printf(" <-- This is a JUMP");

            if(!labelExists(addr)) {

                printf(", name of %s", generateLabelName(SYMBOL_COUNT));

                Label l;
                l.labelName = generateLabelName(SYMBOL_COUNT);
                l.PCAddress = addr;

                SYMBOL_TABLE = realloc(SYMBOL_TABLE, (SYMBOL_COUNT + 1) * sizeof(Label));

                SYMBOL_TABLE[SYMBOL_COUNT] = l;
                
                SYMBOL_COUNT++;

            }

        }

        printf("\n");

    }

    printf("\n");

    fclose(binFile);

}

void readInstructions(char* readfile, char* writefile) {

    FILE* binFile;

    if(!(binFile = fopen(readfile, "rb"))) {

        printf("File %s does not exist.\n", readfile);
        printf(USAGE);
        exit(-1);

    }

    unsigned int instruction;
    
    while(fread(&instruction, 4, 1, binFile)) {

        if(labelExists(INSTRUCTION_ADDR)) {

            printf("\n%s\n", getLabelName(INSTRUCTION_ADDR));
            // TODO: Move this!

        }

        instruction = ntohl(instruction);

        printf("%s", RType(instruction));
        printf("%s", IType(instruction));
        printf("%s", JType(instruction));

        printf("\n");

        INSTRUCTION_ADDR += 2;

    }

    fclose(binFile);

}

// char* disassembleInstruction(unsigned int instruction) {
//     // Gets the corresponding line of code for a given instruction

    

// }

char* RType(unsigned int instruction) {
    // Converts an R-Type instruction to a string
    // If the given instruction is not a valid R-Type, returns an empty string

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    unsigned short int opcode = getOpcode(instruction);
    char* opStr;

    unsigned short int amountOfRegOperands = 3;
    // Default number register operands is 3, COPY, COMPARE, and NOT only have 2

    switch(opcode) {

        case OP_COPY:
            opStr = "COPY";
            amountOfRegOperands = 2;
            break;
            
        case OP_ADD:
            opStr = "ADD"; break;
        case OP_SUBTRACT:
            opStr = "SUBTRACT"; break;
        case OP_MULTIPLY:
            opStr = "MULTIPLY"; break;
        case OP_DIVIDE:
            opStr = "DIVIDE"; break;

        case OP_COMPARE:
            opStr = "COMPARE";
            amountOfRegOperands = 2;
            break;

        case OP_SHIFT_LEFT:
            opStr = "SHIFT-LEFT"; break;
        case OP_SHIFT_RIGHT:
            opStr = "SHIFT-RIGHT"; break;
            
        case OP_AND:
            opStr = "AND"; break;
        case OP_OR:
            opStr = "OR"; break;
        case OP_XOR:
            opStr = "XOR"; break;
        case OP_NAND:
            opStr = "NAND"; break;
        case OP_NOR:
            opStr = "NOR"; break;
        case OP_NOT:
            opStr = "NOT";
            amountOfRegOperands = 2;
            break;

        default: return instructionStr;

    }

    if(amountOfRegOperands == 2) {

        snprintf(instructionStr, MAX_INSTRUCTION_LEN, "%s %s %s", opStr,
        formatRegNum(getRegOperand(instruction, 1)), formatRegNum(getRegOperand(instruction, 2)));

    } else if(amountOfRegOperands == 3) {

        snprintf(instructionStr, MAX_INSTRUCTION_LEN, "%s %s %s %s", opStr,
        formatRegNum(getRegOperand(instruction, 1)), formatRegNum(getRegOperand(instruction, 2)),
        formatRegNum(getRegOperand(instruction, 3)));

    }

    return instructionStr;

}

char* IType(unsigned int instruction) {
    // Converts an I-Type instruction to a string
    // If the given instruction is not a valid I-Type, returns an empty string

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    unsigned short int opcode = getOpcode(instruction);
    char* opStr;

    unsigned short int amountOfRegOperands = 2;
    // Default number register operands is 2, SET only has 1

    switch(opcode) {

        case OP_SET:
            opStr = "SET";
            amountOfRegOperands = 1;
            break;

        case OP_ADD_IMM:
            opStr = "ADD-IMM"; break;
        case OP_SUBTRACT_IMM:
            opStr = "SUBTRACT-IMM"; break;
        case OP_MULTIPLY_IMM:
            opStr = "MULTIPLY-IMM"; break;
        case OP_DIVIDE_IMM:
            opStr = "DIVIDE-IMM"; break;

        case OP_COMPARE_IMM:
            opStr = "COMPARE-IMM"; break;
            
        case OP_SHIFT_LEFT_IMM:
            opStr = "SHIFT-LEFT-IMM"; break;
        case OP_SHIFT_RIGHT_IMM:
            opStr = "SHIFT-RIGHT-IMM"; break;

        case OP_AND_IMM:
            opStr = "AND-IMM"; break;
        case OP_OR_IMM:
            opStr = "OR-IMM"; break;
        case OP_XOR_IMM:
            opStr = "XOR-IMM"; break;
        case OP_NAND_IMM:
            opStr = "NAND-IMM"; break;
        case OP_NOR_IMM:
            opStr = "NOR-IMM"; break;

        case OP_LOAD:
            opStr = "LOAD"; break;
        case OP_STORE:
            opStr = "STORE"; break;

        case OP_HALT:
            instructionStr = "HALT";
            return instructionStr;

        default: return instructionStr;

    }

    if(amountOfRegOperands == 1) {

        snprintf(instructionStr, MAX_INSTRUCTION_LEN, "%s %s %s", opStr,
        formatRegNum(getRegOperand(instruction, 1)),
        formatImmediateVal(getDestOrImmVal(instruction)));

    } else if(amountOfRegOperands == 2) {

        snprintf(instructionStr, MAX_INSTRUCTION_LEN, "%s %s %s %s", opStr,
        formatRegNum(getRegOperand(instruction, 1)), formatRegNum(getRegOperand(instruction, 2)),
        formatImmediateVal(getDestOrImmVal(instruction)));

    }

    return instructionStr;

}

char* JType(unsigned int instruction) {
    // Converts a J-Type instruction to a string
    // If the given instruction is not a valid J-Type, returns an empty string

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    unsigned short int opcode = getOpcode(instruction);
    char* opStr;

    switch(opcode) {

        case OP_JUMP:
            opStr = "JUMP"; break;
        case OP_JUMP_IF_ZERO:
            opStr = "JUMP-IF-ZERO"; break;
        case OP_JUMP_IF_NOTZERO:
            opStr = "JUMP-IF-NOTZERO"; break;
        case OP_JUMP_LINK:
            opStr = "JUMP-LINK"; break;

        default: return instructionStr;

    }

    char* lblStr = getLabelName(getDestOrImmVal(instruction));
    trimLabelColon(lblStr);

    snprintf(instructionStr, MAX_INSTRUCTION_LEN, "%s %s", opStr, lblStr);

    return instructionStr;

}

bool labelExists(unsigned short int addr) {
    // Returns true if a label already exists in the symbol table

    for(int i = 0; i < SYMBOL_COUNT; i++) {

        Label l = SYMBOL_TABLE[i];

        // printf("Requested address: %.4X\nLabel name: %s\nLabel address: %.4X\n", addr, l.labelName, l.PCAddress);

        if(addr == l.PCAddress) return true;

    }

    return false;

}

unsigned short int getOpcode(unsigned int instruction) {
    // Gets the opcode of a given instruction

    return instruction >> 24;

}

char* formatRegNum(unsigned short int regNum) {
    // Translates a register from numerical form to string form

    char* regStr = malloc(4 * sizeof(char));

    switch(regNum) {

        case 0:
            snprintf(regStr, 4, "RZR"); break;
        case 15:
            snprintf(regStr, 4, "RSP"); break;
        case 14:
            snprintf(regStr, 4, "RBP"); break;
        case 13:
            snprintf(regStr, 4, "RLR"); break;
        default:
            snprintf(regStr, 4, "R%i", regNum); break;

    }

    return regStr;

}

char* formatImmediateVal(unsigned short int immVal) {
    // Translates a numerical immediate value to a string starting with #

    char* immStr = malloc(7 * sizeof(char));
    // Max length is 7 because the largest immediate value is 65535, which is 5 digits - plus 2 for '#' and '\0'
    snprintf(immStr, 7, "#%i", immVal);

    return immStr;

}

unsigned short int getRegOperand(unsigned int instruction, unsigned short int opNum) {
    // Gets the first operand of a given instruction

    opNum--;

    if(opNum > 2) {

        printf("Internal error: cannot retrieve register operand %i\n", opNum + 1);
        exit(-2);

    }

    return (instruction & (0x00F00000 >> (4 * opNum))) >> (20 - (4 * opNum));
    // TODO: There is probably a much nicer way to do this, but it works

}

unsigned short int getDestOrImmVal(unsigned int instruction) {
    // Gets the destination address of a J-Type instruction or immediate value of an I-Type instruction

    return instruction & 0xFFFF;

}

char* getLabelName(unsigned short int addr) {
    // Gets the label name associated with a given address

    for(int i = 0; i < SYMBOL_COUNT; i++) {

        Label l = SYMBOL_TABLE[i];

        if(addr == l.PCAddress) return l.labelName;

    }

    printf("Internal error: cannot find label for address %.4X in symbol table\n", addr);
    exit(-2);

}

char* generateLabelName(unsigned short int labelNum) {
    // Generates a generic label name with a given number

    char* name = malloc(14 * sizeof(char));
    snprintf(name, 14, "Label_%i:", labelNum);

    return name;

}

bool isJump(unsigned int instruction) {
    // Returns true if a given instruction is J-Type

    unsigned short int opcode = instruction >> 24;

    return opcode >= OP_JUMP && opcode <= OP_JUMP_LINK;

}

bool endsWith(char* str, char* substr) {
    // Checks if a given string ends with a given substring

    int strlen = strnlen(str, MAX_STRING_LEN);
    int substrlen = strnlen(substr, MAX_STRING_LEN);

    str += (strlen - substrlen);

    return !strncmp(str, substr, MAX_STRING_LEN);

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