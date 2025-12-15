#include <HidHide.h>

// ==========> ViGEmClient Functions <========== \\

// Adds a X360 emuated controller to the given vector
void add_emulated_controller(PVIGEM_CLIENT vigem_client, vector<PVIGEM_TARGET> &emulated_controllers){
	PVIGEM_TARGET new_target = vigem_target_x360_alloc();
	vigem_target_add(vigem_client, new_target);
	emulated_controllers.insert(emulated_controllers.end(), new_target);
}

// Removes a X360 emulated controller from a given vector
void remove_emulated_controller(PVIGEM_CLIENT vigem_client, vector<PVIGEM_TARGET> &emulated_controllers, int index){
	if (emulated_controllers.size() <= index + 1) {
		vigem_target_remove(vigem_client, emulated_controllers[index]);
		vigem_target_free(emulated_controllers[index]);
		emulated_controllers.erase(emulated_controllers.begin() + index);
	}
}


// ==========> SDL Functions <========== \\

// Compares Two given GUID's (Returns a boolean)
bool compare_guids(SDL_GUID guid, SDL_GUID prev_guid){
	char guid_string[33];
	char prev_guid_string[33];
	SDL_GUIDToString(guid, guid_string, sizeof(guid_string));
	SDL_GUIDToString(prev_guid, prev_guid_string, sizeof(prev_guid_string));
	return compare_strings(guid_string, prev_guid_string);
}

// Gets if a given GUID does exists in a given vector (Returns a boolean)
bool controller_guid_exists(vector<SDL_GameController*> game_controllers, SDL_GUID guid){
	bool exists = false;
	SDL_Joystick* joystick;
	for (int i = 0; i < game_controllers.size(); i++){
		joystick = SDL_GameControllerGetJoystick(game_controllers[i]);
		if (compare_guids(guid, SDL_JoystickGetGUID(joystick))) exists = true;
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

// Removes the disconnected game controllers from a given vector
void remove_game_controller(vector<SDL_GameController*> &game_controllers, int index){
	if (game_controllers.size() <= index + 1) {
		SDL_GameControllerClose(game_controllers[index]);
		game_controllers.erase(game_controllers.begin() + index);
	}
}

// Returns the inverted value of the given axis
Sint16 invert_axis(Sint16 axis){
	return -axis - 1;
}

// Prints the information of the game controllers of a given vector
void print_controllers_info(PVIGEM_CLIENT vigem_client, vector<SDL_GameController*> game_controllers, vector<PVIGEM_TARGET> emulated_controllers){
	wstring hidhide_path;
	if (get_hidhide_path(hidhide_path)) cout << "HidHide Path : " << convert_wstring_to_string(hidhide_path) << endl;
	cout << "Game Controllers Connected : " << game_controllers.size() << endl;
	cout << "Emulated Controllers Count : " << emulated_controllers.size() << endl;

	DWORD xinput_index;
	for (int i = 0; i < game_controllers.size(); i++){
		vigem_target_x360_get_user_index(vigem_client, emulated_controllers[i], &xinput_index);
		cout << "Index " << i << " : XInput Index " << xinput_index << " : " << SDL_GameControllerName(game_controllers[i]) << endl;
	}
	cout << endl;

}