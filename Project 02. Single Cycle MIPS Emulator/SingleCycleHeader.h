
#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

extern int Memory[0x00400000];
extern int Register[32];
extern unsigned int PC;
extern unsigned int binIndex;

unsigned int countInst;
unsigned int countR, countI, countJ;
unsigned int countMemoryAccess;
unsigned int TakenBranch;
int UserSelect;
clock_t start, end;
float TimeExcuted;

typedef struct {
	int InstData;
	char InstType;
} InstructionFormat;

typedef struct {
	int opcode;
	int rs;
	int rt;
	int rd;
	int imme;
	int funct;
	int shamt;
	int addr;
} InstructionInfomation;

typedef struct {
	int readReg1;
	int readReg2;
	int writeReg;
	int readData1;
	int readData2;
	int writeData;
} Registers;

typedef struct {
	int data1;
	int data2;
	bool zero;
	int result;
} ArithmeticLogicUnit;

typedef struct {
	int RegDst;
	int Jump;
	int BranchEqual;
	int BranchNotEqual;
	int ALUSrc1;
	int ALUSrc2;
	int ALUCtr;
	int ALUOp;
	int MemtoReg;
	int MemRead;
	int MemWrite;
	int RegWrite;
} ControlSignal;

typedef struct {
	int writeData;
	int readData;
	int address;
} DataMemory;

typedef struct {
	int jumpAddress;
	int branchAddress;
	int Sign;
	int Zero;
} Extend;

InstructionFormat		InstForm;
InstructionInfomation	InstInfo;
Registers				REG;
ArithmeticLogicUnit		ALU;
ControlSignal			CTR;
DataMemory				DataMem;
Extend					EXT;

void DoMemory(void);
void DoFetch(void);			// Instruction Fetch
void DoDecode(void);		// Instruction Decode
void DoControl(void);
void DoExecute(void);		// Execute
void DoRegister(void);
void DoALUControl(void);
void DoMemoryAccess(void);	// Memory Access
void DoWriteBack(void);		// Write Back
void SignExtend(void);
void ZeroExtend(void);

void MUX1(void);
void MUX2_1(void);
void MUX2_2(void);
void MUX3(void);
void MUX4(void);
void MUX5(void);
void JumpAddress(void);
void BranchAddress(void);

void BasicState(void);
void AdditionalState(void);