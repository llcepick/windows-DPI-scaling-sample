The visual studio solution contains 3 projects

1. DPIHelper (static lib)
This is the helper library which contains class DpiHelper. This has functions for setting and getting DPI.
1. DPIScalingMFCApp
This is the MFC UI application sample.
1. DPIScalingCmdLine
This is a command line tool suitable for use for scripting.

Here is how the app looks
![DPI scaling MFC app](DPIScalingMFCAppScreenHelp.png)

The repo is based on the study I (Lihas/Sahil) did by reverse engineering system settings app (immersive control panel), user32.dll (which contains the API used for DPI scaling).
To be able to get and set DPI scaling of a monitor, the DisplayConfigGetDeviceInfo(), and DisplayConfigSetDeviceInfo() functions are used.
These functions are public (their description is provided on MSDN), but they use some undocumented parameters.
The values of type parameter used in DISPLAYCONFIG_DEVICE_INFO_HEADER is in negative range (-3 for get, and -4 for set).
[DISPLAYCONFIG_DEVICE_INFO_TYPE](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ne-wingdi-displayconfig_device_info_type) as it is
documented by Microsoft (and defined in wingdi.h) only contains non-negative values. Thus the ability to get/set DPI
have been made hidden by Microsoft by not making these parameters public.
The structures used in DisplayConfigGet/SetDeviceInfo() are dependent on value of type. Now since type isn't documented
these structures also have not been.

I (Lihas/Sahil) used WinDbg, and ghidra (https://ghidra-sre.org/) in the reverse engineering effort. A big thanks to creators of ghidra!

If you want to set\get DPI scaling from your code, just use the DpiHelper class. The main methods in the class are
1. GetDPIScalingInfo(), and
1. SetDPIScaling()

DpiScalingCmdLine
This was made to allow for scripting of the DPI scaling per monitor. It will support changing one monitor at a time.

Parameters: 

<pre>DpiScalingCmdLine {monitor number (0 based and optional)} {DPI to use}</pre>

- The monitor number is NOT required to set the DPI for the first monitor, but for all secondary it is required.
- The DPI will ONLY accept values greater than 100 - and only those supported by the particular monitor