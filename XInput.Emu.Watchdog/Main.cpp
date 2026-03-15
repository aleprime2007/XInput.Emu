#include <Windows.h>
#include <shellapi.h>
#include <psapi.h>
#define execution_delay 8

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam){
    HWND foreground_window = GetForegroundWindow();
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);

    char buffer[MAX_PATH];
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessId);
    
    if (hProcess){
        if (GetModuleBaseNameA(hProcess, NULL, buffer, MAX_PATH)){
            if (!strcmp(buffer, "steamclientwebhelper.exe")){
                if (hwnd == foreground_window) ShellExecuteW(NULL, L"open", L"sc.exe", L"stop XInput.Emu", L"C:\\Windows\\System32", 0);
            }
            else if (!strcmp(buffer, "GameOverlayUI.exe")) ShellExecuteW(NULL, L"open", L"sc.exe", L"stop XInput.Emu", L"C:\\Windows\\System32", 0);
            //else ShellExecuteW(NULL, L"open", L"sc.exe", L"start XInput.Emu", L"C:\\Windows\\System32", 0);
        }
        CloseHandle(hProcess);
    }
    return TRUE;
}

int main(){
	HWND steam_hwnd;
	HWND steam_bigpicture_hwnd;
	HWND game_overlay_hwnd;

	while (true){
        EnumWindows(EnumWindowsProc, 0);
		Sleep(execution_delay);
	}
}