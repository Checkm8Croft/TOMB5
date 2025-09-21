# TOMB5 Porting in OpenGL
A Tomb Raider 5 porting project from Windows and DirectX to OpenGL and macOS/Linux

## Dependecies
First install the dependencies:
- SDL2 (on macOS install it with brew (if you have MacPorts, you'll have to edit CMake file))
- SDL2 Mixer
- SDL2 Image
- OpenGL (On macOS just install Xcode (command line tools only is good too))
- CMake
- Make or Ninja 

## Building
`mkdir build && cd build`
**macOS Only** (if you have SDL2.framework in /Library/Frameworks) `sudo ln -s /usr/local/include/SDL2/SDL_mixer.h /Library/Frameworks/SDL2.framework/Headers/SDL_mixer.h`
`cmake .. -G Ninja` (or `"Unix Makefiles"` if you want to use make)
`ninja` or `make -j$(nproc)`

### Credits:
Troye: Main programmer, project leader.

ChocolateFan: Programmer, contributor.

thatguyfrombb: Additional decompiling, bug fixing, and documentation.

Raildex: additional fixes.

Checkm8Croft: Porting