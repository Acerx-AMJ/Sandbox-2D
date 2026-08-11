#pragma once
#include <string>

enum class PopupType: char {
   info, error, confirmation
};

struct Popup {
   std::string header;
   std::string body;
   PopupType type;
};

void initPopups();

void insertPopup(const std::string &header, const std::string &body, PopupType type0);
bool isPopupConfirmed();
bool anyPopups();

void updatePopups(float dt);
void renderPopups();
