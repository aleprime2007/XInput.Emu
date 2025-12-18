#include <GameController.h>
#define execution_delay 8

// Windows
const wchar_t* mutex_name = L"Global\\XInput_Emu_v1_x";
HANDLE h_mutex = CreateMutex(NULL, TRUE, mutex_name);
bool is_alredy_running = GetLastError() == ERROR_ALREADY_EXISTS;
wchar_t app_path[32768];

// Init
PVIGEM_CLIENT vigem_client = vigem_alloc();
const VIGEM_ERROR vigem_error = vigem_connect(vigem_client);
int sdl_error = SDL_Init(SDL_INIT_GAMECONTROLLER);
bool has_init = !is_alredy_running && sdl_error >= 0 && vigem_client != nullptr && VIGEM_SUCCESS(vigem_error);
bool executing = true;

wstring error_msg = 
	(wstring)L"ERROR: The program Could not initialize correctly\n"                              +
	(wstring)L"SDL Error: " + convert_string_to_wstring((string)SDL_GetError()) + (wstring)L"\n" +
	(wstring)L"ViGEm Error: " + to_wstring(vigem_error)                                          ;

// Controllers
vector<SDL_GameController*> game_controllers;
vector<PVIGEM_TARGET> emulated_controllers;

SDL_Event event;
SDL_GameControllerType controller_type;
XINPUT_STATE xinput_state;
DWORD xinput_index;
bool is_switch_controller;

// Triggers
Sint16 a_LT;
Sint16 a_RT;


// ==========> C++ Functions <========== \\

// Exits of the program when the function is called
void shutdown(){
	if (vigem_client != nullptr && VIGEM_SUCCESS(vigem_error)){
		for (int i = 0; i < emulated_controllers.size(); i++){
			vigem_target_remove(vigem_client, emulated_controllers[i]);
			vigem_target_free(emulated_controllers[i]);
		}
		vigem_disconnect(vigem_client);
		vigem_free(vigem_client);
	}
	if (sdl_error >= 0) SDL_Quit();
	if (h_mutex != NULL) CloseHandle(h_mutex);
}

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

// Main C++ Program Function
int main(int argc, char* argv[]){
	if (has_init){
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
				i--;
			}
			else{
				controller_type = SDL_GameControllerGetType(game_controllers[i]);
				vigem_target_x360_get_user_index(vigem_client, emulated_controllers[i], &xinput_index);
				XInputGetState(xinput_index, &xinput_state);

				//Axis
				a_LT = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 128;
				a_RT = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 128;

				xinput_state.Gamepad.sThumbLX = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTX);
				xinput_state.Gamepad.sThumbLY = -SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTY) - 1;
				xinput_state.Gamepad.sThumbRX = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_RIGHTX);
				xinput_state.Gamepad.sThumbRY = -SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_RIGHTY) - 1;
				xinput_state.Gamepad.bLeftTrigger = reinterpret_cast<const BYTE*>(&a_LT)[0];
				xinput_state.Gamepad.bRightTrigger = reinterpret_cast<const BYTE*>(&a_RT)[0];

				//Buttons
				is_switch_controller =
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO          ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT  ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR  ;

				xinput_state.Gamepad.wButtons =
					SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_B : SDL_CONTROLLER_BUTTON_A) * XUSB_GAMEPAD_A              +
					SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_A : SDL_CONTROLLER_BUTTON_B) * XUSB_GAMEPAD_B              +
					SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_Y : SDL_CONTROLLER_BUTTON_X) * XUSB_GAMEPAD_X              +
					SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_X : SDL_CONTROLLER_BUTTON_Y) * XUSB_GAMEPAD_Y              +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_BACK)                                               * XUSB_GAMEPAD_BACK           +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_GUIDE)                                              * XUSB_GAMEPAD_GUIDE          +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_START)                                              * XUSB_GAMEPAD_START          +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_LEFTSTICK)                                          * XUSB_GAMEPAD_LEFT_THUMB     +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSTICK)                                         * XUSB_GAMEPAD_RIGHT_THUMB    +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_LEFTSHOULDER)                                       * XUSB_GAMEPAD_LEFT_SHOULDER  +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)                                      * XUSB_GAMEPAD_RIGHT_SHOULDER +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_UP)                                            * XUSB_GAMEPAD_DPAD_UP        +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN)                                          * XUSB_GAMEPAD_DPAD_DOWN      +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT)                                          * XUSB_GAMEPAD_DPAD_LEFT      +
					SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT)                                         * XUSB_GAMEPAD_DPAD_RIGHT     ;

				//Update Controller State
				vigem_target_x360_update(vigem_client, emulated_controllers[i], *reinterpret_cast<XUSB_REPORT*>(&xinput_state.Gamepad));

				//Vibration / Force Feedback
				vigem_target_x360_register_notification(vigem_client, emulated_controllers[i], &force_feedback_callback, nullptr);
			}
		}
		shutdown();
		return 0;
	}
	else{
		if (!is_alredy_running) MessageBox(NULL, error_msg.c_str(), L"XInput.Emu", 16);
		shutdown();
		return -1;
	}
}