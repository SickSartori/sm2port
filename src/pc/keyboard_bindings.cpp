#include "pc/keyboard_bindings.h"

#include <windows.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "pc/settings.h"

namespace sm2::pc {
namespace {

// Mouse buttons are virtual keys too (VK_LBUTTON...): the mouse drives the camera (input_hle.cpp, mouse_look).
const KeyBinding kBindings[kPadInputs] = {
    {PadInput::kRightTrigger, "rt", "RT", "Right trigger", "Grilletto destro", VK_RBUTTON},
    {PadInput::kLeftTrigger, "lt", "LT", "Left trigger", "Grilletto sinistro", 'Q'},
    {PadInput::kMoveLeft, "move_left", "L â", "Move left", "Muovi a sinistra", 'A'},
    {PadInput::kMoveRight, "move_right", "L â", "Move right", "Muovi a destra", 'D'},
    {PadInput::kMoveUp, "move_up", "L â", "Move forward", "Muovi avanti", 'W'},
    {PadInput::kMoveDown, "move_down", "L â", "Move back", "Muovi indietro", 'S'},
    {PadInput::kA, "a", "A", "Jump", "Salta", VK_SPACE},
    {PadInput::kB, "b", "B", "Cling to walls", "Aggrappati ai muri", 'E'},
    {PadInput::kX, "x", "X", "Attack", "Attacca", VK_LBUTTON},
    {PadInput::kY, "y", "Y", "Y button", "Pulsante Y", 'R'},
    {PadInput::kLeftThumb, "ls", "LS", "Left stick click", "Clic leva sinistra", 'C'},
    {PadInput::kBlack, "black", "Black", "Black button", "Pulsante nero", VK_LSHIFT},
    {PadInput::kWhite, "white", "White", "White button", "Pulsante bianco", VK_LCONTROL},
    {PadInput::kStart, "start", "Start", "Pause", "Pausa", VK_RETURN},
    {PadInput::kBack, "back", "Back", "Back", "Indietro", VK_BACK},
    {PadInput::kDpadUp, "up", "D-pad â", "D-pad up", "Croce su", 'I'},
    {PadInput::kDpadDown, "down", "D-pad â", "D-pad down", "Croce giÃ¹", 'K'},
    {PadInput::kDpadLeft, "left", "D-pad â", "D-pad left", "Croce sinistra", 'J'},
    {PadInput::kDpadRight, "right", "D-pad â", "D-pad right", "Croce destra", 'L'},
    {PadInput::kLookLeft, "look_left", "R â", "Camera left", "Telecamera sinistra", VK_LEFT},
    {PadInput::kLookRight, "look_right", "R â", "Camera right", "Telecamera destra", VK_RIGHT},
    {PadInput::kLookUp, "look_up", "R â", "Camera up", "Telecamera su", VK_UP},
    {PadInput::kLookDown, "look_down", "R â", "Camera down", "Telecamera giÃ¹", VK_DOWN},
};

std::atomic<int> g_keys[kPadInputs];  // read by the input thread

void save() {
    std::string text;
    for (const KeyBinding& b : kBindings) {
        char item[48];
        std::snprintf(item, sizeof(item), "%s%s:%d", text.empty() ? "" : " ", b.id, g_keys[static_cast<int>(b.input)].load());
        text += item;
    }
    setting_save_text("keyboard", text);
}

}  // namespace

const KeyBinding& key_binding(PadInput input) { return kBindings[static_cast<int>(input)]; }

int bound_key(PadInput input) { return g_keys[static_cast<int>(input)].load(); }

void bind_key(PadInput input, int virtual_key) {
    // A key does one thing: whatever had it before gets the key this input had.
    const int previous = bound_key(input);
    for (std::atomic<int>& key : g_keys) {
        if (key.load() == virtual_key) key = previous;
    }
    g_keys[static_cast<int>(input)] = virtual_key;
    save();
}

void reset_key_bindings() {
    for (const KeyBinding& b : kBindings) g_keys[static_cast<int>(b.input)] = b.default_key;
    save();
}

void load_key_bindings() {
    for (const KeyBinding& b : kBindings) g_keys[static_cast<int>(b.input)] = b.default_key;
    std::istringstream items(setting_text("keyboard"));
    for (std::string item; items >> item;) {
        const size_t colon = item.find(':');
        if (colon == std::string::npos) continue;
        const std::string id = item.substr(0, colon);
        const int key = std::atoi(item.c_str() + colon + 1);
        for (const KeyBinding& b : kBindings) {
            if (id == b.id && key > 0 && key < 256) g_keys[static_cast<int>(b.input)] = key;
        }
    }
}

}  // namespace sm2::pc
