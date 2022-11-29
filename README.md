SMIS is a project I'm working on that I hope can serve as a learning tool both for me and for others.


What does SMIS mean, anyway?
Well, it is short for "Simple Monolength Instruction Set," and it was designed from the ground up to be a pseudo-architecture/assembly language for beginners in hardware/software interface studies.


How does it accomplish this goal?

	1. It avoids most of the complexity that comes with implementing the x86 instruction set (because of both the number of instructions, and the complex nature of each)
	2. It avoids the occasionally cryptic names of the ARM instruction set (LDURSB etc.), and provides a larger number of basic instructions so as to avoid the need for highly-verbose code
	3. All instructions are named in a clear way (I'm looking at you, x86)
	4. All instructions are the same length (32 bits)
	5. All opcodes are the same length (8 bits) and are just numbers with no special meaning to any of the individual bits, as in ARM
	6. The syntax is strict and highly consistent, making it simpler to learn and simpler to interpret others' code


For now the SMIS toolsuite includes an assembler and a disassembler for SMIS ASM. If you are unfamiliar with these terms, an assembler takes ASM code and converts it into machine code, and a disassembler
takes machine code and converts it back to ASM. The next step is to write a CPU emulator to run the code on, which I will do first in C, and then later in Logisim, if I have the time.


To start writing in SMIS, simply download the assembler (smisasm) and disassembler (smisdis) executables from the repo.

Then, once you write your code in a .txt file, you can assemble it into a .bin file by typing "./smisasm <your asm file.txt> <target output file.bin>". This should work in most Linux distributions that use Bash.

If you want to disassemble a file, use "./smisdis <your executable.bin> <target output file.txt>".


If you need any help, you may check the documentation PDF, or contact me through Github.
