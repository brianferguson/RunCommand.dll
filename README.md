RunCommand.dll
=============

RunCommand.dll is a plugin for [Rainmeter](http://www.rainmeter.net) that will run programs in a normal/hidden/minimized/maximized state. This plugin will also attempt to retrieve any output (stdout) from the program and set that to the "string" value of the plugin.


Contents
-

* [Features](#features)
* [Options](#options)
* [Commands](#commands)
* [Changes](#changes)
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

#####Note:
The original purpose of the plugin was to run command through the command-line interpreter (ie. cmd.exe) without the window showing up. Once I figured that out, it was easy to pipe the output of the command into Rainmeter and set it as the plugin's "string" value. After nearly completing the plugin, I realized that any program whose output is Unicode encoded would not work properly with this plugin. Although I could probably add some sort of "Unicode detection" to the plugin, I don't have any plans to do this at this time. Again, the purpose of the command is run a command without showing the ugly command window.


Options
-

* **Program** - The program to run. The default value is the `%ComSpec%` [environment variable](http://en.wikipedia.org/wiki/ComSpec).
* **Parameter** - Any command-line parameters to send to the program (if any). If no `Program` is given, this option should represent the "command" you want to run from the command line (ie. `wmic os get lastbootuptime`).
* **State** - Initial "state" of the program. Some programs explictly show themselves no matter what the OS tells them, so not all programs will respond to this option. Options include:
  * **Hide** - Attempts to hide the program's window from being visible. Not all programs allow this (like Calc.exe). *This option is not recommended for GUI programs and/or programs that expect any form of input.* If the program hangs or expects input, you may have to explicitly kill the program in Windows Task Manager. This is the default value.
  * **Show** - Shows the program's window.
  * **Minimize** - Attempts to start the program minimized.
  * **Maximize** - Attempts to start the program maximized.
* **FinishAction** - Action that runs after the program has exited.
* **OutputFile** - File where the output is saved.
* **StartInFolder** - Starts the program in this folder.
* **Timeout** - In milliseconds. If positive, will close the program after the given time. The default is `-1`. Even if the close is cancelled, the program will still close after another "timeout" value. For example: If you open notepad with a timeout of 5 seconds, and make a change within those 5 seconds and cancel when asked to save the file...notepad will still close after another 5 seconds.

#####Note:
Because Rainmeter strips any quotes from the start *and* end of an option (if both exist), it is important to double quote the `Program` and `Parameter` options (if needed).  `Program=""C:\Program Files\Some Program\Something with a space.exe""`

Commands
-

* **Run** - Runs the program.

Changes
-
Here is a list of the major changes to the plugin.

#####Version:
* **1.0.0** - Initial Version.

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
Click on each command and watch the output will appear in the "output" meter (if any output is expected).

```ini
[Rainmeter]
Update=1000
BackgroundMode=2
SolidColor=0,0,0

[Command_1]
Measure=Plugin
Plugin=RunCommand
Parameter=dir
State=Hide
;StartInFolder=C:\
OutputFile=test.txt
FinishAction=[!SetOption Output SolidColor "0,0,255"][!SetOption Output MeasureName Command_1][!UpdateMeter Output][!Redraw]

[Command_2]
Measure=Plugin
Plugin=RunCommand
Program=C:\Windows\System32\Notepad.exe
Parameter=test.txt
State=Maximize
Timeout=5000
FinishAction=[!SetOption Output SolidColor "255,0,0"][!SetOption Output MeasureName Command_2][!UpdateMeter Output][!Redraw]

[Command_3]
Measure=Plugin
Plugin=RunCommand
Parameter=wmic os get lastbootuptime
OutputFile=test.txt
FinishAction=[!SetOption Output SolidColor "255,0,255"][!SetOption Output MeasureName Command_3][!UpdateMeter Output][!Redraw]
;Output in "yyyymmddHHMMSS.mmmmmmsUUU" format (CIM_DATETIME). :-)
Substitute="LastBootUpTime":"","#CRLF#":""," ":""

[Command_4]
Measure=Plugin
Plugin=RunCommand
Parameter=wmic path win32_utctime get month, day, year, hour, minute, second, dayofweek, quarter, weekinmonth
FinishAction=[!SetOption Output SolidColor "0,102,0"][!SetOption Output MeasureName Command_4][!UpdateMeter Output][!Redraw]

[Label_1]
Meter=String
FontColor=255,255,255
Text=Command: dir
LeftMouseUpAction=!CommandMeasure Command_1 Run

[Label_2]
Meter=String
FontColor=255,255,255
Text=Command: Notepad
X=22R
LeftMouseUpAction=!CommandMeasure Command_2 Run

[Label_3]
Meter=String
FontColor=255,255,255
Text=Command: wmic os get lastbootuptime
X=22R
LeftMouseUpAction=!CommandMeasure Command_3 Run

[Label_4]
Meter=String
FontColor=255,255,255
Text=Command: wmic path win32_utctime
Y=2R
LeftMouseUpAction=!CommandMeasure Command_4 Run

[Output]
Meter=String
MeasureName=Command_1
Y=5R
W=550
H=150
ClipString=1
FontColor=255,255,255
FontFace=Lucida Console
FontSize=9
AntiAlias=1
```