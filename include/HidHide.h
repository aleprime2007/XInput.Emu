
// ==========> HidHide Functions <========== \\

// Gets the currently installed HidHide Instance Path
bool get_hidhide_path(wstring& hidhide_path){
	wstring result;
	const wchar_t* uninstall = L"SOFTWARE\\Nefarius Software Solutions e.U.\\HidHide";
	bool service_exists = register_key_exists(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\HidHide");
	bool watchdog_exists = register_key_exists(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\HidHideWatchdog.exe");
	bool uninstall_exists = register_key_read_wstring(HKEY_LOCAL_MACHINE, uninstall, L"Path", result);
	if (service_exists && watchdog_exists && uninstall_exists){
		hidhide_path = result;
		return true;
	}
	else return false;
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
void hidhide_app_clean(){
	wstring path;
	wstring command = L"--app-clean";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Grants ability to see hidden devices
void hidhide_app_reg(const wchar_t* app_path){
	wstring path;
	wstring command = L"--app-reg \"" + (wstring)app_path + L"\"";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Revokes ability to see hidden devices
void hidhide_app_unreg(const wchar_t* app_path){
	wstring path;
	wstring command = L"--app-unreg \"" + (wstring)app_path + L"\"";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Deactivates hiding of HID devices
void hidhide_cloak_off(){
	wstring path;
	wstring command = L"--cloak-off";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Activates hiding of HID devices
void hidhide_cloak_on(){
	wstring path;
	wstring command = L"--cloak-on";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Toggles between active and inactive
void hidhide_cloak_toggle(){
	wstring path;
	wstring command = L"--cloak-toggle";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Hide the device specified
void hidhide_dev_hide(const char* device_path){
	wstring path;
	string device_instance_path = convert_to_device_instance_path(device_path);
	wstring command = L"--dev-hide \"" + convert_string_to_wstring(device_instance_path) + L"\"";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Unhide the device specified
void hidhide_dev_unhide(const char* device_path){
	wstring path;
	string device_instance_path = convert_to_device_instance_path(device_path);
	wstring command = L"--dev-unhide \"" + convert_string_to_wstring(device_instance_path) + L"\"";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Turn off inverse application list
void hidhide_inv_off(){
	wstring path;
	wstring command = L"--inv-off";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}

// Turn on inverse application list
void hidhide_inv_on(){
	wstring path;
	wstring command = L"--inv-on";
	if (get_hidhide_path(path)) ShellExecute(NULL, L"open", L"x64\\HidHideCLI.exe", command.c_str(), path.c_str(), SW_HIDE);
}