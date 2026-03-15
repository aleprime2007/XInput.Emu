#include <Gamepad.hpp>
#define execution_delay 8
#define app_name L"XInput.Emu"

SERVICE_STATUS g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;

void ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint){
	static DWORD dwCheckPoint = 1;
	g_ServiceStatus.dwCurrentState = dwCurrentState;
	g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	g_ServiceStatus.dwWaitHint = dwWaitHint;

	g_ServiceStatus.dwControlsAccepted = dwCurrentState != SERVICE_START_PENDING;

	if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED) g_ServiceStatus.dwCheckPoint = 0;
	else g_ServiceStatus.dwCheckPoint = dwCheckPoint++;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode){
	switch (CtrlCode){
		case SERVICE_CONTROL_STOP:
			if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING){
				ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
				SetEvent(g_ServiceStopEvent);
			}
			break;
	}
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv){
	g_StatusHandle = RegisterServiceCtrlHandlerW(app_name, ServiceCtrlHandler);

	if (g_StatusHandle == NULL) return;

	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 0);
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (g_ServiceStopEvent == NULL){
		ReportServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
		return;
	}
	ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

	//Load Settings
	wstring ds4_mode = L"False";
	wstring dev_hiding = L"True";
	wstring sixaxis = L"True";
	wstring dualshock4 = L"True";
	wstring dualsense = L"True";
	wstring joycons = L"True";
	wstring pro_controllers = L"True";
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.Emu", L"DS4Mode", &ds4_mode);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.Emu", L"DevHiding", &dev_hiding);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.Emu", L"Sixaxis", &sixaxis);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.Emu", L"DualShock4", &dualshock4);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.Emu", L"DualSense", &dualsense);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.Emu", L"JoyCons", &joycons);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.Emu", L"ProControllers", &pro_controllers);
	string v_sixaxis = sixaxis != L"True" ? "0x054c/0x0268," : "";
	string v_dualshock4 = dualshock4 != L"True" || ds4_mode == L"True" ? "0x054c/0x05c4,0x054c/0x09cc," : "";
	string v_dualsense = dualsense != L"True" || ds4_mode == L"True" ? "0x054c/0x0ce6,0x054c/0x0df2," : "";
	string v_joycons = joycons != L"True" ? "0x057e/0x2006,0x057e/0x2007," : "";
	string v_pro_controllers = pro_controllers != L"True" ? "0x057e/0x2009," : "";

	//Init ViGEm
	PVIGEM_CLIENT vigem_client = vigem_alloc();
	const VIGEM_ERROR vigem_error = vigem_connect(vigem_client);
	VIGEM_ERROR vigem_last_error;

	//Init SDL
	SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "0");
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3_SIXAXIS_DRIVER, "1");
	SDL_SetHint(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES, (v_sixaxis + v_dualshock4 + v_dualsense + v_joycons + v_pro_controllers).c_str());
	int sdl_error = SDL_Init(SDL_INIT_GAMEPAD);

	bool has_init = sdl_error >= 0 && vigem_client != nullptr && VIGEM_SUCCESS(vigem_error);
	const wstring error_msg =
		(wstring)L"ERROR: The program Could not initialize correctly\n"                              +
		(wstring)L"SDL Error: " + convert_string_to_wstring((string)SDL_GetError()) + (wstring)L"\n" +
		(wstring)L"ViGEm Error: " + to_wstring(vigem_error)                                          ;

	vector<X360_GAMEPAD> x360_gamepads;
	vector<DS4_GAMEPAD> ds4_gamepads;
	wstring hidhide_path = L"";
	size_t index;
	if (has_init){
		SDL_Event event;
		wchar_t app_path[32768];
		GetModuleFileNameW(NULL, app_path, 32768);
		if (get_hidhide_path(&hidhide_path)){
			hidhide_cloak_on(hidhide_path.c_str());
			hidhide_app_reg(hidhide_path.c_str(), app_path);
		}
		if (dev_hiding != L"True") hidhide_path = L"";

		DWORD next_tick;
		DWORD sleep_time;
		float sensor_count;
		int battery_percent;

		while (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING){
			next_tick = GetTickCount() + execution_delay;
			while (SDL_PollEvent(&event)){
				if (ds4_mode != L"True"){
					if (event.gdevice.type == SDL_EVENT_GAMEPAD_ADDED) add_x360_gamepad(&x360_gamepads, vigem_client, &vigem_last_error, event.gdevice.which, hidhide_path.c_str());
					else for (index = 0; index < x360_gamepads.size(); index++) if (!SDL_GamepadConnected(x360_gamepads[index].gamepad)){
						remove_x360_gamepad(&x360_gamepads, vigem_client, &vigem_last_error, index, hidhide_path.c_str());
						break;
					}
					else x360_gamepads[index].update_gamepad(vigem_client, &vigem_last_error);
				}
				else{
					if (event.gdevice.type == SDL_EVENT_GAMEPAD_ADDED) add_ds4_gamepad(&ds4_gamepads, vigem_client, &vigem_last_error, event.gdevice.which, hidhide_path.c_str());
					else for (index = 0; index < ds4_gamepads.size(); index++) if (!SDL_GamepadConnected(ds4_gamepads[index].gamepad)){
						remove_ds4_gamepad(&ds4_gamepads, vigem_client, &vigem_last_error, index, hidhide_path.c_str());
						break;
					}
					else{
						battery_percent = event.jbattery.percent * 255 / 100;
						ds4_gamepads[index].ds4_report.Report.bBatteryLvl = *reinterpret_cast<BYTE*>(&battery_percent);
						ds4_gamepads[index].update_gamepad(vigem_client, &vigem_last_error);
					}
				}
			}
			sleep_time = next_tick - GetTickCount();
			if (sleep_time > 0 && sleep_time <= execution_delay) Sleep(sleep_time);
		}
	}
	else MessageBoxW(NULL, error_msg.c_str(), app_name, MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);

	while (x360_gamepads.size() > 0) remove_x360_gamepad(&x360_gamepads, vigem_client, &vigem_last_error, x360_gamepads.size() - 1, hidhide_path.c_str());
	while (ds4_gamepads.size() > 0) remove_ds4_gamepad(&ds4_gamepads, vigem_client, &vigem_last_error, ds4_gamepads.size() - 1, hidhide_path.c_str());
	if (vigem_client != nullptr && VIGEM_SUCCESS(vigem_error)){
		vigem_disconnect(vigem_client);
		vigem_free(vigem_client);
	}
	if (sdl_error >= 0) SDL_Quit();
	ReportServiceStatus(SERVICE_STOPPED, has_init ? NO_ERROR : ERROR, 0);
}

int _tmain(int argc, TCHAR* argv[]){
	SERVICE_TABLE_ENTRY ServiceTable[] = {
		{(TCHAR*)app_name, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) return GetLastError();
	return 0;
}