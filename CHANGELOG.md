# Changelog

## 0.1.4

### New

- **A fixed time of day**: dawn, day, sunset or night, chosen in the Game settings (in the launcher and in
  the in-game menu). The sky and the city's lights stay at that hour; Game keeps the game's own clock.
- **The HUD keeps its shape in 16:9**: the game draws it with the proportions of 4:3, so the health ring came out a
  third too wide. Setting "Round HUD in 16:9" in Graphics, on by default.
- **Field of view**, from 75% to 130% of the game's own (Graphics); the camera still widens and narrows it as it goes.
- **F7 hides the HUD** and shows it again, for photos and videos.
- **The controller's buttons can be moved around** (Controls, Controller tab): each button of the game comes from the
  one of your pad you choose.
- **The right stick click on the keyboard** (middle mouse button by default).
- **A new version says what is new** when the launcher opens, from its release notes. Update downloads the installer
  with its progress and checks it against SHA256SUMS; Install starts it and closes the launcher. Nothing installs by
  itself, and a portable copy opens the release page instead.
- **Back to the launcher** from the in-game menu (F1): the game closes and the launcher opens again, where the disc
  and the settings that need a restart can be changed.
- **The launcher in Portuguese (Brazil) and Spanish**, keys and buttons included, after Windows' language.

### Improved

- The ambient occlusion (one of the modern effects) no longer leaves a fine grain on bright surfaces in the areas it
  darkens: it uses the same directions for every pixel.
- "Sharp HUD pictures" now covers the health ring and the tutorials' boxes too.
- The HUD keeps its shape in 16:9 also for the minimap and the grey tab beside the health ring.
- The Controls page in tabs (keyboard, controller, PC keys), the keyboard keys in groups: the PC keys were below the
  window's edge. "Down" in Italian and the arrows of the key names showed as odd characters.
- About says where to find the licences, the credits and how to contribute: on GitHub.
- sm2port's shared runtime is now open source, as [xport](https://github.com/SickSartori/xport) (MIT licence), so
  every line of the Windows files can be read and built from public code.

### Fixed

- **The music no longer disappears** after the opening film or the pause menu, and dialogue playing at that moment no
  longer goes silent. The game resumes its sounds with a value the port took as one more pause, and a sound stopped
  while paused kept its channel for ever.

## 0.1.3

### New

- **60 fps is back**, at the right speed: the game's own 59.94 fps mode, with the vertical blanks left at 60 Hz. The
  earlier attempt (0.1.1) also doubled the blanks to 120 Hz, so the game drew 120 frames a second with a 1/60 s step
  and ran at double speed.
- **The World release** of the game (`default.xbe` SHA-256 `e8353bf6...4313af`) is supported: its code is the Italian
  build's, placed a few hundred bytes further on, so every patch and redirected function moves with it.
- **PlayStation, Switch and other controllers** work directly, through SDL3, with the buttons in the Xbox positions:
  no more Steam Input or DS4Windows. Xbox controllers work as before.

### Improved

- **Overlays no longer break the textures**: RivaTuner Statistics Server, Steam, Discord and MSI Afterburner draw
  inside the call that shows a frame, and since that call has a thread of its own they drew with the game's Direct3D
  device while the game was drawing. The window now presents with a device of its own (as mc3port 0.3.6 does).
- Two faults in the game's Direct3D library that fast frames can trigger are closed, as in mc3port: a wait for the
  graphics card that could spin forever, and a frame that waited for the blank counter to come round. The library also
  reaches the graphics card's queue without a trap each time.
- Music and other streams are resampled with the same curve as sound effects (clearer top octave), and a stream told
  to end before its first data no longer stops.
- Compiled pixel shaders are kept in the shader cache too, so a place seen before no longer stutters while they are
  made again. On integrated graphics the automatic resolution stops at 1080 lines.
- sm2port now shares its game-independent runtime and its Direct3D 11 renderer with mc3port, so fixes to either reach
  both.
- On laptops with two graphics cards the game runs on the fast one, not on Windows' default (the integrated one), unless
  Windows' graphics settings say otherwise for `sm2.exe`.
- A disc of another release is refused with its region named (North America, Japan, Europe), in the message and in the
  log, so a report says which version the player has.
- Textures no draw has used for a minute are let go, so integrated graphics no longer run out of memory after a long
  session (on mc3port, new textures then came out black). The log's renderer line says how many are kept.
- A large texture that is written in one place (another texture or vertices sharing its memory) is checked page by
  page, not whole.
- The game no longer waits for the display after a slow frame (a laptop whose fast GPU draws into a display wired to
  the integrated one sat at 30 fps).
- Things drawn with the lights on but without lighting normals keep their own colour, as Direct3D does, instead of
  coming out almost black.
- Settings are written to a temporary file and renamed, so a crash while saving never leaves an empty `sm2port.ini`.
- The update check only follows links into sm2port's own releases.
- The release workflow uses fixed versions of its tools and checks the tag and the notes before building.
- Sound effects played at a pitch other than their own (engines, most loops) are cleaner: the mixer resamples them over
  four frames (Catmull-Rom) instead of two, which left copies of the sound in the top octave.
- The emulated graphics card takes a draw's indices and inline vertices as one block instead of word by word.

### Fixed

- **The mouse was held at the middle of the window** while the game ran: mouse look now reads the mouse's movement
  directly (raw input), so the cursor goes where you move it, and hides only after 3 s without moving.
- **Frame limiters and overlays** (RivaTuner Statistics Server, MSI Afterburner) made the game judder: they wait inside
  the call that shows a frame, which also ran the emulated graphics card. That call now has a thread of its own.
- **The game's clocks start with the game**, like an Xbox's at power on, not with Windows. Counted from Windows' start,
  after a few days of uptime a clock kept as a decimal number can no longer take a 1/60 s step and stands still (on
  mc3port this froze a menu at 60 fps). Windows' fast startup keeps that count across a normal shutdown.
- The Back+Start combo for the in-game menu is read only from what the game sees, so a quick press is no longer lost.
- A timed wait in the kernel ends at its timeout even when only some of the objects it waits for are set.
- A write just past one of the game's buffers lands in a fresh page, as a read there already did, instead of crashing.
- Damaged disc images and executables (sizes or counts out of range) are refused cleanly instead of being read past
  their end, and file names an Xbox drive cannot hold are refused instead of reaching Windows.
- Textures without a palette no longer get a new cache entry every time the palette address changes (the cache grew
  without end).

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
