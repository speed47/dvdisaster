/* Windows console support shared by the GUI executable and its launcher.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef DVDISASTER_WIN_CONSOLE_H
#define DVDISASTER_WIN_CONSOLE_H

#include <windows.h>
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
   if(console->attached)
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
