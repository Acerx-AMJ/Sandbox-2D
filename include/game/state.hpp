#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

// Constants

constexpr float fixedUpdateDT = 1.0f / 60.f;

// Polymorphic state

struct State {
   State() = default;
   virtual ~State() = default;

   // Virtual functions

   virtual void update() = 0;
   virtual void fixedUpdate() = 0;
   virtual void render() const = 0;
   virtual State* change() = 0;

   // Update functions

   void updateStateLogic();
   void updateFadingIn();
   void updateFadingOut();

   // Members

   bool quitState = false;
   bool fadingIn = true;
   bool fadingOut = false;

   float fadeTimer = 0.0f;
   float alpha = 0.0f;
   float accumulator = 0.0f;
   float realDt = 0.0f; // Real DT should be used for timers, whereas DT for everything else
   float dt = 0.0f;
};

#endif
