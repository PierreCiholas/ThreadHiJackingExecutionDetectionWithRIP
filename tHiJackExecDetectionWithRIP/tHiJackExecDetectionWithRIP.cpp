#include <Windows.h>
#include <iostream>

using namespace std;

int main() {
	// Get executable memory, required when Data Execution Prevention (DEP) is active
	void* executableMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (executableMemory == NULL)
		return EXIT_FAILURE;
	cout << "executableMemory 0x" << hex << executableMemory << endl;
	cout << endl;

	UCHAR x64NopNLoop[] = {
		0x90, 0x90, 0x90, 0x90, 0x90, // nops
		0x90, 0x90, 0x90, 0x90, 0x90, // nops
		0x90, 0x90, 0x90, 0x90, 0x90, // nops
		0x90, 0x90, 0x90, 0x90, 0x90, // nops
		0x90, 0x90, 0x90, 0x90, 0x90, // nops
		0x90, 0x90, 0x90, 0x90, 0x90, // nops
		0xEB, 0xFE	// jmp rel8 -2
	};

	printf("ShellCode:\n");
	for (int i(0); i < sizeof(x64NopNLoop); ++i) {
		if (i > 0 && i % 10 == 0)
			printf("\n");
		printf("%02X ", x64NopNLoop[i]);
	}
	printf("\n");

	// Placing shellcode in executable memory
	memcpy(executableMemory, x64NopNLoop, sizeof(x64NopNLoop));
	cout << endl;

	system("pause");

	// Thread execution
	DWORD tid;
	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)executableMemory, NULL, NULL, &tid);
	if (hThread == NULL)
		return EXIT_FAILURE;
	cout << "Executing in new thread, ID " << dec << tid << ", HANDLE 0x" << hex << hThread << endl;

	// Detecting end of execution with RIP
	CONTEXT tContext;
	SecureZeroMemory(&tContext, sizeof(tContext));
	tContext.ContextFlags = CONTEXT_CONTROL;
	DWORD64 endOfShellcode = (DWORD64)executableMemory + sizeof(x64NopNLoop) - 2;
	while (tContext.Rip < endOfShellcode) {
		GetThreadContext(hThread, &tContext);
		cout << "Rip 0x" << hex << tContext.Rip << " (End 0x" << hex << endOfShellcode << ")" << endl;
	}
	cout << "Execution finished!" << endl;

	system("pause");
	return EXIT_SUCCESS;
}