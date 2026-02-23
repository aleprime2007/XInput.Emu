#define WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <SDL3/SDL.h>
#include <Windows.h>
#include <tchar.h>
#include <shellapi.h>
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
