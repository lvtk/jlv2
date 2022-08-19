## Project Arvhived.
JUCE officially now supports LV2 since version 7.0.  Please use it instead.

# LV2 Pugin Hosting for JUCE
A JUCE module which provides a LV2PluginFormat. Many features are working, but this project should be considered pre alpha.

Simple plugins should load and run fine, however handling LV2 Atom ports is not complete.  Midi input should be working. Please report a bug if you find otherwise.

#### Requirements
* JUCE 5.4.4 or higher

#### Supported LV2 Features
* Instance Access
* Log
* Scale Points
* State
* UI
* URID
* Worker

#### Supported LV2 UI Features
* CocoaUI / X11UI
* Idle
* Parent
* Resize
* Touch
