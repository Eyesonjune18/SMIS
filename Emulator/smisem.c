#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>


#define USAGE "Usage: ./smisem <executable .bin file>\n"
#define MAX_STRING_LEN 500

#define MEM MEMORY
#define REG REGISTERS
#define RZR REGISTERS[0x0]
#define RSP REGISTERS[0xF]
#define RBP REGISTERS[0xE]
#define RLR REGISTERS[0xD]

#define PC PROGRAM_COUNTER
#define IR INSTRUCTION_REGISTER

#define ZF ZERO_FLAG
#define SF SIGN_FLAG

#define OP_SET              1
#define OP_COPY             2

#define OP_ADD              3
#define OP_SUBTRACT         4
#define OP_MULTIPLY         5
#define OP_DIVIDE           6
#define OP_MODULO           7

#define OP_COMPARE          8

#define OP_SHIFT_LEFT       9
#define OP_SHIFT_RIGHT      10

#define OP_AND              11
#define OP_OR               12
#define OP_XOR              13
#define OP_NAND             14
#define OP_NOR              15
#define OP_NOT              16

#define OP_ADD_IMM          17
#define OP_SUBTRACT_IMM     18
#define OP_MULTIPLY_IMM     19
#define OP_DIVIDE_IMM       20
#define OP_MODULO_IMM       21

#define OP_COMPARE_IMM      22
#define OP_SHIFT_LEFT_IMM   23
#define OP_SHIFT_RIGHT_IMM  24
#define OP_AND_IMM          25
#define OP_OR_IMM           26
#define OP_XOR_IMM          27
#define OP_NAND_IMM         28
#define OP_NOR_IMM          29

#define OP_LOAD             30
#define OP_STORE            31

#define OP_JUMP             32
#define OP_JUMP_IF_ZERO     33
#define OP_JUMP_IF_NOTZERO  34
#define OP_JUMP_LINK        35

#define OP_HALT             36


uint16_t MEMORY[0xFFFF];
uint16_t REGISTERS[0xF];

uint16_t PROGRAM_COUNTER = 0;
uint32_t INSTRUCTION_REGISTER = 0;

bool ZERO_FLAG = false;
bool SIGN_FLAG = false;


void loadProgram(char* binfile);
void executeProgram();
void executeInstruction();
void grabNextInstruction();
// Program control functions

void setFlags(uint16_t result);

bool RType(uint32_t instruction);
bool IType(uint32_t instruction);
bool JType(uint32_t instruction);

void SET(uint8_t rDest, uint16_t iVal);
void COPY(uint8_t rDest, uint8_t rSrc);

void ADD(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void SUBTRACT(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void MULTIPLY(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void DIVIDE(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void MODULO(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);

void COMPARE(uint8_t rOp1, uint8_t rOp2);

void SHIFT_LEFT(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void SHIFT_RIGHT(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);

void AND(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void OR(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void XOR(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void NAND(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void NOR(uint8_t rDest, uint8_t rOp1, uint8_t rOp2);
void NOT(uint8_t rDest, uint8_t rOp);

void ADD_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void SUBTRACT_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void MULTIPLY_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void DIVIDE_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void MODULO_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);

void COMPARE_IMM(uint8_t rOp1, uint16_t iOp2);

void SHIFT_LEFT_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void SHIFT_RIGHT_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);

void AND_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void OR_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void XOR_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void NAND_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);
void NOR_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2);

void LOAD(uint8_t rDest, uint8_t rBase, uint16_t iOffset);
void STORE(uint8_t rSrc, uint8_t rBase, uint16_t iOffset);

void JUMP(uint16_t destAddr);
void JUMP_IF_ZERO(uint16_t destAddr);
void JUMP_IF_NOTZERO(uint16_t destAddr);
void JUMP_LINK(uint16_t destAddr);

void HALT();
// Instruction execution functions

uint8_t getOpcode(uint32_t instruction);
uint16_t getInstructionHalf1(uint32_t instruction);
uint16_t getInstructionHalf2(uint32_t instruction);
uint8_t getRegOperand(uint32_t instruction, uint8_t opNum);
uint16_t getDestOrImmVal(uint32_t instruction);
// Emulator utility functions

bool endsWith(char* str, char* substr);
// General utility functions


int main(int argc, char** argv) {

    if(argc != 2) {

        printf("Incorrect number of arguments supplied.\n");
        printf(USAGE);
        exit(-1);

    }

    if(!endsWith(argv[1], ".bin")) {

        printf("The supplied file does not have the correct extension.\n");
        printf(USAGE);
        exit(-1);

    }

    loadProgram(argv[1]);
    executeProgram();
    
}

void loadProgram(char* binfile) {
    // Reads the binary file and places it in the memory array

    FILE* program;

    if(!(program = fopen(binfile, "rb"))) {

        printf("File %s does not exist.\n", binfile);
        printf(USAGE);
        exit(-1);

    }

    uint32_t instruction;

    uint16_t storeAddr = 0;
    
    while(fread(&instruction, 4, 1, program)) {

        instruction = ntohl(instruction);

        MEM[storeAddr] = getInstructionHalf1(instruction);
        MEM[storeAddr + 1] = getInstructionHalf2(instruction);
        // Split the instruction into two 16-bit segments to put in memory

        storeAddr += 2;

    }
    
    MEM[storeAddr] = OP_HALT << 8;
    // Add a HALT to the end, in case the ASM programmer forgot to do so

    fclose(program);

}

void executeProgram() {
    // Calls each instruction in the program until reaching a HALT signal

    do {

        grabNextInstruction();
        PC += 2;
        // PC is incremented prior to executing instruction so it does not interfere with J-Type instructions
        executeInstruction();

        RZR = 0x0000;

    } while(IR != 0x00000000);

}

void executeInstruction() {
    // Executes the instruction held in the instruction register

    if(RType(IR)) return;
    else if(IType(IR)) return;
    else if(JType(IR)) return;

    printf("Unknown instruction 0x%.8X at PC address 0x%.4X\n", IR, PC);
    exit(-1);

}

void grabNextInstruction() {
    // Gets the next instruction from memory and places it in the instruction register

    IR = 0;

    IR ^= MEM[PC] << 16;
    IR ^= MEM[PC + 1];

}

void setFlags(uint16_t result) {
    // Sets flags according to the given value, usually the result of an arithmetic operation

    if(result == 0x0000) ZF = true;
    else ZF = false;

    if(result >> 15 == 0x1) SF = true;
    else SF = false;

}

bool RType(uint32_t instruction) {
    // Executes a given R-Type instruction
    // Returns true if the instruction is valid for R-Type, false if it is invalid

    uint8_t opcode = getOpcode(IR);

    uint8_t rDest = getRegOperand(IR, 1);
    uint8_t rOp1 = getRegOperand(IR, 2);
    uint8_t rOp2 = getRegOperand(IR, 3);

    switch(opcode) {

        case OP_COPY: COPY(rDest, rOp1); break;

        case OP_ADD: ADD(rDest, rOp1, rOp2); break;
        case OP_SUBTRACT: SUBTRACT(rDest, rOp1, rOp2); break;
        case OP_MULTIPLY: MULTIPLY(rDest, rOp1, rOp2); break;
        case OP_DIVIDE: DIVIDE(rDest, rOp1, rOp2); break;
        case OP_MODULO: MODULO(rDest, rOp1, rOp2); break;

        case OP_COMPARE: COMPARE(rOp1, rOp2); break;

        case OP_SHIFT_LEFT: SHIFT_LEFT(rDest, rOp1, rOp2); break;
        case OP_SHIFT_RIGHT: SHIFT_RIGHT(rDest, rOp1, rOp2); break;
            
        case OP_AND: AND(rDest, rOp1, rOp2); break;
        case OP_OR: OR(rDest, rOp1, rOp2); break;
        case OP_XOR: XOR(rDest, rOp1, rOp2); break;
        case OP_NAND: NAND(rDest, rOp1, rOp2); break;
        case OP_NOR: NOR(rDest, rOp1, rOp2); break;
        case OP_NOT: NOT(rDest, rOp1); break;

        default: return false;

    }

    return true;

}

bool IType(uint32_t instruction) {
    // Executes a given I-Type instruction
    // Returns true if the instruction is valid for I-Type, false if it is invalid

    uint8_t opcode = getOpcode(IR);

    uint8_t rDest = getRegOperand(IR, 1);
    uint8_t rOp1 = getRegOperand(IR, 2);
    uint8_t iOp2 = getDestOrImmVal(IR);

    switch(opcode) {

        case OP_SET: SET(rDest, iOp2); break;
            
        case OP_ADD_IMM: ADD_IMM(rDest, rOp1, iOp2); break;
        case OP_SUBTRACT_IMM: SUBTRACT_IMM(rDest, rOp1, iOp2); break;
        case OP_MULTIPLY_IMM: MULTIPLY_IMM(rDest, rOp1, iOp2); break;
        case OP_DIVIDE_IMM: DIVIDE_IMM(rDest, rOp1, iOp2); break;
        case OP_MODULO_IMM: MODULO_IMM(rDest, rOp1, iOp2); break;

        case OP_COMPARE_IMM: COMPARE_IMM(rOp1, iOp2); break;

        case OP_SHIFT_LEFT_IMM: SHIFT_LEFT_IMM(rDest, rOp1, iOp2); break;
        case OP_SHIFT_RIGHT_IMM: SHIFT_RIGHT_IMM(rDest, rOp1, iOp2); break;

        case OP_AND_IMM: AND_IMM(rDest, rOp1, iOp2); break;
        case OP_OR_IMM: OR_IMM(rDest, rOp1, iOp2); break;
        case OP_XOR_IMM: XOR_IMM(rDest, rOp1, iOp2); break;
        case OP_NAND_IMM: NAND_IMM(rDest, rOp1, iOp2); break;
        case OP_NOR_IMM: NOR_IMM(rDest, rOp1, iOp2); break;

        case OP_LOAD: LOAD(rDest, rOp1, iOp2); break;
        case OP_STORE: STORE(rDest, rOp1, iOp2); break;

        default: return false;

    }

    return true;

}

bool JType(uint32_t instruction) {
    // Executes a given J-Type instruction
    // Returns true if the instruction is valid for J-Type, false if it is invalid

    uint8_t opcode = getOpcode(IR);

    uint8_t destAddr = getDestOrImmVal(IR);

    switch(opcode) {

        case OP_JUMP: JUMP(destAddr); break;
        case OP_JUMP_IF_ZERO: JUMP_IF_ZERO(destAddr); break;
        case OP_JUMP_IF_NOTZERO: JUMP_IF_NOTZERO(destAddr); break;
        case OP_JUMP_LINK: JUMP_LINK(destAddr); break;

        case OP_HALT: HALT(); break;

        default: return false;

    }

    return true;

}

void SET(uint8_t rDest, uint16_t iVal) {
    // Executes a SET instruction

    REG[rDest] = iVal;

    printf("SET\n");

}

void COPY(uint8_t rDest, uint8_t rSrc) {
    // Executes a COPY instruction

    REG[rDest] = REG[rSrc];

    printf("COPY\n");

}

void ADD(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes an ADD instruction

    REG[rDest] = REG[rOp1] + REG[rOp2];

    setFlags(REG[rDest]);

    printf("ADD\n");

}

void SUBTRACT(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a SUBTRACT instruction

    REG[rDest] = REG[rOp1] - REG[rOp2];

    setFlags(REG[rDest]);

    printf("SUBTRACT\n");

}

void MULTIPLY(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a MULTIPLY instruction

    REG[rDest] = REG[rOp1] * REG[rOp2];

    setFlags(REG[rDest]);

    printf("MULTIPLY\n");

}

void DIVIDE(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a DIVIDE instruction

    REG[rDest] = REG[rOp1] / REG[rOp2];

    setFlags(REG[rDest]);

    printf("DIVIDE\n");

}

void MODULO(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a MODULO instruction

    REG[rDest] = REG[rOp1] % REG[rOp2];

    setFlags(REG[rDest]);

    printf("MODULO\n");

}

void COMPARE(uint8_t rOp1, uint8_t rOp2) {
    // Executes a COMPARE instruction

    uint16_t throwawayVal = REG[rOp1] + REG[rOp2];

    setFlags(throwawayVal);

    printf("COMPARE\n");

}

void SHIFT_LEFT(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a SHIFT-LEFT instruction

    REG[rDest] = REG[rOp1] << REG[rOp2];

    setFlags(REG[rDest]);

    printf("SHIFT-LEFT\n");

}

void SHIFT_RIGHT(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a SHIFT-RIGHT instruction

    REG[rDest] = REG[rOp1] >> REG[rOp2];

    setFlags(REG[rDest]);

    printf("SHIFT-RIGHT\n");

}

void AND(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes an AND instruction

    REG[rDest] = REG[rOp1] & REG[rOp2];

    setFlags(REG[rDest]);

    printf("AND\n");

}

void OR(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes an OR instruction

    REG[rDest] = REG[rOp1] | REG[rOp2];

    setFlags(REG[rDest]);

    printf("OR\n");

}

void XOR(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes an XOR instruction

    REG[rDest] = REG[rOp1] ^ REG[rOp2];

    setFlags(REG[rDest]);

    printf("XOR\n");

}

void NAND(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a NAND instruction

    REG[rDest] = ~(REG[rOp1] & REG[rOp2]);

    setFlags(REG[rDest]);

    printf("NAND\n");

}

void NOR(uint8_t rDest, uint8_t rOp1, uint8_t rOp2) {
    // Executes a NOR instruction

    REG[rDest] = ~(REG[rOp1] | REG[rOp2]);

    setFlags(REG[rDest]);

    printf("NOR\n");

}

void NOT(uint8_t rDest, uint8_t rOp) {
    // Executes a NOT instruction

    REG[rDest] = ~REG[rOp];

    setFlags(REG[rDest]);

    printf("NOT\n");

}

void ADD_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes an ADD-IMM instruction

    REG[rDest] = REG[rOp1] + iOp2;

    setFlags(REG[rDest]);

    printf("ADD-IMM result %i\n", REG[rDest]);

}

void SUBTRACT_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes a SUBTRACT-IMM instruction

    REG[rDest] = REG[rOp1] - iOp2;

    setFlags(REG[rDest]);

    printf("SUBTRACT-IMM\n");

}

void MULTIPLY_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes a MULTIPLY-IMM instruction

    REG[rDest] = REG[rOp1] * iOp2;

    setFlags(REG[rDest]);

    printf("MULTIPLY-IMM\n");

}

void DIVIDE_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes a DIVIDE-IMM instruction

    REG[rDest] = REG[rOp1] / iOp2;

    setFlags(REG[rDest]);

    printf("DIVIDE-IMM\n");

}

void MODULO_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes a MODULO-IMM instruction

    REG[rDest] = REG[rOp1] % iOp2;

    setFlags(REG[rDest]);

    printf("MODULO-IMM\n");

}

void COMPARE_IMM(uint8_t rOp1, uint16_t iOp2) {
    // Executes a COMPARE-IMM instruction

    uint16_t throwawayVal = REG[rOp1] - iOp2;

    setFlags(throwawayVal);

    printf("COMPARE-IMM\n");

}

void SHIFT_LEFT_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes a SHIFT-LEFT-IMM instruction

    REG[rDest] = REG[rOp1] << iOp2;

    setFlags(REG[rDest]);

    printf("SHIFT-LEFT-IMM\n");

}

void SHIFT_RIGHT_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes a SHIFT-RIGHT-IMM instruction

    REG[rDest] = REG[rOp1] >> iOp2;

    setFlags(REG[rDest]);

    printf("SHIFT-RIGHT-IMM\n");

}

void AND_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes an AND-IMM instruction

    REG[rDest] = REG[rOp1] & iOp2;

    setFlags(REG[rDest]);

    printf("AND-IMM\n");

}

void OR_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes an OR-IMM instruction

    REG[rDest] = REG[rOp1] | iOp2;

    setFlags(REG[rDest]);

    printf("OR-IMM\n");

}

void XOR_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes an XOR-IMM instruction

    REG[rDest] = REG[rOp1] ^ iOp2;

    setFlags(REG[rDest]);

    printf("XOR-IMM\n");

}

void NAND_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes a NAND-IMM instruction

    REG[rDest] = ~(REG[rOp1] & iOp2);

    setFlags(REG[rDest]);

    printf("NAND-IMM\n");

}

void NOR_IMM(uint8_t rDest, uint8_t rOp1, uint16_t iOp2) {
    // Executes A NOR-IMM instruction

    REG[rDest] = ~(REG[rOp1] | iOp2);

    setFlags(REG[rDest]);

    printf("NOR-IMM\n");

}

void LOAD(uint8_t rDest, uint8_t rBase, uint16_t iOffset) {
    // Executes a LOAD instruction

    REG[rDest] = MEM[REG[rBase] + iOffset];

    printf("LOAD\n");

}

void STORE(uint8_t rSrc, uint8_t rBase, uint16_t iOffset) {
    // Executes a STORE instruction

    MEM[REG[rBase] + iOffset] = REG[rSrc];

    printf("STORE\n");

}

void JUMP(uint16_t destAddr) {
    // Executes a JUMP instruction

    PC = destAddr;

    printf("JUMP\n");

}

void JUMP_IF_ZERO(uint16_t destAddr) {
    // Executes a JUMP-IF-ZERO instruction

    if(ZF) PC = destAddr;

    printf("JUMP-IF-ZERO\n");

}

void JUMP_IF_NOTZERO(uint16_t destAddr) {
    // Executes a JUMP-IF-NOTZERO instruction

    if(!ZF) PC = destAddr;

    printf("JUMP-IF-NOTZERO\n");

}

void JUMP_LINK(uint16_t destAddr) {
    // Executes a JUMP-LINK instruction

    RLR = PC;
    PC = destAddr;

    printf("JUMP-LINK\n");

}

void HALT() {
    // Executes a HALT instruction

    printf("HALT\n");

    exit(0);

}

uint8_t getOpcode(uint32_t instruction) {
    // Gets the opcode of a given instruction

    return instruction >> 24;

}

uint16_t getInstructionHalf1(uint32_t instruction) {
    // Returns the 16 most significant bits of an instruction

    return (instruction & 0xFFFF0000) >> 16;

}

uint16_t getInstructionHalf2(uint32_t instruction) {
    // Returns the 16 least significant bits of an instruction

    return instruction & 0xFFFF;

}

uint8_t getRegOperand(uint32_t instruction, uint8_t opNum) {
    // Gets the first operand of a given instruction

    opNum--;

    if(opNum > 2) {

        printf("Internal error: cannot retrieve register operand %i at instruction %i\n", opNum + 1, PC);
        exit(-2);

    }

    return (instruction & (0x00F00000 >> (4 * opNum))) >> (20 - (4 * opNum));
    // TODO: There is probably a much nicer way to do this, but it works

}

uint16_t getDestOrImmVal(uint32_t instruction) {
    // Gets the destination address of a J-Type instruction or immediate value of an I-Type instruction

    return instruction & 0xFFFF;

}

bool endsWith(char* str, char* substr) {
    // Checks if a given string ends with a given substring

    int strlen = strnlen(str, MAX_STRING_LEN);
    int substrlen = strnlen(substr, MAX_STRING_LEN);

    str += (strlen - substrlen);

    return !strncmp(str, substr, MAX_STRING_LEN);

}