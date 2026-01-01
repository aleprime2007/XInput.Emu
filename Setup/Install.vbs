InstallPath = Session.Property("CustomActionData")
Set ShellObj = CreateObject("WScript.Shell")
ShellObj.Run "sc create XInput.Emu binPath= """ & InstallPath & "XInput.Emu.exe"" start= auto", 0, True
ShellObj.Run "sc description XInput.Emu ""A service for Windows that allows to automatically emulate any controller as a Xbox 360 Controller""", 0, True
ShellObj.Run "sc start XInput.Emu", 0, True