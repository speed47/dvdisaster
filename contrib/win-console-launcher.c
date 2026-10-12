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
 * its exit code.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

static HANDLE child;

/* The GUI child isn't attached to our console and won't see Ctrl+C,
   so do what the default handler of a console program would do. */
static BOOL WINAPI ctrl_handler(DWORD type)
{
   (void)type;
   if(child) TerminateProcess(child, STATUS_CONTROL_C_EXIT);
   return TRUE;
}

int main(void)
{  static wchar_t exe[32768];
   STARTUPINFOW si = { 0 };
   PROCESS_INFORMATION pi;
   wchar_t *args, *cmd, *slash;
   DWORD n, code;

   /* dvdisaster.exe in the directory of this launcher */
   n = GetModuleFileNameW(NULL, exe, 32768);
   slash = wcsrchr(exe, L'\\');
   if(!n || n >= 32768 - 16 || !slash)
   {  fprintf(stderr, "dvdisaster.com: can't determine own location\n");
      return 1;
   }
   wcscpy(slash + 1, L"dvdisaster.exe");

   /* Skip argv[0] the way the CRT does: up to the closing quote,
      or up to the first blank. Keep the rest as is. */
   args = GetCommandLineW();
   if(*args == L'"')
   {  args++;
      while(*args && *args != L'"') args++;
      if(*args) args++;
   }
   else while(*args && *args != L' ' && *args != L'\t') args++;

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

   SetConsoleCtrlHandler(ctrl_handler, TRUE);
   if(!CreateProcessW(exe, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
   {  fprintf(stderr, "dvdisaster.com: can't start %ls (error %lu)\n", exe, GetLastError());
      return 1;
   }
   child = pi.hProcess;
   CloseHandle(pi.hThread);

   WaitForSingleObject(pi.hProcess, INFINITE);
   if(!GetExitCodeProcess(pi.hProcess, &code)) code = 1;
   return (int)code;
}
