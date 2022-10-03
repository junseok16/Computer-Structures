
/**
*	이 프로그램은 MIPS ISA의 일부를 사용한 텍스트 파일을 읽어 계산을 해주는 프로그램입니다.
*	과목		: 컴퓨터구조와모바일프로세서
*	과제		: HW1 Simple Calculator
*	학과		: 모바일시스템공학과
*	이름		: 탁준석
*	학번		: 32164809
*	담당 교수	: 유시환
*	제출일		: 2021-03-20
*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* 명령 [Opcode, Operand1, Operand2]를 저장할 데이터 노드 */
typedef struct DataNode {
	char Opcode[10];
	char Operand1[10];
	char Operand2[10];
	int count;
	struct DataNode* next;
} DataNode;

/* 첫 번째 데이터 노드를 가리킬 헤드 노드 */
typedef struct HeadNode {
	int count;
	DataNode* head;
} HeadNode;

/* 동적 할당한 데이터 노드를 초기화 */
DataNode* createDataNode(void) {
	DataNode* NewDataNode = (DataNode*)malloc(sizeof(DataNode));
	if (NewDataNode == NULL) return NULL;

	NewDataNode->count = 0;
	NewDataNode->next = NULL;
	return NewDataNode;
}

/* 동적 할당한 헤드 노드를 초기화 */
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

	while ((strcmp(IP->head->Opcode, "H") && strcmp(IP->head->Opcode, "H\n")) != 0) {	// 정지 명령어를 만나면 반복문을 빠져나온다.
		IR = FetchStage(IP);
		Err2 = DecodeAndExcuteStage(IP, HN, IR);
		if (Err2 == -1) break;															// 예외 상황이 발생하면 반복문을 빠져나온다.
	}

	printf("\t=====================================================\n");
	printf("\t프로그램을 종료합니다.\n");
	printf("\t=====================================================\n");

	return 0;
}

/* Input.txt의 명령을 읽어와 데이터 노드를 생성하기 위해 넣을 인수를 확보하는 함수 */
int ReadText(HeadNode* IP) {

	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, "Input.txt", "r");				// Input.txt를 읽기 모드로 연다.
	if (err == 0) {
		printf("\t=====================================================\n");
		printf("\tInput.txt가 정상적으로 열렸습니다.\n");
		printf("\t=====================================================\n");
	}
	else {
		printf("\t=====================================================\n");
		printf("\tInput.txt가 열리지 않았습니다.\n");
		printf("\t=====================================================\n");
		return -1;										// 예외! Input.txt가 정상적으로 열리지 않았을 경우, -1을 반환한다.
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
		if (TextLine == NULL) break;					// 텍스트 파일의 명령을 모두 읽었으면 반복문에서 빠져나온다.

		Opcode = strtok_s(TextLine, " ", &context);		// 첫 번째 공백 전, [+]를 Opcode에 저장한다.
		Operand1 = strtok_s(NULL, " ", &context);		// 두 번째 공백 전, [0x2]을 Operand1에 저장한다.
		Operand2 = strtok_s(NULL, " ", &context);		// 세 번재 공백 전, [0x1]을 Operand2에 저장한다.
		LoadMemory(IP, Opcode, Operand1, Operand2);		// [+], [0x2], [0x1]을 ReadMemery 함수의 인수로 전달한다.
	}
	fclose(fp);											// Input.txt를 닫는다.
	return 0;
}

/* 전달받은 인수를 가지고 데이터 노드를 생성하고, 선형 연결 리스트로 연결하는 함수 */
void LoadMemory(HeadNode* IP, char Opcode[], char Operand1[], char Operand2[]) {

	DataNode* NewDataNode = createDataNode();			// 데이터 노드 하나를 생성한다.
	strcpy_s(NewDataNode->Opcode, 10, Opcode);			// Opcode를 복사해 데이터 노드의 Opcode에 저장한다.

	if (Operand1 == NULL)								// Operand1를 복사해 데이터 노드의 Operand1에 저장한다.
		strcpy_s(NewDataNode->Operand1, 10, "0x0");
	else
		strcpy_s(NewDataNode->Operand1, 10, Operand1);

	if (Operand2 == NULL)								// Operand2를 복사해 데이터 노드의 Operand2에 저장한다.
		strcpy_s(NewDataNode->Operand2, 10, "0x0");
	else
		strcpy_s(NewDataNode->Operand2, 10, Operand2);
	
	/* 데이터 노드를 선형 연결 리스트로 연결 */
	if (IP->head == NULL) {								// 첫 번째 데이터 노드
		IP->head = NewDataNode;
		NewDataNode->next = NULL;
		NewDataNode->count++;
	}
	else {												// 그 외 데이터 노드
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

/* 명령 포인터(Instruction Pointer)가 가리키는 명령을 읽는 함수*/
DataNode* FetchStage(HeadNode* IP) {

	DataNode* IR = createDataNode();
	strcpy_s(IR->Opcode, 10, IP->head->Opcode);
	strcpy_s(IR->Operand1, 10, IP->head->Operand1);
	strcpy_s(IR->Operand2, 10, IP->head->Operand2);
	IR->count = IP->head->count;

	return IR;
}

/* Opcode, Operand1, Operand2를 해독하고 명령어에 맞게 계산을 하는 함수 */
int DecodeAndExcuteStage(HeadNode* IP, HeadNode* HN, DataNode* IR) {

	int NumOperand1, NumOperand2 = 0;

	// Opcode가 +일 때
	if (strcmp(IR->Opcode, "+") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[0] = NumOperand1 + NumOperand2;								// NumOperand1 + NumOperand2
		printf("\tR0: %d = %d + %d\n", R[0], NumOperand1, NumOperand2);
		IP->head = IP->head->next;										// 명령 포인터가 다음 노드를 가리킨다.
	}

	// Opcode가 -일 때
	else if (strcmp(IR->Opcode, "-") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[0] = NumOperand1 - NumOperand2;								// NumOperand1 - NumOperand2
		printf("\tR0: %d = %d - %d\n", R[0], NumOperand1, NumOperand2);
		IP->head = IP->head->next;										// 명령 포인터가 다음 노드를 가리킨다.
	}

	// Opcode가 *일 때
	else if (strcmp(IR->Opcode, "*") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[0] = NumOperand1 * NumOperand2;								// NumOperand1 * NumOperand2
		printf("\tR0: %d = %d * %d\n", R[0], NumOperand1, NumOperand2);
		IP->head = IP->head->next;										// 명령 포인터가 다음 노드를 가리킨다.
	}

	// Opcode가 /일 때
	else if (strcmp(IR->Opcode, "/") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);

		if (NumOperand2 == 0) {												// 예외! 0으로 나누는 경우는 제외한다.
			printf("\t=====================================================\n");
			printf("\t%d 번째 줄의 Operand2는 0일 수 없습니다. 수정하세요.\n", IR->count);
			printf("\t=====================================================\n");
			return  -1;
		}
		else {
			R[0] = NumOperand1 / NumOperand2;								// NumOperand1 / NumOperand2
			printf("\tR0: %d = %d / %d\n", R[0], NumOperand1, NumOperand2);
			IP->head = IP->head->next;										// 명령 포인터가 다음 노드를 가리킨다.
		}
	}

	// Opcode가 M일 때
	else if (strcmp(IR->Opcode, "M") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		R[NumOperand1] = NumOperand2;							// [NumOperand1]번 레지스터에 Operand2를 저장한다.
		printf("\tR%d: %d\n", NumOperand1, NumOperand2);
		IP->head = IP->head->next;								// 명령 포인터가 다음 노드를 가리킨다.
	}

	// Opcode가 C일 때
	else if (strcmp(IR->Opcode, "C") == 0) {
		DecodeOperand(IR, &NumOperand1, &NumOperand2);
		if (NumOperand1 >= NumOperand2) {						// NumOperand1 >= NumOperand2면 R0에 0을 저장한다.
			R[0] = 0;
			printf("\tR0: %d\n", R[0]);
		}
		else {													// NumOperand1 < NumOperand2면 R0에 1을 저장한다.
			R[0] = 1;
			printf("\tR0: %d\n", R[0]);
		}
		IP->head = IP->head->next;								// 명령 포인터가 다음 노드를 가리킨다.
	}

	// Opcode가 B일 때
	else if (strcmp(IR->Opcode, "B") == 0) {
		if (R[0] == 1) {										// R0가 1이면 명령 포인터가 [NumOperand1]번째 노드를 가리킨다.
			NumOperand1 = strtol(IR->Operand1, NULL, 16);
			IP->head = HN->head;

			for (; NumOperand1 >= 2; NumOperand1--)
				IP->head = IP->head->next;
		}
		else IP->head = IP->head->next;							// R0가 0이면 명령 포인터가 다음 노드를 가리킨다.
	}

	// Opcode가 J일 때
	else if (strcmp(IR->Opcode, "J") == 0) {

		NumOperand1 = strtol(IR->Operand1, NULL, 16);
		IP->head = HN->head;

		for (; NumOperand1 >= 2; NumOperand1--)					// 명령 포인터가 [NumOperand1]번째 노드를 가리킨다.
			IP->head = IP->head->next;
	}

	// 예외! Opcode를 잘못 입력했을 때
	else {
		printf("\t=====================================================\n");
		printf("\t%d 번째 줄의 Opcode가 잘못되었습니다. 수정하세요.\n", IR->count);
		printf("\t=====================================================\n");
		return  -1;												// -1을 반환한다.
	}

	return;
}

/* Operand가 레지스터인지 16진수 숫자인지 구분해 NumOperand1, Numoperand2에 각각 저장하는 함수 */
void DecodeOperand(DataNode* IR, int* NumOperand1, int* NumOperand2) {
	
	if (IR->Operand1[0] == 'R') {						// Operand1이 레지스터일 경우, 문자열 레지스터 번호를 정수로 변환한다.
		int temp = atoi(&IR->Operand1[1]);
		
		if(strcmp(IR->Opcode, "M") == 0)
			*NumOperand1 = temp;

		else *NumOperand1 = R[temp];					// 해당 레지스터 값을 NumOperand1에 저장한다.
	}
	else												// 16진수일 경우, 16진수를 10진수로 변환해 NumOperand1에 저장한다,
		*NumOperand1 = strtol(IR->Operand1, NULL, 16);

	if (IR->Operand2[0] == 'R') {						// Operand2가 레지스터일 경우, 문자열 레지스터 번호를 정수로 변환한다.
		int temp = atoi(&IR->Operand2[1]);
		*NumOperand2 = R[temp];							// 해당 레지스터의 값을 NumOperand2에 저장한다.
	}
	else												// 16진수일 경우, 16진수를 10진수로 변환해 NumOperand2에 저장한다.
		*NumOperand2 = strtol(IR->Operand2, NULL, 16);
	return;
}