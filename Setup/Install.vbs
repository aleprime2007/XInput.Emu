Set ShellObj = CreateObject("WScript.Shell")
ShellObj.Run """C:\Program Files\XInput.Emu\Install.bat""", 0, True
Msg = MsgBox("It's necesary to restart your PC. Do you want to restart your PC now?", 48+4, "")
If Msg = vbYes Then ShellObj.Run "shutdown /r /t 0", 0, True