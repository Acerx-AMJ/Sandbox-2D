#include "ui/popup.hpp"
#include "mngr/resource.hpp"
#include "ui/button.hpp"
#include "util/format.hpp"
#include "util/input.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
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

void fadeOut() {
   if (fadedOut) {
      return;
   }
   fadedIn = false;
   fadeTimer += GetFrameTime();
   alpha = (1.0f - fadeTimer / fadeTime) / 2.0f;

   if (fadeTimer >= fadeTime) {
      alpha = fadeTimer = 0.0f;
      fadedOut = true;
   }
}

void fadeIn() {
   if (fadedIn) {
      return;
   }
   fadedOut = false;
   fadeTimer += GetFrameTime();
   alpha = (fadeTimer / fadeTime) / 2.0f;

   if (fadeTimer >= fadeTime) {
      alpha = 0.5f;
      fadeTimer = 0.0f;
      fadedIn = true;
   }
}

// Init functions

void initPopups() {
   confirmationButton.rectangle = {getScreenCenter().x + 120.0f, getScreenCenter().y + 110.0f, buttonWidth, buttonHeight};
   confirmationButton.text = "YES";
   denialButton.rectangle = {getScreenCenter().x - 120.0f, getScreenCenter().y + 110.0f, buttonWidth, buttonHeight};
   denialButton.text = "NO";
   
   okayButton.rectangle = {getScreenCenter().x, getScreenCenter().y + 110.0f, buttonWidth, buttonHeight};
   okayButton.text = "OKAY";

   confirmationButton.texture = denialButton.texture = okayButton.texture = &getTexture("button");
}

// Popup functions

void insertPopup(const std::string &header, const std::string &body, bool confirmation) {
   popups.push_back(Popup{header, body, confirmation});
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

void updatePopups() {
   if (popups.empty()) {
      fadeOut();
      return;
   }

   fadeIn();
   Popup &popup = popups.back();

   if (popup.confirmation) {
      confirmationButton.update();
      denialButton.update();
   
      if (confirmationButton.clicked || handleKeyReleaseWithSound(KEY_ENTER)) {
         wasLastPopupConfirmed = true;
         popups.pop_back();
      }

      if (denialButton.clicked || handleKeyReleaseWithSound(KEY_ESCAPE)) {
         wasLastPopupConfirmed = false;
         popups.pop_back();
      }
   } else {
      okayButton.update();

      if (okayButton.clicked || handleKeyReleaseWithSound(KEY_ENTER) || handleKeyReleaseWithSound(KEY_ESCAPE)) {
         popups.pop_back();
      }
   }
}

// Render function

void renderPopups() {
   drawRect(Fade(BLACK, alpha));

   if (popups.empty()) {
      return;
   }

   Popup &popup = popups.back();
   drawTexture(getTexture("popup_frame"), getScreenCenter(), popupSize);
   drawText(getScreenCenter({0.0f, -110.0f}), popup.header.c_str(), 50.0f);

   wrapText(popup.body, popupSize.x - 30.0f, 25.0f, 1.0f);
   drawText(getScreenCenter({0.0f, -40.0f}), popup.body.c_str(), 25.0f);

   if (popup.confirmation) {
      confirmationButton.render();
      denialButton.render();
   } else {
      okayButton.render();
   }
}
