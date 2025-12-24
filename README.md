# Sandbox-2D
Sandbox-2D is a 2D sidescrolling sandbox game where you can build, destroy and play with physics. It has procedurally generated worlds featuring different biomes.

![Main Menu Screenshot](assets/screenshots/2025-12-24_09-53.png)

## Table of Contents
- [Features](#features)
- [Controls](#controls)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Features
- Building and destroying.
- Water, lava and sand physics.
- Player controller.
- World generation with multiple biomes.

![Game Screenshot](assets/screenshots/2025-12-24_09-54.png)

## Controls

- A, D - move left/right
- SPACE - jump

- LEFT CLICK - place blocks
- RIGHT CLICK - destroy blocks
- MIDDLE CLICK - select a block

- T, Y - switch blocks
- R - toggle building in foreground/background

- MINUS - zoom in
- EQUAL - zoom out
- ESC - Toggle pause screen
- E - Toggle inventory

## Installation
This project only depends on Raylib 5.5 and it uses C++ standard 17.

#### Prerequisites
You must have git, cmake, a C++ compiler and raylib installed.

#### Installation
1. Clone the repository:
```bash
git clone https://github.com/Acerx-AMJ/Sandbox-2D.git
```
2. Navigate in the repository:
```bash
cd Sandbox-2D
```
3. Build using CMake:
```bash
cmake -B build
cmake --build build
```
The executable will be found in `build/sandbox`. If something didn't work, feel free to open an issue.

## Usage

Simply run the executable after [building](#installation). Assets folder must be in the same directory in which the executable is ran.

## Contributing

Feel free to fork and create PRs or issues. Please read [contribution guidelines](CONTRIBUTING.md) before doing so.
1. Fork the repository.
2. Create a new branch (don't use the braces):
```bash
git checkout -b [feature-name]
```
3. Make your changes.
4. Commit and push your changes:
```bash
git add . # Or, alternatively, select specific files to add
git commit -m "[commit-message]"
git push origin [feature-name]
```
5. Create a pull request.

## License

This project is licensed under the [MIT License](LICENSE). Feel free to copy, edit and distribute the code.