#define WIN32_LEAN_AND_MEAN
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

#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <SDL3/SDL.h>
#include <Windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <psapi.h>
#include <Xinput.h>
#include <ViGEm/Client.h>

using namespace std;

wstring_convert<codecvt_utf8<wchar_t>> converter;
vector<wchar_t> buffer;
HKEY hkey;
DWORD dw_type;
DWORD dw_size = 0;

// ==========> Common Functions <========== \\

// Converts a Wide String to a String
string convert_wstring_to_string(const wstring wstr){
	try{
		return converter.to_bytes(wstr);
	}
	catch (...){
		return "";
	}
}

// Converts a String to a Wide String
wstring convert_string_to_wstring(const string str){
	try{
		return converter.from_bytes(str);
	}
	catch (...){
		return L"";
	}
}


// ==========> Windows Functions <========== \\

// Returns True if the given Register Key Exists
bool register_key_exists(HKEY hkey_root, LPCWSTR sub_key){
	if (RegOpenKeyExW(hkey_root, sub_key, 0, KEY_READ, &hkey) == ERROR_SUCCESS){
		RegCloseKey(hkey);
		return true;
	}
	return false;
}

// Returns a string value of a given register key
bool register_key_read_wstring(HKEY hkey_root, LPCWSTR sub_key, LPCWSTR value_name, wstring* result){

	if (RegOpenKeyExW(hkey_root, sub_key, 0, KEY_READ, &hkey) != ERROR_SUCCESS) return false;

	if (RegQueryValueExW(hkey, value_name, NULL, &dw_type, NULL, &dw_size) != ERROR_SUCCESS || dw_type != REG_SZ){
		RegCloseKey(hkey);
		return false;
	}

	buffer = vector<wchar_t>(dw_size);
	if (RegQueryValueExW(hkey, value_name, NULL, NULL, (LPBYTE)buffer.data(), &dw_size) != ERROR_SUCCESS){
		RegCloseKey(hkey);
		return false;
	}

	*result = buffer.data();
	RegCloseKey(hkey);
	return true;
}
