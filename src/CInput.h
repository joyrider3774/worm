#ifndef CINPUT_H
#define CINPUT_H

#define MAXJOYSTICKS 10
#define MAXJOYSTICKBUTTONS 37
#define MAXSPECIALKEYS 1
#define MAXMOUSEBUTTONS 32
#define MAXMOUSES 10

#define SPECIAL_QUIT_EV 0

#define JOYSTICKDEADZONE 15000

#define JOYSTICK_LEFT MAXJOYSTICKBUTTONS-2
#define JOYSTICK_UP MAXJOYSTICKBUTTONS-3
#define JOYSTICK_RIGHT MAXJOYSTICKBUTTONS-4
#define JOYSTICK_DOWN MAXJOYSTICKBUTTONS-5
#define JOYSTICK_NONE MAXJOYSTICKBUTTONS-1

#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL.h>

class CInput {
    private:
        int GetJoystickNr(SDL_JoystickID id);
        int UpdateCounter;
        int PUpdateCounterDelay;
    public:
        SDL_Joystick *Joysticks[MAXJOYSTICKS];
        int OpenJoystickCount;
        bool _JoystickHeld[MAXJOYSTICKS][MAXJOYSTICKBUTTONS];
        bool _SpecialsHeld[MAXSPECIALKEYS];
        bool _KeyboardHeld[SDL_SCANCODE_COUNT];        
        bool _MouseHeld[MAXMOUSES][MAXMOUSEBUTTONS];
        CInput(int UpdateCounterDelay, bool DisableJoysticks);
        ~CInput(void);
        void Update();
        void Reset();
        void SetInputDelay(int UpdateCounterDelay) { PUpdateCounterDelay = UpdateCounterDelay;};
        bool Ready(){ return (UpdateCounter == 0);};
        void Delay(){ UpdateCounter = PUpdateCounterDelay;};
        bool KeyboardHeld(SDL_Keycode code){return _KeyboardHeld[(int)SDL_GetScancodeFromKey(code, NULL)];};
        bool JoystickHeld(int JoystickNr, int JoystickButton) { return _JoystickHeld[JoystickNr][JoystickButton];};
        bool SpecialsHeld(int SpecialKey) {return _SpecialsHeld[SpecialKey];};
        bool MouseHeld(int MouseNr, int MouseButton){return _MouseHeld[MouseNr][MouseButton];};
};

#endif
