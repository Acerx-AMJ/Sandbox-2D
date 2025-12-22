### Contributing to Sandbox-2D
Thanks for taking the time to contribute! These are guidelines on how to contribute better, not strict rules. Feel free to propose changes to this document.

### ToC
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Enhancements](#suggesting-enhancements)
- [Contributing Code](#contributing-code)
- [Pull Requests](#pull-requests)
- [Code Style](#code-style)

### Reporting Bugs
Before creating a new issue, please check if there isn't already a similar one. Issues can be created [here](https://github.com/Acerx-AMJ/Sandbox-2D/issues).

An issue should contain the following:
- Your operating system
- Way of installation (github, itch, etc.)
- What's wrong
- Expected result
- How to reproduce the issue and can it reliably be reproduced

The issue title should be short and descriptive, for example the title for an issue of a crash when opening the inventory might be "Opening inventory causes crash.". You should describe the exact steps you took to reproduce the problem.

### Suggesting Enhancements
Just like bug reports, enhancements are tracked in [Issues](https://github.com/Acerx-AMJ/Sandbox-2D/issues). Before creating a new issue, please check if the functionality isn't already present.

Enhancements should have a descriptive title and description. You should describe current behaviour and explain what behaviour you were expecting and why this may be useful. If you know other projects that have implemented it, make sure to list them.

### Contributing Code
To get started fork the repository, clone the fork locally, and make sure that the project builds and runs on your machine. There is a guide for building in [README.md](README.md).

When you feel comfortable with the project, please read the [PR guidelines](#pull-requests) and [code style guidelines](#code-style) and create a pull request.

### Pull Requests
When creating a PR, please add a descriptive title and list all changes made in description. If needed, add screenshots or GIFs. Commit small changes, but frequently, ideally a single commit should not exceed 150 changed lines unless necessary.

Commit messages and PRs must be gramatically correct. In commit messages explain what was changed and where.

Do not create PRs of incomplete features or broken builds. Make sure to test your code before sending it.

### Code Style
These are all recommendations. When modifying existing code, try to follow the style of the surrounding code. Your PR won't get declined because it doesn't follow this (unless it's completely unreadable). Just try your best at writting clean code and it'll be fine.

#### Formatting
- Use 3 spaces for indentation, not tabs.
- Opening braces go on the same line, with a space before them.
- Pointers should go before name in variables (`int *pointer`) and after variable in functions (`int* getPointer()`).
#### Naming
Write descriptive names. If a short name is descriptive, use that. Do not shorten common words like position, velocity and pointer to pos, vel and ptr.

- Classes, structs and enums: PascalCase
- Functions, variables, constants and files: camelCase
#### File Structure
Header files go in include/ and source files in src/. Both directories have equal folders, that must be mirrored (unless a file is header only):
- game/ - contains game logic and states.
- mngr/ - contains managers of a specific task.
- objs/ - contains objects used in the game.
- ui/ - strictly contains just UI elements (buttons, scrollframes, etc.).
- util/ - contains utility headers.

Header files must use #ifndef guards and they should be defined as FOLDER_FILE_HPP. For example include/util/fileio.hpp has the include header UTIL_FILEIO_HPP.

Sort headers alphabetically from A to Z. Include order is:
1. Project headers.
2. Third-party headers.
3. Standard library headers.
#### C++ Practices
- Do not use macros, use constexpr where needed.
- Do not use `using namespace ...` unless it's for literals (e.g.: `using namespace std::string_literals`).
- Only use `class` keyword for `enum class`, use `struct` for objects. Do not use `private:` or `protected:`.
- Use `enum class` when an enum is not defined in an object.
- Prefer range-based `for` when appropriate.
- Only use polymorphism when the benefits outweight the negatives, prefer composition.
- Try to avoid allocating memory on the heap yourself. Using standard containers like std::vector is completely fine.
- Avoid templates outside utility functions.
- Apply const correctness where possible.
- Only group variables if no value is assigned to any (`int a, b, c;` and `int a = 1; int b = 2; int c = 3;`) 
#### Error Handling
- Try to handle gracefully without crashing.
- If a failure is unrecoverable, use the util/format.hpp warn or assert functions.
- Do not use try/catch blocks.
#### Comments
- Use comments for separating code blocks or to describe why you're using a specific solution.
- Do not use multi-line comments.
