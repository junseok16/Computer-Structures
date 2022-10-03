
/**
*	�� ���α׷��� MIPS ISA�� �Ϻθ� ����� �ؽ�Ʈ ������ �о� ����� ���ִ� ���α׷��Դϴ�.
*	����		: ��ǻ�ͱ����͸�������μ���
*	����		: HW1 Simple Calculator
*	�а�		: ����Ͻý��۰��а�
*	�̸�		: Ź�ؼ�
*	�й�		: 32164809
*	��� ����	: ����ȯ
*	������		: 2021-03-20
*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* ��� [Opcode, Operand1, Operand2]�� ������ ������ ��� */
typedef struct DataNode {
	char Opcode[10];
	char Operand1[10];
	char Operand2[10];
	int count;
	struct DataNode* next;
} DataNode;

/* ù ��° ������ ��带 ����ų ��� ��� */
typedef struct HeadNode {
	int count;
	DataNode* head;
} HeadNode;

/* ���� �Ҵ��� ������ ��带 �ʱ�ȭ */
DataNode* createDataNode(void) {
	DataNode* NewDataNode = (DataNode*)malloc(sizeof(DataNode));
	if (NewDataNode == NULL) return NULL;

	NewDataNode->count = 0;
	NewDataNode->next = NULL;
	return NewDataNode;
}

/* ���� �Ҵ��� ��� ��带 �ʱ�ȭ */
HeadNode* createHeadNode(void) {
	HeadNode* NewHeadNode = (HeadNode*)malloc(sizeof(HeadNode));
	if (NewHeadNode == NULL) return NULL;

	NewHeadNode->count = 0;
	NewHeadNode->head = NULL;
	return NewHeadNode;
}

DataNode* FetchStage(HeadNode* IP);
int ReadText(HeadNode* IP);
void LoadMemory(HeadNode* IP, char Opcode[], char Operand1[], char Operand2[]);
int DecodeAndExcuteStage(HeadNode* IP, HeadNode* HN, DataNode* IR);
void DecodeOperand(DataNode* IR, int* NumOperand1, int* NumOperand2);
int R[10] = { 0 };

int main(void)
{
	HeadNode* IP = createHeadNode();
	HeadNode* HN = createHeadNode();
	DataNode* IR;
	int Err1, Err2;

	Err1 = ReadText(IP);
	HN->head = IP->head;
	if (Err1 == -1) return 0;

	while ((strcmp(IP->head->Opcode, "H") && strcmp(IP->head->Opcode, "H\n")) != 0) {	// ���� ��ɾ ������ �ݺ����� �������´�.
		IR = FetchStage(IP);
		Err2 = DecodeAndExcuteStage(IP, HN, IR);
		if (Err2 == -1) break;															// ���� ��Ȳ�� �߻��ϸ� �ݺ����� �������´�.
	}

	printf("\t=====================================================\n");
	printf("\t���α׷��� �����մϴ�.\n");
	printf("\t=====================================================\n");

	return 0;
}

/* Input.txt�� ����� �о�� ������ ��带 �����ϱ� ���� ���� �μ��� Ȯ���ϴ� �Լ� */
int ReadText(HeadNode* IP) {

	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, "Input.txt", "r");				// Input.txt�� �б� ���� ����.
	if (err == 0) {
		printf("\t=====================================================\n");
		printf("\tInput.txt�� ���������� ���Ƚ��ϴ�.\n");
		printf("\t=====================================================\n");
	}
	else {
		printf("\t=====================================================\n");
		printf("\tInput.txt�� ������ �ʾҽ��ϴ�.\n");
		printf("\t=====================================================\n");
		return -1;										// ����! Input.txt�� ���������� ������ �ʾ��� ���, -1�� ��ȯ�Ѵ�.
	}

	// ex. [+ 0x2 0x1]
	while (1) {
		char MaxArr[30];
		char* context = NULL;
		char* TextLine;
		char* Opcode;
		char* Operand1;
		char* Operand2;

		TextLine = fgets(MaxArr, 30, fp);
		if (TextLine == NULL) break;					// �ؽ�Ʈ ������ ����� ��� �о����� �ݺ������� �������´�.

		Opcode = strtok_s(TextLine, " ", &context);		// ù ��° ���� ��, [+]�� Opcode�� �����Ѵ�.
		Operand1 = strtok_s(NULL, " ", &context);		// �� ��° ���� ��, [0x2]�� Operand1�� �����Ѵ�.
		Operand2 = strtok_s(NULL, " ", &context);		// �� ���� ���� ��, [0x1]�� Operand2�� �����Ѵ�.
		LoadMemory(IP, Opcode, Operand1, Operand2);		// [+], [0x2], [0x1]�� ReadMemery �Լ��� �μ��� �����Ѵ�.
	}
	fclose(fp);											// Input.txt�� �ݴ´�.
	return 0;
}

/* ���޹��� �μ��� ������ ������ ��带 �����ϰ�, ���� ���� ����Ʈ�� �����ϴ� �Լ� */
void LoadMemory(HeadNode* IP, char Opcode[], char Operand1[], char Operand2[]) {

	DataNode* NewDataNode = createDataNode();			// ������ ��� �ϳ��� �����Ѵ�.
	strcpy_s(NewDataNode->Opcode, 10, Opcode);			// Opcode�� ������ ������ ����� Opcode�� �����Ѵ�.

	if (Operand1 == NULL)								// Operand1�� ������ ������ ����� Operand1�� �����Ѵ�.
		strcpy_s(NewDataNode->Operand1, 10, "0x0");
	else
		strcpy_s(NewDataNode->Operand1, 10, Operand1);

	if (Operand2 == NULL)								// Operand2�� ������ ������ ����� Operand2�� �����Ѵ�.
		strcpy_s(NewDataNode->Operand2, 10, "0x0");
	else
		strcpy_s(NewDataNode->Operand2, 10, Operand2);
	
	/* ������ ��带 ���� ���� ����Ʈ�� ���� */
	if (IP->head == NULL) {								// ù ��° ������ ���
		IP->head = NewDataNode;
		NewDataNode->next = NULL;
		NewDataNode->count++;
	}
	else {												// �� �� ������ ���
		DataNode* Tail = IP->head;

		while (Tail->next != NULL)
			Tail = Tail->next;
		Tail->next = NewDataNode;
		NewDataNode->count = (Tail->count) + 1;
		NewDataNode->next = NULL;
	}

	IP->count++;
	return;
}

/* ��� ������(Instruction Pointer)�� ����Ű�� ����� �д� �Լ�*/
DataNode* FetchStage(HeadNode* IP) {

	DataNode* IR = createDataNode();
	strcpy_s(IR->Opcode, 10, IP->head->Opcode);
	strcpy_s(IR->Operand1, 10, IP->head->Operand1);
	strcpy_s(IR->Operand2, 10, IP->head->Operand2);
	IR->count = IP->head->count;

	return IR;
}

/* Opcode, Operand1, Operand2�� �ص��ϰ� ��ɾ �°� ����� �ϴ� �Լ� */
int DecodeAndExcuteStage(HeadNode* IP, HeadNode* HN, DataNode* IR) {

	int NumOperand1, NumOperand2 = 0;

	// Opcode�� +�� ��
	if (strcmp(IR->Opcode, "+") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[0] = NumOperand1 + NumOperand2;								// NumOperand1 + NumOperand2
		printf("\tR0: %d = %d + %d\n", R[0], NumOperand1, NumOperand2);
		IP->head = IP->head->next;										// ��� �����Ͱ� ���� ��带 ����Ų��.
	}

	// Opcode�� -�� ��
	else if (strcmp(IR->Opcode, "-") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[0] = NumOperand1 - NumOperand2;								// NumOperand1 - NumOperand2
		printf("\tR0: %d = %d - %d\n", R[0], NumOperand1, NumOperand2);
		IP->head = IP->head->next;										// ��� �����Ͱ� ���� ��带 ����Ų��.
	}

	// Opcode�� *�� ��
	else if (strcmp(IR->Opcode, "*") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[0] = NumOperand1 * NumOperand2;								// NumOperand1 * NumOperand2
		printf("\tR0: %d = %d * %d\n", R[0], NumOperand1, NumOperand2);
		IP->head = IP->head->next;										// ��� �����Ͱ� ���� ��带 ����Ų��.
	}

	// Opcode�� /�� ��
	else if (strcmp(IR->Opcode, "/") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);

		if (NumOperand2 == 0) {												// ����! 0���� ������ ���� �����Ѵ�.
			printf("\t=====================================================\n");
			printf("\t%d ��° ���� Operand2�� 0�� �� �����ϴ�. �����ϼ���.\n", IR->count);
			printf("\t=====================================================\n");
			return  -1;
		}
		else {
			R[0] = NumOperand1 / NumOperand2;								// NumOperand1 / NumOperand2
			printf("\tR0: %d = %d / %d\n", R[0], NumOperand1, NumOperand2);
			IP->head = IP->head->next;										// ��� �����Ͱ� ���� ��带 ����Ų��.
		}
	}

	// Opcode�� M�� ��
	else if (strcmp(IR->Opcode, "M") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[NumOperand1] = NumOperand2;							// [NumOperand1]�� �������Ϳ� Operand2�� �����Ѵ�.
		printf("\tR%d: %d\n", NumOperand1, NumOperand2);
		IP->head = IP->head->next;								// ��� �����Ͱ� ���� ��带 ����Ų��.
	}

	// Opcode�� C�� ��
	else if (strcmp(IR->Opcode, "C") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		if (NumOperand1 >= NumOperand2) {						// NumOperand1 >= NumOperand2�� R0�� 0�� �����Ѵ�.
			R[0] = 0;
			printf("\tR0: %d\n", R[0]);
		}
		else {													// NumOperand1 < NumOperand2�� R0�� 1�� �����Ѵ�.
			R[0] = 1;
			printf("\tR0: %d\n", R[0]);
		}
		IP->head = IP->head->next;								// ��� �����Ͱ� ���� ��带 ����Ų��.
	}

	// Opcode�� B�� ��
	else if (strcmp(IR->Opcode, "B") == 0) {
		if (R[0] == 1) {										// R0�� 1�̸� ��� �����Ͱ� [NumOperand1]��° ��带 ����Ų��.
			NumOperand1 = strtol(IR->Operand1, NULL, 16);
			IP->head = HN->head;

			for (; NumOperand1 >= 2; NumOperand1--)
				IP->head = IP->head->next;
		}
		else IP->head = IP->head->next;							// R0�� 0�̸� ��� �����Ͱ� ���� ��带 ����Ų��.
	}

	// Opcode�� J�� ��
	else if (strcmp(IR->Opcode, "J") == 0) {

		NumOperand1 = strtol(IR->Operand1, NULL, 16);
		IP->head = HN->head;

		for (; NumOperand1 >= 2; NumOperand1--)					// ��� �����Ͱ� [NumOperand1]��° ��带 ����Ų��.
			IP->head = IP->head->next;
	}

	// ����! Opcode�� �߸� �Է����� ��
	else {
		printf("\t=====================================================\n");
		printf("\t%d ��° ���� Opcode�� �߸��Ǿ����ϴ�. �����ϼ���.\n", IR->count);
		printf("\t=====================================================\n");
		return  -1;												// -1�� ��ȯ�Ѵ�.
	}

	return;
}

/* Operand�� ������������ 16���� �������� ������ NumOperand1, Numoperand2�� ���� �����ϴ� �Լ� */
void DecodeOperand(DataNode* IR, int* NumOperand1, int* NumOperand2) {
	
	if (IR->Operand1[0] == 'R') {						// Operand1�� ���������� ���, ���ڿ� �������� ��ȣ�� ������ ��ȯ�Ѵ�.
		int temp = atoi(&IR->Operand1[1]);
		
		if(strcmp(IR->Opcode, "M") == 0)
			*NumOperand1 = temp;

		else *NumOperand1 = R[temp];					// �ش� �������� ���� NumOperand1�� �����Ѵ�.
	}
	else												// 16������ ���, 16������ 10������ ��ȯ�� NumOperand1�� �����Ѵ�,
		*NumOperand1 = strtol(IR->Operand1, NULL, 16);

	if (IR->Operand2[0] == 'R') {						// Operand2�� ���������� ���, ���ڿ� �������� ��ȣ�� ������ ��ȯ�Ѵ�.
		int temp = atoi(&IR->Operand2[1]);
		*NumOperand2 = R[temp];							// �ش� ���������� ���� NumOperand2�� �����Ѵ�.
	}
	else												// 16������ ���, 16������ 10������ ��ȯ�� NumOperand2�� �����Ѵ�.
		*NumOperand2 = strtol(IR->Operand2, NULL, 16);
	return;
}