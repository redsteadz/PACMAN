# PACMAN

![PACMAN gameplay preview](preview.jpg)

A Pac-Man-inspired arcade game built in C++ with [raylib](https://www.raylib.com/). The project explores grid movement, distinct ghost targeting rules, game-state transitions, visual effects, and an in-game map editor.

## Highlights

- Four ghosts with chase, scatter, frightened, and eaten states
- Different targeting behaviour for Blinky, Pinky, Inky, and Clyde
- Pellets, score, lives, power states, pause support, music, and sound effects
- Animated movement and impact effects
- A built-in editor for loading and saving binary map layouts

## Controls

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` | Change Pac-Man's direction |
| `Space` | Use Pac-Man's limited movement boost |
| Pause button | Pause or resume the simulation |
| Edit button | Open the map editor |

The map editor exposes its save/load and tile controls in the game UI.

## Build and run

You need a C++ compiler, CMake, and a raylib installation discoverable by CMake.

~~~bash
git clone https://github.com/redsteadz/PACMAN.git
cd PACMAN
cmake -S . -B build
cmake --build build
cd build
./Pacman
~~~

CMake copies the required `assets/` directory into the build directory.

## Status

This is a personal game project and learning playground rather than a production Pac-Man implementation. The gameplay, AI experiments, effects, and editor are all contained in the current executable.
