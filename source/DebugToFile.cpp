#include <windows.h>
#include <iostream>

struct DebugBuffer
{
	DWORD   processId;
	char    debugText[4096-sizeof(DWORD)];
};

class IDebugEventHandler
{
public:
	virtual void OnDebugEvent (const DebugBuffer* debugBuffer) = 0;
};

class DebugMonitor
{
public:
	DebugMonitor (IDebugEventHandler* eventHandler) :
		eventHandler (eventHandler),
		mutexHandle (NULL),
		bufferReadyEventHandle (NULL),
		dataReadyEventHandle (NULL),
		fileMappingHandle (NULL),
		mapViewOfFileHandle (NULL),
		threadHandle (NULL),
		running (false)
	{
	}

	~DebugMonitor ()
	{
	}

	bool Start ()
	{
		mutexHandle = OpenMutex (SYNCHRONIZE, FALSE, L"DBWinMutex");
		if (mutexHandle == NULL) {
			int error = GetLastError ();
			return false;
		}

		bufferReadyEventHandle = OpenEvent (SYNCHRONIZE, FALSE, L"DBWIN_BUFFER_READY");
		if (bufferReadyEventHandle == NULL) {
			bufferReadyEventHandle = CreateEvent (NULL, FALSE, TRUE, L"DBWIN_BUFFER_READY");
			if (bufferReadyEventHandle == NULL) {
				return false;
			}
		}

		dataReadyEventHandle = OpenEvent (SYNCHRONIZE, FALSE, L"DBWIN_DATA_READY");
		if (dataReadyEventHandle == NULL) {
			dataReadyEventHandle = CreateEvent (NULL, FALSE, FALSE, L"DBWIN_DATA_READY");
			if (dataReadyEventHandle == NULL) {
				return false;
			}
		}

		fileMappingHandle = OpenFileMapping (FILE_MAP_READ, FALSE, L"DBWIN_BUFFER");
		if (fileMappingHandle == NULL) {
			fileMappingHandle = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof (DebugBuffer), L"DBWIN_BUFFER");
			if (fileMappingHandle == NULL) {
				return false;
			}
		}

		mapViewOfFileHandle = MapViewOfFile (fileMappingHandle, SECTION_MAP_READ, 0, 0, 0);
		if (mapViewOfFileHandle == NULL) {
			return false;
		}

		threadHandle = CreateThread (NULL, 0, DebugMonitorThread, this, 0, NULL);
		if (threadHandle == NULL) {
			return false;
		}

		running = true;
		return true;
	}

	void Stop ()
	{
		running = false;

		if (threadHandle != NULL) {
			WaitForSingleObject (threadHandle, INFINITE);
		}

		if (mutexHandle != NULL) {
			CloseHandle (mutexHandle);
			mutexHandle = NULL;
		}

		if (fileMappingHandle != NULL) {
			UnmapViewOfFile (fileMappingHandle);
			CloseHandle (fileMappingHandle);
			fileMappingHandle = NULL;
		}

		if (bufferReadyEventHandle != NULL) {
			CloseHandle (bufferReadyEventHandle);
			bufferReadyEventHandle = NULL;
		}

		if (dataReadyEventHandle != NULL) {
			CloseHandle (dataReadyEventHandle);
			dataReadyEventHandle = NULL;
		}
	}

private:
	static DWORD WINAPI DebugMonitorThread (void *userData)
	{
		DebugMonitor* debugMonitor = (DebugMonitor*) userData;
		if (debugMonitor != NULL) {
			debugMonitor->ProcessEvents ();
		}
		return 0;
	}

	DWORD ProcessEvents ()
	{
		while (running) {
			DWORD result = WaitForSingleObject (dataReadyEventHandle, 100);
			if (result == WAIT_OBJECT_0) {
				DebugBuffer* debugBuffer = (DebugBuffer*) mapViewOfFileHandle;
				if (eventHandler != NULL) {
					eventHandler->OnDebugEvent (debugBuffer);
				}
				SetEvent (bufferReadyEventHandle);
			}
		}
		return 0;
	}

	IDebugEventHandler* eventHandler;

	HANDLE mutexHandle;
	HANDLE bufferReadyEventHandle;
	HANDLE dataReadyEventHandle;
	HANDLE fileMappingHandle;
	HANDLE mapViewOfFileHandle;
	HANDLE threadHandle;

	bool running;
};

class DebugFileWriter : public IDebugEventHandler
{
public:
	DebugFileWriter (const std::wstring& logFileName, DWORD processIdentifier)
	{
		fileHandle = CreateFile (logFileName.c_str (), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		processId = processIdentifier;
	}

	~DebugFileWriter ()
	{
		CloseHandle (fileHandle);
	}

	virtual void OnDebugEvent (const DebugBuffer* debugBuffer) override
	{
		if (processId == debugBuffer->processId) {
			DWORD bytesWritten = 0;
			WriteFile (fileHandle, debugBuffer->debugText, (DWORD) strlen (debugBuffer->debugText), &bytesWritten, NULL);
		}
	}

private:
	HANDLE	fileHandle;
	DWORD	processId;
};

int wmain (int argc, wchar_t* argv[])
{
	if (argc < 3) {
		std::cout << "Usage: DebugMonitor.exe [DebugLogFileName] [ApplicationName] <ApplicationArguments>" << std::endl;
		return 1;
	}

	std::wstring debugLogfileName = argv[1];
	std::wstring commandArguments = argv[2];
	for (int i = 3; i < argc; i++) {
		commandArguments += L" \"";
		commandArguments += argv[i];
		commandArguments += L"\"";
	}

	wchar_t commandArgumentsString[4096];
	wcscpy_s (commandArgumentsString, 4096, commandArguments.c_str ());

	STARTUPINFO startupInfo;
	ZeroMemory (&startupInfo, sizeof (STARTUPINFO));

	PROCESS_INFORMATION processInfo;
	ZeroMemory (&processInfo, sizeof (PROCESS_INFORMATION));

	BOOL processCreated = CreateProcess (NULL, commandArgumentsString, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo);
	if (!processCreated) {
		std::cout << "Error: Failed to start application" << std::endl;
		return 1;
	}

	if (processCreated) {
		DebugFileWriter debugFileWriter (debugLogfileName, processInfo.dwProcessId);
		DebugMonitor debugMonitor (&debugFileWriter);

		debugMonitor.Start ();
		WaitForSingleObject (processInfo.hProcess, INFINITE);
		debugMonitor.Stop ();

		CloseHandle (processInfo.hProcess);
		CloseHandle (processInfo.hThread);
	}

	DWORD exitCode = 0;
	GetExitCodeProcess (processInfo.hProcess, &exitCode);
	return exitCode;
}
