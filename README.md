RunCommand.dll (Beta)
=============

RunCommand.dll is a plugin for [Rainmeter](http://www.rainmeter.net) that will run programs in a normal/hidden/minimized/maximized state. This plugin will also attempt to retrieve any output (stdout) from the program and set that to the "string" value of the plugin.

#####Note:
This plugin requires Rainmeter version 3.0.2 (r2161) or higher. Make sure to set the "Minimum Rainmeter Version" to `3.0.2.2161` in the [Skin Packager](http://docs.rainmeter.net/manual/publishing-skins).


Contents
-
* [Features](#features)
* [Options](#options)
* [Commands](#commands)
* [Changes](#changes)
* [Error Codes](#error-codes)
* [Download](#download)
* [Build Instructions](#build-instructions)
* [Examples](#examples)


Features
-
Here are some of the features of the RunCommand plugin:

* Run a program in a hidden state. Great for running command-line programs without showing the ugly command-line window.
* Can also start to a program in a minimized or maximized state.
* The plugin also attempts to get the output (stdout) of the program and set it as the plugin "string" value. If the output is only numbers, the "string" value will be converted to its number value.
* Can save the output of the program to a file.
* Can run the program from a specific directory.
* Can terminate the program after a given "timeout".
* If `State=Hide`, the program will quit if the skin is unloaded (or Rainmeter itself).

#####Note:
As of version 0.0.1.4 Beta, the number value of the measure will indicate if the command finished with/without any errors. You can get the number value of the measure by using [section variables](http://docs.rainmeter.net/manual-beta/variables/section-variables#Colon). Before the command has first run, the number value of the measure will be `-1`. While the command is running, the number value of the measure will be `0`. If an error occurs, the number value  of the measure will indicate the type of [error](#error-codes) that has occurred. If the program finishes without error, the number value of the measure will be `1` meaning "Success". These can be useful when used with [IfActions](http://docs.rainmeter.net/manual-beta/measures/general-options/ifactions) to run specific actions for different errors.


Options
-
* **Program** - The program to run. The default value is the `%ComSpec%` [environment variable](http://en.wikipedia.org/wiki/ComSpec).
* **Parameter** - Any command-line parameters to send to the program (if any). If no `Program` is given, this option should represent the "command" you want to run from the command line (ie. `wmic os get lastbootuptime`).
* **State** - Initial "state" of the program. Some programs explictly show themselves no matter what the OS tells them, so not all programs will respond to this option. Options include:
  * **Hide** - Attempts to hide the program's window from being visible. Not all programs allow this (like Calc.exe). **This option is not recommended for GUI programs and/or programs that expect any form of input.** If the program hangs or expects input, you may have to explicitly kill the program in Windows Task Manager. This is the default value.
  * **Show** - Shows the program's window.
  * **Minimize** - Attempts to start the program minimized.
  * **Maximize** - Attempts to start the program maximized.
* **FinishAction** - Action that runs after the program has exited.
* **OutputFile** - File where the output is saved.
* **OutputType** - Type of output that is **expected** from the program.
  * **ANSI** - Output is expected to be ANSI encoded. The `OutputFile` will also be ANSI encoded.
  * **UTF8** - Output is expected to be UTF-8 encoded. The `OutputFile` will also be UTF-8 encoded.
  * **UTF16** - Output is expected to be UTF-16LE encoded. The `OutputFile` will also be UTF-16LE encoded.
* **StartInFolder** - Starts the program in this folder.
* **Timeout** - In milliseconds. If positive, will either "Close" or "Kill" the program (depending on the `State`) after the given time. The default is `-1`, which means no timeout. The `FinishAction` will run even if the program does not terminate (eg. if closing "Notepad", clicking "Cancel" when asked to save the document). It should be noted that this value is **independent** of the `Update` or `UpdateDivider` values of the skin.

#####Note:
Because Rainmeter strips any quotes from the start *and* end of an option (if both exist), it is important to double quote the `Program` and `Parameter` options (if needed).  `Program=""C:\Program Files\Some Program\Something with a space.exe""`

Commands
-
* **Run** - Runs the program.  **Example:** `!CommandMeasure MeasureName Run`
* **Close** - Closes the program properly - meaning the program may prompt the user to close. Some programs can cancel the closing of the program, in which the plugin will still gather any output.  **Example:** `!CommandMeasure MeasureName Close`
* **Kill** - Terminates the program without notice. This command should be used if the program was started in the hidden state. ie. `State=Hide`  **Example:** `!CommandMeasure MeasureName Kill`

Changes
-
Here is a list of the major changes to the plugin.

#####Version:
* **0.0.1.4** - Fixed buffer overrun issue with a large amount of data being output. Improved logging of errors. Number value of measure will now indicate a "success" code or "error code".
* **0.0.1.3** - Improved performance when waiting for the program to finish. Programs are terminated on skin refresh (or Rainmeter exit) regardless of `State`. Changed `Timeout` behavior.
* **0.0.1.2** - Added the `Close` and `Kill` commands.
* **0.0.1.1** - FinishAction should not deadlock anymore.
* **0.0.1.0** - Major change to OutputType. Auto detection is removed.
* **0.0.0.3** - Fixed Unicode detection. Added OutputType. Hidden program now close if the skin and/or Rainmeter is closed.
* **0.0.0.2** - Added Unicode support. Minor tweaks.
* **0.0.0.1** - Initial Version.

Error Codes
-
Here is a list of possible error codes:

* **100** - Unknown command. This happens when an invalid [command](#commands) was sent to the plugin.
* **101** - Program still running. This happens when trying to `Run` the program again before it has finished running.
* **102** - Program not running. This happens when trying to `Close` or `Kill` a program that is not running.
* **103** - Cannot start program. This happens when the `Program` option is invalid. Usually a wrong path or typo in the `Program` option is the cause.
* **104** - Cannot save file. This happens when there is an error saving the file. This could happen if writing to a locked location or file. The string value of the plugin will still contain any output of the program.
* **105** - Cannot terminate process. There was an error when attempting to close or kill the program. This should rarely happen. If so, it might be necessary to open the Windows Task Manager and manually close the program.
* **106** - Cannot create pipe. This happens when the plugin cannot create a [pipe](http://en.wikipedia.org/wiki/Anonymous_pipe#Microsoft_Windows) to the program trying to run. The program will not run if this error happens. This error should rarely happen.

#####Note:
It should be noted that these codes are for the plugin **only** and have nothing to do with any error encountered with the program itself.

Download
-
####To download the current source code:

* Using git: `git clone git@github.com:brianferguson/RunCommand.dll.git`
* Download as a [.zip](https://github.com/brianferguson/RunCommand.dll/zipball/master)

####To download current plugin (.dll):

* [32-bit version](https://github.com/brianferguson/RunCommand.dll/blob/master/PluginRunCommand/x32/Release/RunCommand.dll?raw=true)
* [64-bit version](https://github.com/brianferguson/RunCommand.dll/blob/master/PluginRunCommand/x64/Release/RunCommand.dll?raw=true)


Build Instructions
-
This plugin was written in C++ using the [Rainmeter Plugin SDK](https://github.com/rainmeter/rainmeter-plugin-sdk)

The plugin can be built using the free Visual Studio Express 2012 for Windows Desktop
or any paid version of VS2012 (e.g. Professional). Note VS Update 1 is required for targeting Windows XP.

1. [Visual Studio 2012 Express Edition](http://microsoft.com/visualstudio/eng/products/visual-studio-express-for-windows-desktop)
2. [Visual Studio 2012 Update 1](http://microsoft.com/visualstudio/eng/downloads#d-visual-studio-2012-update) (needed to target Windows XP)

Once you have Visual Studio install (with Update 1), then you can open "PluginRunCommand.sln" and build from there.


Examples
-

####Example 1:
Click on each command and the output will appear in the "output" meter (if any output is expected). A "status" will appear next to each command.

```ini
[Rainmeter]
Update=1000
BackgroundMode=2
SolidColor=0,0,0

[Metadata]
Name=RunCommand
Author=Brian
Information=Click on each command and the RunCommand plugin will attempt to run the command. Output is given at the bottom.
License=Creative Commons Attribution-Non-Commercial-Share Alike 3.0
Version=0.0.1.4 (Beta)

; Because the default program is "%ComSpec% /U /C", the output should be in UTF16 for common commands like "dir".
; But not all commands will output in UTF16 (see the "wmic" command below).
[Command_1]
Measure=Plugin
Plugin=RunCommand
Parameter=dir
OutputFile=Command1.txt
FinishAction=[!UpdateMeasure #CURRENTSECTION#][!SetOption Output MeasureName Command_1][!UpdateMeter Output][!Redraw]
IfBelowValue=1
IfBelowAction=[!SetOption Status_1 SolidColor "0,0,0"][!UpdateMeter Status_1]
IfEqualValue=1
IfEqualAction=[!SetOption Status_1 SolidColor "0,102,0"][!UpdateMeter Status_1]
IfAboveValue=1
IfAboveAction=[!SetOption Status_1 SolidColor "255,0,0"][!UpdateMeter Status_1][!About]


; The wmic command seems to output in ANSI format, so set OutputType=ANSI
[Command_2]
Measure=Plugin
Plugin=RunCommand
Parameter=wmic os get lastbootuptime
OutputFile=Command2.txt
OutputType=ANSI
FinishAction=[!UpdateMeasure #CURRENTSECTION#][!SetOption Output MeasureName Command_2][!UpdateMeter Output][!Redraw]
;Output in "yyyymmddHHMMSS.mmmmmmsUUU" format (CIM_DATETIME).
IfBelowValue=1
IfBelowAction=[!SetOption Status_2 SolidColor "0,0,0"][!UpdateMeter Status_2]
IfEqualValue=1
IfEqualAction=[!SetOption Status_2 SolidColor "0,102,0"][!UpdateMeter Status_2]
IfAboveValue=1
IfAboveAction=[!SetOption Status_2 SolidColor "255,0,0"][!UpdateMeter Status_2][!About]

[Command_3]
Measure=Plugin
Plugin=RunCommand
Parameter=wmic path win32_utctime get month, day, year, hour, minute, second, dayofweek, quarter, weekinmonth
OutputFile=Command3.txt
OutputType=ANSI
FinishAction=[!UpdateMeasure #CURRENTSECTION#][!SetOption Output MeasureName Command_3][!UpdateMeter Output][!Redraw]
IfBelowValue=1
IfBelowAction=[!SetOption Status_3 SolidColor "0,0,0"][!UpdateMeter Status_3]
IfEqualValue=1
IfEqualAction=[!SetOption Status_3 SolidColor "0,102,0"][!UpdateMeter Status_3]
IfAboveValue=1
IfAboveAction=[!SetOption Status_3 SolidColor "255,0,0"][!UpdateMeter Status_3][!About]

; dir command with an invalid OutputFile. The output will be fine, but there will be an error when saving the file.
[Command_4]
Measure=Plugin
Plugin=RunCommand
Parameter=dir
OutputFile=<InvalidFile>.txt
FinishAction=[!UpdateMeasure #CURRENTSECTION#][!SetOption Output MeasureName Command_4][!UpdateMeter Output][!Redraw]
IfBelowValue=1
IfBelowAction=[!SetOption Status_4 SolidColor "0,0,0"][!UpdateMeter Status_4]
IfEqualValue=1
IfEqualAction=[!SetOption Status_4 SolidColor "0,102,0"][!UpdateMeter Status_4]
IfAboveValue=1
IfAboveAction=[!SetOption Status_4 SolidColor "255,0,0"][!UpdateMeter Status_4][!About]

; Example of an invalid program (eg. typo in the Program option)
[Command_5]
Measure=Plugin
Plugin=RunCommand
Program=""%WINDIR%\system32\Notepad.exr""
FinishAction=[!UpdateMeasure #CURRENTSECTION#][!SetOption Output MeasureName Command_5][!UpdateMeter Output][!Redraw]
IfBelowValue=1
IfBelowAction=[!SetOption Status_5 SolidColor "0,0,0"][!UpdateMeter Status_5]
IfEqualValue=1
IfEqualAction=[!SetOption Status_5 SolidColor "0,102,0"][!UpdateMeter Status_5]
IfAboveValue=1
IfAboveAction=[!SetOption Status_5 SolidColor "255,0,0"][!UpdateMeter Status_5][!About]


[sLabel]
Y=2R
W=300
H=12
FontFace=Lucida Console
FontSize=9
FontColor=255,255,255,150
AntiAlias=1
MouseOverAction=[!SetOption #CURRENTSECTION# FontColor 255,255,255][!UpdateMeter #CURRENTSECTION#][!Redraw]
MouseLeaveAction=[!SetOption #CURRENTSECTION# FontColor 255,255,255,150][!UpdateMeter #CURRENTSECTION#][!Redraw]

[sStatus]
X=10R
Y=r
W=265
FontColor=255,255,255
Prefix="Status: "
DynamicVariables=1
MouseOverAction=""
MouseLeaveAction=""

[Label_1]
Meter=String
MeterStyle=sLabel
Text=Command: dir
LeftMouseUpAction=[!CommandMeasure Command_1 Run][!UpdateMeasure Command_1][!UpdateMeter Status_1][!Redraw]

[Status_1]
Meter=String
MeterStyle=sLabel | sStatus
Text=[Command_1:]

[Label_2]
Meter=String
MeterStyle=sLabel
Text=Command: wmic os get lastboottime
LeftMouseUpAction=[!CommandMeasure Command_2 Run][!UpdateMeasure Command_2][!UpdateMeter Status_2][!Redraw]

[Status_2]
Meter=String
MeterStyle=sLabel | sStatus
Text=[Command_2:]

[Label_3]
Meter=String
MeterStyle=sLabel
Text=Command: wmic path win32_utctime
LeftMouseUpAction=[!CommandMeasure Command_3 Run][!UpdateMeasure Command_3][!UpdateMeter Status_3][!Redraw]

[Status_3]
Meter=String
MeterStyle=sLabel | sStatus
Text=[Command_3:]

[Label_4]
Meter=String
MeterStyle=sLabel
Text=Command: dir (with invalid OutputFile)
LeftMouseUpAction=[!CommandMeasure Command_4 Run][!UpdateMeasure Command_4][!UpdateMeter Status_4][!Redraw]

[Status_4]
Meter=String
MeterStyle=sLabel | sStatus
Text=[Command_4:]

[Label_5]
Meter=String
MeterStyle=sLabel
Text=Command: Notepad.exr (Typo in Program)
LeftMouseUpAction=[!CommandMeasure Command_5 Run][!UpdateMeasure Command_5][!UpdateMeter Status_5][!Redraw]

[Status_5]
Meter=String
MeterStyle=sLabel | sStatus
Text=[Command_5:]


[Output]
Meter=String
MeasureName=Command_1
MeterStyle=sLabel
Y=5R
W=575
H=200
FontColor=255,255,255
ClipString=1
SolidColor=50,50,50
Padding=0,2,0,0
MouseOverAction=""
MouseLeaveAction=""
```
