#if _M_X64
#define hidhide_exe L"x64\\HidHideCLI.exe"
#elif _M_IX86
#define hidhide_exe L"HidHideCLI.exe"
#endif

#define install_key L"SOFTWARE\\Nefarius Software Solutions e.U.\\HidHide"
#define service_key L"SYSTEM\\CurrentControlSet\\Services\\HidHide"
#define watchdog_key L"SYSTEM\\CurrentControlSet\\Services\\HidHideWatchdog.exe"

wstring hidhide_command;

// ==========> HidHide Functions <========== \\

// Gets the currently installed HidHide Instance Path
bool get_hidhide_path(wstring* hidhide_path){
	if (register_key_exists(HKEY_LOCAL_MACHINE, service_key) && register_key_exists(HKEY_LOCAL_MACHINE, watchdog_key))
	return register_key_read_wstring(HKEY_LOCAL_MACHINE, install_key, L"Path", hidhide_path);
	return false;
}

string convert_to_device_instance_path(string device_symbolic_path){
	string result = device_symbolic_path;
	if (device_symbolic_path.size() > 43){
		if (device_symbolic_path[result.size() - 1] == '}') result.erase(result.begin() + result.size() - 39, result.end());
		if (device_symbolic_path[3] == '\\') result.erase(result.begin(), result.begin() + 4);
		for (int i = 0; i < result.size(); i++) if (result[i] == '#') result.replace(result.begin() + i, result.begin() + i + 1, "\\");
	}
	return result;
}

// Remove absent registered applications
void hidhide_app_clean(const wchar_t* hidhide_path){
	hidhide_command = L"--app-clean";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Grants ability to see hidden devices
void hidhide_app_reg(const wchar_t* hidhide_path, const wchar_t* app_path){
	hidhide_command = L"--app-reg \"" + (wstring)app_path + L"\"";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Revokes ability to see hidden devices
void hidhide_app_unreg(const wchar_t* hidhide_path, const wchar_t* app_path) {
	hidhide_command = L"--app-unreg \"" + (wstring)app_path + L"\"";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Deactivates hiding of HID devices
void hidhide_cloak_off(const wchar_t* hidhide_path){
	hidhide_command = L"--cloak-off";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Activates hiding of HID devices
void hidhide_cloak_on(const wchar_t* hidhide_path){
	hidhide_command = L"--cloak-on";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Toggles between active and inactive
void hidhide_cloak_toggle(const wchar_t* hidhide_path){
	hidhide_command = L"--cloak-toggle";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Hide the device specified
void hidhide_dev_hide(const wchar_t* hidhide_path, const char* device_path){
	hidhide_command = L"--dev-hide \"" + convert_string_to_wstring(convert_to_device_instance_path(device_path)) + L"\"";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Unhide the device specified
void hidhide_dev_unhide(const wchar_t* hidhide_path, const char* device_path){
	hidhide_command = L"--dev-unhide \"" + convert_string_to_wstring(convert_to_device_instance_path(device_path)) + L"\"";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Turn off inverse application list
void hidhide_inv_off(const wchar_t* hidhide_path){
	hidhide_command = L"--inv-off";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}

// Turn on inverse application list
void hidhide_inv_on(const wchar_t* hidhide_path){
	hidhide_command = L"--inv-on";
	ShellExecuteW(NULL, L"open", hidhide_exe, hidhide_command.c_str(), hidhide_path, SW_HIDE);
}