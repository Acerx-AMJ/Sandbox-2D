#ifndef UI_POPUP_HPP
#define UI_POPUP_HPP

#include <string>

struct Popup {
   std::string header;
   std::string body;
   bool confirmation = false;
};

void initPopups();

void insertPopup(const std::string &header, const std::string &body, bool confirmation);
bool isPopupConfirmed();
bool anyPopups();

void updatePopups(float dt);
void renderPopups();

#endif
