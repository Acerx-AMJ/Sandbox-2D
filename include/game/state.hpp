#pragma once
constexpr float fixedUpdateDT = 1.0f / 60.f;

struct State {
   State() = default;
   virtual ~State() = default;

   // Virtual functions

   virtual void update() = 0;
   virtual void fixedUpdate() {};
   virtual void updateResponsiveness() {}; // udpate UI element sizes based on window size

   virtual void render() = 0;
   virtual State* change() = 0;

   // built-in functions

   void updateStateLogic();
   void renderStateLogic();
   void updateFadingIn();
   void updateFadingOut();

   // Members

   int lastWidth = 0;
   int lastHeight = 0;

   bool quitState = false;
   bool fadingIn = true;
   bool fadingOut = false;

   float fadeTimer = 0.0f;
   float alpha = 0.0f;
   float accumulator = 0.0f;
   float realDt = 0.0f; // Real DT should be used for timers, whereas DT for everything else
   float dt = 0.0f;
};
