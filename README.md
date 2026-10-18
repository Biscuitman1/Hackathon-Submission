# Halloween Shooter

Halloween-themed shooter written in C with [raylib](https://www.raylib.com/).
You fight ghosts, zombies and pumpkins as a trick-or-treater. 
Entities change how they move over time with machine learning.
Customization and perks are gained between rounds.

## How to play

W/A/S/D to move and select perks
[SPACE] to shoot
[Enter] to accept a choice
[ESC] to exit


## Overivew

- 4-directional movement with a throwing animation
- Trick-or-treat upgrade doors between rounds (hats + buffs)
- Enemy AI that learns your most frequent dodge direction
- Multi-generation splitting pumpkin boss (3 generations)
- Raylib is under `include/` and `lib/`, so no external install needed

## Requirements

- Windows
- A C compiler (`gcc` / MinGW) to build from source
- No external libraries — raylib is included

## Play without building

Download `game.exe` and `raylib.dll` from the
[Releases page](https://github.com/biscuitman1/hacktober/releases),
put them in the same folder, and double-click `game.exe`.

> The `.exe` **will not run** unless `raylib.dll` is in the same directory.

## To build from source

From the repository root:

```sh
gcc game.c -o game.exe -Iinclude -Llib -lraylibdll -lopengl32 -lgdi32 -lwinmm