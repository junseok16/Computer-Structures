
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
	int inst;
} InstructionMemory;

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
} Instruction;

typedef struct {
	int readReg1;
	int readReg2;
	int writeReg;
	int readData1;
	int readData2;
	int writeData;
} Registers;

typedef struct {
	int operand1;
	int operand2;
	int result;
	bool zero;
} ArithmeticLogicUnit;

typedef struct {
	int RegDst;
	int Jump;
	int BranchEqual;
	int BranchNotEqual;
	int ALUSrcA;
	int ALUSrcB;
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
	int imme;
} Extend;

struct CACHE {
	unsigned int tag;
	unsigned int valid;
	unsigned int dirty;
	unsigned int sca;	// second chance algorithm
	int DATA[16];		// 64B cache line size; 16 words
};

typedef struct {
	int tag;
	int index;
	int offset;
} Address;

InstructionMemory		InstMem;
InstructionFormat		InstForm;
Instruction				INST;
Registers				REG;
ArithmeticLogicUnit		ALU;
ControlSignal			CTR;
DataMemory				DataMem;
Extend					EXT;
Address					ADR;

void ReadBinaryFiles(void);
void Fetch(void);
void Decode(void);
void Control(void);
void Execute(void);
void Register(void);
void ALUControl(void);
void MemoryAccess(void);
void WriteBack(void);
void ExtendImmediate(void);

void MUX_RegDst(void);
void MUX_ALUSrcA(void);
void MUX_ALUSrcB(void);
void MUX_MemtoReg(void);
void MUX_Branch(void);
void MUX_Jump(void);
void JumpAddress(void);
void BranchAddress(void);

void BasicImplementation(void);
void AdditionalImplementation(void);

////////////////

void SetCache(void);
void checkCache(void);
int readMem(int address);
void writeMem(int address, int writeData);
void Read_updateCache(int address);
void Write_updateCache(int address);

////////////////

int MEMORY[0x00400000];
int REGISTER[32];
int printOption;
unsigned int PC;
unsigned int countInst, countMemoryAccess, countTakenBranch;
unsigned int countR, countI, countJ;

int indexLength, tagLength, offsetLength, field;
int countColdMiss, countConflictMiss, countHit;
int setCacheSize, setCachelineSize;
double hitRate, missRate, AMAT;
bool hit, cold, conflict;

MEMORY[0x00400000] = { 0, };
REGISTER[32] = { 0, };
PC = 0x00000000;

//struct CACHE* DirectCACHE;	// Direct mapping cache
//struct CACHE* SetCACHE[2];	// 2-Way set associateive cache
struct CACHE* SetCACHE[4];		// 4-Way set associateive cache
struct CACHE* Walker;
int* oldField;

void main(void) {
	REGISTER[31] = 0xffffffff;
	REGISTER[29] = 0x01000000;
	ReadBinaryFiles();
	SetCache();

	while (PC != 0xffffffff) {
		Fetch();
		Decode();
		Execute();
		MemoryAccess();
		WriteBack();
		BasicImplementation();
	}
	AdditionalImplementation();
	return;
}

////////////////////////////////////////////////////////////

void SetCache(void) {
	setCacheSize = 12;
	setCachelineSize = 6;
	indexLength = setCacheSize - setCachelineSize;
	offsetLength = setCachelineSize;
	tagLength = 32 - (indexLength + offsetLength);

	oldField = malloc(sizeof(struct CACHE) * pow(2.0, (double)indexLength));

	if (oldField == NULL) {
		printf("oldField의 동적 할당에 문제가 발생했습니다.");
		return;
	}

	for (int i = 0; i < pow(2.0, (double)indexLength); i++) {
		memset(&oldField[i], 0, sizeof(struct CACHE));
	}

	for (int i = 0; i < 4; i++) {
		SetCACHE[i] = malloc(sizeof(struct CACHE) * pow(2.0, (double)indexLength));

		if (SetCACHE[i] == NULL) {
			printf("SetCACHE[i]의 동적 할당에 문제가 발생했습니다.");
			return;
		}

		for (int j = 0; j < pow(2.0, (double)indexLength); j++) {
			memset(&SetCACHE[i][j], 0, sizeof(struct CACHE));
		}
	}

	countColdMiss, countConflictMiss, countHit = 0;
	hit, cold, conflict = false;
	hitRate, missRate, AMAT = 0.0;

	printf("\n\t=============================================================");
	printf("\n\t 프로그램이 곧 시작합니다. 잠시만 기다려주세요.");
	return;
}

int readMem(int address) {
	ADR.tag = (address >> (indexLength + offsetLength)) & 0x000fffff;
	ADR.index = (address >> offsetLength) & 0x0000003f;
	ADR.offset = address & 0x0000003f;

	checkCache();
	Read_updateCache(address);
	return Walker->DATA[ADR.offset / 4];
}

void writeMem(int address, int writeData) {
	ADR.tag = (address >> (indexLength + offsetLength)) & 0x000fffff;
	ADR.index = (address >> offsetLength) & 0x0000003f;
	ADR.offset = address & 0x0000003f;

	checkCache();
	Write_updateCache(address);
	Walker->DATA[ADR.offset / 4] = writeData;
	return;
}

void checkCache(void) {
	for (int i = 0; i < 4; i++) {
		Walker = &SetCACHE[i][ADR.index];

		switch(Walker->valid) {
			case 0:
				hit = false;
				cold = true;
				conflict = false;
				field = i;
				countColdMiss++;
				return;
			break;

			case 1:
				if (Walker->tag == ADR.tag) {
					Walker->sca = 1;
					hit = true;
					cold = false;
					conflict = false;
					field = i;
					countHit++;
					return;
				}
				else if (Walker->tag != ADR.tag) {
					if(i == 3) {
						hit = false;
						cold = false;
						conflict = true;
						countConflictMiss++;
						return;
					}
				}
				else;
			break;

			default:
			break;
		}
	}
	return;
}

void Read_updateCache(int address) {
	if (hit) {
		Walker = &SetCACHE[field][ADR.index];
		return;
	}

	if (cold) {
		Walker = &SetCACHE[field][ADR.index];
		int newAddress = address & 0xffffffc0;

		Walker->tag = ADR.tag;
		Walker->valid = 1;
		Walker->dirty = 0;
		Walker->sca = 0;

		for (int i = 0; i < 16; i++) {
			Walker->DATA[i] = MEMORY[(newAddress / 4) + i];
		}
		return;
	}

	if (conflict) {
		bool escape = false;

		while (!escape) {
			Walker = &SetCACHE[oldField[ADR.index]][ADR.index];

			if (Walker->sca == 0) {
				field = oldField[ADR.index]++;
				escape = true;

				if (oldField[ADR.index] > 3)
					oldField[ADR.index] = 0;
			}
			else if (Walker->sca == 1) {
				Walker->sca = 0;
				oldField[ADR.index]++;

				if (oldField[ADR.index] > 3)
					oldField[ADR.index] = 0;
			}
			else;
		}

		Walker = &SetCACHE[field][ADR.index];
		int oldAddress = ((Walker->tag << indexLength) + ADR.index) << offsetLength;
		int newAddress = address & 0xffffffc0;

		if (Walker->dirty == 1) {
			for (int i = 0; i < 16; i++) {
				MEMORY[oldAddress / 4 + i] = Walker->DATA[i];
			}
		}

		for (int i = 0; i < 16; i++) {
			Walker->DATA[i] = MEMORY[(newAddress / 4) + i];
		}

		Walker->tag = ADR.tag;
		Walker->valid = 1;
		Walker->dirty = 0;
		Walker->sca = 0;
		return;
	}
}

void Write_updateCache(int address) {
	if (hit) {
		Walker = &SetCACHE[field][ADR.index];
		Walker->dirty = 1;
		return;
	}

	if (cold) {
		Walker = &SetCACHE[field][ADR.index];
		Walker->tag = ADR.tag;
		Walker->valid = 1;
		Walker->dirty = 1;
		Walker->sca = 0;
		return;
	}

	if (conflict) {
		bool escape = false;

		while(!escape) {
			Walker = &SetCACHE[oldField[ADR.index]][ADR.index];

			if (Walker->sca == 0) {
				field = oldField[ADR.index]++;
				escape = true;

				if (oldField[ADR.index] > 3)
					oldField[ADR.index] = 0;
			}
			else if (Walker->sca == 1) {
				Walker->sca = 0;
				oldField[ADR.index]++;

				if (oldField[ADR.index] > 3)
					oldField[ADR.index] = 0;
			}
			else;
		}

		Walker = &SetCACHE[field][ADR.index];
		int oldAddress = ((Walker->tag << indexLength) + ADR.index) << offsetLength;

		if (Walker->dirty == 1) {
			for (int i = 0; i < 16; i++) {
				MEMORY[oldAddress / 4 + i] = Walker->DATA[i];
			}
		}

		Walker->tag = ADR.tag;
		Walker->valid = 1;
		Walker->sca = 0;
		Walker->dirty = 1;
		return;
	}
}

////////////////////////////////////////////////////////////

void ReadBinaryFiles(void) {
	FILE* binFile;
	int Index = 0, Error = 0;

	char fileName[32];
	fileName[31] = 0;
	printf("\n\t=============================================================");
	printf("\n\t SINGLE CYCLE MIPS에 오신 것을 환영합니다.");
	printf("\n\t bin 파일을 입력하세요. > ");

	while (1) {
		scanf_s("%s", fileName, 32);
		Error = fopen_s(&binFile, fileName, "rb");

		if (binFile == 0 || Error == 1) {
			printf("\n\t bin 파일 불러오기 실패! 파일을 다시 입력하세요. > ");
			continue;
		}
		else
			break;
	}

	while (1) {
		fread(&MEMORY[Index], sizeof(int), 1, binFile);

		if (feof(binFile))
			break;
		else {
			InstForm.InstData =
				(((MEMORY[Index] & 0x000000ff) << 24) & 0xff000000) |
				(((MEMORY[Index] & 0x0000ff00) << 8) & 0x00ff0000) |
				(((MEMORY[Index] & 0x00ff0000) >> 8) & 0x0000ff00) |
				(((MEMORY[Index] & 0xff000000) >> 24) & 0x000000ff);
			MEMORY[Index] = InstForm.InstData;
			Index++;
		}
	}
	fclose(binFile);
	printf("\t=============================================================");
	printf("\n\t 0. No");
	printf("\n\t 1. Yes");
	printf("\n\t=============================================================");
	printf("\n\t 모든 실행 결과를 보겠습니까? > ");

	while (1) {
		scanf_s("%d", &printOption);
		if ((printOption != 0) && (printOption != 1)) {
			printf("\n\t 다시 입력하세요. > ");
			continue;
		}
		else
			break;
	}
	return;
}

void Fetch(void) {
	InstMem.inst = readMem(PC);
	PC = PC + 4;
	return;
}

void Decode(void) {
	INST.opcode = ((InstMem.inst & 0xfc000000) >> 26) & 0x0000003f;
	INST.rs = ((InstMem.inst & 0x03e00000) >> 21) & 0x0000001f;
	INST.rt = ((InstMem.inst & 0x001f0000) >> 16) & 0x0000001f;
	INST.rd = ((InstMem.inst & 0x0000f800) >> 11) & 0x0000001f;
	INST.shamt = ((InstMem.inst & 0x000007c0) >> 6) & 0x0000001f;
	INST.funct = InstMem.inst & 0x0000003f;
	INST.imme = InstMem.inst & 0x0000ffff;
	INST.addr = InstMem.inst & 0x03ffffff;

	if (INST.opcode == 0x0)
		countR++;
	else if (INST.opcode == 0x2 || INST.opcode == 0x3)
		countJ++;
	else
		countI++;

	Control();
	Register();
	ExtendImmediate();
	return;
}

void Control(void) {
	// RegDst Control
	switch (INST.opcode) {
	case 0x0:// jalr
		if (INST.funct == 0x9)
			CTR.RegDst = 2;
		else
			CTR.RegDst = 1;
		break;

	case 0x3:// jal
		CTR.RegDst = 2;
		break;

	default:
		CTR.RegDst = 0;
		break;
	}

	// MemtoReg Control
	switch (INST.opcode) {
	case 0x23:// lw
		CTR.MemtoReg = 1;
		break;

	case 0x3:// jal
		CTR.MemtoReg = 2;
		break;

	case 0x0:// jalr
		if (INST.funct == 0x9)
			CTR.MemtoReg = 2;
		else
			CTR.MemtoReg = 0;
		break;

	default:
		CTR.MemtoReg = 0;
		break;
	}

	// MemRead, MemWrite Control
	switch (INST.opcode) {
	case 0x23:// lw
		CTR.MemRead = 1;
		CTR.MemWrite = 0;
		break;

	case 0x2b:// sw
		CTR.MemRead = 0;
		CTR.MemWrite = 1;
		break;

	default:
		CTR.MemRead = 0;
		CTR.MemWrite = 0;
		break;
	}

	// Jump Control
	switch (INST.opcode) {
	case 0x0:
		if (INST.funct == 0x8 || INST.funct == 0x9)
			CTR.Jump = 2;
		else
			CTR.Jump = 0;
		break;

	case 0x2:
	case 0x3:
		CTR.Jump = 1;
		break;

	default:
		CTR.Jump = 0;
		break;
	}

	// BranchEqual, BranchNotEqual Control
	switch (INST.opcode) {
	case 0x4:// beq
		CTR.BranchEqual = 1;
		CTR.BranchNotEqual = 0;
		break;

	case 0x5:// bne
		CTR.BranchEqual = 0;
		CTR.BranchNotEqual = 1;
		break;

	default:
		CTR.BranchEqual = 0;
		CTR.BranchNotEqual = 0;
		break;
	}

	// RegWrite Control
	switch (INST.opcode) {
	case 0x4:
	case 0x5:
	case 0x2b:
	case 0x2:
		CTR.RegWrite = 0;
		break;

	case 0x0:
		if (INST.funct == 0x8)
			CTR.RegWrite = 0;
		else
			CTR.RegWrite = 1;
		break;

	default:
		CTR.RegWrite = 1;
		break;
	}

	// ALUSrc Control
	switch (INST.opcode) {
	case 0x4:// beq
	case 0x5:// bne
		CTR.ALUSrcA = 0;
		CTR.ALUSrcB = 0;
		break;

	case 0xf:// lui
		CTR.ALUSrcA = 2;
		CTR.ALUSrcB = 1;
		break;

	case 0x0:// sll, srl
		if (INST.funct == 0x0 || INST.funct == 0x2) {
			CTR.ALUSrcA = 1;
			CTR.ALUSrcB = 2;
		}
		else {
			CTR.ALUSrcA = 0;
			CTR.ALUSrcB = 0;
		}
		break;

	default:
		CTR.ALUSrcA = 0;
		CTR.ALUSrcB = 1;
		break;
	}

	// ALUOp Control
	switch (INST.opcode) {
	case 0x0:// R type
		CTR.ALUOp = 0b000;// 0; depends on function
		break;

	case 0xc:// andi
		CTR.ALUOp = 0b001;// 1; and
		break;

	case 0xd:// ori
		CTR.ALUOp = 0b010;// 2; or
		break;

	case 0x8:// addi
	case 0x9:// addiu
	case 0x23:// lw
	case 0x2b:// sw
	case 0xf:// lui(0 + imm << 16b'0)
		CTR.ALUOp = 0b011;// 3; add
		break;

	case 0x4:// beq
	case 0x5:// bne
		CTR.ALUOp = 0b100;// 4; sub
		break;

	case 0xa:// slti
	case 0xb:// sltiu
		CTR.ALUOp = 0b101;// 5; slt
		break;

	default:
		CTR.ALUOp = 0b000;
		break;
	}
	return;
}

void Register(void) {
	REG.readData1 = REGISTER[INST.rs];
	REG.readData2 = REGISTER[INST.rt];
	MUX_RegDst();
	return;
}

void ALUControl(void) {
	if (CTR.ALUOp == 0b000) {
		switch (INST.funct) {
		case 0x24:
			CTR.ALUCtr = 0b000;// and
			break;

		case 0x25:
			CTR.ALUCtr = 0b001;// or
			break;

		case 0x27:
			CTR.ALUCtr = 0b010;// nor
			break;

		case 0x00:
			CTR.ALUCtr = 0b011;// sll
			break;

		case 0x02:
			CTR.ALUCtr = 0b100;// srl
			break;

		case 0x20:
		case 0x21:
			CTR.ALUCtr = 0b101;// add, addu
			break;

		case 0x22:
		case 0x23:
			CTR.ALUCtr = 0b110;// sub, subu
			break;

		case 0x2a:
		case 0x2b:
			CTR.ALUCtr = 0b111;// slt, sltu
			break;

		case 0x08:
		case 0x09:
			CTR.ALUCtr = -1;// jr, jalr
			break;
		}
	}
	else {
		switch (CTR.ALUOp) {
		case 0b001: // andi (and 연산)
			CTR.ALUCtr = 0b000;
			break;

		case 0b010: // ori (or 연산)
			CTR.ALUCtr = 0b001;
			break;

		case 0b011: // addi, addiu, lw, sw (add 연산)
			CTR.ALUCtr = 0b101;
			break;

		case 0b100: // beq, bne (sub 연산)
			CTR.ALUCtr = 0b110;
			break;

		case 0b101: // slti, sltiu (slt 연산)
			CTR.ALUCtr = 0b111;
			break;

		default:
			CTR.ALUCtr = -1;
			break;
		}
	}
	return;
}

void Execute(void) {
	ALUControl();
	MUX_ALUSrcA();
	MUX_ALUSrcB();

	switch (CTR.ALUCtr) {
	case 0b000: // and
		ALU.result = ALU.operand1 & ALU.operand2;
		break;

	case 0b001: // or
		ALU.result = ALU.operand1 | ALU.operand2;
		break;

	case 0b010: // nor
		ALU.result = ~(ALU.operand1 | ALU.operand2);
		break;

	case 0b011: // sll
		ALU.result = ALU.operand1 << ALU.operand2;
		break;

	case 0b100: // srl
		ALU.result = ALU.operand1 >> ALU.operand2;
		break;

	case 0b101: // add
		ALU.result = ALU.operand1 + ALU.operand2;
		break;

	case 0b110: // sub
		ALU.result = ALU.operand1 - ALU.operand2;
		if (ALU.result == 0)
			ALU.zero = true;
		else
			ALU.zero = false;
		break;

	case 0b111: // slt
		ALU.result = (ALU.operand1 < ALU.operand2) ? 1 : 0;
		break;

	default:
		break;
	}
	return;
}

void MemoryAccess(void) {
	DataMem.address = ALU.result;
	DataMem.writeData = REG.readData2;

	if (CTR.MemRead == 1) {
		//DataMem.readData = MEMORY[DataMem.address / 4];
		DataMem.readData = readMem(DataMem.address);
		countMemoryAccess++;
	}
	else if (CTR.MemWrite == 1) {
		//MEMORY[DataMem.address / 4] = DataMem.writeData;
		writeMem(DataMem.address, DataMem.writeData);
		countMemoryAccess++;
	}
	else;
	return;
}

void WriteBack(void) {
	MUX_MemtoReg();
	switch (CTR.RegWrite) {
	case 0:
		break;

	case 1:
		REGISTER[REG.writeReg] = REG.writeData;
		break;

	default:
		break;
	}
	MUX_Branch();
	MUX_Jump();

	countInst++;
	Walker = NULL;
	return;
}

void MUX_RegDst(void) {// RegDst MUX
	switch (CTR.RegDst) {
	case 0:
		REG.writeReg = INST.rt;
		break;

	case 1:
		REG.writeReg = INST.rd;
		break;

	case 2:// jal, jalr
		REG.writeReg = 31;
		break;

	default:
		break;
	}
	return;
}

void MUX_ALUSrcA(void) {// ALUSrc1 MUX
	switch (CTR.ALUSrcA) {
	case 0:
		ALU.operand1 = REG.readData1;
		break;

	case 1:// sll, srl
		ALU.operand1 = REG.readData2;
		break;

	case 2:// lui
		ALU.operand1 = 0;

		break;
	default:
		break;
	}
	return;
}

void MUX_ALUSrcB(void) {
	switch (CTR.ALUSrcB) {
	case 0:
		ALU.operand2 = REG.readData2;
		break;

	case 1:
		ALU.operand2 = EXT.imme;
		break;

	case 2:// sll, srl
		ALU.operand2 = INST.shamt;
		break;

	default:
		break;
	}
	return;
}

void MUX_MemtoReg(void) {// MemtoReg MUX
	switch (CTR.MemtoReg) {
	case 0:
		REG.writeData = ALU.result;
		break;

	case 1:
		REG.writeData = DataMem.readData;
		break;

	case 2:// jal, jalr
		REG.writeData = PC + 4;
		break;

	default:
		break;
	}
	return;
}

void MUX_Branch(void) {// Branch MUX
	BranchAddress();

	if ((CTR.BranchEqual == 1) && (ALU.zero == true)) {// beq
		PC = PC + EXT.branchAddress;
		countTakenBranch++;
	}
	else if ((CTR.BranchNotEqual == 1) && (!ALU.zero == true)) {// bne
		PC = PC + EXT.branchAddress;
		countTakenBranch++;
	}
	else
		PC = PC;
	return;
}

void MUX_Jump(void) {// Jump MUX
	JumpAddress();

	switch (CTR.Jump) {
	case 0:
		PC = PC;
		break;

	case 1:// j, jal
		PC = EXT.jumpAddress;
		break;

	case 2:// jr, jalr
		PC = ALU.operand1;
		break;

	default:
		break;
	}
	return;
}

void JumpAddress(void) {
	EXT.jumpAddress = (PC & 0xf0000000) | (INST.addr << 2);
	return;
}

void BranchAddress(void) {
	switch (INST.imme >> 15) {
	case 0:
		EXT.branchAddress = (0x00000000 | INST.imme) << 2;
		break;

	case 1:
		EXT.branchAddress = (0xffff0000 | INST.imme) << 2;
		break;

	default:
		break;
	}
	return;
}

void ExtendImmediate(void) {
	if (INST.opcode == 0xc || INST.opcode == 0xd)// ZeroExtimm
		EXT.imme = INST.imme | 0x00000000;

	else if (INST.opcode == 0xf)// Load Upper immediate(lui)
		EXT.imme = INST.imme << 16;

	else {
		switch (INST.imme >> 15) {
		case 0:
			EXT.imme = INST.imme | 0x00000000;
			break;

		case 1:
			EXT.imme = INST.imme | 0xffff0000;
			break;

		default:
			break;
		}
	}
	return;
}

void BasicImplementation(void) {
	if (printOption == 1) {
		printf("\n\t=============================================================");
		printf("\n\t %dth Instruction\t: 0x%08x", countInst, InstMem.inst);
		printf("\n\t PC\t\t: 0x%08x", PC - 4);
		printf("\n\t R[rs](R[%02d])\t: 0x%08x", INST.rs, REGISTER[INST.rs]);
		printf("\n\t R[rt](R[%02d])\t: 0x%08x", INST.rt, REGISTER[INST.rt]);
		printf("\n\t R[rd](R[%02d])\t: 0x%08x", INST.rd, REGISTER[INST.rd]);

		if (CTR.MemWrite == 1)
			printf("\n\t M[0x%08x]\t: 0x%08x", DataMem.address / 4, DataMem.writeData);

		printf("\n\t v0(R[2])\t: %d", REGISTER[2]);
	}
	else;
	return;
}

void AdditionalImplementation(void) {
	hitRate = (double)countHit / ((double)countHit + (double)countColdMiss + (double)countConflictMiss);
	missRate = 1 - hitRate;
	AMAT = hitRate * 1.0 + missRate * 1000.0;

	printf("\n\t=============================================================");
	printf("\n\t v0(R[2])\t\t: %d", REGISTER[2]);
	printf("\n\t Cycle\t\t\t: %d", countInst);
	printf("\n\t Memory Access\t\t: %d", countMemoryAccess);
	printf("\n\t Taken Branch\t\t: %d", countTakenBranch);

	printf("\n\t R-type Operation\t: %d", countR);
	printf("\n\t I-type Operation\t: %d", countI);
	printf("\n\t J-type Operation\t: %d", countJ);

	printf("\n\t Hit\t\t\t: %d", countHit);
	printf("\n\t Cold Miss\t\t: %d", countColdMiss);
	printf("\n\t Conflict Miss\t\t: %d", countConflictMiss);

	printf("\n\t Hit Rate\t\t: %f", hitRate);
	printf("\n\t Miss Rate\t\t: %f", missRate);
	printf("\n\t AMAT\t\t\t: %f", AMAT);
	printf("\n\t=============================================================");
	printf("\n\t SINCLE CYCLE MIPS를 이용해주셔서 감사합니다.");
	printf("\n");
	return;
}