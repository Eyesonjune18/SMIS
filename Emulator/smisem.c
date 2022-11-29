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
#define PC PROGRAM_COUNTER
#define IR INSTRUCTION_REGISTER

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


uint16_t MEMORY[0xFFFF];
uint16_t REGISTERS[0xF];

uint16_t PROGRAM_COUNTER = 0;
uint32_t INSTRUCTION_REGISTER = 0;

void loadProgram(char* binfile);
void executeProgram();
void executeInstruction();
void grabNextInstruction();

uint16_t getInstructionHalf1(uint32_t instruction);
uint16_t getInstructionHalf2(uint32_t instruction);

bool endsWith(char* str, char* substr);


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

    // for(int i = 0; MEM[i]; i += 2) printf("%.4X%.4X\n", MEM[i], MEM[i + 1]);

    fclose(program);

}

void executeProgram() {
    // Calls each instruction in the program until reaching a HALT signal

    do {

        grabNextInstruction();
        PC += 2;
        executeInstruction();

    } while(IR);

}

void executeInstruction() {
    // Executes the instruction held in the instruction register



}

void grabNextInstruction() {
    // Gets the next instruction from memory and places it in the instruction register

    IR = 0;

    IR ^= MEM[PC] << 16;
    IR ^= MEM[PC + 1];

}

uint16_t getInstructionHalf1(uint32_t instruction) {
    // Returns the 16 most significant bits of an instruction

    return (instruction & 0xFFFF0000) >> 16;

}

uint16_t getInstructionHalf2(uint32_t instruction) {
    // Returns the 16 least significant bits of an instruction

    return instruction & 0xFFFF;

}

bool endsWith(char* str, char* substr) {
    // Checks if a given string ends with a given substring

    int strlen = strnlen(str, MAX_STRING_LEN);
    int substrlen = strnlen(substr, MAX_STRING_LEN);

    str += (strlen - substrlen);

    return !strncmp(str, substr, MAX_STRING_LEN);

}