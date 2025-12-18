#include <GameController.h>
#define execution_delay 8

// SDL Buttons
#define BTN_A     SDL_CONTROLLER_BUTTON_A
#define BTN_B     SDL_CONTROLLER_BUTTON_B
#define BTN_X     SDL_CONTROLLER_BUTTON_X
#define BTN_Y     SDL_CONTROLLER_BUTTON_Y
#define BTN_BK    SDL_CONTROLLER_BUTTON_BACK
#define BTN_GD    SDL_CONTROLLER_BUTTON_GUIDE
#define BTN_ST    SDL_CONTROLLER_BUTTON_START
#define BTN_LS    SDL_CONTROLLER_BUTTON_LEFTSTICK
#define BTN_RS    SDL_CONTROLLER_BUTTON_RIGHTSTICK
#define BTN_LB    SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define BTN_RB    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define BTN_DP_UP SDL_CONTROLLER_BUTTON_DPAD_UP
#define BTN_DP_DN SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define BTN_DP_L  SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define BTN_DP_R  SDL_CONTROLLER_BUTTON_DPAD_RIGHT

// XINPUT Buttons
#define XBTN_A     XUSB_GAMEPAD_A
#define XBTN_B     XUSB_GAMEPAD_B
#define XBTN_X     XUSB_GAMEPAD_X
#define XBTN_Y     XUSB_GAMEPAD_Y
#define XBTN_BK    XUSB_GAMEPAD_BACK
#define XBTN_GD    XUSB_GAMEPAD_GUIDE
#define XBTN_ST    XUSB_GAMEPAD_START
#define XBTN_LS    XUSB_GAMEPAD_LEFT_THUMB
#define XBTN_RS    XUSB_GAMEPAD_RIGHT_THUMB
#define XBTN_LB    XUSB_GAMEPAD_LEFT_SHOULDER
#define XBTN_RB    XUSB_GAMEPAD_RIGHT_SHOULDER
#define XBTN_DP_UP XUSB_GAMEPAD_DPAD_UP
#define XBTN_DP_DN XUSB_GAMEPAD_DPAD_DOWN
#define XBTN_DP_L  XUSB_GAMEPAD_DPAD_LEFT
#define XBTN_DP_R  XUSB_GAMEPAD_DPAD_RIGHT

// Windows
const wchar_t* mutex_name = L"Global\\XInput_Emu_v1_x";
HANDLE h_mutex = CreateMutex(NULL, TRUE, mutex_name);
bool is_alredy_running = GetLastError() == ERROR_ALREADY_EXISTS;
bool is_system = is_running_as_system();
wchar_t app_path[32768];

// Init
PVIGEM_CLIENT vigem_client = vigem_alloc();
const VIGEM_ERROR vigem_error = vigem_connect(vigem_client);
int sdl_error = SDL_Init(SDL_INIT_GAMECONTROLLER);
bool has_init = !is_alredy_running && sdl_error >= 0 && vigem_client != nullptr && VIGEM_SUCCESS(vigem_error);
bool executing = true;

const wstring error_msg = 
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
bool sw_ctrl;

// Triggers
Sint16 t_LT;
Sint16 t_RT;

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
		if (!is_system){
			MessageBox(NULL, L"ERROR: The program must run as SYSTEM", L"XInput.Emu", MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);
			executing = false;
		}

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

				//Triggers
				t_LT = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 128;
				t_RT = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 128;

				//Axis
				xinput_state.Gamepad.sThumbLX = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTX);
				xinput_state.Gamepad.sThumbLY = -SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTY) - 1;
				xinput_state.Gamepad.sThumbRX = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_RIGHTX);
				xinput_state.Gamepad.sThumbRY = -SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_RIGHTY) - 1;
				xinput_state.Gamepad.bLeftTrigger = reinterpret_cast<const BYTE*>(&t_LT)[0];
				xinput_state.Gamepad.bRightTrigger = reinterpret_cast<const BYTE*>(&t_RT)[0];

				//Buttons
				sw_ctrl =
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO          ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT  ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR  ;

				xinput_state.Gamepad.wButtons =
					SDL_GameControllerGetButton(game_controllers[i], sw_ctrl ? BTN_B : BTN_A) * XBTN_A     +
					SDL_GameControllerGetButton(game_controllers[i], sw_ctrl ? BTN_A : BTN_B) * XBTN_B     +
					SDL_GameControllerGetButton(game_controllers[i], sw_ctrl ? BTN_Y : BTN_X) * XBTN_X     +
					SDL_GameControllerGetButton(game_controllers[i], sw_ctrl ? BTN_X : BTN_Y) * XBTN_Y     +
					SDL_GameControllerGetButton(game_controllers[i], BTN_BK)                  * XBTN_BK    +
					SDL_GameControllerGetButton(game_controllers[i], BTN_GD)                  * XBTN_GD    +
					SDL_GameControllerGetButton(game_controllers[i], BTN_ST)                  * XBTN_ST    +
					SDL_GameControllerGetButton(game_controllers[i], BTN_LS)                  * XBTN_LS    +
					SDL_GameControllerGetButton(game_controllers[i], BTN_RS)                  * XBTN_RS    +
					SDL_GameControllerGetButton(game_controllers[i], BTN_LB)                  * XBTN_LB    +
					SDL_GameControllerGetButton(game_controllers[i], BTN_RB)                  * XBTN_RB    +
					SDL_GameControllerGetButton(game_controllers[i], BTN_DP_UP)               * XBTN_DP_UP +
					SDL_GameControllerGetButton(game_controllers[i], BTN_DP_DN)               * XBTN_DP_DN +
					SDL_GameControllerGetButton(game_controllers[i], BTN_DP_L)                * XBTN_DP_L  +
					SDL_GameControllerGetButton(game_controllers[i], BTN_DP_R)                * XBTN_DP_R  ;

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
		if (!is_alredy_running) MessageBox(NULL, error_msg.c_str(), L"XInput.Emu", MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION);
		shutdown();
		return -1;
	}
}