#include <HidHide.h>
#define guid_size 33

// SDL Buttons
#define BTN_A      SDL_GAMEPAD_BUTTON_SOUTH
#define BTN_B      SDL_GAMEPAD_BUTTON_EAST
#define BTN_X      SDL_GAMEPAD_BUTTON_WEST
#define BTN_Y      SDL_GAMEPAD_BUTTON_NORTH
#define BTN_BK     SDL_GAMEPAD_BUTTON_BACK
#define BTN_GD     SDL_GAMEPAD_BUTTON_GUIDE
#define BTN_ST     SDL_GAMEPAD_BUTTON_START
#define BTN_LS     SDL_GAMEPAD_BUTTON_LEFT_STICK
#define BTN_RS     SDL_GAMEPAD_BUTTON_RIGHT_STICK
#define BTN_LB     SDL_GAMEPAD_BUTTON_LEFT_SHOULDER
#define BTN_RB     SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER
#define BTN_DP_UP  SDL_GAMEPAD_BUTTON_DPAD_UP
#define BTN_DP_DN  SDL_GAMEPAD_BUTTON_DPAD_DOWN
#define BTN_DP_L   SDL_GAMEPAD_BUTTON_DPAD_LEFT
#define BTN_DP_R   SDL_GAMEPAD_BUTTON_DPAD_RIGHT
#define BTN_M1     SDL_GAMEPAD_BUTTON_MISC1
#define BTN_RP1    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1
#define BTN_LP1    SDL_GAMEPAD_BUTTON_LEFT_PADDLE1
#define BTN_RP2    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2
#define BTN_LP2    SDL_GAMEPAD_BUTTON_LEFT_PADDLE2
#define BTN_TCH    SDL_GAMEPAD_BUTTON_TOUCHPAD
#define BTN_M2     SDL_GAMEPAD_BUTTON_MISC2
#define BTN_M3     SDL_GAMEPAD_BUTTON_MISC3
#define BTN_M4     SDL_GAMEPAD_BUTTON_MISC4
#define BTN_M5     SDL_GAMEPAD_BUTTON_MISC5
#define BTN_M6     SDL_GAMEPAD_BUTTON_MISC6

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

// Returns the Product ID equivalent to a SDL_JoystickType
USHORT get_xinput_pid(SDL_JoystickType joystick_type){
	switch (joystick_type){
		case SDL_JOYSTICK_TYPE_GUITAR:       return 0x02AE;
		case SDL_JOYSTICK_TYPE_FLIGHT_STICK: return 0x02A1;
		case SDL_JOYSTICK_TYPE_WHEEL:        return 0x02A0;
		case SDL_JOYSTICK_TYPE_DANCE_PAD:    return 0x0291;
		default:                             return 0x028E;
	}
}

struct GAMEPAD{
	SDL_JoystickID joystick_id;
	PVIGEM_TARGET vigem_target;
	DWORD xinput_index = -1;
	const char* device_path;
	SDL_hid_device_info device_info;
	GAMEPAD(SDL_JoystickID id){
		joystick_id = id;
		device_path = SDL_GetGamepadPathForID(joystick_id);
		device_info = *SDL_hid_get_device_info(SDL_hid_open_path(device_path));
	}
public:
	void open_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error){
		SDL_OpenGamepad(joystick_id);
		vigem_target = vigem_target_x360_alloc();
		vigem_target_set_pid(vigem_target, get_xinput_pid(SDL_GetJoystickTypeForID(joystick_id)));
		*vigem_last_error = vigem_target_add(vigem_client, vigem_target);
		*vigem_last_error = vigem_target_x360_get_user_index(vigem_client, vigem_target, &xinput_index);
	}
	void close_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error){
		SDL_CloseGamepad(SDL_GetGamepadFromID(joystick_id));
		*vigem_last_error = vigem_target_remove(vigem_client, vigem_target);
		vigem_target_free(vigem_target);
		xinput_index = -1;
	}
};

// Adds the connected game controllers to a given vector
bool add_gamepad(vector<GAMEPAD>* gamepads, PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, SDL_JoystickID id, const wchar_t* hidhide_path){
	if (SDL_IsGamepad(id)){
		GAMEPAD gamepad = GAMEPAD(id);
		if (lstrcmpW(hidhide_path, L"")) hidhide_dev_hide(hidhide_path, gamepad.device_path);
		if (gamepad.device_info.usage == 4 || gamepad.device_info.usage == 5){
			gamepad.open_gamepad(vigem_client, vigem_last_error);
			(*gamepads).insert((*gamepads).end(), gamepad);
			return true;
		}
	}
	SDL_FlushEvent(SDL_EVENT_GAMEPAD_ADDED);
	return false;
}

// Removes the disconnected game controllers from a given vector
void remove_gamepad(vector<GAMEPAD>* gamepads, PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, UINT index){
	if (index < (*gamepads).size()){
		(*gamepads)[index].close_gamepad(vigem_client, vigem_last_error);
		(*gamepads).erase((*gamepads).begin() + index);
	}
}

// Updates the state of a XINPUT_GAMEPAD with the inputs of a SDL_Gamepad*
void update_xinput_gamepad(SDL_Gamepad* gamepad, XINPUT_GAMEPAD* xinput_gamepad, Sint16* t_LT, Sint16* t_RT){

	//Triggers
	*t_LT = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) / 128;
	*t_RT = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / 128;

	//Axis
	(*xinput_gamepad).sThumbLX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
	(*xinput_gamepad).sThumbLY = -SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) - 1;
	(*xinput_gamepad).sThumbRX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
	(*xinput_gamepad).sThumbRY = -SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) - 1;
	(*xinput_gamepad).bLeftTrigger = *reinterpret_cast<const BYTE*>(t_LT);
	(*xinput_gamepad).bRightTrigger = *reinterpret_cast<const BYTE*>(t_RT);

	//Buttons
	SDL_GamepadType gamepad_type = SDL_GetGamepadType(gamepad);
	bool tch_as_bk = gamepad_type == SDL_GAMEPAD_TYPE_PS4 || gamepad_type == SDL_GAMEPAD_TYPE_PS5;
	(*xinput_gamepad).wButtons =
		SDL_GetGamepadButton(gamepad, BTN_A)                          * XBTN_A     +
		SDL_GetGamepadButton(gamepad, BTN_B)                          * XBTN_B     +
		SDL_GetGamepadButton(gamepad, BTN_X)                          * XBTN_X     +
		SDL_GetGamepadButton(gamepad, BTN_Y)                          * XBTN_Y     +
		SDL_GetGamepadButton(gamepad, tch_as_bk ? BTN_TCH : BTN_BK)   * XBTN_BK    +
		SDL_GetGamepadButton(gamepad, BTN_GD)                         * XBTN_GD    +
		SDL_GetGamepadButton(gamepad, BTN_ST)                         * XBTN_ST    +
		SDL_GetGamepadButton(gamepad, BTN_LS)                         * XBTN_LS    +
		SDL_GetGamepadButton(gamepad, BTN_RS)                         * XBTN_RS    +
		SDL_GetGamepadButton(gamepad, BTN_LB)                         * XBTN_LB    +
		SDL_GetGamepadButton(gamepad, BTN_RB)                         * XBTN_RB    +
		SDL_GetGamepadButton(gamepad, BTN_DP_UP)                      * XBTN_DP_UP +
		SDL_GetGamepadButton(gamepad, BTN_DP_DN)                      * XBTN_DP_DN +
		SDL_GetGamepadButton(gamepad, BTN_DP_L)                       * XBTN_DP_L  +
		SDL_GetGamepadButton(gamepad, BTN_DP_R)                       * XBTN_DP_R  ;
}