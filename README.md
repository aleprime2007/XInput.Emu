# XInput.Emu 

XInput.Emu is a service for Windows that allows to automatically emulate any controller as a Xbox 360 Controller, without needing to configure anything. While XInput.Emu runs in background, it detects which controllers are connected and connect a Xbox 360 Emulated Controller for each one of them.

<h2>Main Features</h2>

- <b>Controller Emulation: </b> XInput.Emu uses [ViGEm Client](https://github.com/nefarius/ViGEmClient) and [ViGEm Bus](https://github.com/nefarius/ViGEmBus) to emulate a Xbox 360 Controller. Since the Emulated Controller is a XInput Compatible Controller it works in any game with controller support.
- <b>Automatic Controller Mapping: </b> XInput.Emu doesn't use DirectInput. Instead, it uses [Simple DirectMedia Layer (SDL)](https://github.com/libsdl-org/SDL) for controller detection and mappings. This makes XInput.Emu Compatible with any controller with a known mapping by SDL.
- <b>Vibration and Force Feedback: </b> Since [ViGEm Client](https://github.com/nefarius/ViGEmClient) can get the vibration data, [SDL](https://github.com/libsdl-org/SDL) can send that data to the Real Controller.
- <b>Automatic Device Hiding: </b> To avoid Double Input, XInput.Emu uses [HidHide](https://github.com/nefarius/HidHide) to automatically hide any connected controller and only show the emulated controller.
- <b>Configuration Tool: </b> XInput.Emu has a simple Configuration Tool to change some options.

<h2>Which controllers are compatible?</h2>
Almost All controllers are compatible, But here is a list of some known controllers that are compatible:

- Sony DualShock1 and DualShock2
- Sony DualShock3/SIXAXIS (Only if used with [DsHidMini](https://github.com/nefarius/DsHidMini) in SXS Mode)
- Sony DualShock4
- Sony DualSense
- Nintendo Retro Controllers
- Nintendo Gamecube Controller
- Nintendo Switch JoyCons
- Nintendo Switch Pro Controller

<b>NOTE: </b> If there's a non-compatible controller XInput.Emu will ignore it. In that case, you can use [XOutput](https://github.com/csutorasa/XOutput) or [x360ce](https://github.com/x360ce/x360ce).

<h2>Installation Process</h2>
Before installing XInput.Emu, you must install some dependencies in order to get XInput.Emu working correctly:

- [DirectX](https://www.microsoft.com/en-us/download/details.aspx?id=35)
- [ViGEm Bus](https://github.com/nefarius/ViGEmBus/releases/latest)
- [HidHide](https://github.com/nefarius/HidHide/releases/latest)

To install XInput.Emu, you must download the .msi installer in the [Releases Page](https://github.com/aleprime2007/XInput.Emu/releases/latest) depending of your system (x64 if 64-bit, x86 if 32-bit). Then you have to open the installer and install XInput.Emu as any other software.

<h2>Building</h2>

To build this project you'll need Visual Studio 2019 or later and the C++ Desktop Development Kit.
