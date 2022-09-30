
#include "SingleCycleHeader.h"

void DoMemory(void) {
	FILE* binFile;
	int Index = 0, Error = 0;

	char fileName[32];
	fileName[31] = 0;
	printf("\n\t=============================================================");
	printf("\n\tSINGLE CYCLE MIPS에 오신 것을 환영합니다.");
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
		fread(&Memory[Index], sizeof(int), 1, binFile);

		if (feof(binFile))
			break;
		else {
			InstForm.InstData =
				(((Memory[Index] & 0x000000ff) << 24) & 0xff000000) |
				(((Memory[Index] & 0x0000ff00) << 8) & 0x00ff0000) |
				(((Memory[Index] & 0x00ff0000) >> 8) & 0x0000ff00) |
				(((Memory[Index] & 0xff000000) >> 24) & 0x000000ff);
			Memory[Index] = InstForm.InstData;
			Index++;
		}
	}
	fclose(binFile);
	printf("\t=============================================================");
	printf("\n\t모든 실행 결과를 보겠습니까? (0 또는 1) > ");

	while (1) {
		scanf_s("%d", &UserSelect);
		if ((UserSelect != 0) && (UserSelect != 1)) {
			printf("\n\t다시 입력하세요. > ");
			continue;
		}
		else
			break;
	}

	printf("\t=============================================================");
	printf("\n\t곧 프로그램이 시작합니다.");
	Sleep(3000);
	printf("\n\t=============================================================");
	printf("\n\t%s을 처리하는 중입니다. 잠시만 기다려주세요.", fileName);
	return;
}

void DoFetch(void) {
	binIndex = PC / 4;
	PC = PC + 4;
	return;
}

void DoDecode(void) {
	InstInfo.opcode = ((Memory[binIndex] & 0xfc000000) >> 26) & 0x0000003f;
	InstInfo.rs = ((Memory[binIndex] & 0x03e00000) >> 21) & 0x0000001f;
	InstInfo.rt = ((Memory[binIndex] & 0x001f0000) >> 16) & 0x0000001f;
	InstInfo.rd = ((Memory[binIndex] & 0x0000f800) >> 11) & 0x0000001f;
	InstInfo.shamt = ((Memory[binIndex] & 0x000007c0) >> 6) & 0x0000001f;
	InstInfo.funct = Memory[binIndex] & 0x0000003f;
	InstInfo.imme = Memory[binIndex] & 0x0000ffff;
	InstInfo.addr = Memory[binIndex] & 0x03ffffff;

	if (InstInfo.opcode == 0x0)
		countR++;
	else if (InstInfo.opcode == 0x2 || InstInfo.opcode == 0x3)
		countJ++;
	else
		countI++;

	DoControl();
	DoRegister();
	SignExtend();
	ZeroExtend();
	return;
}

void DoControl(void) {

	/***** Default Control *****/
	CTR.RegDst = 0;
	CTR.Jump = 0;
	CTR.BranchEqual = 0;
	CTR.BranchNotEqual = 0;
	CTR.ALUOp = 0b000;
	CTR.ALUSrc1 = 0;
	CTR.ALUSrc2 = 1;
	CTR.MemRead = 0;
	CTR.MemWrite = 0;
	CTR.MemtoReg = 0;
	CTR.RegWrite = 1;

	/***** RegDst Control *****/
	if (InstInfo.opcode == 0x0) // R-type
		CTR.RegDst = 1;
	if (InstInfo.opcode == 0x3 || (InstInfo.opcode == 0x0 && InstInfo.funct == 0x9)) // jal, jalr
		CTR.RegDst = 2;

	/***** MemtoReg Control *****/
	if (InstInfo.opcode == 0x23) // lw
		CTR.MemtoReg = 1;
	if (InstInfo.opcode == 0x3 || (InstInfo.opcode == 0x0 && InstInfo.funct == 0x9)) // jal, jalr
		CTR.MemtoReg = 2;
	if (InstInfo.opcode == 0xf) // lui
		CTR.MemtoReg = 3;

	/***** MemRead, MemWrite Control *****/
	if (InstInfo.opcode == 0x23) // lw
		CTR.MemRead = 1;
	if (InstInfo.opcode == 0x2b) // sw
		CTR.MemWrite = 1;

	/***** Jump Control *****/
	if (InstInfo.opcode == 0x2 || InstInfo.opcode == 0x3) // j, jal
		CTR.Jump = 1;
	if ((InstInfo.opcode == 0x0 && InstInfo.funct == 0x8) || (InstInfo.opcode == 0x0 && InstInfo.funct == 0x9)) // jr, jalr
		CTR.Jump = 2;

	/***** BranchEqual, BranchNotEqual Control *****/
	if (InstInfo.opcode == 0x4) // beq
		CTR.BranchEqual = 1;
	if (InstInfo.opcode == 0x5) // bne
		CTR.BranchNotEqual = 1;

	/***** RegWrite Control *****/
	if (InstInfo.opcode == 0x4 || InstInfo.opcode == 0x5 || InstInfo.opcode == 0x2b || InstInfo.opcode == 0x2 || (InstInfo.opcode == 0x0 && InstInfo.funct == 0x8))
		CTR.RegWrite = 0;

	/***** ALUSrc Control *****/
	if (InstInfo.opcode == 0x0 || InstInfo.opcode == 0x4 || InstInfo.opcode == 0x5) // R-type, beq, bne 제외
		CTR.ALUSrc2 = 0;
	if ((InstInfo.opcode == 0x0 && InstInfo.funct == 0x0) || (InstInfo.opcode == 0x0 && InstInfo.funct == 0x2)) { // sll, srl
		CTR.ALUSrc1 = 1;
		CTR.ALUSrc2 = 2;
	}

	/***** ALUOp Control *****/
	if (InstInfo.opcode == 0x0)
		CTR.ALUOp = 0b000;
	if (InstInfo.opcode == 0xc)
		CTR.ALUOp = 0b001;
	if (InstInfo.opcode == 0xd)
		CTR.ALUOp = 0b010;
	if (InstInfo.opcode == 0x8 || InstInfo.opcode == 0x9 || InstInfo.opcode == 0x23 || InstInfo.opcode == 0x2b)
		CTR.ALUOp = 0b011;
	if (InstInfo.opcode == 0x4 || InstInfo.opcode == 0x5)
		CTR.ALUOp = 0b100;
	if (InstInfo.opcode == 0xa || InstInfo.opcode == 0xb)
		CTR.ALUOp = 0b101;

	return;
}

void DoRegister(void) {
	REG.readData1 = Register[InstInfo.rs];
	REG.readData2 = Register[InstInfo.rt];
	MUX1();
	return;
}

void DoALUControl(void) {
	switch (CTR.ALUOp) {
	case 0b000:
		if (InstInfo.funct == 0x24) // and
			CTR.ALUCtr = 0b000;
		else if (InstInfo.funct == 0x25) // or
			CTR.ALUCtr = 0b001;
		else if (InstInfo.funct == 0x27) // nor
			CTR.ALUCtr = 0b010;
		else if (InstInfo.funct == 0x00) // sll
			CTR.ALUCtr = 0b011;
		else if (InstInfo.funct == 0x02) // srl
			CTR.ALUCtr = 0b100;
		else if (InstInfo.funct == 0x20 || InstInfo.funct == 0x21) // add, addu
			CTR.ALUCtr = 0b101;
		else if (InstInfo.funct == 0x22 || InstInfo.funct == 0x23) // sub, subu
			CTR.ALUCtr = 0b110;
		else if (InstInfo.funct == 0x2a || InstInfo.funct == 0x2b) // slt, sltu
			CTR.ALUCtr = 0b111;
		else if (InstInfo.funct == 0x8) // jr
			CTR.ALUCtr = -1;
		else if (InstInfo.funct == 0x9) // jalr
			CTR.ALUCtr = -1;
		else;
		break;

	case 0b001: // andi (and 연산 이용)
		CTR.ALUCtr = 0b000;
		break;

	case 0b010: // ori (or 연산 이용)
		CTR.ALUCtr = 0b001;
		break;

	case 0b011: // addi, addiu, lw, sw (add 연산 이용)
		CTR.ALUCtr = 0b101;
		break;

	case 0b100: // beq, bne (sub 연산 이용)
		CTR.ALUCtr = 0b110;
		break;

	case 0b101: // slti, sltiu (slt 연산 이용)
		CTR.ALUCtr = 0b111;
		break;

	default:
		CTR.ALUCtr = -1;
		break;
	}
	return;
}

void DoExecute(void) {
	DoALUControl();
	MUX2_1();
	MUX2_2();

	switch (CTR.ALUCtr) {
	case 0b000: // and
		ALU.result = ALU.data1 & ALU.data2;
		break;

	case 0b001: // or
		ALU.result = ALU.data1 | ALU.data2;
		break;

	case 0b010: // nor
		ALU.result = ~(ALU.data1 | ALU.data2);
		break;

	case 0b011: // sll
		ALU.result = ALU.data1 << ALU.data2;
		break;

	case 0b100: // srl
		ALU.result = ALU.data1 >> ALU.data2;
		break;

	case 0b101: // add
		ALU.result = ALU.data1 + ALU.data2;
		break;

	case 0b110: // sub
		ALU.result = ALU.data1 - ALU.data2;
		if (ALU.result == 0)
			ALU.zero = true;
		else
			ALU.zero = false;
		break;

	case 0b111: // slt
		ALU.result = (ALU.data1 < ALU.data2) ? 1 : 0;
		break;

	default:
		break;
	}
	return;
}

void DoMemoryAccess(void) {
	DataMem.address = ALU.result;
	DataMem.writeData = REG.readData2;

	if (CTR.MemRead == 1) {
		DataMem.readData = Memory[DataMem.address / 4];
		countMemoryAccess++;
	}
	else if (CTR.MemWrite == 1) {
		Memory[DataMem.address / 4] = DataMem.writeData;
		countMemoryAccess++;
	}
	else;
	return;
}

void DoWriteBack(void) {
	MUX3();
	switch (CTR.RegWrite) {
	case 0:
		break;

	case 1:
		Register[REG.writeReg] = REG.writeData;
		break;

	default:
		break;
	}
	MUX4();
	MUX5();
	return;
}

void MUX1(void) {// RegDst MUX
	switch (CTR.RegDst) {
	case 0:
		REG.writeReg = InstInfo.rt;
		break;

	case 1:
		REG.writeReg = InstInfo.rd;
		break;

	case 2:// jal, jalr
		REG.writeReg = 31;
		break;

	default:
		break;
	}
	return;
}

void MUX2_1(void) {// ALUSrc1 MUX
	switch (CTR.ALUSrc1) {
	case 0:
		ALU.data1 = REG.readData1;
		break;

	case 1:// sll, srl
		ALU.data1 = REG.readData2;
		break;

	default:
		break;
	}
	return;
}

void MUX2_2(void) {// ALUSrc2 MUX
	switch (CTR.ALUSrc2) {
	case 0:
		ALU.data2 = REG.readData2;
		break;

	case 1:// InstInfo.funct에서 InstInfo.opcode로 수정했다. 찾는데 많은 시간을 많이 소모했다.
		if (InstInfo.opcode == 0xc || InstInfo.opcode == 0xd)
			ALU.data2 = EXT.Zero;
		else
			ALU.data2 = EXT.Sign;
		break;

	case 2:// sll, srl
		ALU.data2 = InstInfo.shamt;
		break;

	default:
		break;
	}
	return;
}

void MUX3(void) {// MemtoReg MUX
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

	case 3:// lui
		REG.writeData = InstInfo.imme << 16;
		break;

	default:
		break;
	}
	return;
}

void MUX4(void) {// Branch MUX
	BranchAddress();

	if ((CTR.BranchEqual == 1) && (ALU.zero == true)) {// beq
		PC = PC + EXT.branchAddress;
		TakenBranch++;
	}
	else if ((CTR.BranchNotEqual == 1) && (!ALU.zero == true)) {// bne
		PC = PC + EXT.branchAddress;
		TakenBranch++;
	}
	else
		PC = PC;
	return;
}

void MUX5(void) {// Jump MUX
	JumpAddress();

	switch (CTR.Jump) {
	case 0:
		PC = PC;
		break;

	case 1:// j, jal
		PC = EXT.jumpAddress;
		break;

	case 2:// jr, jalr
		PC = ALU.data1;
		break;

	default:
		break;
	}
	return;
}

void JumpAddress(void) {
	EXT.jumpAddress = (PC & 0xf0000000) | (InstInfo.addr << 2);
	return;
}

void BranchAddress(void) {
	switch (InstInfo.imme >> 15) {
	case 0:
		EXT.branchAddress = (0x00000000 | InstInfo.imme) << 2;
		break;

	case 1:
		EXT.branchAddress = (0xffff0000 | InstInfo.imme) << 2;
		break;

	default:
		break;
	}
	return;
}

void SignExtend(void) {
	switch (InstInfo.imme >> 15) {
	case 0:
		EXT.Sign = InstInfo.imme | 0x00000000;
		break;

	case 1:
		EXT.Sign = InstInfo.imme | 0xffff0000;
		break;

	default:
		break;
	}
	return;
}

void ZeroExtend(void) {
	EXT.Zero = InstInfo.imme | 0x00000000;
	return;
}

void BasicState(void) {
	if (UserSelect == 1) {
		printf("\n\t=============================================================");
		printf("\n\t%d번째 명령\t: 0x%08x", countInst, Memory[binIndex]);
		printf("\n\tPC\t\t: 0x%08x", PC - 4);
		printf("\n\tR[rs](R[%02d])\t: 0x%08x", InstInfo.rs, Register[InstInfo.rs]);
		printf("\n\tR[rt](R[%02d])\t: 0x%08x", InstInfo.rt, Register[InstInfo.rt]);
		printf("\n\tR[rd](R[%02d])\t: 0x%08x", InstInfo.rd, Register[InstInfo.rd]);

		if(CTR.MemWrite == 1)
			printf("\n\tM[0x%08x]\t: 0x%08x", DataMem.address / 4, DataMem.writeData);

		printf("\n\tv0(R[2])\t: %d", Register[2]);
	}
	else;
	return;
}

void AdditionalState(void) {
	printf("\n\t=============================================================");
	printf("\n\tv0(R[2])\t\t: %d", Register[2]);
	printf("\n\tExecuted Instruction\t: %d", countInst);
	printf("\n\tR-type Instruction\t: %d", countR);
	printf("\n\tI-type Instruction\t: %d", countI);
	printf("\n\tJ-type Instruction\t: %d", countJ);
	printf("\n\tMemory Access\t\t: %d", countMemoryAccess);
	printf("\n\tTaken Branch\t\t: %d", TakenBranch);
	printf("\n\t프로그램 실행 시간\t: %f초", TimeExcuted);
	printf("\n\t=============================================================");
	printf("\n\tSINCLE CYCLE MIPS를 이용해주셔서 감사합니다.");
	printf("\n");
	return;
}