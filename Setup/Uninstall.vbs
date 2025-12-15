Set ShellObj = CreateObject("WScript.Shell")
ShellObj.Run "taskkill /f /im ""XInput.Emu.exe""", 0, True
ShellObj.Run "schtasks /end /tn ""\XInput.Emu"" /f", 0, True
ShellObj.Run "schtasks /delete /tn ""\XInput.Emu"" /f", 0, True