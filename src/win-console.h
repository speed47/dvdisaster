/* Windows console support shared by the GUI executable and its launcher.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef DVDISASTER_WIN_CONSOLE_H
#define DVDISASTER_WIN_CONSOLE_H

#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <wchar.h>

/* Only the launcher sets this variable. The executable consumes it before
   starting any other processes, and stops the handles being inherited again. */
#define DVDISASTER_CONSOLE_HANDOFF L"DVDISASTER_CONSOLE_HANDOFF"

typedef struct
{  HANDLE gui_event;
   HANDLE mutex;
   BOOL attached;
} WinConsole;

static inline BOOL WinConsoleInit(WinConsole *console, int argc)
{  wchar_t value[80], extra;
   DWORD length, flags;

   length = GetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, value, 80);
   SetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, NULL);
   if(length)
   {  if(length >= 80 || swscanf(value, L"%p %p %lc",
                                &console->gui_event, &console->mutex, &extra) != 2)
         return FALSE;
      if(!GetHandleInformation(console->gui_event, &flags) || !(flags & HANDLE_FLAG_INHERIT)
         || !GetHandleInformation(console->mutex, &flags) || !(flags & HANDLE_FLAG_INHERIT)
         || !SetHandleInformation(console->gui_event, HANDLE_FLAG_INHERIT, 0)
         || !SetHandleInformation(console->mutex, HANDLE_FLAG_INHERIT, 0))
         return FALSE;
   }

   /* Attach early: version output and errors can occur during option parsing.
      Keep inherited files and pipes as they are. */
   if(argc > 1)
   {  int out = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_UNKNOWN;
      int err = GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_UNKNOWN;

      if((out || err) && AttachConsole(ATTACH_PARENT_PROCESS))
      {  console->attached = TRUE;
         if(out) freopen("CONOUT$", "w", stdout);
         if(err) freopen("CONOUT$", "w", stderr);
      }
   }
   return TRUE;
}

static inline BOOL WinConsoleReleaseOutputPipes(void)
{  FILE *streams[2] = { stdout, stderr };
   DWORD ids[2] = { STD_OUTPUT_HANDLE, STD_ERROR_HANDLE };
   HANDLE native[2], crt[2];
   BOOL native_pipe[2], crt_pipe[2];

   /* Snapshot both streams before reopening either: stdout and stderr may
      share a Windows handle (2>&1), or the CRT may own separate duplicates. */
   for(int i=0; i<2; i++)
   {  int fd = _fileno(streams[i]);
      native[i] = GetStdHandle(ids[i]);
      crt[i] = fd >= 0 ? (HANDLE)_get_osfhandle(fd) : INVALID_HANDLE_VALUE;
      native_pipe[i] = GetFileType(native[i]) == FILE_TYPE_PIPE;
      crt_pipe[i] = GetFileType(crt[i]) == FILE_TYPE_PIPE;
   }

   for(int i=0; i<2; i++)
      if(crt_pipe[i] || (native_pipe[i] && crt[i] == INVALID_HANDLE_VALUE))
         if(!freopen("NUL", "w", streams[i]))
         {  SetLastError(ERROR_OPEN_FAILED);
            return FALSE;
         }

   for(int i=0; i<2; i++)
      if(native_pipe[i])
      {  /* freopen closes CRT-owned handles. Release any additional Win32
            pipe handle exactly once, then update the standard-handle table. */
         if(native[i] != crt[0] && native[i] != crt[1]
            && (i == 0 || native[i] != native[0]))
            if(!CloseHandle(native[i])) return FALSE;
         if(!SetStdHandle(ids[i], (HANDLE)_get_osfhandle(_fileno(streams[i]))))
            return FALSE;
      }
   return TRUE;
}

static inline BOOL WinConsoleEnterGUI(WinConsole *console)
{  BOOL ok = TRUE;

   fflush(stdout);
   fflush(stderr);
   if(console->mutex)
   {  DWORD result = WaitForSingleObject(console->mutex, INFINITE);
      if(result != WAIT_OBJECT_0 && result != WAIT_ABANDONED)
         return FALSE;
   }

   /* The launcher takes the same mutex before cancelling. Once this event
      is signalled, it must never terminate the GUI, even before its main
      thread has noticed the event. Detach before making that promise. */
   /* Pipe readers must see EOF when the launcher exits, rather than waiting
      for the GUI. Keep disk-file logging and direct .exe launches intact. */
   if(console->gui_event)
      ok = WinConsoleReleaseOutputPipes();
   if(ok && console->attached)
   {  ok = FreeConsole();
      if(ok) console->attached = FALSE;
   }
   if(ok && console->gui_event)
      ok = SetEvent(console->gui_event);

   if(console->mutex)
   {  ReleaseMutex(console->mutex);
      CloseHandle(console->mutex);
      CloseHandle(console->gui_event);
      console->mutex = console->gui_event = NULL;
   }
   return ok;
}

#endif
