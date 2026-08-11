#pragma once

// Update input

void updateInput();
void setInputBlocking(bool block);

// Get key functions

bool isKeyDown(int key);
bool isKeyPressed(int key);
bool isKeyReleased(int key);
bool isKeyRepeated(int key);

bool handleKeyPressWithSound(int key);

// Get mouse functions

void setMouseOnUI(bool onUI);
void resetMousePress(int button);

bool isMouseDownUI(int button);
bool isMousePressedUI(int button);
bool isMouseReleasedUI(int button);

bool isMouseDownOutsideUI(int button);
bool isMousePressedOutsideUI(int button);
bool isMouseReleasedOutsideUI(int button);

bool isMouseDown(int button);
bool isMousePressed(int button);
bool isMouseReleased(int button);
