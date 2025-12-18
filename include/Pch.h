#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <SDL.h>
#include <Windows.h>
#include <shellapi.h>
#include <Xinput.h>
#include <ViGEm/Client.h>

using namespace std;


// ==========> Common Functions <========== \\

// Converts a Wide String to a String
string convert_wstring_to_string(const wstring &wstr){
	try{
		wstring_convert<codecvt_utf8<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}
	catch (...){
		return "";
	}
}

// Converts a String to a Wide String
wstring convert_string_to_wstring(const string &str){
	try{
		wstring_convert<codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(str);
	}
	catch (...){
		return L"";
	}
}


// ==========> Windows Functions <========== \\

// Cecks if running as SYSTEM (returns a boolean)
bool is_running_as_system(){
	bool is_system = false;
	PSID system_sid = nullptr;
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&nt_authority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &system_sid)){
		BOOL result = FALSE;
		if (CheckTokenMembership(NULL, system_sid, &result)) is_system = (result == TRUE);
	}
	if (system_sid) FreeSid(system_sid);
	return is_system;
}

// Returns True if the given Register Key Exists
bool register_key_exists(HKEY hkey, LPCWSTR path){
	bool result = false;
	HKEY output_hkey;
	LONG l_res = RegOpenKeyEx(hkey, path, 0, KEY_READ, &output_hkey);
	result = l_res == ERROR_SUCCESS;
	if (result) RegCloseKey(output_hkey);
	return result;
}

// Returns a string value of a given register key
bool register_key_read_wstring(HKEY hkey_root, const wstring &sub_key, const wstring &value_name, wstring &result){
	HKEY hkey;
	DWORD dw_type;
	DWORD dw_size = 0;

	if (RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_READ, &hkey) != ERROR_SUCCESS) return false;

	if (RegQueryValueEx(hkey, value_name.c_str(), NULL, &dw_type, NULL, &dw_size) != ERROR_SUCCESS || dw_type != REG_SZ){
		RegCloseKey(hkey);
		return false;
	}

	vector<wchar_t> buffer(dw_size / sizeof(wchar_t));
	if (RegQueryValueEx(hkey, value_name.c_str(), NULL, NULL, (LPBYTE)buffer.data(), &dw_size) != ERROR_SUCCESS){
		RegCloseKey(hkey);
		return false;
	}

	RegCloseKey(hkey);
	result = buffer.data();
	return true;
}
