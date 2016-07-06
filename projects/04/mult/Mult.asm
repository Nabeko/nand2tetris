// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.
	// Set R2 to be zero
	@R2
	M=0
	// Check the R0 and R1 is zero or not, if yes, goto ZERO
	@R0
	D=M
	@ZERO
	D;JEQ
	@R1
	D=M
	@ZERO
	D;JEQ

	// use R0 as an increment, R1 as times and R2 to store sum 
(LOOP)
	// if ( R1 == 0 ) goto END
	@R1
	D=M
	@END
	D;JEQ
	// R2 = R2 + R0
	@R0
	D=M
	@R2
	M=D+M
	// R1--
	@R1
	M=M-1
	@LOOP
	0;JMP

(ZERO)
	@R2
	M=0

(END)
	@END
	0;JMP
