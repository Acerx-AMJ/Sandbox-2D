#include "mngr/input.hpp"
#include "ui/button.hpp"
#include "ui/popup.hpp"
#include "SRU/audio.hpp"
#include "SRU/assets.hpp"
#include "SRU/render.hpp"
#include "SRU/text.hpp"
#include "SRU/util.hpp"
#include <vector>

// Constants

constexpr Vector2 popupSize = V2(0.463f, 0.347f);
constexpr Vector2 headerPosition = V2(0.5f, 0.12f);
constexpr Vector2 bodyPosition = V2(0.5f, 0.21f);
constexpr Vector2 denialPosition = V2(0.275f, 0.85f);
constexpr Vector2 confirmationPosition = V2(0.725f, 0.85f);
constexpr Vector2 okayPosition = V2(0.5f, 0.85f);

constexpr float fadeTime = 0.6f;
constexpr float headerFontSize = 50.0f;
constexpr float bodyFontSize = 25.0f;

// Globals

static std::vector<Popup> popups;
static bool wasLastPopupConfirmed = false;
static float fadeTimer = 0.0f;
static float alpha = 0.0f;
static bool fadedIn = false, fadedOut = true;

static Rectangle rect;
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

// Init functions

void initPopups() {
   Texture texture = getTexture("button");
   Font font = getFont("andy");

   confirmationButton.init(font, texture, CENTER, "YES");
   denialButton.init(font, texture, CENTER, "NO");
   okayButton.init(font, texture, CENTER, "OKAY");
   updatePopupResponsiveness();
}

// Popup functions

void insertPopup(const std::string &header, const std::string &body, PopupType type) {
   std::string wrappedBody = wrap(body, getFont("andy"), rect.width - mapRatioToWidth(0.05f, rect, CUBIC_RATIO), getFontSizeScaled(bodyFontSize));
   popups.emplace_back(header, wrappedBody, type);

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

// Update

void updatePopupResponsiveness() {
   rect = mapRatioToArea(R4(CENTER, popupSize), CENTER, WINDOW_AREA, CUBIC_RATIO);

   Vector2 size = mapRatioToArea(buttonSize, WINDOW_AREA, CUBIC_RATIO);
   denialButton.rect = R4(mapRatioToArea(denialPosition, rect), size);
   confirmationButton.rect = R4(mapRatioToArea(confirmationPosition, rect), size);
   okayButton.rect = R4(mapRatioToArea(okayPosition, rect), size);
}

void updatePopups(float dt) {
   if (popups.empty()) {
      fadeOut(dt);
      return;
   }

   fadeIn(dt);
   Popup &popup = popups.back();

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
   drawRect(getWindowArea(), TOP_LEFT, Fade(BLACK, alpha));
   if (popups.empty()) {
      return;
   }
   Font font = getFont("andy");
   Popup &popup = popups.back();

   drawTexture("popup_frame", rect, TOP_LEFT);
   drawTextResponsive(font, headerPosition, popup.header.c_str(), headerFontSize, CENTER, WHITE, rect);
   drawTextResponsive(font, bodyPosition, popup.body.c_str(), bodyFontSize, TOP_CENTER, WHITE, rect);

   if (popup.type == PopupType::confirmation) {
      confirmationButton.render();
      denialButton.render();
   } else {
      okayButton.render();
   }
}
