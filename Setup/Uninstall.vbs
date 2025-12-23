Set ShellObj = CreateObject("WScript.Shell")
ShellObj.Run "sc stop XInput.Emu", 0, True
ShellObj.Run "sc delete XInput.Emu", 0, True