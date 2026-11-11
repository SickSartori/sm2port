#include "pc/keyboard_bindings.h"

#include <windows.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "pc/settings.h"

namespace xport::pc {
namespace {

// Mouse buttons are virtual keys too (VK_LBUTTON...): the mouse drives the camera (input_hle.cpp, mouse_look).
const KeyBinding kBindings[kPadInputs] = {
    {PadInput::kRightTrigger, "rt", "RT", "Right trigger", "Grilletto destro", VK_RBUTTON},
    {PadInput::kLeftTrigger, "lt", "LT", "Left trigger", "Grilletto sinistro", 'Q'},
    {PadInput::kMoveLeft, "move_left", "L ←", "Move left", "Muovi a sinistra", 'A'},
    {PadInput::kMoveRight, "move_right", "L →", "Move right", "Muovi a destra", 'D'},
    {PadInput::kMoveUp, "move_up", "L ↑", "Move forward", "Muovi avanti", 'W'},
    {PadInput::kMoveDown, "move_down", "L ↓", "Move back", "Muovi indietro", 'S'},
    {PadInput::kA, "a", "A", "Jump", "Salta", VK_SPACE},
    {PadInput::kB, "b", "B", "Cling to walls", "Aggrappati ai muri", 'E'},
    {PadInput::kX, "x", "X", "Attack", "Attacca", VK_LBUTTON},
    {PadInput::kY, "y", "Y", "Y button", "Pulsante Y", 'R'},
    {PadInput::kLeftThumb, "ls", "LS", "Left stick click", "Clic leva sinistra", 'C'},
    {PadInput::kBlack, "black", "Black", "Black button", "Pulsante nero", VK_LSHIFT},
    {PadInput::kWhite, "white", "White", "White button", "Pulsante bianco", VK_LCONTROL},
    {PadInput::kStart, "start", "Start", "Pause", "Pausa", VK_RETURN},
    {PadInput::kBack, "back", "Back", "Back", "Indietro", VK_BACK},
    {PadInput::kDpadUp, "up", "D-pad ↑", "D-pad up", "Croce su", 'I'},
    {PadInput::kDpadDown, "down", "D-pad ↓", "D-pad down", "Croce giù", 'K'},
    {PadInput::kDpadLeft, "left", "D-pad ←", "D-pad left", "Croce sinistra", 'J'},
    {PadInput::kDpadRight, "right", "D-pad →", "D-pad right", "Croce destra", 'L'},
    {PadInput::kLookLeft, "look_left", "R ←", "Camera left", "Telecamera sinistra", VK_LEFT},
    {PadInput::kLookRight, "look_right", "R →", "Camera right", "Telecamera destra", VK_RIGHT},
    {PadInput::kLookUp, "look_up", "R ↑", "Camera up", "Telecamera su", VK_UP},
    {PadInput::kLookDown, "look_down", "R ↓", "Camera down", "Telecamera giù", VK_DOWN},
    {PadInput::kRightThumb, "rs", "RS", "Right stick click", "Clic leva destra", VK_MBUTTON},
};

const PadButtonBinding kPadButtonBindings[kPadButtons] = {
    {PadButton::kA, "a", "A", "Jump", "Salta"},
    {PadButton::kB, "b", "B", "Cling to walls", "Aggrappati ai muri"},
    {PadButton::kX, "x", "X", "Attack", "Attacca"},
    {PadButton::kY, "y", "Y", "Y button", "Pulsante Y"},
    {PadButton::kWhite, "lb", "LB", "LB (White)", "LB (Bianco)"},
    {PadButton::kBlack, "rb", "RB", "RB (Black)", "RB (Nero)"},
    {PadButton::kLeftTrigger, "lt", "LT", "Left trigger", "Grilletto sinistro"},
    {PadButton::kRightTrigger, "rt", "RT", "Right trigger", "Grilletto destro"},
    {PadButton::kLeftThumb, "ls", "LS", "Left stick click", "Clic leva sinistra"},
    {PadButton::kRightThumb, "rs", "RS", "Right stick click", "Clic leva destra"},
    {PadButton::kStart, "start", "Start", "Pause", "Pausa"},
    {PadButton::kBack, "back", "Back", "Back", "Indietro"},
    {PadButton::kDpadUp, "up", "D-pad ↑", "D-pad up", "Croce su"},
    {PadButton::kDpadDown, "down", "D-pad ↓", "D-pad down", "Croce giù"},
    {PadButton::kDpadLeft, "left", "D-pad ←", "D-pad left", "Croce sinistra"},
    {PadButton::kDpadRight, "right", "D-pad →", "D-pad right", "Croce destra"},
};
std::atomic<uint8_t> g_pad_sources[kPadButtons];  // read by the input thread

void save_pad_buttons() {
    std::string text;
    for (const PadButtonBinding& b : kPadButtonBindings) {
        if (!text.empty()) text += ' ';
        text += b.id;
        text += ':';
        text += kPadButtonBindings[g_pad_sources[static_cast<int>(b.button)].load()].id;
    }
    setting_save_text("controller", text);
}

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

const PadButtonBinding& pad_button_binding(PadButton button) { return kPadButtonBindings[static_cast<int>(button)]; }

PadButton pad_button_source(PadButton game) { return static_cast<PadButton>(g_pad_sources[static_cast<int>(game)].load()); }

void bind_pad_button(PadButton game, PadButton source) {
    const uint8_t previous = g_pad_sources[static_cast<int>(game)].load();
    for (std::atomic<uint8_t>& s : g_pad_sources) {
        if (s.load() == static_cast<uint8_t>(source)) s = previous;
    }
    g_pad_sources[static_cast<int>(game)] = static_cast<uint8_t>(source);
    save_pad_buttons();
}

void reset_pad_buttons() {
    for (int i = 0; i < kPadButtons; ++i) g_pad_sources[i] = static_cast<uint8_t>(i);
    save_pad_buttons();
}

void load_key_bindings() {
    for (int i = 0; i < kPadButtons; ++i) g_pad_sources[i] = static_cast<uint8_t>(i);
    std::istringstream pairs(setting_text("controller"));
    for (std::string pair; pairs >> pair;) {
        const size_t colon = pair.find(':');
        if (colon == std::string::npos) continue;
        int game = -1, source = -1;
        for (int i = 0; i < kPadButtons; ++i) {
            if (pair.compare(0, colon, kPadButtonBindings[i].id) == 0) game = i;
            if (pair.compare(colon + 1, std::string::npos, kPadButtonBindings[i].id) == 0) source = i;
        }
        if (game >= 0 && source >= 0) g_pad_sources[game] = static_cast<uint8_t>(source);
    }
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

}  // namespace xport::pc
