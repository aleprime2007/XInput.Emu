#include <GameController.h>
#define execution_delay 8
#define app_name L"XInput.Emu"

// Callback for Force Feedback
VOID CALLBACK force_feedback_callback(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
){
	SDL_RumbleGamepad((SDL_Gamepad*)UserData, LargeMotor * 257, SmallMotor * 257, -1);
	SDL_SetGamepadPlayerIndex((SDL_Gamepad*)UserData, LedNumber);
}

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
	wstring dev_hiding = L"";
	wstring sixaxis = L"";
	wstring dualshock4 = L"";
	wstring dualsense = L"";
	wstring joycons = L"";
	wstring pro_controllers = L"";
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.emu", L"DevHiding", dev_hiding);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.emu", L"Sixaxis", sixaxis);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.emu", L"DualShock4", dualshock4);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.emu", L"DualSense", dualsense);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.emu", L"JoyCons", joycons);
	register_key_read_wstring(HKEY_LOCAL_MACHINE, L"SOFTWARE\\XInput.emu", L"ProControllers", pro_controllers);
	string v_sixaxis = sixaxis != L"True" ? "0x054c/0x0268," : "";
	string v_dualshock4 = dualshock4 != L"True" ? "0x054c/0x05c4,0x054c/0x09cc," : "";
	string v_dualsense = dualsense != L"True" ? "0x054c/0x0ce6,0x054c/0x0df2," : "";
	string v_joycons = joycons != L"True" ? "0x057e/0x2006,0x057e/0x2007," : "";
	string v_pro_controllers = joycons != L"True" ? "0x057e/0x2009," : "";

	//Init ViGEm
	PVIGEM_CLIENT vigem_client = vigem_alloc();
	const VIGEM_ERROR vigem_error = vigem_connect(vigem_client);
	VIGEM_ERROR vigem_last_error;

	//Init SDL
	SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "0");
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3_SIXAXIS_DRIVER, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_ENHANCED_REPORTS, "1");
	SDL_SetHint(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES, (v_sixaxis + v_dualshock4 + v_dualsense + v_joycons + v_pro_controllers).c_str());
	int sdl_error = SDL_Init(SDL_INIT_GAMEPAD);

	bool has_init = sdl_error >= 0 && vigem_client != nullptr && VIGEM_SUCCESS(vigem_error);
	const wstring error_msg =
		(wstring)L"ERROR: The program Could not initialize correctly\n"                              +
		(wstring)L"SDL Error: " + convert_string_to_wstring((string)SDL_GetError()) + (wstring)L"\n" +
		(wstring)L"ViGEm Error: " + to_wstring(vigem_error)                                          ;

	vector<SDL_Gamepad*> game_controllers;
	vector<PVIGEM_TARGET> emulated_controllers;
	int index;
	if (has_init){
		SDL_Event event;
		DWORD xinput_index;
		XINPUT_STATE xinput_state;
		Sint16 t_LT;
		Sint16 t_RT;

		wchar_t app_path[32768];
		GetModuleFileNameW(NULL, app_path, 32768);
		wstring hidhide_path = L"";
		if (get_hidhide_path(hidhide_path)){
			hidhide_cloak_on(hidhide_path.c_str());
			hidhide_app_reg(hidhide_path.c_str(), app_path);
		}
		if (dev_hiding != L"True") hidhide_path = L"";

		DWORD next_tick;
		DWORD sleep_time;

		while (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING){
			next_tick = GetTickCount() + execution_delay;
			while (SDL_PollEvent(&event)){
				if (event.gdevice.type == SDL_EVENT_GAMEPAD_ADDED)
				if (add_game_controller(game_controllers, event.gdevice.which, hidhide_path.c_str()))
				add_emulated_controller(vigem_client, emulated_controllers, vigem_last_error);

				for (index = 0; index < game_controllers.size(); index++)
				if (!SDL_GamepadConnected(game_controllers[index])){
					remove_game_controller(game_controllers, index);
					remove_emulated_controller(vigem_client, emulated_controllers, index, vigem_last_error);
					break;
				}
				else{
					vigem_last_error = vigem_target_x360_get_user_index(vigem_client, emulated_controllers[index], &xinput_index);
					XInputGetState(xinput_index, &xinput_state);

					update_xinput_gamepad(game_controllers[index], xinput_state.Gamepad, t_LT, t_RT);

					//Update Controller State
					vigem_last_error = vigem_target_x360_update(vigem_client, emulated_controllers[index], *reinterpret_cast<XUSB_REPORT*>(&xinput_state.Gamepad));

					//Vibration / Force Feedback
					vigem_last_error = vigem_target_x360_register_notification(vigem_client, emulated_controllers[index], &force_feedback_callback, game_controllers[index]);
				}
			}
			sleep_time = next_tick - GetTickCount();
			if (sleep_time > 0 && sleep_time <= execution_delay) Sleep(sleep_time);
		}
	}
	else MessageBoxW(NULL, error_msg.c_str(), app_name, MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);

	if (vigem_client != nullptr && VIGEM_SUCCESS(vigem_error)){
		for (index = 0; index < emulated_controllers.size(); index++){
			vigem_last_error = vigem_target_remove(vigem_client, emulated_controllers[index]);
			vigem_target_free(emulated_controllers[index]);
		}
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