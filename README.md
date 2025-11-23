# Sandbox-2D
Sandbox-2D is a 2D sidescrolling sandbox where you can build, destroy and play with physics.

![Game Screenshot](assets/screenshots/demo.gif)

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

## Controls

- ESC - regenerate world
- A, D - move left/right
- SPACE - jump
- Q, E - switch blocks
- R - toggle building in foreground/background
- LEFT CLICK - destroy a block
- RIGHT CLICK - place a block
- MIDDLE CLICK - select a block

## Installation

#### Prerequisites
You must have git, cmake, a C++ compiler, raylib and nlohmann-json installed.

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

Feel free to fork, create PRs or issues.
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