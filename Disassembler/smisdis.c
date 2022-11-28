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

bool labelExists(unsigned short int addr);
unsigned short int getDestAddress(unsigned int instruction);
char* generateLabelName(unsigned short int labelNum);
bool isJump(unsigned int instruction);
bool endsWith(char* str, char* substr);


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
    // readInstructions(argv[1], argv[2]);

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
        
        unsigned short int addr = getDestAddress(instruction);

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

    fclose(binFile);

}

void readInstructions(char* readfile, char* writefile) {

    

}

bool labelExists(unsigned short int addr) {
    // Returns true if a label already exists in the symbol table

    for(int i = 0; i < SYMBOL_COUNT; i++) {

        Label l = SYMBOL_TABLE[i];

        if(addr == l.PCAddress) return true;

    }

    return false;

}

unsigned short int getDestAddress(unsigned int instruction) {
    // Gets the destination address of a J-Type instruction

    return instruction & 0x10;

}

char* generateLabelName(unsigned short int labelNum) {
    // Generates a generic label name with a given number

    char* name = malloc(14 * sizeof(char));
    snprintf(name, 14, "Label %i:", labelNum);

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