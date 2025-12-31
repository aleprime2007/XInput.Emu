# XInput.Emu 

XInput.Emu is a service for Windows that allows to automatically emulate any controller as a Xbox 360 Controller, without needing to configure anything. While XInput.Emu runs in background, it detects which controllers are connected and connect a Xbox 360 Emulated Controller for each one of them.

<h2>How does XInput.Emu work?</h2>

XInput.Emu uses [Simple DirectMedia Layer (SDL)](https://github.com/libsdl-org/SDL) for controller detection and [ViGEm Client](https://github.com/nefarius/ViGEmClient) for controller emulation. Also, XInput.Emu uses [HidHide](https://github.com/nefarius/HidHide) for device hiding (Optional). SDL gets the value of the controllers axis and buttons and ViGEm Client updates the state of the Xbox 360 Emulated Controllers.

<h2>Installation Process</h2>
Before installing XInput.Emu, you must install some dependencies in order to get XInput.Emu working correctly:

- [DirectX](https://www.microsoft.com/en-us/download/details.aspx?id=35)
- [ViGEm Bus](https://github.com/nefarius/ViGEmBus/releases/latest)
- [HidHide](https://github.com/nefarius/HidHide/releases/latest) (Optional)

To install XInput.Emu, you must download the .msi installer in the [Releases Page](https://github.com/aleprime2007/XInput.Emu/releases/latest). Then you have to open the installer and install XInput.Emu as any other software. NOTE: XInput.Emu will automatically install in <b>C:\Program Files\XInput.Emu\ </b>

<h2>Building</h2>

To build this project you'll need Visual Studio 2019 or later and the C++ Desktop Development Kit.
