
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <string.h>

// Structures Declaration
typedef struct {
	int inst;
} InstructionMemory;

typedef struct {
	int opcode;
	int rs;
	int rt;
	int rd;
	int imm;
	int funct;
	int shamt;
	int addr;
} Instruction;

typedef struct {
	int readReg1;
	int readReg2;
	int writeReg;
	int readData1;
	int readData2;
	int writeData;
} RegisterFiles;

typedef struct {
	// control signals for execution stage
	int RegDst;
	int ALUOp;
	int ALUCtr;
	int ALUSrcA;
	int ALUSrcB;

	// for memory access stage
	int BranchEqual;
	int BranchNotEqual;
	int Jump;
	int MemRead;
	int MemWrite;

	// for write back stage
	int RegWrite;
	int MemtoReg;
} ControlSignal;

typedef struct {
	int operand1;
	int operand2;
	int result;
	bool zero;
} ArithmeticLogicUnit;

typedef struct {
	int writeData;
	int readData;
	int address;
} DataMemory;

typedef struct {
	int jumpAddress;
	int branchAddress;
	int imm;
} Extend;

typedef struct {
	// others
	int PC, PC4;
	int inst;
	int valid;

	// branch prediction
	int PHTindex;
	int BranchPrediction;
} IFID_LATCH;

typedef struct {
	// control signals
	int RegDst;
	int ALUOp;
	int ALUSrcA, ALUSrcB;
	int Jump, BranchEqual, BranchNotEqual;
	int MemRead, MemWrite;
	int RegWrite;
	int MemtoReg;

	// others
	int PC, PC4, PC_Jump;
	int readData1;
	int readData2;
	int extimm;
	int shamt;
	int writeReg;
	int valid;

	// forwarding unit
	int rs_Forward;
	int rt_Forward;

	// branch prediction
	int PHTindex;
	int BranchPrediction;
} IDEX_LATCH;

typedef struct {
	// control signals
	int Jump, Branch;
	int MemRead, MemWrite;
	int RegWrite;
	int MemtoReg;

	// others
	int PC, PC4, PC_Jump, PC_Branch;
	int ALUresult;
	int readData2;
	int writeReg;
	int valid;
} EXMEM_LATCH;

typedef struct {
	// control signals
	int RegWrite;

	// others
	int PC_Jump;
	int writeData;
	int writeReg;
	int valid;
} MEMWB_LATCH;

typedef struct {
	// forwarding unit signals
	unsigned int SelectA;
	unsigned int SelectB;
	unsigned int SelectC;
	unsigned int SelectD;
} Forwarding;

InstructionMemory		InstMem;
Instruction				INST;
RegisterFiles			REG;
ControlSignal			CTR;
ArithmeticLogicUnit		ALU;
DataMemory				DataMem;
Extend					EXT;
Forwarding				FORWARD;

// Latch Declaration
IFID_LATCH IFID_IN, IFID_OUT;
IDEX_LATCH IDEX_IN, IDEX_OUT;
EXMEM_LATCH EXMEM_IN, EXMEM_OUT;
MEMWB_LATCH MEMWB_IN, MEMWB_OUT;

// Function Declaration
void Initialize(void);
void ReadBinaryFiles(void);
void Fetch(void);
void Decode(void);
void Execute(void);
void MemoryAccess(void);
int WriteBack(void);
void LatchUpdate(void);
void BasicImplementation(void);
void AdditionalImplementation(void);

void DynamicBranchPrediction(void);
int checkBTB(int PC);
void updateBTB(void);
int checkPHT(int PC);
void updatePHT(void);

void Control(void);
void ALUControl(void);
void SignExtend(void);
void JumpAddress(void);
void BranchAddress(void);

void ForwardingABUnit(void);
void MUX_ALUSrcA(void);
void MUX_ALUSrcB(void);
void ForwardingCDUnit(void);
void MUX_ForwardC(void);
void MUX_ForwardD(void);

void MUX_RegDst(void);
void MUX_MemtoReg(void);
void MUX_Jump(void);
void MUX_Branch(void);
void MUX_ForwardA(void);
void MUX_ForwardB(void);

// Global Variables
extern int INSTMEMORY[0x00400000];// Insturction memory, 16MB
extern int DATAMEMORY[0x00400000];// Data memory, 16MB
extern int Register[32];

unsigned int PC;
unsigned int printOption;
unsigned int predictOption;
unsigned int countInst;
unsigned int countNop;
unsigned int countCycle;
unsigned int countLW, countSW;
unsigned int countPrediction;
unsigned int countJump;
float countCorrectPrediction;
float countWrongPrediction;
unsigned int countNotTakenBranch, countTakenBranch;
float BranchAccuracy;

int eof;					// End of file
int GHR;					// Global History Register
extern int BTB[0x8000][2];	// Branch Target Buffer, 256KB
extern int PHT[0x8000];		// Pattern History Table, 128KB
int hit, taken;
int BTBindex;
int BranchTarget;