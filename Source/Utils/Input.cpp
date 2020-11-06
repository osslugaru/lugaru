/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Utils/Input.hpp"

bool Input::keyDown[Input::KEYMAX];
bool Input::keyPressed[Input::KEYMAX];
SDL_GameController* Input::controller = nullptr;
int Input::joyid = -1;

void Input::Tick()
{
    SDL_PumpEvents();
    int numkeys;
    const Uint8* keyState = SDL_GetKeyboardState(&numkeys);
    for (int i = 0; i < numkeys; i++) {
        keyPressed[i] = !keyDown[i] && keyState[i];
        keyDown[i] = keyState[i];
    }
    Uint8 mb = SDL_GetMouseState(NULL, NULL);
    for (int i = 1; i < 6; i++) {
        keyPressed[SDL_NUM_SCANCODES + i] = !keyDown[SDL_NUM_SCANCODES + i] && (mb & SDL_BUTTON(i));
        keyDown[SDL_NUM_SCANCODES + i] = (mb & SDL_BUTTON(i));
    }
    if (controller) {
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
            int j = SDL_NUM_SCANCODES + 6 + i;
            bool pressed = SDL_GameControllerGetButton(controller, SDL_GameControllerButton(i));
            keyPressed[j] = !keyDown[j] && pressed;
            keyDown[j] = pressed;
        }
        for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; i++) {
            Sint16 state = SDL_GameControllerGetAxis(controller, SDL_GameControllerAxis(i));
            bool negative = (state < -Input::AXIS_THRESHOLD);
            bool positive = (state > Input::AXIS_THRESHOLD);
            int j = SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * i;
            keyPressed[j] = !keyDown[j] && positive;
            keyDown[j] = positive;
            j++;
            keyPressed[j] = !keyDown[j] && negative;
            keyDown[j] = negative;
        }
    }
}

void Input::loadGamepads()
{
    printf("There are %d gamepads\n", SDL_NumJoysticks());
    /* Open the first available controller */
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (loadGamepad(i)) {
            return;
        }
    }
}

bool Input::loadGamepad(int i)
{
    const char *name = SDL_GameControllerNameForIndex(i);
    if (name) {
        printf("Opening gamepad %i (%s)\n", i, name);
    } else {
        printf("Opening gamepad %i\n", i);
    }
    if (SDL_IsGameController(i)) {
        SDL_GameController* controllertmp = SDL_GameControllerOpen(i);
        if (controllertmp) {
            if (controller == nullptr) {
                SDL_Joystick *joy = SDL_GameControllerGetJoystick(controller);
                joyid = SDL_JoystickInstanceID(joy);
                controller = controllertmp;
            }
            return true;
        } else {
            fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
            return false;
        }
    } else {
        fprintf(stderr, "Controller not supported or invalid %i\n", i);
        return false;
    }
}

void Input::removeGamepad(int id)
{
    if ((joyid == id) && controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
}

void Input::closeGamepads()
{
    removeGamepad(joyid);
}

int Input::getScancode(SDL_Event evenement)
{
    switch (evenement.type) {
        case SDL_KEYDOWN:
            return evenement.key.keysym.scancode;
            break;
        case SDL_MOUSEBUTTONDOWN:
            return SDL_NUM_SCANCODES + evenement.button.button;
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            return SDL_NUM_SCANCODES + 6 + evenement.cbutton.button;
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (evenement.caxis.value > Input::AXIS_THRESHOLD) {
                return SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * evenement.caxis.axis;
            } else if (evenement.caxis.value < -Input::AXIS_THRESHOLD) {
                return SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX + 2 * evenement.caxis.axis + 1;
            }
            break;
        default:
            break;
    }
    return -1;
}

bool Input::isKeyDown(int k)
{
    if (k >= KEYMAX) {
        return false;
    }
    return keyDown[k];
}

bool Input::isKeyPressed(int k)
{
    if (k >= KEYMAX) {
        return false;
    }
    return keyPressed[k];
}

const char* Input::keyToChar(unsigned short i)
{
    if (i < SDL_NUM_SCANCODES) {
        return SDL_GetKeyName(SDL_GetKeyFromScancode(SDL_Scancode(i)));
    } else if (i == MOUSEBUTTON_LEFT) {
        return "mouse left button";
    } else if (i == MOUSEBUTTON_RIGHT) {
        return "mouse right button";
    } else if (i == MOUSEBUTTON_MIDDLE) {
        return "mouse middle button";
    } else if (i == MOUSEBUTTON_X1) {
        return "mouse button 4";
    } else if (i == MOUSEBUTTON_X2) {
        return "mouse button 5";
    } else if (i < SDL_NUM_SCANCODES + 6 + SDL_CONTROLLER_BUTTON_MAX) {
        return SDL_GameControllerGetStringForButton(SDL_GameControllerButton(i - SDL_NUM_SCANCODES - 6));
    } else {
        return SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis((i - SDL_NUM_SCANCODES - 6 - SDL_CONTROLLER_BUTTON_MAX) / 2));
    }
}

bool Input::MouseClicked()
{
    return isKeyPressed(MOUSEBUTTON_LEFT);
}

void Input::handleEvent(SDL_Event event)
{
    switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            loadGamepad(event.cdevice.which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            removeGamepad(event.cdevice.which);
            break;
    }
}
