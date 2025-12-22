#ifndef UTIL_INPUT_HPP
#define UTIL_INPUT_HPP

void resetInput();

void resetMouseUIInput(int mouse);
void setMouseOnUI(int mouse);
bool isMousePressedUI(int mouse);

bool isMousePressedOutsideUI(int mouse);
bool isMouseDownOutsideUI(int mouse);

// Checks if a key is pressed and if it is, plays a sound
bool handleKeyPressWithSound(int key);

#endif
