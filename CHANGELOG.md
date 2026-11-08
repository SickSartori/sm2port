# Changelog

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
