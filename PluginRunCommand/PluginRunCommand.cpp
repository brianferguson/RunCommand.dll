/*
  Copyright (C) 2013 Brian Ferguson

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "PluginRunCommand.h"

#define MAX_LINE_LENGTH	4096

const WCHAR* err_UnknownCmd = L"RunCommand.dll: Error (100) Unknown command";
const WCHAR* err_CmdRunning = L"RunCommand.dll: Error (101) Command still running";
const WCHAR* err_CreatePipe = L"RunCommand.dll: Error (102) Cannot create pipe";	// Rare!
const WCHAR* err_Process    = L"RunCommand.dll: Error (103) Cannot start process";
const WCHAR* err_Terminate  = L"RunCommand.dll: Error (104) Cannot terminate process";	// Rare!
const WCHAR* err_SaveFile   = L"RunCommand.dll: Error (105) Cannot save file";

void RunCommand(Measure* measure);
BOOL WINAPI TerminateApp(HANDLE& hProc, DWORD& dwPID, DWORD dwTimeout);
BOOL CALLBACK TerminateAppEnum(HWND hwnd, LPARAM lParam);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH notification calls.
			DisableThreadLibraryCalls(hinstDLL);
			break;

		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	Measure* measure = new Measure;
	*data = measure;

	measure->skin = RmGetSkin(rm);
	measure->hwnd = RmGetSkinWindow(rm);
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;

	std::lock_guard<std::recursive_mutex> lock(measure->mutex);

	measure->parameter = RmReadString(rm, L"Parameter", L"");
	measure->finishAction = RmReadString(rm, L"FinishAction", L"", false);
	measure->outputFile = RmReadPath(rm, L"OutputFile", L"");
	measure->folder = RmReadPath(rm, L"StartInFolder", L" ");	// Space is intentional!
	measure->timeout = RmReadInt(rm, L"Timeout", -1);

	const WCHAR* state = RmReadString(rm, L"State", L"HIDE");
	if (_wcsicmp(state, L"SHOW") == 0)
	{
		measure->state = SW_SHOW;
	}
	else if (_wcsicmp(state, L"MAXIMIZE") == 0)
	{
		measure->state = SW_MAXIMIZE;
	}
	else if (_wcsicmp(state, L"MINIMIZE") == 0)
	{
		measure->state = SW_MINIMIZE;
	}
	else
	{
		measure->state = SW_HIDE;
	}

	// Grab "%COMSPEC% environment variable
	measure->program = RmReadString(rm, L"Program", RmReplaceVariables(rm, L"\"%COMSPEC%\" /U /C"));
	if (measure->program.empty())
	{
		// Assume "cmd.exe" exists!
		measure->program = L"cmd.exe /U /C";
	}

	const WCHAR* type = RmReadString(rm, L"OutputType", L"UTF16");
	if (_wcsicmp(type, L"ANSI") == 0)
	{
		measure->outputType = OUTPUTTYPE_ANSI;
	}
	else if (_wcsicmp(type, L"UTF8") == 0)
	{
		measure->outputType = OUTPUTTYPE_UTF8;
	}
	else
	{
		measure->outputType = OUTPUTTYPE_UTF16;
	}
}

PLUGIN_EXPORT double Update(void* data)
{
	Measure* measure = (Measure*)data;

	double value = 0.0f;

	{
		std::lock_guard<std::recursive_mutex> lock(measure->mutex);
		if (!measure->result.empty())
		{
			value = _wtoi(measure->result.c_str());
		}
	}

	return value;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure* measure = (Measure*)data;
	static std::wstring result = L"";

	{
		std::unique_lock<std::recursive_mutex> lock(measure->mutex, std::defer_lock);

		if (lock.try_lock())
		{
			result = measure->result.c_str();
			lock.unlock();
		}
	}

	return result.c_str();
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	Measure* measure = (Measure*)data;

	if (_wcsicmp(args, L"RUN") == 0)
	{
		std::lock_guard<std::recursive_mutex> lock(measure->mutex);
		if (!measure->threadActive && !measure->program.empty())
		{
			std::thread thread(RunCommand, measure);
			thread.detach();

			measure->threadActive = true;
		}
		else
		{
			RmLog(LOG_ERROR, err_CmdRunning);	// Command still running
		}
	}
	else
	{
		RmLog(LOG_NOTICE, err_UnknownCmd);	// Unknown command
	}
}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;

	{
		std::lock_guard<std::recursive_mutex> lock(measure->mutex);
		if (measure->threadActive)
		{
			// Increment ref count of this module so that it will not be
			// unloaded prior to thread completion.
			DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;
			HMODULE module;
			GetModuleHandleEx(flags, (LPCWSTR)DllMain, &module);

			// Tell the thread to perform any cleanup
			measure->threadActive = false;
			return;
		}
	}

	delete measure;
}

void RunCommand(Measure* measure)
{
	WORD showWindow = 0;
	std::wstring command;
	std::wstring folder;
	int timeout = -1;
	std::wstring result;
	OutputType type;
	HWND hwnd;

	// Grab values from the measure
	{
		std::lock_guard<std::recursive_mutex> lock(measure->mutex);

		showWindow = measure->state;
		folder = measure->folder;
		timeout = measure->timeout;

		command = measure->program;
		command += L" ";
		command += measure->parameter;

		type = measure->outputType;
		hwnd = measure->hwnd;
	}

	HANDLE read = nullptr;
	HANDLE write = nullptr;

/*	Instead of trying to keep track of the following handles,
	use an array to make cleanup easier.

	HANDLE hOutputReadTmp;	0
	HANDLE hOutputWrite;	1
	HANDLE hInputWriteTmp;	2
	HANDLE hInputRead;		3
	HANDLE hErrorWrite;		4
*/
	HANDLE loadHandles[5];
	for (int i = 0; i < sizeof(loadHandles)/sizeof(loadHandles[0]); ++i)
	{
		loadHandles[i] = INVALID_HANDLE_VALUE;
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	HANDLE hProc = GetCurrentProcess();	// Do not close this handle!

	// Create pipe for stdin, stdout, stderr
	if (CreatePipe(&loadHandles[0], &loadHandles[1], &sa, 0) &&
		DuplicateHandle(hProc, loadHandles[1], hProc, &loadHandles[4], 0, TRUE, DUPLICATE_SAME_ACCESS) &&
		CreatePipe(&loadHandles[3], &loadHandles[2], &sa, 0) &&
		DuplicateHandle(hProc, loadHandles[0], hProc, &read, 0, FALSE, DUPLICATE_SAME_ACCESS) &&
		DuplicateHandle(hProc, loadHandles[2], hProc, &write, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		BYTE buffer[MAX_LINE_LENGTH];
		DWORD bytesRead = 0;
		DWORD totalBytes = 0;
		DWORD bytesLeft = 0;
		DWORD exit = 0;

		auto SetResult = [&](BYTE* buffer)
		{
			switch (type)
			{
			case OUTPUTTYPE_ANSI:
				result += StringUtil::Widen((LPCSTR)buffer);
				break;

			case OUTPUTTYPE_UTF8:
				result += StringUtil::WidenUTF8((LPCSTR)buffer);
				break;

			default:
			case OUTPUTTYPE_UTF16:
				result += (LPCWSTR)buffer;
				break;
			}
		};

		// Process and Startup info
		PROCESS_INFORMATION pi;
		SecureZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

		STARTUPINFO si;
		SecureZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = showWindow;
		si.hStdOutput  = loadHandles[1];
		si.hStdInput   = loadHandles[3];
		si.hStdError   = loadHandles[4];

		// Start process
		if (CreateProcess(nullptr, &command[0], NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, &folder[0], &si, &pi))
		{
			// Send command
			DWORD written;
			WriteFile(write, &command[0], MAX_LINE_LENGTH, &written, NULL);

			std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

			// Read output of program (if any)
			for (;;)
			{
				// Check if there is any data to to read
				PeekNamedPipe(read, buffer, MAX_LINE_LENGTH, &bytesRead, &totalBytes, &bytesLeft);
				if (bytesRead != 0)
				{
					if (totalBytes > MAX_LINE_LENGTH)
					{
						while (bytesRead >= MAX_LINE_LENGTH)
						{
							ReadFile(read, buffer, MAX_LINE_LENGTH, &bytesRead, NULL);
							buffer[bytesRead] = '\0';

							SetResult(buffer);
						}
					}
					else
					{
						ReadFile(read, buffer, MAX_LINE_LENGTH, &bytesRead, NULL);
						buffer[bytesRead] = '\0';

						SetResult(buffer);
					}
				}

				// Close hidden program if skin is closed or timeout is reached (if exists)
				if ((showWindow == SW_HIDE && !IsWindow(hwnd)) ||
					(timeout >= 0 && std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now() - start).count() > timeout))
				{
					if (!TerminateApp(pi.hProcess, pi.dwProcessId, (DWORD)timeout))
					{
						RmLog(LOG_ERROR, err_Terminate);	// Could not terminate process (very rare!)
					}
					break;
				}

				// Check to see if the program is still running
				GetExitCodeProcess(pi.hProcess, &exit);
				if (exit != STILL_ACTIVE) break;

				SecureZeroMemory(buffer, sizeof(buffer));	// clear the buffer
			}

			// Close process handles
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		else
		{
			RmLog(LOG_ERROR, err_Process);	// Cannot create process
		}
	}
	else
	{
		RmLog(LOG_ERROR, err_CreatePipe);	// Cannot create pipe
	}

	// Close handles
	for (int i = 0; i < sizeof(loadHandles)/sizeof(loadHandles[0]); ++i)
	{
		if (loadHandles[i] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(loadHandles[i]);
		}
	}

	CloseHandle(write);
	CloseHandle(read);

	// Remove any carriage returns
	size_t pos = 0;
	while ((pos = result.find(L"\r", pos)) != std::wstring::npos)
	{
		result.erase(pos++, 1);
	}

	HMODULE module = nullptr;

	{
		std::unique_lock<std::recursive_mutex> lock(measure->mutex);

		if (measure->threadActive)
		{
			measure->result = result;
			measure->result.shrink_to_fit();

			if (!measure->outputFile.empty())
			{
				std::wstring encoding = L"w+";
				switch (type)
				{
				case OUTPUTTYPE_UTF8: { encoding.append(L", ccs=UTF-8"); break; }
				case OUTPUTTYPE_UTF16: { encoding.append(L", ccs=UTF-16LE"); break; }
				}

				FILE* file;
				if (_wfopen_s(&file, measure->outputFile.c_str(), encoding.c_str()) == 0)
				{
					fputws(result.c_str(), file);
				}
				else
				{
					RmLog(LOG_ERROR, err_SaveFile);	// Cannot save file
				}

				if (file)
				{
					fclose(file);
				}
			}

			measure->threadActive = false;

			// Unlock mutex here to prevent deadlock in FinishAction
			lock.unlock();

			if (!measure->finishAction.empty())
			{
				RmExecute(measure->skin, measure->finishAction.c_str());
			}

			return;
		}
	}

	delete measure;

	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	GetModuleHandleEx(flags, (LPCWSTR)DllMain, &module);

	if (module)
	{
		// Decrement the ref count and possibly unload the module if this is the last instance.
		FreeLibraryAndExitThread(module, 0);
	}
}

// Terminate "cleanly" per KB178893
BOOL WINAPI TerminateApp(HANDLE& hProc, DWORD& dwPID, DWORD dwTimeout)
{
	BOOL ret = FALSE;

	EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM) dwPID);

	if (WaitForSingleObject(hProc, dwTimeout) != WAIT_OBJECT_0)
	{
		ret = TerminateProcess(hProc, 0);
	}
	else
	{
		ret = TRUE;
	}

	return ret;
}

BOOL CALLBACK TerminateAppEnum(HWND hwnd, LPARAM lParam)
{
	DWORD dwID;

	GetWindowThreadProcessId(hwnd, &dwID);

	if (dwID == (DWORD)lParam)
	{
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	}

	return TRUE;
}
