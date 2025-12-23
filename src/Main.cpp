#include <GameController.h>

bool executing = true;
vector<SDL_GameController*> game_controllers;
vector<PVIGEM_TARGET> emulated_controllers;
SDL_Event event;
SDL_GameControllerType controller_type;
XINPUT_STATE xinput_state;
DWORD xinput_index;
bool sw_ctrl;
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
	for (int i = 0; i < emulated_controllers.size(); i++) 
	if (emulated_controllers[i] == Target) 
	SDL_GameControllerRumble(game_controllers[i], SmallMotor, LargeMotor, execution_delay);
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
				executing = false;
				ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
				SetEvent(g_ServiceStopEvent);
			}
			break;
	}
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv){
	g_StatusHandle = RegisterServiceCtrlHandler(L"XInput.Emu", ServiceCtrlHandler);

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

	// Init
	PVIGEM_CLIENT vigem_client = vigem_alloc();
	const VIGEM_ERROR vigem_error = vigem_connect(vigem_client);
	int sdl_error = SDL_Init(SDL_INIT_GAMECONTROLLER);
	bool has_init = sdl_error >= 0 && vigem_client != nullptr && VIGEM_SUCCESS(vigem_error);
	const wstring error_msg =
		(wstring)L"ERROR: The program Could not initialize correctly\n"                              +
		(wstring)L"SDL Error: " + convert_string_to_wstring((string)SDL_GetError()) + (wstring)L"\n" +
		(wstring)L"ViGEm Error: " + to_wstring(vigem_error)                                          ;

	if (has_init){
		wchar_t app_path[32768];
		GetModuleFileName(NULL, app_path, 32768);
		hidhide_app_reg(app_path);

		while (executing){
			SDL_Delay(execution_delay);
			SDL_PollEvent(&event);
			SDL_GameControllerUpdate();

			if (event.cdevice.type == SDL_CONTROLLERDEVICEADDED)
			if (add_game_controller(game_controllers, event.cdevice.which))
			add_emulated_controller(vigem_client, emulated_controllers);

			for (int i = 0; i < game_controllers.size(); i++)
			if (SDL_GameControllerGetAttached(game_controllers[i]) == SDL_FALSE){
				remove_game_controller(game_controllers, i);
				remove_emulated_controller(vigem_client, emulated_controllers, i);
				break;
			}
			else{
				controller_type = SDL_GameControllerGetType(game_controllers[i]);
				vigem_target_x360_get_user_index(vigem_client, emulated_controllers[i], &xinput_index);
				XInputGetState(xinput_index, &xinput_state);
				sw_ctrl =
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO          ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT  ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR  ;

				update_xinput_state(game_controllers, i, xinput_state, t_LT, t_RT, sw_ctrl);

				//Update Controller State
				vigem_target_x360_update(vigem_client, emulated_controllers[i], *reinterpret_cast<XUSB_REPORT*>(&xinput_state.Gamepad));

				//Vibration / Force Feedback
				vigem_target_x360_register_notification(vigem_client, emulated_controllers[i], &force_feedback_callback, nullptr);
			}
		}
	}
	else MessageBox(NULL, error_msg.c_str(), L"XInput.Emu", MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);

	if (vigem_client != nullptr && VIGEM_SUCCESS(vigem_error)){
		for (int i = 0; i < emulated_controllers.size(); i++){
			vigem_target_remove(vigem_client, emulated_controllers[i]);
			vigem_target_free(emulated_controllers[i]);
		}
		vigem_disconnect(vigem_client);
		vigem_free(vigem_client);
	}
	if (sdl_error >= 0) SDL_Quit();
	ReportServiceStatus(SERVICE_STOPPED, has_init ? NO_ERROR : ERROR, 0);
}

int _tmain(int argc, TCHAR* argv[]){
	SERVICE_TABLE_ENTRY ServiceTable[] = {
		{(TCHAR*)L"XInput.Emu", (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) return GetLastError();
	return 0;
}