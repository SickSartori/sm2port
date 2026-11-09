# Changelog

## Unreleased

### New

- **PlayStation, Switch and other controllers** work directly, through SDL3, with the buttons in the Xbox positions:
  no more Steam Input or DS4Windows. Xbox controllers work as before.

### Improved

- A disc of another release is refused with its region named (North America, Japan, Europe), in the message and in the
  log, so a report says which version the player has.

### Improved

- Sound effects played at a pitch other than their own (engines, most loops) are cleaner: the mixer resamples them over
  four frames (Catmull-Rom) instead of two, which left copies of the sound in the top octave. Music is unchanged.
- The emulated graphics card takes a draw's indices and inline vertices as one block instead of word by word.

### Fixed

- **Frame limiters and overlays** (RivaTuner Statistics Server, MSI Afterburner) made the game judder: they wait inside
  the call that shows a frame, which also ran the emulated graphics card. That call now has a thread of its own.

## 0.1.2

### New

- **Your files are easy to find**: saves, settings, logs and mods now go in `Documents\sm2port`. If you played an earlier version,
  your files stay where they were, in `%LOCALAPPDATA%\sm2port`, until the launcher's first page moves them on request;
  nothing is overwritten.

### Removed

- The 60 fps option: it did not run right yet, so the game runs at 30 fps, like the Xbox. The setting in
  `sm2port.ini` and the launcher rows are gone; an old `frame_rate` line is ignored.

### Fixed

- Textures are sharper: the game asks for slightly sharper texture detail on many surfaces (the filter's level-of-detail
  bias), and it was not applied, so they came out a level too blurred.
- Started by RecompLauncher, the game keeps its saves, settings and logs in its install folder from the first start, so
  they no longer seem to vanish when RecompLauncher writes its settings file.

## 0.1.1

### New

- **Crash reports**: the first start after a crash puts the log and the crash dump in `sm2port-crash-report.zip` in the logs
  folder, and the launcher says "The game crashed last time" with a link to report it.
- **Logs folder**: `logs` keeps the log of this start and of the two before (`sm2port.1.log`, `sm2port.2.log`). The log and
  the dump of 0.1.0 are moved there by themselves. The launcher's About page opens the folder.
- **Lost graphics card**: the game restarts once, with a message; the second time it tells you to update the driver or
  lower the resolution.
- **Mods**: files in `mods\<path on the disc>`, in the data folder, replace the disc's files (read-only).
- Messages in the log for stalls and late frames, and for a frame limiter such as RTSS, to help with reports of stutter.
- New music for the launcher, made for sm2port.
- An installer (`sm2port-<version>-windows-x86-setup.exe`) beside the portable zip, with sm2port's own pictures.
- `recomplauncher.json`, so that RecompLauncher can start the game and offer its main settings.

### Fixed

- **The sky**: the blocks of other colours are gone. The stars are drawn as points instead of filled shapes, the sky's
  dome fills the frame, and light blends that use a constant alpha now use it.
- Frames the game did not change are no longer presented again, so forced V-Sync or a frame limiter (RTSS) no longer
  slows down or blocks the game.
- The times shown for saves follow the time zone of Windows (they were in UTC).
- Region-free or re-signed dumps of the same game (same code, different executable header) are accepted.
- In the launcher, the A button works as soon as a page opens (it did not on the Exit page of the in-game menu).

### Improved

- The launcher's background no longer has horizontal light trails; the rain and the spider webs remain.
- The settings `hud_aspect`, `hud_style` and `hud_units`, and the speed blur and light streaks, are gone: the game does
  not use them, and in 16:9 the first could narrow parts of the picture.
- The program is built with link-time optimisation, and the project has automated tests (`ctest`).

### Known issues

- Some textures look wrong in places (blurred or stretched surfaces, odd stripes on some buildings).
- Some sound calls (3D positioning) are not emulated yet.
- 60 fps is experimental.

## 0.1.0 (first test version)

First public test of sm2port.

### New

- Spider-Man 2 for the Xbox runs on Windows: the Italian and the European (French, German, Spanish) releases, from the
  title screen to the missions, with the films, the music and the sound effects.
- Widescreen 16:9 (the game's own mode) showing the whole 576-line PAL picture, 30 or 60 fps, internal resolution up to
  4K, anisotropic filtering and optional modern effects.
- Keyboard and mouse: WASD to move, the mouse (or the arrow keys) for the camera, rebindable keys, and Xbox controllers.
- A launcher with new artwork and spider-web animations, an in-game menu (F1) and a new program icon.
- DirectSound entry points of this game replaced by the port's mixer; entry points not identified yet are logged.

### Known issues

- Rectangular blocks of other colours in the sky in some views.
- Some sound calls (3D positioning) are not emulated yet.
- 60 fps is experimental.
