#include <GameController.h>
#define execution_delay 8

// Windows
HWND main_console_window = GetConsoleWindow();
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
bool debug = false;

// Controllers
vector<SDL_GameController*> game_controllers;
vector<PVIGEM_TARGET> emulated_controllers;

SDL_Event event;
SDL_GameControllerType controller_type;
XINPUT_STATE xinput_state;
DWORD xinput_index;

// Axis
Sint16 a_LX;
Sint16 a_LY;
Sint16 a_RX;
Sint16 a_RY;
Sint16 a_LT;
Sint16 a_RT;

// Buttons
Uint8 b_A;
Uint8 b_B;
Uint8 b_X;
Uint8 b_Y;
Uint8 b_BACK;
Uint8 b_GUIDE;
Uint8 b_START;
Uint8 b_LS;
Uint8 b_RS;
Uint8 b_LB;
Uint8 b_RB;
Uint8 b_DPAD_UP;
Uint8 b_DPAD_DOWN;
Uint8 b_DPAD_LEFT;
Uint8 b_DPAD_RIGHT;

bool is_switch_controller;
bool is_button;


// ==========> C++ Functions <========== \\

// Exits of the program when the function is called
void exit(){
	executing = false;
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
		for (int i = 0; i < argc; i++) if (compare_strings(argv[i], "/debug")) debug = true;
		if (!debug) ShowWindow(main_console_window, SW_HIDE);
		else {
			system("cls");
			print_controllers_info(vigem_client, game_controllers, emulated_controllers);
		}
		GetModuleFileName(NULL, app_path, 32768);
		hidhide_app_reg(app_path);

		while (executing) {
			SDL_Delay(execution_delay);
			SDL_PollEvent(&event);
			SDL_GameControllerUpdate();

			if (event.cdevice.type == SDL_CONTROLLERDEVICEADDED) {
				if (add_game_controller(game_controllers, event.cdevice.which)) add_emulated_controller(vigem_client, emulated_controllers);
				if (debug) {
					system("cls");
					print_controllers_info(vigem_client, game_controllers, emulated_controllers);
				}
			}

			for (int i = 0; i < game_controllers.size(); i++) if (SDL_GameControllerGetAttached(game_controllers[i]) == SDL_FALSE) {
				remove_game_controller(game_controllers, i);
				remove_emulated_controller(vigem_client, emulated_controllers, i);
				if (debug) {
					system("cls");
					print_controllers_info(vigem_client, game_controllers, emulated_controllers);
				}
				i--;
			}
			else {
				controller_type = SDL_GameControllerGetType(game_controllers[i]);
				vigem_target_x360_get_user_index(vigem_client, emulated_controllers[i], &xinput_index);
				XInputGetState(xinput_index, &xinput_state);

				//Axis
				a_LX = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTX);
				a_LY = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTY);
				a_RX = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_RIGHTX);
				a_RY = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_RIGHTY);
				a_LT = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 128;
				a_RT = SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 128;

				xinput_state.Gamepad.sThumbLX = a_LX;
				xinput_state.Gamepad.sThumbLY = invert_axis(a_LY);
				xinput_state.Gamepad.sThumbRX = a_RX;
				xinput_state.Gamepad.sThumbRY = invert_axis(a_RY);
				xinput_state.Gamepad.bLeftTrigger = reinterpret_cast<const BYTE*>(&a_LT)[0];
				xinput_state.Gamepad.bRightTrigger = reinterpret_cast<const BYTE*>(&a_RT)[0];

				//Buttons
				is_switch_controller =
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO          ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT  ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT ||
					controller_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR  ;

				b_A = SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_B : SDL_CONTROLLER_BUTTON_A);
				b_B = SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_A : SDL_CONTROLLER_BUTTON_B);
				b_X = SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_Y : SDL_CONTROLLER_BUTTON_X);
				b_Y = SDL_GameControllerGetButton(game_controllers[i], is_switch_controller ? SDL_CONTROLLER_BUTTON_X : SDL_CONTROLLER_BUTTON_Y);
				b_BACK = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_BACK);
				b_GUIDE = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_GUIDE);
				b_START = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_START);
				b_LS = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_LEFTSTICK);
				b_RS = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSTICK);
				b_LB = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
				b_RB = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
				b_DPAD_UP = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_UP);
				b_DPAD_DOWN = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
				b_DPAD_LEFT = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
				b_DPAD_RIGHT = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

				xinput_state.Gamepad.wButtons =
					b_A          * XUSB_GAMEPAD_A              +
					b_B          * XUSB_GAMEPAD_B              +
					b_X          * XUSB_GAMEPAD_X              +
					b_Y          * XUSB_GAMEPAD_Y              +
					b_BACK       * XUSB_GAMEPAD_BACK           +
					b_GUIDE      * XUSB_GAMEPAD_GUIDE          +
					b_START      * XUSB_GAMEPAD_START          +
					b_LS         * XUSB_GAMEPAD_LEFT_THUMB     +
					b_RS         * XUSB_GAMEPAD_RIGHT_THUMB    +
					b_LB         * XUSB_GAMEPAD_LEFT_SHOULDER  +
					b_RB         * XUSB_GAMEPAD_RIGHT_SHOULDER +
					b_DPAD_UP    * XUSB_GAMEPAD_DPAD_UP        +
					b_DPAD_DOWN  * XUSB_GAMEPAD_DPAD_DOWN      +
					b_DPAD_LEFT  * XUSB_GAMEPAD_DPAD_LEFT      +
					b_DPAD_RIGHT * XUSB_GAMEPAD_DPAD_RIGHT     ;

				is_button =
					b_A         ||
					b_B         ||
					b_X         ||
					b_Y         ||
					b_BACK      ||
					b_GUIDE     ||
					b_START     ||
					b_LS        ||
					b_RS        ||
					b_LB        ||
					b_RB        ||
					b_DPAD_UP   ||
					b_DPAD_DOWN ||
					b_DPAD_LEFT ||
					b_DPAD_RIGHT;

				//Update Controller State
				vigem_target_x360_update(vigem_client, emulated_controllers[i], *reinterpret_cast<XUSB_REPORT*>(&xinput_state.Gamepad));

				//Vibration / Force Feedback
				vigem_target_x360_register_notification(vigem_client, emulated_controllers[i], &force_feedback_callback, nullptr);

				//Print Debug Information
				if (debug && event.cbutton.type == SDL_CONTROLLERBUTTONDOWN && is_button) {

					//Print game_controller Info
					cout << "Index " << i << " : XInput Index " << xinput_index << " : " << SDL_GameControllerName(game_controllers[i]);

					//Print Buttons
					if (b_A) cout << " : Button A";
					if (b_B) cout << " : Button B";
					if (b_X) cout << " : Button X";
					if (b_Y) cout << " : Button Y";
					if (b_BACK) cout << " : Button BACK";
					if (b_GUIDE) cout << " : Button GUIDE";
					if (b_START) cout << " : Button START";
					if (b_LS) cout << " : Button LS";
					if (b_RS) cout << " : Button RS";
					if (b_LB) cout << " : Button LB";
					if (b_RB) cout << " : Button RB";
					if (b_DPAD_UP) cout << " : Button DPAD UP";
					if (b_DPAD_DOWN) cout << " : Button DPAD DOWN";
					if (b_DPAD_LEFT) cout << " : Button DPAD LEFT";
					if (b_DPAD_RIGHT) cout << " : Button DPAD RIGHT";
					cout << endl;
				}
			}
		}
		exit();
		return 0;
	}
	else{
		cout << "ERROR: The program Could not initialize correctly" << endl << SDL_GetError() << endl << vigem_error << endl;
		exit();
		return 1;
	}
}