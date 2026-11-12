/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2019-2021 The dvdisaster development team.
 *
 *  This file is part of dvdisaster.
 *
 *  dvdisaster is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  dvdisaster is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dvdisaster. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Console launcher for the Windows GUI build (built as dvdisaster.com).
 *
 * dvdisaster.exe is a GUI subsystem program, so cmd and PowerShell neither
 * wait for it nor get its exit code. When the user types "dvdisaster",
 * the shells pick dvdisaster.com first (PATHEXT), which runs dvdisaster.exe
 * from the same directory with our std handles, waits for it and returns
 * its exit code. If the executable selects GUI mode, it signals the launcher
 * to exit and continues independently of the console.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "../src/win-console.h"

static HANDLE child;
static CRITICAL_SECTION child_lock;
static WinConsole console;

/* The CRT permits quotes around individual parts of argv[0]. Unlike later
   arguments, backslashes are literal here. Preserve the argument tail as is. */
static const wchar_t *skip_program_name(const wchar_t *command)
{  BOOL quoted = FALSE;

   while(*command)
   {  if(*command == L'"')
         quoted = !quoted;
      else if(!quoted && (*command == L' ' || *command == L'\t'))
         break;
      command++;
   }
   return command;
}

/* A CLI child with redirected output may not be attached to our console.
   Forward cancellation until it has explicitly handed off to GUI mode. */
static BOOL WINAPI ctrl_handler(DWORD type)
{
   (void)type;
   EnterCriticalSection(&child_lock);
   if(child)
   {  DWORD result = WaitForSingleObject(console.mutex, INFINITE);
      if(result == WAIT_OBJECT_0 || result == WAIT_ABANDONED)
      {  if(WaitForSingleObject(console.gui_event, 0) != WAIT_OBJECT_0)
            TerminateProcess(child, STATUS_CONTROL_C_EXIT);
         ReleaseMutex(console.mutex);
      }
   }
   LeaveCriticalSection(&child_lock);
   return TRUE;
}

int main(void)
{  static wchar_t exe[32768];
   STARTUPINFOW si = { 0 };
   PROCESS_INFORMATION pi;
   SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
   HANDLE waits[2];
   wchar_t handoff[80];
   const wchar_t *args;
   wchar_t *cmd, *slash;
   DWORD n, code;

   /* dvdisaster.exe in the directory of this launcher */
   n = GetModuleFileNameW(NULL, exe, 32768);
   slash = wcsrchr(exe, L'\\');
   if(!n || n >= 32768 - 16 || !slash)
   {  fprintf(stderr, "dvdisaster.com: can't determine own location\n");
      return 1;
   }
   wcscpy(slash + 1, L"dvdisaster.exe");

   args = skip_program_name(GetCommandLineW());

   cmd = malloc((wcslen(exe) + wcslen(args) + 3) * sizeof(wchar_t));
   if(!cmd) return 1;
   wcscpy(cmd, L"\"");
   wcscat(cmd, exe);
   wcscat(cmd, L"\"");
   wcscat(cmd, args);

   si.cb = sizeof(si);
   si.dwFlags = STARTF_USESTDHANDLES;
   si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
   si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
   si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

   console.gui_event = CreateEventW(&sa, TRUE, FALSE, NULL);
   console.mutex = CreateMutexW(&sa, FALSE, NULL);
   if(!console.gui_event || !console.mutex)
   {  fprintf(stderr, "dvdisaster.com: can't create GUI handoff (error %lu)\n", GetLastError());
      return 1;
   }
   swprintf(handoff, 80, L"%p %p", console.gui_event, console.mutex);
   if(!SetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, handoff))
   {  fprintf(stderr, "dvdisaster.com: can't pass GUI handoff (error %lu)\n", GetLastError());
      return 1;
   }

   /* A control event during process creation must wait until the child
      handle is published, rather than being silently discarded. */
   InitializeCriticalSection(&child_lock);
   EnterCriticalSection(&child_lock);
   if(!SetConsoleCtrlHandler(ctrl_handler, TRUE))
   {  fprintf(stderr, "dvdisaster.com: can't install control handler (error %lu)\n", GetLastError());
      LeaveCriticalSection(&child_lock);
      return 1;
   }
   if(!CreateProcessW(exe, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
   {  fprintf(stderr, "dvdisaster.com: can't start %ls (error %lu)\n", exe, GetLastError());
      LeaveCriticalSection(&child_lock);
      return 1;
   }
   child = pi.hProcess;
   LeaveCriticalSection(&child_lock);
   SetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, NULL);
   free(cmd);
   CloseHandle(pi.hThread);

   waits[0] = pi.hProcess;
   waits[1] = console.gui_event;
   n = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
   if(n == WAIT_OBJECT_0)
   {  if(!GetExitCodeProcess(pi.hProcess, &code)) code = 1;
   }
   else if(n == WAIT_OBJECT_0 + 1) code = 0;
   else
   {  fprintf(stderr, "dvdisaster.com: can't wait for child (error %lu)\n", GetLastError());
      code = 1;
   }

   EnterCriticalSection(&child_lock);
   child = NULL;
   CloseHandle(pi.hProcess);
   LeaveCriticalSection(&child_lock);
   if(n == WAIT_OBJECT_0 + 1)
      puts("dvdisaster: GUI detached from console. You can close this console window.");
   /* Keep the handler's synchronization objects alive until process exit. */
   return (int)code;
}
