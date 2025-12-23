#include <HidHide.h>
#define guid_size 33

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

// ==========> ViGEmClient Functions <========== \\

// Adds a X360 emuated controller to the given vector
void add_emulated_controller(PVIGEM_CLIENT vigem_client, vector<PVIGEM_TARGET> &emulated_controllers){
	PVIGEM_TARGET new_target = vigem_target_x360_alloc();
	vigem_target_add(vigem_client, new_target);
	emulated_controllers.insert(emulated_controllers.end(), new_target);
}

// Removes a X360 emulated controller from a given vector
void remove_emulated_controller(PVIGEM_CLIENT vigem_client, vector<PVIGEM_TARGET> &emulated_controllers, int index){
	if (index < emulated_controllers.size()){
		vigem_target_remove(vigem_client, emulated_controllers[index]);
		vigem_target_free(emulated_controllers[index]);
		emulated_controllers.erase(emulated_controllers.begin() + index);
	}
}


// ==========> SDL Functions <========== \\

// Compares Two given GUID's (Returns a boolean)
bool compare_guids(SDL_JoystickGUID guid1, SDL_JoystickGUID guid2){
	char guid_string1[guid_size];
	char guid_string2[guid_size];
	SDL_JoystickGetGUIDString(guid1, guid_string1, guid_size);
	SDL_JoystickGetGUIDString(guid2, guid_string2, guid_size);
	return !strcmp(guid_string1, guid_string2);
}

// Gets if a given GUID does exists in a given vector (Returns a boolean)
bool controller_guid_exists(vector<SDL_GameController*> game_controllers, SDL_JoystickGUID guid){
	bool exists = false;
	SDL_Joystick* joystick;
	for (int i = (int)game_controllers.size() - 1; i >= 0; i--){
		joystick = SDL_GameControllerGetJoystick(game_controllers[i]);
		if (compare_guids(guid, SDL_JoystickGetGUID(joystick))){
			exists = true;
			break;
		}
	}
	return exists;
}

// Adds the connected game controllers to a given vector
bool add_game_controller(vector<SDL_GameController*> &game_controllers, int index){
	bool result = false;
	SDL_GameControllerType controller_type = SDL_GameControllerTypeForIndex(index);
	if (controller_type != SDL_CONTROLLER_TYPE_XBOX360 && controller_type != SDL_CONTROLLER_TYPE_XBOXONE){
		hidhide_cloak_on();
		hidhide_dev_hide(SDL_GameControllerPathForIndex(index));
		result = !controller_guid_exists(game_controllers, SDL_JoystickGetDeviceGUID(index));
	}
	if (result) game_controllers.insert(game_controllers.end(), SDL_GameControllerOpen(index));
	else SDL_FlushEvent(SDL_CONTROLLERDEVICEADDED);
	return result;
}

void update_xinput_state(vector<SDL_GameController*> game_controllers, int index, XINPUT_STATE &xinput_state, Sint16 &t_LT, Sint16 &t_RT, bool sw_ctrl = false){

	//Triggers
	t_LT = SDL_GameControllerGetAxis(game_controllers[index], SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 128;
	t_RT = SDL_GameControllerGetAxis(game_controllers[index], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 128;

	//Axis
	xinput_state.Gamepad.sThumbLX = SDL_GameControllerGetAxis(game_controllers[index], SDL_CONTROLLER_AXIS_LEFTX);
	xinput_state.Gamepad.sThumbLY = -SDL_GameControllerGetAxis(game_controllers[index], SDL_CONTROLLER_AXIS_LEFTY) - 1;
	xinput_state.Gamepad.sThumbRX = SDL_GameControllerGetAxis(game_controllers[index], SDL_CONTROLLER_AXIS_RIGHTX);
	xinput_state.Gamepad.sThumbRY = -SDL_GameControllerGetAxis(game_controllers[index], SDL_CONTROLLER_AXIS_RIGHTY) - 1;
	xinput_state.Gamepad.bLeftTrigger = reinterpret_cast<const BYTE*>(&t_LT)[0];
	xinput_state.Gamepad.bRightTrigger = reinterpret_cast<const BYTE*>(&t_RT)[0];

	//Buttons
	xinput_state.Gamepad.wButtons =
		SDL_GameControllerGetButton(game_controllers[index], sw_ctrl ? BTN_B : BTN_A) * XBTN_A     +
		SDL_GameControllerGetButton(game_controllers[index], sw_ctrl ? BTN_A : BTN_B) * XBTN_B     +
		SDL_GameControllerGetButton(game_controllers[index], sw_ctrl ? BTN_Y : BTN_X) * XBTN_X     +
		SDL_GameControllerGetButton(game_controllers[index], sw_ctrl ? BTN_X : BTN_Y) * XBTN_Y     +
		SDL_GameControllerGetButton(game_controllers[index], BTN_BK)                  * XBTN_BK    +
		SDL_GameControllerGetButton(game_controllers[index], BTN_GD)                  * XBTN_GD    +
		SDL_GameControllerGetButton(game_controllers[index], BTN_ST)                  * XBTN_ST    +
		SDL_GameControllerGetButton(game_controllers[index], BTN_LS)                  * XBTN_LS    +
		SDL_GameControllerGetButton(game_controllers[index], BTN_RS)                  * XBTN_RS    +
		SDL_GameControllerGetButton(game_controllers[index], BTN_LB)                  * XBTN_LB    +
		SDL_GameControllerGetButton(game_controllers[index], BTN_RB)                  * XBTN_RB    +
		SDL_GameControllerGetButton(game_controllers[index], BTN_DP_UP)               * XBTN_DP_UP +
		SDL_GameControllerGetButton(game_controllers[index], BTN_DP_DN)               * XBTN_DP_DN +
		SDL_GameControllerGetButton(game_controllers[index], BTN_DP_L)                * XBTN_DP_L  +
		SDL_GameControllerGetButton(game_controllers[index], BTN_DP_R)                * XBTN_DP_R  ;
}

// Removes the disconnected game controllers from a given vector
void remove_game_controller(vector<SDL_GameController*> &game_controllers, int index){
	if (index < game_controllers.size()){
		SDL_GameControllerClose(game_controllers[index]);
		game_controllers.erase(game_controllers.begin() + index);
	}
}