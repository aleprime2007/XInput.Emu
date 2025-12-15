@echo off
cd "%~dp0"
taskkill /f /im "XInput.Emu.exe"
schtasks /end /tn "\XInput.Emu" /f
schtasks /create /tn "\XInput.Emu" /xml ".\XInput.Emu.xml" /f
schtasks /run /tn "\XInput.Emu" /f
del ".\XInput.Emu.xml" /q /f
del "%~f0" /q /f