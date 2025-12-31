#include <GameController.h>
#define execution_delay 8
#define app_name L"XInput.Emu"

vector<SDL_Gamepad*> game_controllers;
vector<PVIGEM_TARGET> emulated_controllers;
SDL_Event event;
SDL_Gamepad* game_controller;
SDL_GamepadType controller_type;
XINPUT_STATE xinput_state;
DWORD xinput_index;
bool is_switch_controller;
Sint16 t_LT;
Sint16 t_RT;

// Callback for Force Feedback
VOID CALLBACK force_feedback_callback(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
){
	SDL_RumbleGamepad(game_controller, (SmallMotor + 1) * 256 - 1, (LargeMotor + 1) * 256 - 1, execution_delay);
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
	g_StatusHandle = RegisterServiceCtrlHandler(app_name, ServiceCtrlHandler);

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

	//Init ViGEm
	PVIGEM_CLIENT vigem_client = vigem_alloc();
	const VIGEM_ERROR vigem_error = vigem_connect(vigem_client);
	VIGEM_ERROR vigem_last_error;

	//Init SDL
	SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "0");
	int sdl_error = SDL_Init(SDL_INIT_GAMEPAD);

	DWORD win_last_error;
	bool has_init = sdl_error >= 0 && vigem_client != nullptr && VIGEM_SUCCESS(vigem_error);
	const wstring error_msg =
		(wstring)L"ERROR: The program Could not initialize correctly\n"                              +
		(wstring)L"SDL Error: " + convert_string_to_wstring((string)SDL_GetError()) + (wstring)L"\n" +
		(wstring)L"ViGEm Error: " + to_wstring(vigem_error)                                          ;

	int index;
	if (has_init){
		wchar_t app_path[32768];
		win_last_error = GetModuleFileNameW(NULL, app_path, 32768);
		wstring hidhide_path;
		bool hidhide_exists = get_hidhide_path(hidhide_path);
		if (hidhide_exists) hidhide_app_reg(hidhide_path.c_str(), app_path);
		while (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING){
			SDL_Delay(execution_delay);
			while (SDL_PollEvent(&event)){
				if (event.cdevice.type == SDL_EVENT_GAMEPAD_ADDED)
				if (add_game_controller(game_controllers, event.gdevice.which, hidhide_exists ? hidhide_path.c_str() : L""))
				add_emulated_controller(vigem_client, emulated_controllers, vigem_last_error);

				for (index = 0; index < game_controllers.size(); index++)
				if (!SDL_GamepadConnected(game_controllers[index])){
					remove_game_controller(game_controllers, index);
					remove_emulated_controller(vigem_client, emulated_controllers, index, vigem_last_error);
					break;
				}
				else{
					game_controller = game_controllers[index];
					controller_type = SDL_GetGamepadType(game_controller);
					vigem_last_error = vigem_target_x360_get_user_index(vigem_client, emulated_controllers[index], &xinput_index);
					win_last_error = XInputGetState(xinput_index, &xinput_state);

					update_xinput_gamepad(game_controller, xinput_state.Gamepad, t_LT, t_RT);

					//Update Controller State
					vigem_last_error = vigem_target_x360_update(vigem_client, emulated_controllers[index], *reinterpret_cast<XUSB_REPORT*>(&xinput_state.Gamepad));

					//Vibration / Force Feedback
					vigem_last_error = vigem_target_x360_register_notification(vigem_client, emulated_controllers[index], &force_feedback_callback, nullptr);
				}
			}
		}
	}
	else MessageBox(NULL, error_msg.c_str(), app_name, MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);

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