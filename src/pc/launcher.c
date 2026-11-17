/* sm2.exe: linked at the XBE base address (0x10000) with a large uninitialised section, so Windows itself
 * reserves the address range the game expects before anything else (heaps, stacks, DLLs) can land there.
 *
 * It then hands over to sm2runtime.dll, which overwrites this whole image with the XBE. No CRT, no globals
 * other than the reservation, and nothing may run here after sm2_run is called. */

#include <windows.h>

#include "restart_codes.h"

/* Covers the XBE image (0x52F9C0 bytes for SM2) with room for sections loaded on demand. */
#define XBE_RESERVE_SIZE 0x01000000u

/* In the ordinary uninitialised data (.data, no file bytes): a section of its own with an unusual name and no data is
 * what packed executables look like, and antivirus heuristics flagged sm2.exe for it. */
unsigned char g_xbe_reserve[XBE_RESERVE_SIZE];

typedef void(__cdecl* Sm2RunFunction)(void* reserved, unsigned long reserved_size);

/* Started by RecompLauncher: this process starts the game as a child and waits, so the process RecompLauncher watches
 * lasts until the player quits, through the game's restarts (restart_codes.h), and ends with the game's exit code.
 * FALSE when the first child cannot start: the game then runs here, as without RecompLauncher. */
static BOOL supervise(DWORD* exit_code) {
    wchar_t exe[MAX_PATH];
    const wchar_t* line = GetCommandLineW();
    wchar_t* command = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, ((SIZE_T)lstrlenW(line) + 1) * sizeof(wchar_t));
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits;
    STARTUPINFOW startup;
    PROCESS_INFORMATION process;
    HANDLE job;
    BOOL first = TRUE;
    if (!command || !GetModuleFileNameW(NULL, exe, MAX_PATH)) return FALSE;
    SetEnvironmentVariableW(L"SM2_SUPERVISED", L"1");
    /* If this process is ended (RecompLauncher's Stop), the game goes with it; what the game opens (a browser, a
     * folder) leaves the job and stays. */
    job = CreateJobObjectW(NULL, NULL);
    if (job) {
        RtlSecureZeroMemory(&limits, sizeof(limits));
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
        SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    }
    for (;;) {
        /* RecompLauncher's window state (shown, maximised) carries over; its handles do not. */
        GetStartupInfoW(&startup);
        startup.dwFlags &= ~STARTF_USESTDHANDLES;
        startup.cbReserved2 = 0;
        startup.lpReserved2 = NULL;
        lstrcpyW(command, line); /* CreateProcessW may write to it */
        if (!CreateProcessW(exe, command, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startup, &process)) {
            if (first) {
                SetEnvironmentVariableW(L"SM2_SUPERVISED", NULL);
                return FALSE;
            }
            *exit_code = 1;
            return TRUE;
        }
        first = FALSE;
        if (job) AssignProcessToJobObject(job, process.hProcess);
        AllowSetForegroundWindow(process.dwProcessId); /* the game's window in front, as if RecompLauncher started it */
        ResumeThread(process.hThread);
        CloseHandle(process.hThread);
        WaitForSingleObject(process.hProcess, INFINITE);
        if (!GetExitCodeProcess(process.hProcess, exit_code)) *exit_code = 1;
        CloseHandle(process.hProcess);
        /* What the old process set before starting the next one itself (launcher_ui.cpp). */
        SetEnvironmentVariableW(L"SM2_SKIP_LAUNCHER", NULL);
        SetEnvironmentVariableW(L"SM2_SHOW_LAUNCHER", NULL);
        SetEnvironmentVariableW(L"SM2_MOVE_OLD_DATA", NULL);
        if (*exit_code == SM2_EXIT_RESTART_GAME) {
            SetEnvironmentVariableW(L"SM2_SKIP_LAUNCHER", L"1");
        } else if (*exit_code == SM2_EXIT_BACK_TO_LAUNCHER) {
            SetEnvironmentVariableW(L"SM2_SHOW_LAUNCHER", L"1");
        } else if (*exit_code == SM2_EXIT_MOVE_OLD_DATA) {
            SetEnvironmentVariableW(L"SM2_MOVE_OLD_DATA", L"1");
            SetEnvironmentVariableW(L"SM2_SHOW_LAUNCHER", L"1"); /* it was in the launcher, even with a disc given */
        } else {
            return TRUE;
        }
    }
}

void __cdecl LauncherEntry(void) {
    DWORD exit_code;
    if (GetEnvironmentVariableW(L"RECOMPLAUNCHER", NULL, 0) && !GetEnvironmentVariableW(L"SM2_SUPERVISED", NULL, 0) &&
        supervise(&exit_code)) {
        ExitProcess(exit_code);
    }
    HMODULE runtime = LoadLibraryW(L"sm2runtime.dll");
    Sm2RunFunction run = runtime ? (Sm2RunFunction)GetProcAddress(runtime, "sm2_run") : NULL;
    if (!run) {
        /* The DLL may be missing, quarantined by an antivirus, or one of the Windows components it needs may be
         * (Media Foundation is absent from Windows N editions): the error says which. wsprintf and FormatMessage
         * need no C runtime. */
        DWORD error = GetLastError();
        char reason[256];
        char message[768];
        if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, reason,
                            sizeof(reason), NULL)) {
            reason[0] = 0;
        }
        wsprintfA(message,
                  "Cannot load sm2runtime.dll (error %lu). %s\n\n"
                  "The file must be next to sm2.exe. If it is there, a Windows component is missing: on Windows N "
                  "editions install the Media Feature Pack (Settings > Apps > Optional features). An antivirus may "
                  "also have quarantined the file.",
                  error, reason);
        MessageBoxA(NULL, message, "sm2port", MB_ICONERROR | MB_OK);
        ExitProcess(1);
    }
    run(g_xbe_reserve, XBE_RESERVE_SIZE);
    ExitProcess(1); /* not reached */
}
