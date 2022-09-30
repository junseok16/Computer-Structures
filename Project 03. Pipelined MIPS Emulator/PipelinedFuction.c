
#include "PipelinedHeader.h"

void Initialize(void) {
	// initialize register, pc, and count values 
	Register[31] = 0xffffffff;
	Register[29] = 0x01000000;
	PC = 0x00000000;
	countCycle = 0;
	countJump = 0;
	printOption, predictOption = 0;
	countInst, countNop, countLW, countSW = 0;
	countPrediction = 0;
	countCorrectPrediction, countWrongPrediction = 0;
	countNotTakenBranch, countTakenBranch, BranchAccuracy = 0.0;
	eof, hit, taken = 0;
	BTBindex, BranchTarget = 0;

	// initialize IFID LATCH to 0
	memset(&IFID_IN, 0, sizeof(IFID_IN));
	memset(&IFID_OUT, 0, sizeof(IFID_OUT));

	// initialize IDEX LATCH to 0
	memset(&IDEX_IN, 0, sizeof(IDEX_IN));
	memset(&IDEX_OUT, 0, sizeof(IDEX_OUT));

	// initialize EXMEM LATCH to 0
	memset(&EXMEM_IN, 0, sizeof(EXMEM_IN));
	memset(&EXMEM_OUT, 0, sizeof(EXMEM_OUT));

	// initialize MEMWB LATCH to 0
	memset(&MEMWB_IN, 0, sizeof(MEMWB_IN));
	memset(&MEMWB_OUT, 0, sizeof(MEMWB_OUT));

	printf("\n\t=============================================================");
	printf("\n\tLATCH가 초기화되었습니다.");
	return;
}

void ReadBinaryFiles(void) {
	FILE* binFile;
	int Index = 0, Error = 0;

	char fileName[32];
	fileName[31] = 0;
	printf("\n\t=============================================================");
	printf("\n\tPIPELINED MIPS에 오신 것을 환영합니다.");
	printf("\n\tbin 파일을 입력하세요. > ");

	while (1) {
		scanf_s("%s", fileName, 32);
		Error = fopen_s(&binFile, fileName, "rb");

		if (binFile == 0 || Error == 1) {
			printf("\n\tbin 파일 불러오기 실패! 파일을 다시 입력하세요. > ");
			continue;
		}
		else
			break;
	}

	while (1) {
		fread(&INSTMEMORY[Index], sizeof(int), 1, binFile);

		if (feof(binFile))
			break;
		else {
			InstMem.inst =
				(((INSTMEMORY[Index] & 0x000000ff) << 24) & 0xff000000) |
				(((INSTMEMORY[Index] & 0x0000ff00) << 8) & 0x00ff0000) |
				(((INSTMEMORY[Index] & 0x00ff0000) >> 8) & 0x0000ff00) |
				(((INSTMEMORY[Index] & 0xff000000) >> 24) & 0x000000ff);
			INSTMEMORY[Index] = InstMem.inst;
			Index++;
		}
	}
	fclose(binFile);
	printf("\t=============================================================");
	printf("\n\t0. N O");
	printf("\n\t1. YES");
	printf("\n\t=============================================================");
	printf("\n\t모든 실행 결과를 보겠습니까? > ");

	while (1) {
		scanf_s("%d", &printOption);
		if ((printOption != 1) && (printOption != 0)) {
			printf("\n\t다시 입력하세요. > ");
			continue;
		}
		else
			break;
	}

	printf("\t=============================================================");
	printf("\n\t1. Always not taken");
	printf("\n\t2. One level branch prediction");
	printf("\n\t3. Two level global branch prediction");
	printf("\n\t4. Two level gshare branch prediction");
	printf("\n\t=============================================================");
	printf("\n\t어떤 분기 예측을 실행하겠습니까? > ");

	while (1) {
		scanf_s("%d", &predictOption);
		if ((predictOption != 1) && (predictOption != 2) && (predictOption != 3)
			&& (predictOption != 4) && (predictOption != 5)) {
			printf("\n\t다시 입력하세요. > ");
			continue;
		}
		else
			break;
	}

	printf("\t=============================================================");
	printf("\n\t곧 프로그램이 시작합니다.");
	Sleep(1000);
	printf("\n\t=============================================================");
	printf("\n\t%s을 처리하는 중입니다. 잠시만 기다려주세요.", fileName);
	printf("\n\t=============================================================");
	return;
}

////////////////////////////////////////

void Fetch(void) {
	if (printOption) printf("\n\t CYCLE No.%d\n", countCycle);

	// count the number of clock cycle
	countCycle++;
	countInst++;

	switch (predictOption) {
	case 1:
		MUX_Branch();
		MUX_Jump();
		break;

	case 2:
	case 3:
	case 4:
		MUX_Jump();
		break;

	default:
		break;
	}

	// fetch instruction from instruction memory
	InstMem.inst = INSTMEMORY[PC / 4];

	// set valid
	if (PC == 0xffffffff) {// for the last instruction
		IFID_IN.valid = 0;
		if (printOption) printf("\n\t [I F]\tvalid\t: 0");
		return;
	}
	else if (InstMem.inst == 0x00000000) {// for nop instruction
		IFID_IN.valid = 0;
		countNop++;
		PC = PC + 4;
		if (printOption) printf("\n\t [I F]\tnop\t: 0x%08x", InstMem.inst);
		return;
	}
	else
		IFID_IN.valid = 1;

	// store to IFID_IN LATCH
	IFID_IN.PC = PC;
	IFID_IN.PC4 = PC + 4;
	IFID_IN.inst = InstMem.inst;

	if (printOption) {
		printf("\n\t [I F]\tpc\t: 0x%08x", PC);
		printf("\n\t\tinst\t: 0x%08x", InstMem.inst);
	}

	switch (predictOption) {
	case 1:
		PC = PC + 4;
		countPrediction++;
		break;
	case 2:
	case 3:
	case 4:
		DynamicBranchPrediction();
		break;
	}
	return;
}

void MUX_Branch(void) {
	switch (EXMEM_OUT.Branch) {
	case 0:
		PC = PC;// pc + 4
		break;

	case 1:
		PC = EXMEM_OUT.PC_Branch;// branch target
		break;

	default:
		break;
	}
	return;
}

void MUX_Jump(void) {
	switch (IDEX_OUT.Jump) {
	case 0:
		PC = PC;// pc + 4 or branch target
		break;

	case 1:
		PC = IDEX_OUT.PC_Jump;// jump target
		countJump++;
		break;

	default:
		break;
	}
	return;
}

void DynamicBranchPrediction(void) {
	hit = checkBTB(PC);

	switch (predictOption) {
	case 2:// one level branch prediction
		taken = checkPHT(PC);
		IFID_IN.PHTindex = PC;
		break;

	case 3:// two level global branch prediction
		taken = checkPHT(((PC & 0x3) << 4) | (GHR & 0xf));
		IFID_IN.PHTindex = ((PC & 0x3) << 4) | (GHR & 0xf);
		break;

	case 4:// two level gshare branch prediction
		taken = checkPHT((PC & 0x3f) ^ GHR);
		IFID_IN.PHTindex = (PC & 0x3f) ^ GHR;
		break;

	default:
		break;
	}

	switch (hit) {
	case 0:
		IFID_IN.BranchPrediction = 0;
		PC = PC + 4;
		countPrediction++;
		break;

	case 1:
		if (taken == 0b00 || taken == 0b01) {
			IFID_IN.BranchPrediction = 0;
			PC = PC + 4;
			countPrediction++;
		}
		else if (taken == 0b10 || taken == 0b11) {
			IFID_IN.BranchPrediction = 1;
			PC = BranchTarget;
			countPrediction++;
		}
		else;
		break;

	default:
		break;
	}
	return;
}

int checkBTB(int PC) {
	for (int i = 0; i <= BTBindex; i++) {
		if (BTB[i][0] == PC && PC != 0) { 
			BranchTarget = BTB[i][1];
			return 1;
		}
		else;
	}
	return 0;
}

int checkPHT(int Index) {
	return PHT[Index];
}

////////////////////////////////////////

void Decode(void) {
	// set valid
	IDEX_IN.valid = IFID_OUT.valid;
	if (!IFID_OUT.valid) {
		if (printOption) printf("\n\t [I D]\tvalid\t: 0");
		return;
	}

	// decode instruction
	INST.opcode = ((IFID_OUT.inst & 0xfc000000) >> 26) & 0x0000003f;
	INST.rs = ((IFID_OUT.inst & 0x03e00000) >> 21) & 0x0000001f;
	INST.rt = ((IFID_OUT.inst & 0x001f0000) >> 16) & 0x0000001f;
	INST.rd = ((IFID_OUT.inst & 0x0000f800) >> 11) & 0x0000001f;
	INST.shamt = ((IFID_OUT.inst & 0x000007c0) >> 6) & 0x0000001f;
	INST.funct = IFID_OUT.inst & 0x0000003f;
	INST.imm = IFID_OUT.inst & 0x0000ffff;
	INST.addr = IFID_OUT.inst & 0x03ffffff;

	// set control signals and extend immediate
	Control();
	SignExtend();

	// register file
	REG.readReg1 = INST.rs;
	REG.readReg2 = INST.rt;
	REG.readData1 = Register[REG.readReg1];
	REG.readData2 = Register[REG.readReg2];

	/***** store to IDEX_IN LATCH *****/
	// store control signals for execution stage
	IDEX_IN.RegDst = CTR.RegDst;
	IDEX_IN.ALUOp = CTR.ALUOp;
	IDEX_IN.ALUSrcA = CTR.ALUSrcA;
	IDEX_IN.ALUSrcB = CTR.ALUSrcB;

	// store control signals for memory access stage
	IDEX_IN.BranchEqual = CTR.BranchEqual;
	IDEX_IN.BranchNotEqual = CTR.BranchNotEqual;
	IDEX_IN.MemRead = CTR.MemRead;
	IDEX_IN.MemWrite = CTR.MemWrite;

	// store control signals for write back stage
	IDEX_IN.RegWrite = CTR.RegWrite;
	IDEX_IN.MemtoReg = CTR.MemtoReg;
	IDEX_IN.Jump = CTR.Jump;

	// forward unit
	IDEX_IN.rs_Forward = INST.rs;
	IDEX_IN.rt_Forward = INST.rt;

	// store pc
	IDEX_IN.shamt = INST.shamt;
	IDEX_IN.PC = IFID_OUT.PC;
	IDEX_IN.PC4 = IFID_OUT.PC4;

	// Branch Prediction
	IDEX_IN.PHTindex = IFID_OUT.PHTindex;
	IDEX_IN.BranchPrediction = IFID_OUT.BranchPrediction;

	// Data Dependency No.3
	// forwarding from write back stage(세 단계 이전 명령어와 비교한다.)
	ForwardingCDUnit();
	MUX_ForwardC();
	MUX_ForwardD();

	// MUX_RegDst를 Execution이 아닌 Decode에서 판단, JumpAddress;
	MUX_RegDst();
	JumpAddress();

	if (printOption) {
		printf("\n\t [I D]\topcode\t: 0x%08x", INST.opcode);
		printf("\n\t\trs\t: 0x%08x", INST.rs);
		printf("\n\t\trt\t: 0x%08x", INST.rt);
		printf("\n\t\timm\t: 0x%08x", INST.imm);
		printf("\n\t\tshamt\t: 0x%08x", INST.shamt);
		printf("\n\t\tfunct\t: 0x%08x", INST.funct);
	}
	return;
}

void Control(void) {
	/***** RegDst Control *****/
	switch (INST.opcode) {
	case 0x0:// R type
		if (INST.funct == 0x9)// jalr
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

	/***** ALUOp Control *****/
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

	/***** ALUSrcA, B Control *****/
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

	case 0x0:// R type
		if (INST.funct == 0x0 || INST.funct == 0x2) {// sll, srl
			CTR.ALUSrcA = 1;
			CTR.ALUSrcB = 2;
		}
		else {
			CTR.ALUSrcA = 0; /* DEBUG1 */
			CTR.ALUSrcB = 0;
		}
		break;

	default:
		CTR.ALUSrcA = 0;
		CTR.ALUSrcB = 1;
		break;
	}

	/***** Jump Control *****/
	switch (INST.opcode) {
	case 0x0:
		if (INST.funct == 0x8 || INST.funct == 0x9)
			CTR.Jump = 1;
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

	/***** BranchEqual, BranchNotEqual Control *****/
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

	/***** MemRead, MemWrite Control *****/
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

	/***** MemtoReg Control *****/
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

	/***** RegWrite Control *****/
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
			CTR.RegWrite = 1; /* DEBUG2 */
		break;
	default:
		CTR.RegWrite = 1;
		break;
	}
	return;
}

void SignExtend(void) {
	if (INST.opcode == 0xc || INST.opcode == 0xd)// ZeroExtimm
		EXT.imm = INST.imm | 0x00000000;

	else if (INST.opcode == 0xf)// Load Upper immediate(lui)
		EXT.imm = INST.imm << 16;

	else {// SignExtimm
		switch (INST.imm >> 15) {
		case 0:
			EXT.imm = INST.imm | 0x00000000;
			break;

		case 1:
			EXT.imm = INST.imm | 0xffff0000;
			break;

		default:
			break;
		}
	}
	// store sign extend immediate to latch
	IDEX_IN.extimm = EXT.imm;
	return;
}

void MUX_RegDst(void) {
	switch (CTR.RegDst) {
	case 0:
		IDEX_IN.writeReg = INST.rt;
		break;

	case 1:
		IDEX_IN.writeReg = INST.rd;
		break;

	case 2:// jal, jalr
		IDEX_IN.writeReg = 31;
		break;

	default:
		break;
	}
	return;
}

void ForwardingCDUnit(void) {
	// default SelectC, SelectD
	FORWARD.SelectC = 0b00;
	FORWARD.SelectD = 0b00;

	// check write back forwarding for SelectC ($0 제외)
	if (MEMWB_OUT.RegWrite && (MEMWB_OUT.writeReg != 0) && (REG.readReg1 == MEMWB_OUT.writeReg))
		FORWARD.SelectC = 0b01;

	// check write back forwarding for SelectD ($0 제외)
	if ((MEMWB_OUT.RegWrite && (MEMWB_OUT.writeReg != 0) && REG.readReg2 == MEMWB_OUT.writeReg))
		FORWARD.SelectD = 0b01;

	return;
}

void MUX_ForwardC(void) {
	switch (FORWARD.SelectC) {
	case 0:
		IDEX_IN.readData1 = REG.readData1;
		break;

	case 1:// forwarding from write back stage
		IDEX_IN.readData1 = MEMWB_OUT.writeData;
		break;

	default:
		break;
	}
	return;
}

void MUX_ForwardD(void) {
	switch (FORWARD.SelectD) {
	case 0:
		IDEX_IN.readData2 = REG.readData2;
		break;

	case 1:// forwarding from write back stage
		IDEX_IN.readData2 = MEMWB_OUT.writeData;
		break;

	default:
		break;
	}
	return;
}

void JumpAddress(void) {// consider nop instruction after j, jal, jr, jalr
	EXT.jumpAddress = (IFID_OUT.PC4 & 0xf0000000) | (INST.addr << 2);

	/* Debug */
	if ((INST.opcode == 0x0 && INST.funct == 0x8) || (INST.opcode == 0x0 && INST.funct == 0x9)) {// jr, jalr
		if (INST.rs && (INST.rs == EXMEM_OUT.writeReg))// check execution forwarding for jump address ($0 제외)
			IDEX_IN.PC_Jump = EXMEM_OUT.ALUresult;

		else if (INST.rs && (INST.rs == MEMWB_OUT.writeReg))// check memory access forwarding for jump address ($0 제외)
			IDEX_IN.PC_Jump = MEMWB_OUT.writeData;

		else
			IDEX_IN.PC_Jump = Register[REG.readReg1];
	}
	else
		IDEX_IN.PC_Jump = EXT.jumpAddress;
	return;
}

////////////////////////////////////////

void Execute(void) {
	// set valid
	EXMEM_IN.valid = IDEX_OUT.valid;

	if (!IDEX_OUT.valid) {
		if (printOption) printf("\n\t [E X]\tvalid\t: 0");
		return;
	}

	// Data Dependency No.1, No.2; Forwarding
	ForwardingABUnit();
	MUX_ForwardA();
	MUX_ForwardB();

	EXMEM_IN.readData2 = ALU.operand2;

	// read IDEX_OUT
	MUX_ALUSrcA();
	MUX_ALUSrcB();
	ALUControl();

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

	// set branch address
	BranchAddress();

	// store to EXMEM_IN LATCH
	EXMEM_IN.MemRead = IDEX_OUT.MemRead;
	EXMEM_IN.MemWrite = IDEX_OUT.MemWrite;
	EXMEM_IN.RegWrite = IDEX_OUT.RegWrite;
	EXMEM_IN.MemtoReg = IDEX_OUT.MemtoReg;
	EXMEM_IN.PC_Jump = IDEX_OUT.PC_Jump;
	EXMEM_IN.PC = IDEX_OUT.PC;
	EXMEM_IN.PC4 = IDEX_OUT.PC4;
	EXMEM_IN.ALUresult = ALU.result;
	EXMEM_IN.writeReg = IDEX_OUT.writeReg;

	if (printOption == 1) printf("\n\t [E X]\tResult\t: 0x%08x", ALU.result);
	return;
}

void ForwardingABUnit(void) {
	// default SelectA, SelectB
	FORWARD.SelectA = 0b00;
	FORWARD.SelectB = 0b00;

	// check execution forwarding for SelectA ($0 제외)
	if (EXMEM_OUT.RegWrite && (EXMEM_OUT.writeReg != 0) && (EXMEM_OUT.writeReg == IDEX_OUT.rs_Forward))
		FORWARD.SelectA = 0b01;
	// check memory access forwarding for SelectA ($0 제외, EX 전방 전달 제외)
	else if (MEMWB_OUT.RegWrite && (MEMWB_OUT.writeReg != 0) && (MEMWB_OUT.writeReg == IDEX_OUT.rs_Forward) && (EXMEM_OUT.writeReg != IDEX_OUT.rs_Forward))
		FORWARD.SelectA = 0b10;
	else;

	// check execution forwarding for SelectB ($0 제외)
	if (EXMEM_OUT.RegWrite && (EXMEM_OUT.writeReg != 0) && (EXMEM_OUT.writeReg == IDEX_OUT.rt_Forward))
		FORWARD.SelectB = 0b01;
	// check memory access forwarding for SelectB ($0 제외, EX 전방 전달 제외)
	else if (MEMWB_OUT.RegWrite && (MEMWB_OUT.writeReg != 0) && (MEMWB_OUT.writeReg == IDEX_OUT.rt_Forward) && (EXMEM_OUT.writeReg != IDEX_OUT.rt_Forward))
		FORWARD.SelectB = 0b10;
	else;
	return;
}

void MUX_ForwardA(void) {
	switch (FORWARD.SelectA) {
	case 0:
		ALU.operand1 = IDEX_OUT.readData1;
		break;

	case 1:// forwarding from execution stage
		ALU.operand1 = EXMEM_OUT.ALUresult;
		break;

	case 2:// forwarding from memory access stage
		ALU.operand1 = MEMWB_OUT.writeData;
		break;

	default:
		break;
	}
	return;
}

void MUX_ForwardB(void) {
	switch (FORWARD.SelectB) {
	case 0:
		ALU.operand2 = IDEX_OUT.readData2;
		break;

	case 1:// forwarding from execution stage
		ALU.operand2 = EXMEM_OUT.ALUresult;
		break;

	case 2:// forwarding from memory access stage
		ALU.operand2 = MEMWB_OUT.writeData;
		break;

	default:
		break;
	}
	return;
}

void MUX_ALUSrcA(void) {
	switch (IDEX_OUT.ALUSrcA) {
	case 0:
		ALU.operand1 = ALU.operand1;
		break;

	case 1:
		ALU.operand1 = ALU.operand2;
		break;

	case 2:// lui
		ALU.operand2 = 0;
		break;

	default:
		break;
	}
	return;
}

void MUX_ALUSrcB(void) {
	switch (IDEX_OUT.ALUSrcB) {
	case 0:
		ALU.operand2 = ALU.operand2;
		break;

	case 1:
		ALU.operand2 = IDEX_OUT.extimm;// SignExtimm, ZeroExtimm
		break;

	case 2:// sll, srl
		ALU.operand2 = IDEX_OUT.shamt;
		break;

	default:
		break;
	}
	return;
}

void BranchAddress(void) {// since there is nop instruction after beq and bne, we can determine pc at execution stage, not decode stage
	EXT.branchAddress = IDEX_OUT.PC4 + (IDEX_OUT.extimm << 2);
	EXMEM_IN.Branch = ((IDEX_OUT.BranchEqual == 1) && (ALU.zero == true)) || ((IDEX_OUT.BranchNotEqual == 1) && (!ALU.zero == true));

	switch (predictOption) {
		// 정적 분기 예측(static branch prediction)
	case 1:	// 1. Always not taken, or flush
		if (EXMEM_IN.Branch) {
			IFID_IN.valid = 0;// IFID_IN LATCH flush
			EXMEM_IN.PC_Branch = EXT.branchAddress;
			countInst--;
			countWrongPrediction++;
			countTakenBranch++;
		}
		else {
			if (IDEX_OUT.BranchEqual == 0 && IDEX_OUT.BranchNotEqual == 0)
				countCorrectPrediction += 0;// do nothing
			else {
				countCorrectPrediction++;
				countNotTakenBranch++;
			}
		}
		break;

		// 동적 분기 예측(dynamic branch prediction)
	case 2:// case 2: One level branch prediction
	case 3:// case 3: Two level global branch prediction
	case 4:// case 4: Two level gshare branch prediction

		// [상황 1: 분기를 한다고 예측했는데, 분기를 안 한 경우]
		if (IDEX_OUT.BranchPrediction == 1 && EXMEM_IN.Branch == 0) {
			// wrong prediction; flush both IF and ID stage
			IFID_IN.valid = 0;
			IDEX_IN.valid = 0;
			PC = IDEX_OUT.PC + 4;
			countWrongPrediction++;
			countInst = countInst - 2;

			// 원인 1: 분기 명령어가 아니어서 분기를 안 한 경우
			if (IDEX_OUT.BranchEqual == 0 && IDEX_OUT.BranchNotEqual == 0)
				PC = PC;
			// 원인 2: 분기를 안 하는 분기 명령어여서 분기를 안 한 경우
			else
				countNotTakenBranch++;
		}

		// [상황 2: 분기를 안 한다고 예측했는데, 분기를 한 경우]
		else if (IDEX_OUT.BranchPrediction == 0 && EXMEM_IN.Branch == 1) {
			// wrong prediction; since ID stage is nop, only flush IF stage.
			IFID_IN.valid = 0;
			PC = EXT.branchAddress;
			countWrongPrediction++;
			countTakenBranch++;
			countInst--;
		}

		// [상황 3: 분기를 한다고 예측했는데, 분기를 한 경우]
		else if (IDEX_OUT.BranchPrediction == 1 && EXMEM_IN.Branch == 1) {
			countCorrectPrediction++;
			countTakenBranch++;
		}

		// [상황 4: 분기를 안 한다고 예측했는데, 분기를 안 한 경우]
		else if (IDEX_OUT.BranchPrediction == 0 && EXMEM_IN.Branch == 0) {

			// 원인 1: 분기 명령어가 아니어서 분기를 안 한 경우 
			if (IDEX_OUT.BranchEqual == 0 && IDEX_OUT.BranchNotEqual == 0)
				PC = PC;
			// 원인 2: 분기를 안 하는 분기 명령어여서 분기를 안 한 경우
			else {
				countCorrectPrediction++;
				countNotTakenBranch++;
			}
		}
		else;

		updateBTB();
		updatePHT();
		break;

	default:
		break;
	}
	return;
}

void updateBTB(void) {
	if ((IDEX_OUT.BranchEqual == 1) || (IDEX_OUT.BranchNotEqual == 1)) {
		for (int i = 0; i <= BTBindex; i++) {
			if (BTB[i][0] == IDEX_OUT.PC) return;
		}
		BTB[BTBindex][0] = IDEX_OUT.PC;
		BTB[BTBindex][1] = EXT.branchAddress;
		BTBindex++;
	}
	return;
}

void updatePHT(void) {
	if ((IDEX_OUT.BranchEqual == 1) || (IDEX_OUT.BranchNotEqual == 1)) {
		if (EXMEM_IN.Branch == 1) {
			PHT[IDEX_OUT.PHTindex]++;
			if (PHT[IDEX_OUT.PHTindex] > 0b11) PHT[IDEX_OUT.PHTindex] = 0b11;
			GHR = ((GHR << 1) + 1) & 0x0000003f;// update 6bit GHR
		}
		else if (EXMEM_IN.Branch == 0) {
			PHT[IDEX_OUT.PHTindex]--;
			if (PHT[IDEX_OUT.PHTindex] < 0b00) PHT[IDEX_OUT.PHTindex] = 0b00;
			GHR = ((GHR << 1) + 0) & 0x0000003f;// update 6bit GHR
		}
		else;
	}
	else;
	return;
}

void ALUControl(void) {
	// set ALUCtr control signal
	if (IDEX_OUT.ALUOp == 0b000) {
		switch (IDEX_OUT.extimm & 0x0000003f) {
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

		default:
			break;
		}
	}
	else {
		switch (IDEX_OUT.ALUOp) {
		case 0b001: // andi (by using and)
			CTR.ALUCtr = 0b000;
			break;

		case 0b010: // ori (by using or)
			CTR.ALUCtr = 0b001;
			break;

		case 0b011: // addi, addiu, lw, sw, lui (by using add)
			CTR.ALUCtr = 0b101;
			break;

		case 0b100: // beq, bne (by using sub)
			CTR.ALUCtr = 0b110;
			break;

		case 0b101: // slti, sltiu (by using slt)
			CTR.ALUCtr = 0b111;
			break;

		default:
			CTR.ALUCtr = -1;
			break;
		}
	}
	return;
}

////////////////////////////////////////

void MemoryAccess(void) {
	// set valid
	MEMWB_IN.valid = EXMEM_OUT.valid;

	if (!EXMEM_OUT.valid) {
		if (printOption) printf("\n\t [MEM]\tvalid\t: 0");
		return;
	}

	// load from EXMEM_OUT LATCH
	DataMem.address = EXMEM_OUT.ALUresult;
	DataMem.writeData = EXMEM_OUT.readData2;

	// load word operation
	if (EXMEM_OUT.MemRead == 1) {
		DataMem.readData = DATAMEMORY[DataMem.address / 4];
		countLW++;
		if (printOption) printf("\n\t [MEM]\tLoad\t: 0x%08x from DATAMEMORY[0x%08x]", DATAMEMORY[DataMem.address / 4], DataMem.address / 4);
	}
	// store word operation
	else if (EXMEM_OUT.MemWrite == 1) {
		DATAMEMORY[DataMem.address / 4] = DataMem.writeData;
		countSW++;
		if (printOption) printf("\n\t [MEM]\tStore\t: 0x%08x to DATAMEMORY[0x%08x]", DataMem.writeData, DataMem.address / 4);
	}
	else
		if (printOption) printf("\n\t [MEM]\tvalid\t: 1");

	// store to MEMWB_IN LATCH
	MEMWB_IN.RegWrite = EXMEM_OUT.RegWrite;
	MEMWB_IN.writeReg = EXMEM_OUT.writeReg;
	MEMWB_IN.PC_Jump = EXMEM_OUT.PC_Jump;
	MUX_MemtoReg();
	return;
}

void MUX_MemtoReg(void) {
	switch (EXMEM_OUT.MemtoReg) {
	case 0:
		MEMWB_IN.writeData = EXMEM_OUT.ALUresult;
		break;

	case 1:
		MEMWB_IN.writeData = DataMem.readData;
		break;

	case 2:// jal, jalr
		MEMWB_IN.writeData = EXMEM_OUT.PC4 + 4;
		break;

	default:
		break;
	}
	return;
}

int WriteBack(void) {
	// escape condition, when pc is 0xffffffff
	if (MEMWB_OUT.PC_Jump == 0xffffffff) {
		countInst++;
		if (printOption) printf("\n\t [W B]\tvalid\t: 0");
		return -1;
	}

	// set valid
	if (!MEMWB_OUT.valid) {
		if (printOption) printf("\n\t [W B]\tvalid\t: 0");
		return 0;
	}

	switch (MEMWB_OUT.RegWrite) {
	case 0:
		if (printOption) printf("\n\t [W B]\tvalid\t: 1");
		break;

	case 1:
		Register[MEMWB_OUT.writeReg] = MEMWB_OUT.writeData;
		if (printOption) printf("\n\t [W B]\tR[%d]\t: 0x%08x", MEMWB_OUT.writeReg, MEMWB_OUT.writeData);
		break;

	default:
		break;
	}
	return 0;
}

void LatchUpdate(void) {
	// LATCH_OUT are updated from LATCH_IN
	IFID_OUT = IFID_IN;
	IDEX_OUT = IDEX_IN;;
	EXMEM_OUT = EXMEM_IN;
	MEMWB_OUT = MEMWB_IN;

	// initialize LATCH_IN to 0
	memset(&IFID_IN, 0, sizeof(IFID_IN));
	memset(&IDEX_IN, 0, sizeof(IDEX_IN));
	memset(&EXMEM_IN, 0, sizeof(EXMEM_IN));
	memset(&MEMWB_IN, 0, sizeof(MEMWB_IN));

	if (printOption) printf("\n\t=============================================================");
	return;
}

void BasicImplementation(void) {
	printf("\n\t=============================================================");
	printf("\n\t < Basic Implementation >\n");
	printf("\n\tTotal Cycle\t: %d", countCycle + 1);
	printf("\n\tv0(R[2])\t: %d", Register[2]);
	return;
}

void AdditionalImplementation(void) {
	if (countCorrectPrediction + countWrongPrediction == 0)
		BranchAccuracy = 0.0;
	else
		BranchAccuracy = 100 * countCorrectPrediction / (countCorrectPrediction + countWrongPrediction);

	printf("\n\t=============================================================");
	printf("\n\t < Additional Implementation >\n");
	printf("\n\tExecuted Instruction\t: %d", countInst-4);
	printf("\n\tNop Instruction\t\t: %d", countNop);
	printf("\n\tMemory Access\t\t: %d", countLW + countSW);
	printf("\n\tTaken Branch\t\t: %d", countTakenBranch);
	printf("\n\tNot Taken Branch\t: %d", countNotTakenBranch);
	printf("\n\tJump Instruction\t: %d", countJump);
	printf("\n\tTotal Prediction\t: %d", countPrediction);
	printf("\n\tCorrect Prediction\t: %.0f", countCorrectPrediction);
	printf("\n\tWrong Prediction\t: %.0f", countWrongPrediction);
	printf("\n\tBranch Accuracy\t\t: %.4f%%", BranchAccuracy);
	printf("\n\t=============================================================");
	printf("\n\tPIPELINED MIPS를 이용해주셔서 감사합니다.");
	printf("\n");
	return;
}