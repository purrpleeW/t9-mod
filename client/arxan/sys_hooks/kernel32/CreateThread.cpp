#include "common.hpp"
#include "arxan/sys_hooks.hpp"

template <>
HANDLE Client::Arxan::SysHooks::HK_CreateThread::hkCallback(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	if (g_Debugging) {
		return m_Original(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
	}

	// Loop each thread and attach breakpoints
	HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	THREADENTRY32 entry = {};
	entry.dwSize = sizeof(THREADENTRY32);
	Thread32First(snapshotHandle, &entry);

	do {
		if (entry.th32OwnerProcessID == GetCurrentProcessId()) {
			HANDLE currentThread = OpenThread(THREAD_ALL_ACCESS, false, entry.th32ThreadID);
			bool setThreadResult = 1;

			if (!g_SuspendNewThreads) {
				setThreadResult = SetThreadContext(currentThread, &g_CurrentContext);
			}
		}
	} while (Thread32Next(snapshotHandle, &entry));

	HANDLE thread = m_Original(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

	if (g_SuspendNewThreads) {
		SuspendThread(thread);
	}

	return thread;
}
