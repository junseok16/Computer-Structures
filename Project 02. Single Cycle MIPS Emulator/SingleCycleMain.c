
#include "SingleCycleHeader.h"

Memory[0x00400000]	= { 0, };// 16 Megabytes Memory
Register[32]		= { 0, };
PC					= 0x00000000;// Program Counter
binIndex			= 0x00000000;

void main(void) {
	start = clock();
	Register[31] = 0xffffffff;
	Register[29] = 0x01000000;// Stack Pointer

	DoMemory();
	while (PC != 0xffffffff) {
		DoFetch();			// Instruction Fetch
		DoDecode();			// Instruction Decode
		DoExecute();		// Execute
		DoMemoryAccess();	// MemoryAccess
		DoWriteBack();		// WriteBack

		countInst++;
		BasicState();		// �⺻ ������ ����մϴ�.
	}
	end = clock();
	TimeExcuted = end - start;
	TimeExcuted /= 1000;
	AdditionalState();		// �߰� ������ ����մϴ�.
	return;
}
