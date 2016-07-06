// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, the
// program clears the screen, i.e. writes "white" in every pixel.

// Put your code here.
(MONITOR)
	// set pos to be SCREEN
	@SCREEN
	D=A
	@pos
	M=D

	@KBD
	// if no key has been preesed, then goto WHITE
	D=M
	@WHITE
	D;JEQ
	// else goto BLACK
	@BLACK
	0;JMP

(BLACK)
	@val
	M=-1 // All set be 1, so it is black
	@LOOP
	0;JMP
(WHITE)
	@val
	M=0 // All set be 0, so it is white
	@LOOP
	0;JMP

(LOOP)
	@SCREEN
	D=A
	@pos
	D=M-D
	// if pos is 8192, go back to MONITOR(SCREEN size is 8k word)
	@8192
	D=D-A
	@MONITOR
	D;JEQ

	// set val to RAM[pos]
	@val
	D=M
	@pos
	A=M
	M=D

	@pos
	M=M+1
	@LOOP
	0;JMP
