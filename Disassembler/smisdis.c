/*

SMIS ASM general-purpose disassembler

Documentation for the SMIS assembly language is hosted at https://github.com/Eyesonjune18/SMIS/blob/main/Documentation/SMIS.pdf

Program overview:

    The disassembly work is done in two passes.

    (Setup) The input .bin machine code file and the output .txt ASM file are opened.

    (Pass 1)
        The machine code file is scanned for jump labels by reading J-Type instruction
        destination addresses. These addresses are placed into the symbol table, along with
        a generic label name. Each symbol represents a name and a target program counter address
        (to be checked against later for jump instructions).

    (Pass 2)
        Once the symbol table has been created, the second pass parses all instructions,
        including their operands, into the ASM file. Jump instruction addresses are checked
        against the symbol table, and if the label is found, they are disassembled into their
        corresponding label name. If a label does not exist, the file cannot be disassembled.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>


#define USAGE "Usage: ./smisdis <input .bin machine code file> <output .txt ASM file>\n"
#define MAX_INSTRUCTION_LEN 50
#define MAX_STRING_LEN 500
#define INT_LIMIT 65535
#define INSTRUCTION_NUMBER INSTRUCTION_ADDR / 2

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
    uint16_t PCAddress;

} Label;


Label* SYMBOL_TABLE;
// Stores all labels in the assembled file
uint32_t SYMBOL_COUNT = 0;
// Stores the amount of symbols to avoid iterating over unallocated pointers

uint16_t INSTRUCTION_ADDR = 0;
// Instruction address is stored for symbol table usage


void createLabels(char* readfile);
void readInstructions(char* readfile, char* writefile);
// Program control functions

char* disassembleInstruction(uint32_t instruction);
char* RType(uint32_t instruction);
char* IType(uint32_t instruction);
char* JType(uint32_t instruction);
// Instruction disassembly functions

char* formatRegNum(uint16_t regNum);
char* formatImmediateVal(uint16_t immVal);
bool labelExists(uint16_t addr);
uint8_t getOpcode(uint32_t instruction);
uint8_t getRegOperand(uint32_t instruction, uint8_t opNum);
uint16_t getDestOrImmVal(uint32_t instruction);
char* getLabelName(uint16_t addr);
char* generateLabelName(uint16_t labelNum);
bool isJump(uint32_t instruction);
// Disassembler utility functions

bool isEmpty(char* str);
bool endsWith(char* str, char* substr);
void addLineBreak(char* str);
void trimLabelColon(char* str);
void trimChar(char* str, char c);
// General utility functions


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

    uint32_t instruction;
    
    while(fread(&instruction, 4, 1, binFile)) {

        instruction = ntohl(instruction);
        
        uint16_t addr = getDestOrImmVal(instruction);

        if(isJump(instruction)) {
        

            if(!labelExists(addr)) {

                Label l;
                l.labelName = generateLabelName(SYMBOL_COUNT);
                l.PCAddress = addr;

                SYMBOL_TABLE = realloc(SYMBOL_TABLE, (SYMBOL_COUNT + 1) * sizeof(Label));

                SYMBOL_TABLE[SYMBOL_COUNT] = l;
                
                SYMBOL_COUNT++;

            }

        }

    }

    fclose(binFile);

}

void readInstructions(char* readfile, char* writefile) {

    FILE* binFile;
    FILE* txtFile;

    if(!(binFile = fopen(readfile, "rb"))) {

        printf("File %s does not exist.\n", readfile);
        printf(USAGE);
        exit(-1);

    }

    if(!(txtFile = fopen(writefile, "w"))) {

        printf("File %s does not exist.\n", writefile);
        printf(USAGE);
        exit(-1);

    }

    uint32_t instruction;
    
    while(fread(&instruction, 4, 1, binFile)) {

        instruction = ntohl(instruction);

        if(labelExists(INSTRUCTION_ADDR)) {

            if(INSTRUCTION_ADDR != 0) fputc('\n', txtFile);
            fprintf(txtFile, "%s\n", getLabelName(INSTRUCTION_ADDR));

        }

        fprintf(txtFile, "%s\n", disassembleInstruction(instruction));

        INSTRUCTION_ADDR += 2;

    }

    freopen(writefile, "r", txtFile);

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));
    while(fgets(instructionStr, MAX_INSTRUCTION_LEN, txtFile)) printf("%s", instructionStr);
    free(instructionStr);
    // TODO: Possible refactor into separate function

    fclose(binFile);
    fclose(txtFile);

}

char* disassembleInstruction(uint32_t instruction) {
    // Gets the corresponding line of code for a given instruction

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));

    char* rStr = RType(instruction);
    char* iStr = IType(instruction);
    char* jStr = JType(instruction);

    if(!isEmpty(rStr)) instructionStr = rStr;
    else if(!isEmpty(iStr)) instructionStr = iStr;
    else if(!isEmpty(jStr)) instructionStr = jStr;
    else {

        printf("Instruction %i did not match any known opcodes\n", INSTRUCTION_NUMBER);
        exit(-1);

    }

    return instructionStr;

}

char* RType(uint32_t instruction) {
    // Converts an R-Type instruction to a string
    // If the given instruction is not a valid R-Type, returns an empty string

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));
    *instructionStr = '\0';

    uint16_t opcode = getOpcode(instruction);
    char* opStr;

    uint16_t amountOfRegOperands = 3;
    // Default number register operands is 3, COPY, COMPARE, and NOT only have 2
    bool noDestRegAltMode = false;
    // For COMPARE, there is no destination register, and the registers are placed in RO1 and RO2 instead
    // This shifts the operand getter over 1

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
            noDestRegAltMode = true;
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
        formatRegNum(getRegOperand(instruction, 1 + noDestRegAltMode)), formatRegNum(getRegOperand(instruction, 2 + noDestRegAltMode)));

    } else if(amountOfRegOperands == 3) {

        snprintf(instructionStr, MAX_INSTRUCTION_LEN, "%s %s %s %s", opStr,
        formatRegNum(getRegOperand(instruction, 1)), formatRegNum(getRegOperand(instruction, 2)),
        formatRegNum(getRegOperand(instruction, 3)));

    }

    return instructionStr;

}

char* IType(uint32_t instruction) {
    // Converts an I-Type instruction to a string
    // If the given instruction is not a valid I-Type, returns an empty string

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));
    *instructionStr = '\0';

    uint16_t opcode = getOpcode(instruction);
    char* opStr;

    uint16_t amountOfRegOperands = 2;
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

char* JType(uint32_t instruction) {
    // Converts a J-Type instruction to a string
    // If the given instruction is not a valid J-Type, returns an empty string

    char* instructionStr = malloc(MAX_INSTRUCTION_LEN * sizeof(char));
    *instructionStr = '\0';

    uint16_t opcode = getOpcode(instruction);
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

        case OP_HALT:
            instructionStr = "HALT";
            return instructionStr;

        default: return instructionStr;

    }

    char* lblStr = getLabelName(getDestOrImmVal(instruction));
    trimLabelColon(lblStr);

    snprintf(instructionStr, MAX_INSTRUCTION_LEN, "%s %s", opStr, lblStr);

    return instructionStr;

}

char* formatRegNum(uint16_t regNum) {
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

char* formatImmediateVal(uint16_t immVal) {
    // Translates a numerical immediate value to a string starting with #

    char* immStr = malloc(7 * sizeof(char));
    // Max length is 7 because the largest immediate value is 65535, which is 5 digits - plus 2 for '#' and '\0'
    snprintf(immStr, 7, "#%i", immVal);

    return immStr;

}

bool labelExists(uint16_t addr) {
    // Returns true if a label already exists in the symbol table

    for(int i = 0; i < SYMBOL_COUNT; i++) {

        Label l = SYMBOL_TABLE[i];

        // printf("Requested address: %.4X\nLabel name: %s\nLabel address: %.4X\n", addr, l.labelName, l.PCAddress);

        if(addr == l.PCAddress) return true;

    }

    return false;

}

uint8_t getOpcode(uint32_t instruction) {
    // Gets the opcode of a given instruction

    return instruction >> 24;

}

uint8_t getRegOperand(uint32_t instruction, uint8_t opNum) {
    // Gets the first operand of a given instruction

    opNum--;

    if(opNum > 2) {

        printf("Internal error: cannot retrieve register operand %i at instruction %i\n", opNum + 1, INSTRUCTION_NUMBER);
        exit(-2);

    }

    return (instruction & (0x00F00000 >> (4 * opNum))) >> (20 - (4 * opNum));
    // TODO: There is probably a much nicer way to do this, but it works

}

uint16_t getDestOrImmVal(uint32_t instruction) {
    // Gets the destination address of a J-Type instruction or immediate value of an I-Type instruction

    return instruction & 0xFFFF;

}

char* getLabelName(uint16_t addr) {
    // Gets the label name associated with a given address

    for(int i = 0; i < SYMBOL_COUNT; i++) {

        Label l = SYMBOL_TABLE[i];

        if(addr == l.PCAddress) {
            
            char* lblName = malloc(MAX_INSTRUCTION_LEN * sizeof(char));
            strncpy(lblName, l.labelName, MAX_INSTRUCTION_LEN);
            
            return lblName;
        
        }

    }

    printf("Internal error: cannot find label for address %.4X in symbol table at instruction %i\n", addr, INSTRUCTION_NUMBER);
    exit(-2);

}

char* generateLabelName(uint16_t labelNum) {
    // Generates a generic label name with a given number

    char* name = malloc(14 * sizeof(char));
    snprintf(name, 14, "Label_%i:", labelNum);

    return name;

}

bool isJump(uint32_t instruction) {
    // Returns true if a given instruction is J-Type

    uint8_t opcode = getOpcode(instruction);

    return opcode >= OP_JUMP && opcode <= OP_JUMP_LINK;

}

bool isEmpty(char* str) {
    // Checks if a given string is empty (starts with null terminator)

    return *str == '\0';

}

bool endsWith(char* str, char* substr) {
    // Checks if a given string ends with a given substring

    int strlen = strnlen(str, MAX_STRING_LEN);
    int substrlen = strnlen(substr, MAX_STRING_LEN);

    str += (strlen - substrlen);

    return !strncmp(str, substr, MAX_STRING_LEN);

}

void addLineBreak(char* str) {
    // Adds a trailing line break to a given string

    int len = strnlen(str, MAX_STRING_LEN) + 2;
    
    str[len - 2] = '\n';
    str[len - 1] = '\0';

}

void trimLabelColon(char* str) {
    // Trims a trailing colon from a given string

    trimChar(str, ':');

}

void trimChar(char* str, char c) {
    // Trims the first instance of a given character from the end of a given string
    // If the string does not contain the character, it remains unchanged

    int len = strnlen(str, MAX_STRING_LEN);

    for(int i = len; i >= 0; i--) {
        
        if(str[i] == c) {
            
            str[i] = '\0';
            return;

        }

    }

}