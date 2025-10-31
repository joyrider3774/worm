# Worm
![DownloadCountTotal](https://img.shields.io/github/downloads/joyrider3774/worm/total?label=total%20downloads&style=plastic) ![DownloadCountLatest](https://img.shields.io/github/downloads/joyrider3774/worm/latest/total?style=plastic) ![LatestVersion](https://img.shields.io/github/v/tag/joyrider3774/worm?label=Latest%20version&style=plastic) ![License](https://img.shields.io/github/license/joyrider3774/worm?style=plastic)

![screenshot 1](screenshots/screenshot1.png)

This is my worm game in SDL3, which is a remake of the classic copter / worm game with multiple gameplay modes

## Mouse Controls

| Button     | Action                                                              |
|------------|---------------------------------------------------------------------|
| LEFT       | Start GAME when pressed in upper & middle part, Repeadetly tap to move Worm. If pressed in lower part increases seed |
| RIGHT      | Select Game Mode on Titlescreen when pressed in upper & middle part. If pressed in lower part increases seed by 15 |

## Controls Keyboard

| Key        | Action                                        |
|------------|-----------------------------------------------|
| A/SPACE    | Start GAME, Repeadetly tap to move Worm       |
| LEFT/RIGHT | Select Game Mode on Titlescreen               |
| UP/DOWN    | Select Seed, increases by 15 when hold        |
| L/B        | Select Seed, increases by 15                  |        
| R          | Reset scores                                  |

## Controls XBox Controller

| Button     | Action                                        |
|------------|-----------------------------------------------|
| A          | Start GAME, Repeadetly tap to move Worm       |
| LEFT/RIGHT | Select Game Mode on Titlescreen               |
| UP/DOWN    | Select Seed,                                  |
| L/B        | Select Seed, increases by 15                  |        
| START      | Holding Start for half a second resets scores |

## Aim of the game
Repeadetly Press / hold A button to move the worm around, dont hit obstacles and walls, try to gain highest score.

## Seed System
The game has a seed system, you can choose a number between 0 and 9999 for the seed.
Selecting seed 0 will produce random levels on ever (re)start, but selecting any higher number will make sure the level is the same for anyone playing on the same game mode and with the same seed

## Game Modes
There are 5 Game modes to play on, each with their own specifics

### Mode A
- decreasing space for the worm to move in
- 5 obstacles
- Constant slower speed

### Mode B
- Fixed space for the worm to move in
- No obstacles
- Increasing speed

### Mode C
- Fixed space for the worm to move in
- 2 obstacles
- Increasing speed 

### Mode D
- Decreasing space for the worm to move in
- No obstacles
- Increasing speed

### Mode E
- Pacman mode: Collect the yellow squares, missing one is game over
- Fixed space for the worm to move in
- No obstacles
- Constant slower speed

## Credits
The game was made possible after watching this youtube movie: [https://www.youtube.com/watch?v=W-3okcjOFnY](https://www.youtube.com/watch?v=W-3okcjOFnY)

## License
* License of my part of game's code is MIT
