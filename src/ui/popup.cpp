#include "SRU/text.hpp"
#include "mngr/input.hpp"
#include "ui/button.hpp"
#include "ui/popup.hpp"
#include "util/position.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"
#include "SRU/render.hpp"
#include <vector>

// Constants

constexpr Vector2 popupSize = {500.0f, 375.0f};
constexpr float fadeTime = 0.6f;

// Globals

static std::vector<Popup> popups;
static bool wasLastPopupConfirmed = false;
static float fadeTimer = 0.0f;
static float alpha = 0.0f;
static bool fadedIn = false, fadedOut = true;

static Button confirmationButton;
static Button denialButton;
static Button okayButton;

// Helper functions

void fadeOut(float dt) {
   if (fadedOut) {
      return;
   }
   fadedIn = false;
   fadeTimer += dt;
   alpha = (1.0f - fadeTimer / fadeTime) / 2.0f;

   if (fadeTimer >= fadeTime) {
      alpha = fadeTimer = 0.0f;
      fadedOut = true;
   }
}

void fadeIn(float dt) {
   if (fadedIn) {
      return;
   }
   fadedOut = false;
   fadeTimer += dt;
   alpha = (fadeTimer / fadeTime) / 2.0f;

   if (fadeTimer >= fadeTime) {
      alpha = 0.5f;
      fadeTimer = 0.0f;
      fadedIn = true;
   }
}

void setPopupSize() {
   float wr = getWidthRatio();
   float hr = getHeightRatio();

   denialButton.rectangle = {getScreenCenter().x - 120.0f * wr, getScreenCenter().y + 110.0f * hr, buttonWidth * wr, buttonHeight * hr};
   confirmationButton.rectangle = {getScreenCenter().x + 120.0f * wr, getScreenCenter().y + 110.0f * hr, buttonWidth * wr, buttonHeight * hr};
   okayButton.rectangle = {getScreenCenter().x, getScreenCenter().y + 110.0f * hr, buttonWidth * wr, buttonHeight * hr};
}

// Init functions

void initPopups() {
   confirmationButton.text = "YES";
   denialButton.text = "NO";
   okayButton.text = "OKAY";
   confirmationButton.texture = denialButton.texture = okayButton.texture = &getTexture("button");
}

// Popup functions

void insertPopup(const std::string &header, const std::string &body, PopupType type) {
   popups.push_back(Popup{header, body, type});
   setPopupSize();

   if (type == PopupType::error) {
      playSound("failure");
   } else {
      playSound("notice");
   }
}

bool isPopupConfirmed() {
   bool confirmed = wasLastPopupConfirmed;
   if (confirmed) {
      wasLastPopupConfirmed = false;
   }
   return confirmed;
}

bool anyPopups() {
   return !popups.empty();
}

// Update function

void updatePopups(float dt) {
   if (popups.empty()) {
      fadeOut(dt);
      return;
   }

   fadeIn(dt);
   Popup &popup = popups.back();
   setPopupSize();

   if (popup.type == PopupType::confirmation) {
      confirmationButton.update(dt);
      denialButton.update(dt);
   
      if (confirmationButton.clicked || handleKeyPressWithSound(KEY_ENTER)) {
         wasLastPopupConfirmed = true;
         popups.pop_back();
      }

      if (denialButton.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
         wasLastPopupConfirmed = false;
         popups.pop_back();
      }
   } else {
      okayButton.update(dt);

      if (okayButton.clicked || handleKeyPressWithSound(KEY_ENTER) || handleKeyPressWithSound(KEY_ESCAPE)) {
         popups.pop_back();
      }
   }
}

// Render function

void renderPopups() {
   drawRect({0.0f, 0.0f}, getWindowSize(), Fade(BLACK, alpha));

   if (popups.empty()) {
      return;
   }

   float wr = getWidthRatio();
   float hr = getHeightRatio();

   Popup &popup = popups.back();
   drawTextureCentered(getTexture("popup_frame"), getScreenCenter(), applyResponsiveness(popupSize));
   drawTextCentered("andy", getScreenCenter({0.0f, hr * -125.0f}), popup.header.c_str(), getFontSize(50.0f));

   std::string wrappedBody = popup.body;
   wrapInPlace(wrappedBody, getFont("andy"), (popupSize.x - 30.0f) * wr, getFontSize(25.0f));
   drawTextCentered("andy", getScreenCenter({0.0f, hr * -40.0f}), wrappedBody.c_str(), getFontSize(25.0f), WHITE, getFontSize(1.0f));

   if (popup.type == PopupType::confirmation) {
      confirmationButton.render();
      denialButton.render();
   } else {
      okayButton.render();
   }
}
