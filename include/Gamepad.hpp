#include <HidHide.hpp>

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

// DS4 Buttons
#define DS4BTN_X       DS4_BUTTON_CROSS
#define DS4BTN_C       DS4_BUTTON_CIRCLE
#define DS4BTN_S       DS4_BUTTON_SQUARE
#define DS4BTN_T       DS4_BUTTON_TRIANGLE
#define DS4BTN_SH      DS4_BUTTON_SHARE
#define DS4BTN_TCH     DS4_SPECIAL_BUTTON_TOUCHPAD
#define DS4BTN_PS      DS4_SPECIAL_BUTTON_PS
#define DS4BTN_OP      DS4_BUTTON_OPTIONS
#define DS4BTN_L3      DS4_BUTTON_THUMB_LEFT
#define DS4BTN_R3      DS4_BUTTON_THUMB_RIGHT
#define DS4BTN_L1      DS4_BUTTON_SHOULDER_LEFT
#define DS4BTN_R1      DS4_BUTTON_SHOULDER_RIGHT
#define DS4BTN_DP_UP   DS4_BUTTON_DPAD_NORTH
#define DS4BTN_DP_DN   DS4_BUTTON_DPAD_SOUTH
#define DS4BTN_DP_L    DS4_BUTTON_DPAD_WEST
#define DS4BTN_DP_R    DS4_BUTTON_DPAD_EAST
#define DS4BTN_DP_UP_L DS4_BUTTON_DPAD_NORTHWEST
#define DS4BTN_DP_UP_R DS4_BUTTON_DPAD_NORTHEAST
#define DS4BTN_DP_DN_L DS4_BUTTON_DPAD_SOUTHWEST
#define DS4BTN_DP_DN_R DS4_BUTTON_DPAD_SOUTHEAST
#define DS4BTN_DP_NONE DS4_BUTTON_DPAD_NONE

// Returns the Product ID equivalent to a SDL_JoystickType
USHORT get_x360_pid(SDL_JoystickType joystick_type){
	switch (joystick_type){
		case SDL_JOYSTICK_TYPE_GUITAR:       return 0x02AE;
		case SDL_JOYSTICK_TYPE_FLIGHT_STICK: return 0x02A1;
		case SDL_JOYSTICK_TYPE_WHEEL:        return 0x02A0;
		case SDL_JOYSTICK_TYPE_DANCE_PAD:    return 0x0291;
		default:                             return 0x028E;
	}
}

// Callback for Force Feedback
VOID CALLBACK x360_force_feedback_callback(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
){
	SDL_RumbleGamepad((SDL_Gamepad*)UserData, LargeMotor * 257, SmallMotor * 257, -1);
}

// Callback for Force Feedback
VOID CALLBACK ds4_force_feedback_callback(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	DS4_LIGHTBAR_COLOR LightbarColor,
	LPVOID UserData
){
	SDL_RumbleGamepad((SDL_Gamepad*)UserData, LargeMotor * 257, SmallMotor * 257, -1);
	SDL_SetGamepadLED((SDL_Gamepad*)UserData, LightbarColor.Red, LightbarColor.Green, LightbarColor.Blue);
}

struct X360_GAMEPAD{
	SDL_JoystickID joystick_id;
	SDL_Gamepad* gamepad;
	SDL_GamepadType gamepad_type;
	PVIGEM_TARGET vigem_target;
	DWORD xinput_index = -1;
	XINPUT_STATE xinput_state;
	const char* device_path;
	SDL_hid_device_info device_info;
	X360_GAMEPAD(SDL_JoystickID id){
		joystick_id = id;
		device_path = SDL_GetGamepadPathForID(joystick_id);
		device_info = *SDL_hid_get_device_info(SDL_hid_open_path(device_path));
	}
private:
	Sint16 t_LT;
	Sint16 t_RT;
	bool tch_as_bk;
public:
	void open_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, const wchar_t* hidhide_path){
		if (lstrcmpW(hidhide_path, L"")) hidhide_dev_hide(hidhide_path, device_path);
		gamepad = SDL_OpenGamepad(joystick_id);
		gamepad_type = SDL_GetGamepadType(gamepad);
		vigem_target = vigem_target_x360_alloc();
		vigem_target_set_pid(vigem_target, get_x360_pid(SDL_GetJoystickTypeForID(joystick_id)));
		*vigem_last_error = vigem_target_add(vigem_client, vigem_target);
		*vigem_last_error = vigem_target_x360_get_user_index(vigem_client, vigem_target, &xinput_index);
	}
	void close_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, const wchar_t* hidhide_path){
		SDL_CloseGamepad(SDL_GetGamepadFromID(joystick_id));
		*vigem_last_error = vigem_target_remove(vigem_client, vigem_target);
		vigem_target_free(vigem_target);
		xinput_index = -1;
		if (lstrcmpW(hidhide_path, L"")) hidhide_dev_unhide(hidhide_path, device_path);
	}
	void update_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error){
		XInputGetState(xinput_index, &xinput_state);

		//Axis
		t_LT = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) / 128;
		t_RT = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / 128;
		xinput_state.Gamepad.sThumbLX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
		xinput_state.Gamepad.sThumbLY = -SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) - 1;
		xinput_state.Gamepad.sThumbRX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
		xinput_state.Gamepad.sThumbRY = -SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) - 1;
		xinput_state.Gamepad.bLeftTrigger = *reinterpret_cast<const BYTE*>(&t_LT);
		xinput_state.Gamepad.bRightTrigger = *reinterpret_cast<const BYTE*>(&t_RT);

		//Buttons
		tch_as_bk = gamepad_type == SDL_GAMEPAD_TYPE_PS4 || gamepad_type == SDL_GAMEPAD_TYPE_PS5;
		xinput_state.Gamepad.wButtons =
			SDL_GetGamepadButton(gamepad, BTN_A)                        * XBTN_A     +
			SDL_GetGamepadButton(gamepad, BTN_B)                        * XBTN_B     +
			SDL_GetGamepadButton(gamepad, BTN_X)                        * XBTN_X     +
			SDL_GetGamepadButton(gamepad, BTN_Y)                        * XBTN_Y     +
			SDL_GetGamepadButton(gamepad, tch_as_bk ? BTN_TCH : BTN_BK) * XBTN_BK    +
			SDL_GetGamepadButton(gamepad, BTN_GD)                       * XBTN_GD    +
			SDL_GetGamepadButton(gamepad, BTN_ST)                       * XBTN_ST    +
			SDL_GetGamepadButton(gamepad, BTN_LS)                       * XBTN_LS    +
			SDL_GetGamepadButton(gamepad, BTN_RS)                       * XBTN_RS    +
			SDL_GetGamepadButton(gamepad, BTN_LB)                       * XBTN_LB    +
			SDL_GetGamepadButton(gamepad, BTN_RB)                       * XBTN_RB    +
			SDL_GetGamepadButton(gamepad, BTN_DP_UP)                    * XBTN_DP_UP +
			SDL_GetGamepadButton(gamepad, BTN_DP_DN)                    * XBTN_DP_DN +
			SDL_GetGamepadButton(gamepad, BTN_DP_L)                     * XBTN_DP_L  +
			SDL_GetGamepadButton(gamepad, BTN_DP_R)                     * XBTN_DP_R  ;

		*vigem_last_error = vigem_target_x360_update(vigem_client, vigem_target, *reinterpret_cast<XUSB_REPORT*>(&xinput_state.Gamepad));
		*vigem_last_error = vigem_target_x360_register_notification(vigem_client, vigem_target, &x360_force_feedback_callback, gamepad);
	}
};

struct DS4_GAMEPAD{
	SDL_JoystickID joystick_id;
	SDL_Gamepad* gamepad;
	SDL_GamepadType gamepad_type;
	PVIGEM_TARGET vigem_target;
	DS4_REPORT_EX ds4_report;
	const char* device_path;
	SDL_hid_device_info device_info;
	DS4_GAMEPAD(SDL_JoystickID id){
		joystick_id = id;
		device_path = SDL_GetGamepadPathForID(joystick_id);
		device_info = *SDL_hid_get_device_info(SDL_hid_open_path(device_path));
	}
private:
	Sint16 a_LX;
	Sint16 a_LY;
	Sint16 a_RX;
	Sint16 a_RY;
	Sint16 t_LT;
	Sint16 t_RT;
	bool dpad_none;
	bool dpad_up;
	bool dpad_dn;
	bool dpad_l;
	bool dpad_r;
	bool dpad_up_l;
	bool dpad_up_r;
	bool dpad_dn_l;
	bool dpad_dn_r;
	float gyro[3];
	float accel[3];
public:
	void open_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, const wchar_t* hidhide_path){
		if (lstrcmpW(hidhide_path, L"")) hidhide_dev_hide(hidhide_path, device_path);
		gamepad = SDL_OpenGamepad(joystick_id);
		gamepad_type = SDL_GetGamepadType(gamepad);
		vigem_target = vigem_target_ds4_alloc();
		*vigem_last_error = vigem_target_add(vigem_client, vigem_target);
	}
	void close_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, const wchar_t* hidhide_path){
		SDL_CloseGamepad(SDL_GetGamepadFromID(joystick_id));
		*vigem_last_error = vigem_target_remove(vigem_client, vigem_target);
		vigem_target_free(vigem_target);
		if (lstrcmpW(hidhide_path, L"")) hidhide_dev_unhide(hidhide_path, device_path);
	}
	void update_gamepad(PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error){

		//Axis
		a_LX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / 256 + 128;
		a_LY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / 256 + 128;
		a_RX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX) / 256 + 128;
		a_RY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) / 256 + 128;
		t_LT = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) / 128;
		t_RT = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / 128;
		ds4_report.Report.bThumbLX = *reinterpret_cast<const BYTE*>(&a_LX);
		ds4_report.Report.bThumbLY = *reinterpret_cast<const BYTE*>(&a_LY);
		ds4_report.Report.bThumbRX = *reinterpret_cast<const BYTE*>(&a_RX);
		ds4_report.Report.bThumbRY = *reinterpret_cast<const BYTE*>(&a_RY);
		ds4_report.Report.bTriggerL = *reinterpret_cast<const BYTE*>(&t_LT);
		ds4_report.Report.bTriggerR = *reinterpret_cast<const BYTE*>(&t_RT);

		//Buttons
		dpad_none = 
			!SDL_GetGamepadButton(gamepad, BTN_DP_UP) &&
			!SDL_GetGamepadButton(gamepad, BTN_DP_DN) &&
			!SDL_GetGamepadButton(gamepad, BTN_DP_L)  &&
			!SDL_GetGamepadButton(gamepad, BTN_DP_R)  ;
		dpad_up_l =
			SDL_GetGamepadButton(gamepad, BTN_DP_UP)  &&
			SDL_GetGamepadButton(gamepad, BTN_DP_L)   ;
		dpad_up_r =
			SDL_GetGamepadButton(gamepad, BTN_DP_UP)  &&
			SDL_GetGamepadButton(gamepad, BTN_DP_R)   ;
		dpad_dn_l =
			SDL_GetGamepadButton(gamepad, BTN_DP_DN)  &&
			SDL_GetGamepadButton(gamepad, BTN_DP_L)   ;
		dpad_dn_r =
			SDL_GetGamepadButton(gamepad, BTN_DP_DN)  &&
			SDL_GetGamepadButton(gamepad, BTN_DP_R)   ;
		dpad_up = !dpad_up_l && !dpad_up_r && !dpad_dn_l && !dpad_dn_r ? SDL_GetGamepadButton(gamepad, BTN_DP_UP) : 0;
		dpad_dn = !dpad_up_l && !dpad_up_r && !dpad_dn_l && !dpad_dn_r ? SDL_GetGamepadButton(gamepad, BTN_DP_DN) : 0;
		dpad_l  = !dpad_up_l && !dpad_up_r && !dpad_dn_l && !dpad_dn_r ? SDL_GetGamepadButton(gamepad, BTN_DP_L)  : 0;
		dpad_r  = !dpad_up_l && !dpad_up_r && !dpad_dn_l && !dpad_dn_r ? SDL_GetGamepadButton(gamepad, BTN_DP_R)  : 0;
		ds4_report.Report.wButtons =
			SDL_GetGamepadButton(gamepad, BTN_A)      * DS4BTN_X       +
			SDL_GetGamepadButton(gamepad, BTN_B)      * DS4BTN_C       +
			SDL_GetGamepadButton(gamepad, BTN_X)      * DS4BTN_S       +
			SDL_GetGamepadButton(gamepad, BTN_Y)      * DS4BTN_T       +
			SDL_GetGamepadButton(gamepad, BTN_ST)     * DS4BTN_OP      +
			SDL_GetGamepadButton(gamepad, BTN_LS)     * DS4BTN_L3      +
			SDL_GetGamepadButton(gamepad, BTN_RS)     * DS4BTN_R3      +
			SDL_GetGamepadButton(gamepad, BTN_LB)     * DS4BTN_L1      +
			SDL_GetGamepadButton(gamepad, BTN_RB)     * DS4BTN_R1      +
			dpad_up                                   * DS4BTN_DP_UP   +
			dpad_dn                                   * DS4BTN_DP_DN   +
			dpad_l                                    * DS4BTN_DP_L    +
			dpad_r                                    * DS4BTN_DP_R    +
			dpad_up_l                                 * DS4BTN_DP_UP_L +
			dpad_up_r                                 * DS4BTN_DP_UP_R +
			dpad_dn_l                                 * DS4BTN_DP_DN_L +
			dpad_dn_r                                 * DS4BTN_DP_DN_R +
			dpad_none                                 * DS4BTN_DP_NONE +
			SDL_GetGamepadButton(gamepad, BTN_M1)     * DS4BTN_SH      ;
		ds4_report.Report.bSpecial =
			SDL_GetGamepadButton(gamepad, BTN_BK)     * DS4BTN_TCH     +
			SDL_GetGamepadButton(gamepad, BTN_GD)     * DS4BTN_PS      ;

		//Gyro
		SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_GYRO, gyro, 3);
		ds4_report.Report.wGyroX = gyro[0];
		ds4_report.Report.wGyroY = gyro[1];
		ds4_report.Report.wGyroZ = gyro[2];

		//Accel
		SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_ACCEL, accel, 3);
		ds4_report.Report.wAccelX = accel[0];
		ds4_report.Report.wAccelY = accel[1];
		ds4_report.Report.wAccelZ = accel[2];

		//Touchpad
		ds4_report.Report.sCurrentTouch.bTouchData1[0] = 0;
		ds4_report.Report.sCurrentTouch.bTouchData1[1] = 0;
		ds4_report.Report.sCurrentTouch.bTouchData1[2] = 0;
		ds4_report.Report.sCurrentTouch.bTouchData2[0] = 0;
		ds4_report.Report.sCurrentTouch.bTouchData2[1] = 0;
		ds4_report.Report.sCurrentTouch.bTouchData2[2] = 0;

		*vigem_last_error = vigem_target_ds4_update_ex(vigem_client, vigem_target, ds4_report);
		*vigem_last_error = vigem_target_ds4_register_notification(vigem_client, vigem_target, &ds4_force_feedback_callback, gamepad);
	}
};

// Adds the connected gamepad to a given vector
bool add_x360_gamepad(vector<X360_GAMEPAD>* x360_gamepads, PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, SDL_JoystickID id, const wchar_t* hidhide_path){
	if (SDL_IsGamepad(id)){
		X360_GAMEPAD gamepad = X360_GAMEPAD(id);
		if (gamepad.device_info.usage == 4 || gamepad.device_info.usage == 5){
			gamepad.open_gamepad(vigem_client, vigem_last_error, hidhide_path);
			(*x360_gamepads).insert((*x360_gamepads).end(), gamepad);
			return true;
		}
		else if (lstrcmpW(hidhide_path, L"")) hidhide_dev_hide(hidhide_path, gamepad.device_path);
	}
	SDL_FlushEvent(SDL_EVENT_GAMEPAD_ADDED);
	return false;
}

// Adds the connected game controllers to a given vector
bool add_ds4_gamepad(vector<DS4_GAMEPAD>* ds4_gamepads, PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, SDL_JoystickID id, const wchar_t* hidhide_path){
	if (SDL_IsGamepad(id)){
		DS4_GAMEPAD gamepad = DS4_GAMEPAD(id);
		if (gamepad.device_info.usage == 4 || gamepad.device_info.usage == 5){
			gamepad.open_gamepad(vigem_client, vigem_last_error, hidhide_path);
			(*ds4_gamepads).insert((*ds4_gamepads).end(), gamepad);
			return true;
		}
		else if (lstrcmpW(hidhide_path, L"")) hidhide_dev_hide(hidhide_path, gamepad.device_path);
	}
	SDL_FlushEvent(SDL_EVENT_GAMEPAD_ADDED);
	return false;
}

// Removes the disconnected gamepad from a given vector
void remove_x360_gamepad(vector<X360_GAMEPAD>* x360_gamepads, PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, size_t index, const wchar_t* hidhide_path){
	if (index < (*x360_gamepads).size()){
		(*x360_gamepads)[index].close_gamepad(vigem_client, vigem_last_error, hidhide_path);
		(*x360_gamepads).erase((*x360_gamepads).begin() + index);
	}
}

// Removes the disconnected game controllers from a given vector
void remove_ds4_gamepad(vector<DS4_GAMEPAD>* ds4_gamepads, PVIGEM_CLIENT vigem_client, VIGEM_ERROR* vigem_last_error, size_t index, const wchar_t* hidhide_path){
	if (index < (*ds4_gamepads).size()){
		(*ds4_gamepads)[index].close_gamepad(vigem_client, vigem_last_error, hidhide_path);
		(*ds4_gamepads).erase((*ds4_gamepads).begin() + index);
	}
}