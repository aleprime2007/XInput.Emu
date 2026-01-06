#include <HidHide.h>
#define guid_size 33

// SDL Buttons
#define BTN_A     SDL_GAMEPAD_BUTTON_SOUTH
#define BTN_B     SDL_GAMEPAD_BUTTON_EAST
#define BTN_X     SDL_GAMEPAD_BUTTON_WEST
#define BTN_Y     SDL_GAMEPAD_BUTTON_NORTH
#define BTN_BK    SDL_GAMEPAD_BUTTON_BACK
#define BTN_GD    SDL_GAMEPAD_BUTTON_GUIDE
#define BTN_ST    SDL_GAMEPAD_BUTTON_START
#define BTN_LS    SDL_GAMEPAD_BUTTON_LEFT_STICK
#define BTN_RS    SDL_GAMEPAD_BUTTON_RIGHT_STICK
#define BTN_LB    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER
#define BTN_RB    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER
#define BTN_DP_UP SDL_GAMEPAD_BUTTON_DPAD_UP
#define BTN_DP_DN SDL_GAMEPAD_BUTTON_DPAD_DOWN
#define BTN_DP_L  SDL_GAMEPAD_BUTTON_DPAD_LEFT
#define BTN_DP_R  SDL_GAMEPAD_BUTTON_DPAD_RIGHT

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

const char* device_path;
unsigned short device_usage;


// ==========> ViGEmClient Functions <========== \\

// Adds a X360 emuated controller to the given vector
void add_emulated_controller(PVIGEM_CLIENT vigem_client, vector<PVIGEM_TARGET> &emulated_controllers, USHORT product_id, VIGEM_ERROR &vigem_last_error){
	emulated_controllers.insert(emulated_controllers.end(), vigem_target_x360_alloc());
	vigem_target_set_pid(emulated_controllers[emulated_controllers.size() - 1], product_id);
	vigem_last_error = vigem_target_add(vigem_client, emulated_controllers[emulated_controllers.size() - 1]);
}

// Removes a X360 emulated controller from a given vector
void remove_emulated_controller(PVIGEM_CLIENT vigem_client, vector<PVIGEM_TARGET> &emulated_controllers, int index, VIGEM_ERROR &vigem_last_error){
	if (index < emulated_controllers.size()){
		vigem_last_error = vigem_target_remove(vigem_client, emulated_controllers[index]);
		vigem_target_free(emulated_controllers[index]);
		emulated_controllers.erase(emulated_controllers.begin() + index);
	}
}


// ==========> SDL Functions <========== \\

// Adds the connected game controllers to a given vector
bool add_game_controller(vector<SDL_Gamepad*> &game_controllers, SDL_JoystickID id, const wchar_t* hidhide_path){
	if (SDL_IsGamepad(id)){
		device_path = SDL_GetGamepadPathForID(id);
		device_usage = SDL_hid_get_device_info(SDL_hid_open_path(device_path))->usage;
		if (wstring(hidhide_path) != wstring(L"")) hidhide_dev_hide(hidhide_path, device_path);
		if (device_usage == 4 || device_usage == 5){
			game_controllers.insert(game_controllers.end(), SDL_OpenGamepad(id));
			return true;
		}
	}
	SDL_FlushEvent(SDL_EVENT_GAMEPAD_ADDED);
	return false;
}

// Removes the disconnected game controllers from a given vector
void remove_game_controller(vector<SDL_Gamepad*> &game_controllers, int index){
	if (index < game_controllers.size()){
		SDL_CloseGamepad(game_controllers[index]);
		game_controllers.erase(game_controllers.begin() + index);
	}
}


// ==========> Common Functions <========== \\

// Updates the state of a XINPUT_GAMEPAD with the inputs of a SDL_Gamepad*
void update_xinput_gamepad(SDL_Gamepad* game_controller, XINPUT_GAMEPAD &xinput_gamepad, Sint16 &t_LT, Sint16 &t_RT){

	//Triggers
	t_LT = SDL_GetGamepadAxis(game_controller, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) / 128;
	t_RT = SDL_GetGamepadAxis(game_controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / 128;

	//Axis
	xinput_gamepad.sThumbLX = SDL_GetGamepadAxis(game_controller, SDL_GAMEPAD_AXIS_LEFTX);
	xinput_gamepad.sThumbLY = -SDL_GetGamepadAxis(game_controller, SDL_GAMEPAD_AXIS_LEFTY) - 1;
	xinput_gamepad.sThumbRX = SDL_GetGamepadAxis(game_controller, SDL_GAMEPAD_AXIS_RIGHTX);
	xinput_gamepad.sThumbRY = -SDL_GetGamepadAxis(game_controller, SDL_GAMEPAD_AXIS_RIGHTY) - 1;
	xinput_gamepad.bLeftTrigger = reinterpret_cast<const BYTE*>(&t_LT)[0];
	xinput_gamepad.bRightTrigger = reinterpret_cast<const BYTE*>(&t_RT)[0];

	//Buttons
	xinput_gamepad.wButtons =
		SDL_GetGamepadButton(game_controller, BTN_A)     * XBTN_A     +
		SDL_GetGamepadButton(game_controller, BTN_B)     * XBTN_B     +
		SDL_GetGamepadButton(game_controller, BTN_X)     * XBTN_X     +
		SDL_GetGamepadButton(game_controller, BTN_Y)     * XBTN_Y     +
		SDL_GetGamepadButton(game_controller, BTN_BK)    * XBTN_BK    +
		SDL_GetGamepadButton(game_controller, BTN_GD)    * XBTN_GD    +
		SDL_GetGamepadButton(game_controller, BTN_ST)    * XBTN_ST    +
		SDL_GetGamepadButton(game_controller, BTN_LS)    * XBTN_LS    +
		SDL_GetGamepadButton(game_controller, BTN_RS)    * XBTN_RS    +
		SDL_GetGamepadButton(game_controller, BTN_LB)    * XBTN_LB    +
		SDL_GetGamepadButton(game_controller, BTN_RB)    * XBTN_RB    +
		SDL_GetGamepadButton(game_controller, BTN_DP_UP) * XBTN_DP_UP +
		SDL_GetGamepadButton(game_controller, BTN_DP_DN) * XBTN_DP_DN +
		SDL_GetGamepadButton(game_controller, BTN_DP_L)  * XBTN_DP_L  +
		SDL_GetGamepadButton(game_controller, BTN_DP_R)  * XBTN_DP_R  ;
}

// Returns the Product ID equivalent to a SDL_JoystickType
USHORT get_xinput_product_id(SDL_JoystickType joystick_type){
	switch (joystick_type){
		case SDL_JOYSTICK_TYPE_GUITAR:       return 0x02AE;
		case SDL_JOYSTICK_TYPE_FLIGHT_STICK: return 0x02A1;
		case SDL_JOYSTICK_TYPE_WHEEL:        return 0x02A0;
		case SDL_JOYSTICK_TYPE_DANCE_PAD:    return 0x0291;
		default:                             return 0x028E;
	}
}