# TOMB5 Porting in OpenGL
A Tomb Raider 5 porting project from Windows and DirectX to OpenGL and macOS/Linux

## NOTE
This project has the tomb5 files ported, so they actually compile, but the game mechanics not, for these need to be really implemented, and since I don't know a shit of GL, I'll leave this repo as is. So I'm not going to make other changes to the project until a Pull Request for who really knows about GL. 
So I'll not archive the repo, I'll leave it open for who wants to fork it for have a goos base without need to change DX instructions. 
So Pull Requests are welcome :)

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