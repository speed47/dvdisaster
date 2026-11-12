/* Native Windows regression tests for the launcher cancellation / GUI handoff.
 * Includes the launcher to call its control handler at deterministic points.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <assert.h>
#define main launcher_main
#include "../contrib/win-console-launcher.c"
#undef main

static HANDLE handler_started;

static DWORD WINAPI cancel_thread(void *unused)
{
   (void)unused;
   SetEvent(handler_started);
   ctrl_handler(CTRL_C_EVENT);
   return 0;
}

static HANDLE start_cancel(void)
{  HANDLE thread;
   ResetEvent(handler_started);
   thread = CreateThread(NULL, 0, cancel_thread, NULL, 0, NULL);
   assert(thread);
   assert(WaitForSingleObject(handler_started, 5000) == WAIT_OBJECT_0);
   return thread;
}

static void finish_cancel(HANDLE thread)
{
   assert(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
   CloseHandle(thread);
}

static HANDLE start_child(void)
{  wchar_t exe[32768], command[] = L"test-child";
   STARTUPINFOW si = { 0 };
   PROCESS_INFORMATION pi;

   assert(GetModuleFileNameW(NULL, exe, 32768));
   si.cb = sizeof(si);
   /* No application code needs to run: test that cancellation terminates
      this process, while a completed GUI handoff leaves it alive. */
   assert(CreateProcessW(exe, command, NULL, NULL, FALSE,
                         CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi));
   CloseHandle(pi.hThread);
   return pi.hProcess;
}

static void reset_handoff(void)
{
   console.gui_event = CreateEventW(NULL, TRUE, FALSE, NULL);
   console.mutex = CreateMutexW(NULL, FALSE, NULL);
   assert(console.gui_event && console.mutex);
}

static void close_handoff(void)
{
   if(child)
   {  TerminateProcess(child, 0);
      assert(WaitForSingleObject(child, 5000) == WAIT_OBJECT_0);
      CloseHandle(child);
      child = NULL;
   }
   CloseHandle(console.gui_event);
   CloseHandle(console.mutex);
}

static WinConsole receive_handoff(void)
{  WinConsole gui = { 0 };
   HANDLE event_copy, mutex_copy, self = GetCurrentProcess();
   wchar_t value[80];
   DWORD flags;

   /* Duplicate handles emulate the child's independent inherited handles. */
   assert(DuplicateHandle(self, console.gui_event, self, &event_copy,
                          0, TRUE, DUPLICATE_SAME_ACCESS));
   assert(DuplicateHandle(self, console.mutex, self, &mutex_copy,
                          0, TRUE, DUPLICATE_SAME_ACCESS));
   swprintf(value, 80, L"%p %p", event_copy, mutex_copy);
   assert(SetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, value));
   assert(WinConsoleInit(&gui, 1));
   assert(!GetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, value, 80));
   assert(GetHandleInformation(gui.gui_event, &flags) && !(flags & HANDLE_FLAG_INHERIT));
   assert(GetHandleInformation(gui.mutex, &flags) && !(flags & HANDLE_FLAG_INHERIT));
   return gui;
}

int main(void)
{  HANDLE thread;
   DWORD code;
   WinConsole gui;

   InitializeCriticalSection(&child_lock);
   handler_started = CreateEventW(NULL, TRUE, FALSE, NULL);
   assert(handler_started);

   /* A control event arriving before publication of the child is retained. */
   reset_handoff();
   EnterCriticalSection(&child_lock);
   thread = start_cancel();
   child = start_child();
   LeaveCriticalSection(&child_lock);
   finish_cancel(thread);
   assert(WaitForSingleObject(child, 5000) == WAIT_OBJECT_0);
   assert(GetExitCodeProcess(child, &code) && code == STATUS_CONTROL_C_EXIT);
   close_handoff();

   /* Cancellation racing with a handoff must recheck the event while holding
      the shared mutex, even if the handler has already started running. */
   reset_handoff();
   child = start_child();
   gui = receive_handoff();
   assert(WaitForSingleObject(console.mutex, 5000) == WAIT_OBJECT_0);
   thread = start_cancel();
   assert(WinConsoleEnterGUI(&gui));
   assert(WaitForSingleObject(console.gui_event, 0) == WAIT_OBJECT_0);
   assert(ReleaseMutex(console.mutex));
   finish_cancel(thread);
   assert(WaitForSingleObject(child, 0) == WAIT_TIMEOUT);

   /* Neither closing the console nor Ctrl+Break may kill a handed-off GUI. */
   assert(ctrl_handler(CTRL_CLOSE_EVENT));
   assert(ctrl_handler(CTRL_BREAK_EVENT));
   assert(WaitForSingleObject(child, 0) == WAIT_TIMEOUT);
   close_handoff();

   /* Direct GUI launches have no launcher protocol to complete. */
   gui = (WinConsole){ 0 };
   assert(WinConsoleInit(&gui, 1));
   assert(WinConsoleEnterGUI(&gui));

   CloseHandle(handler_started);
   DeleteCriticalSection(&child_lock);
   puts("Windows console cancellation and GUI handoff tests passed.");
   return 0;
}
