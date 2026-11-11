#pragma once

// The keyboard keys that stand in for controller 1's buttons and sticks, editable in the launcher and stored in
// sm2port.ini (keyboard = "a:32 b:27 ..." with Windows virtual-key codes).

#include <cstdint>

namespace xport::pc {

enum class PadInput : uint8_t {
    kRightTrigger, kLeftTrigger, kMoveLeft, kMoveRight, kMoveUp, kMoveDown,
    kA, kB, kX, kY, kLeftThumb, kBlack, kWhite,
    kStart, kBack, kDpadUp, kDpadDown, kDpadLeft, kDpadRight,
    kLookLeft, kLookRight, kLookUp, kLookDown, kRightThumb,
    kCount
};
constexpr int kPadInputs = static_cast<int>(PadInput::kCount);

struct KeyBinding {
    PadInput input;
    const char* id;      // in sm2port.ini
    const char* button;  // the controller part, as printed on the pad
    const char* english;
    const char* italian;
    const char* portuguese;
    const char* spanish;
    int default_key;
};

const KeyBinding& key_binding(PadInput input);
int bound_key(PadInput input);                // its Windows virtual-key code
void bind_key(PadInput input, int virtual_key);  // and saves all bindings
void reset_key_bindings();
void load_key_bindings();  // these and the controller's buttons below, from the settings (after settings_load)

// The controller's buttons can trade places: each button the game sees comes from a button of the PC's controller,
// by default the same one (sm2port.ini: controller = "a:b b:a ...").
enum class PadButton : uint8_t {
    kA, kB, kX, kY, kWhite, kBlack, kLeftTrigger, kRightTrigger, kLeftThumb, kRightThumb,
    kStart, kBack, kDpadUp, kDpadDown, kDpadLeft, kDpadRight,
    kCount
};
constexpr int kPadButtons = static_cast<int>(PadButton::kCount);

struct PadButtonBinding {
    PadButton button;
    const char* id;    // in sm2port.ini
    const char* icon;  // as printed on the pad (the launcher draws it)
    const char* english;
    const char* italian;
    const char* portuguese;
    const char* spanish;
};

const PadButtonBinding& pad_button_binding(PadButton button);
PadButton pad_button_source(PadButton game);          // the PC controller's button that gives `game`
void bind_pad_button(PadButton game, PadButton source);  // whatever had `source` gets `game`'s; saves
void reset_pad_buttons();

}  // namespace xport::pc
