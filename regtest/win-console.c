/* Native Windows regression tests for launcher arguments and GUI handoff.
 * Includes the launcher to call its control handler at deterministic points.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <assert.h>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>
#define main launcher_main
#include "../contrib/win-console-launcher.c"
#undef main

static HANDLE handler_started;

static void check_arguments(void)
{  static const struct
   {  const wchar_t *command;
      const wchar_t *arguments;
   } cases[] = {
      { L"dvdisaster --version", L" --version" },
      { L"\"C:\\Program Files\\dvdisaster\\dvdisaster.com\" --version", L" --version" },
      { L"C:\\\"Program Files\"\\dvdisaster\\dvdisaster.com --version", L" --version" },
      { L"\"C:\\Program Files\"\\dvdisaster\\dvdisaster.com --version", L" --version" },
      { L"C:\\\"Program Files\"\\\"DVD Tools\"\\dvdisaster.com\t-r -i \"disc image.iso\"",
        L"\t-r -i \"disc image.iso\"" },
      { L"dvdisaster  \t-i \"disc image.iso\" \"\" \"a\\\"b\" \"C:\\dir\\\\\"",
        L"  \t-i \"disc image.iso\" \"\" \"a\\\"b\" \"C:\\dir\\\\\"" },
      { L"dvdisaster", L"" },
      { L"\"C:\\Program Files\\dvdisaster.com\"", L"" },
      { L"C:\\\"Program Files\"\\dvdisaster.com", L"" },
      { L"dvdisaster \t", L" \t" }
   };

   for(unsigned i=0; i<sizeof(cases)/sizeof(cases[0]); i++)
      assert(!wcscmp(skip_program_name(cases[i].command), cases[i].arguments));
}

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

static int stream_child(const char *mode, const char *ready_value)
{  WinConsole gui = { 0 };
   HANDLE ready;
   DWORD written;

   assert(sscanf(ready_value, "%p", &ready) == 1);
   assert(WinConsoleInit(&gui, 3));
   if(!strcmp(mode, "--gui-extra-streams"))
   {  DWORD ids[2] = { STD_OUTPUT_HANDLE, STD_ERROR_HANDLE };
      for(int i=0; i<2; i++)
      {  HANDLE duplicate;
         assert(DuplicateHandle(GetCurrentProcess(), GetStdHandle(ids[i]),
                                GetCurrentProcess(), &duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS));
         assert(SetStdHandle(ids[i], duplicate));
      }
   }
   fputs("stdout before\n", stdout);
   fputs("stderr before\n", stderr);
   if(strcmp(mode, "--cli-streams")) assert(WinConsoleEnterGUI(&gui));
   fputs("stdout after\n", stdout);
   fputs("stderr after\n", stderr);
   fflush(stdout);
   fflush(stderr);
   assert(WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "stdout native\n", 14, &written, NULL));
   assert(WriteFile(GetStdHandle(STD_ERROR_HANDLE), "stderr native\n", 14, &written, NULL));
   assert(SetEvent(ready));
   Sleep(INFINITE);
   return 0;
}

static HANDLE make_output(BOOL disk, HANDLE *reader)
{  SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
   HANDLE writer;

   if(disk)
   {  wchar_t path[MAX_PATH], name[MAX_PATH];
      assert(GetTempPathW(MAX_PATH, path));
      assert(GetTempFileNameW(path, L"dvd", 0, name));
      writer = CreateFileW(name, GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           &sa, OPEN_EXISTING, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
      assert(writer != INVALID_HANDLE_VALUE);
      *reader = writer;
   }
   else
   {  assert(CreatePipe(reader, &writer, &sa, 0));
      assert(SetHandleInformation(*reader, HANDLE_FLAG_INHERIT, 0));
   }
   return writer;
}

static void check_output(HANDLE reader, BOOL released)
{  char output[1024];
   DWORD available, length;
   BOOL pipe = GetFileType(reader) == FILE_TYPE_PIPE;

   if(pipe)
   {  assert(PeekNamedPipe(reader, NULL, 0, NULL, &available, NULL));
      assert(available > 0 && available < sizeof(output));
   }
   else
   {  assert(SetFilePointer(reader, 0, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER);
      available = sizeof(output)-1;
   }
   assert(ReadFile(reader, output, available, &length, NULL));
   output[length] = 0;
   assert(strstr(output, "before"));
   if(pipe && released)
   {  assert(!strstr(output, "after") && !strstr(output, "native"));
      assert(!PeekNamedPipe(reader, NULL, 0, NULL, &available, NULL));
      assert(GetLastError() == ERROR_BROKEN_PIPE);
   }
   else
   {  assert(strstr(output, "after") && strstr(output, "native"));
      if(pipe) assert(PeekNamedPipe(reader, NULL, 0, NULL, &available, NULL));
   }
}

static HANDLE find_child(DWORD parent)
{  PROCESSENTRY32W entry = { 0 };
   HANDLE process = NULL;
   HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   assert(snapshot != INVALID_HANDLE_VALUE);
   entry.dwSize = sizeof(entry);
   if(Process32FirstW(snapshot, &entry))
      do
      {  if(entry.th32ParentProcessID == parent && !wcscmp(entry.szExeFile, L"dvdisaster.exe"))
         {  process = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
            break;
         }
      } while(Process32NextW(snapshot, &entry));
   CloseHandle(snapshot);
   assert(process);
   return process;
}

static void check_streams(int layout, BOOL managed, const wchar_t *mode, const wchar_t *launcher)
{  SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
   STARTUPINFOW si = { 0 };
   PROCESS_INFORMATION pi;
   wchar_t exe[32768], command[128], value[80];
   HANDLE out_reader, err_reader;
   HANDLE ready = CreateEventW(&sa, TRUE, FALSE, NULL);
   BOOL released = managed && wcscmp(mode, L"--cli-streams");

   assert(ready);
   reset_handoff();
   if(managed && !launcher)
   {  assert(SetHandleInformation(console.gui_event, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
      assert(SetHandleInformation(console.mutex, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
      swprintf(value, 80, L"%p %p", console.gui_event, console.mutex);
      assert(SetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, value));
   }
   si.cb = sizeof(si);
   si.dwFlags = STARTF_USESTDHANDLES;
   si.hStdOutput = make_output(layout == 2 || layout == 4, &out_reader);
   if(layout == 1)
   {  si.hStdError = si.hStdOutput;
      err_reader = out_reader;
   }
   else si.hStdError = make_output(layout == 3 || layout == 4, &err_reader);
   assert(GetModuleFileNameW(NULL, exe, 32768));
   swprintf(command, 128, L"test-child %ls %p", mode, ready);
   assert(CreateProcessW(launcher ? launcher : exe, command, NULL, NULL, TRUE, CREATE_NO_WINDOW,
                         NULL, NULL, &si, &pi));
   child = pi.hProcess;
   CloseHandle(pi.hThread);
   SetEnvironmentVariableW(DVDISASTER_CONSOLE_HANDOFF, NULL);
   if(si.hStdOutput != out_reader) CloseHandle(si.hStdOutput);
   if(si.hStdError != si.hStdOutput && si.hStdError != err_reader) CloseHandle(si.hStdError);

   assert(WaitForSingleObject(ready, 5000) == WAIT_OBJECT_0);
   if(launcher)
   {  DWORD exit_code;
      assert(WaitForSingleObject(pi.hProcess, 5000) == WAIT_OBJECT_0);
      assert(GetExitCodeProcess(pi.hProcess, &exit_code) && exit_code == 0);
      child = find_child(pi.dwProcessId);
      CloseHandle(pi.hProcess);
   }
   assert(WaitForSingleObject(child, 0) == WAIT_TIMEOUT);
   if(released && !launcher) assert(WaitForSingleObject(console.gui_event, 0) == WAIT_OBJECT_0);
   check_output(out_reader, released);
   if(err_reader != out_reader) check_output(err_reader, released);
   close_handoff();
   CloseHandle(ready);
   CloseHandle(out_reader);
   if(err_reader != out_reader) CloseHandle(err_reader);
}

static void check_launcher_streams(void)
{  wchar_t temp[MAX_PATH], directory[MAX_PATH], launcher[MAX_PATH], exe[MAX_PATH], self[32768];

   /* Run the real launcher with this test executable as its GUI child. This
      also catches extra pipe writers inherited through the launcher itself. */
   assert(GetTempPathW(MAX_PATH, temp));
   assert(GetTempFileNameW(temp, L"dvd", 0, directory));
   assert(DeleteFileW(directory));
   assert(CreateDirectoryW(directory, NULL));
   swprintf(launcher, MAX_PATH, L"%ls\\dvdisaster.com", directory);
   swprintf(exe, MAX_PATH, L"%ls\\dvdisaster.exe", directory);
   assert(GetModuleFileNameW(NULL, self, 32768));
   assert(CopyFileW(self, exe, TRUE));
   assert(CopyFileW(L"dvdisaster.com", launcher, TRUE));
   for(int layout=0; layout<5; layout++)
      check_streams(layout, TRUE, L"--gui-streams", launcher);
   assert(DeleteFileW(exe));
   assert(DeleteFileW(launcher));
   assert(RemoveDirectoryW(directory));
}

int main(int argc, char *argv[])
{  HANDLE thread;
   DWORD code;
   WinConsole gui;
   int saved_stdout, saved_stderr;

   if(argc == 3) return stream_child(argv[1], argv[2]);
   check_arguments();
   /* The in-process handoff test may release our CI output pipes too. */
   saved_stdout = _dup(_fileno(stdout));
   saved_stderr = _dup(_fileno(stderr));
   assert(saved_stdout >= 0 && saved_stderr >= 0);
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

   assert(!_dup2(saved_stdout, _fileno(stdout)));
   assert(!_dup2(saved_stderr, _fileno(stderr)));
   _close(saved_stdout);
   _close(saved_stderr);

   for(int layout=0; layout<5; layout++) check_streams(layout, TRUE, L"--gui-streams", NULL);
   check_streams(0, TRUE, L"--gui-extra-streams", NULL);
   check_streams(0, FALSE, L"--gui-streams", NULL);
   check_streams(0, TRUE, L"--cli-streams", NULL);
   check_launcher_streams();

   CloseHandle(handler_started);
   DeleteCriticalSection(&child_lock);
   puts("Windows console argument, cancellation, output pipe and GUI handoff tests passed.");
   return 0;
}
