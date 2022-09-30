
#include "PipelinedHeader.h"

INSTMEMORY[0x00400000]	= { 0, };// Insturction memory, 16MB
DATAMEMORY[0x00400000]	= { 0, };// Data memory, 16MB
Register[32]			= { 0, };
BTB[0x8000][2]			= { 0, };// Branch Target Buffer, 256KB
PHT[0x8000]				= { 0, };// Pattern History Table, 128KB; default as strongly not taken

int main(void) {
	Initialize();
	ReadBinaryFiles();

	while (true) {
		Fetch();
		Decode();
		Execute();
		MemoryAccess();
		eof = WriteBack();

		if(eof == -1) break;
		LatchUpdate();
	}
	BasicImplementation();
	AdditionalImplementation();
	return 0;
}