# Changelog

## 0.2.1

### New

- **Controller rumble** in the settings (Game), also in the in-game menu: how strongly the controllers rumble, from 0
  (off) to 100% of what the game asks, the default. It changes at the game's next rumble.
- **Gamma and brightness** in the settings (Effects), also in the in-game menu, for a screen that shows the game too
  dark or too light: they change the 3D scene only, at once, with the modern effects on or off; the HUD, the menus and
  the films stay as they are.

### Improved

- **More settings on RecompLauncher's page**: 120 frames per second, 21:9, the field of view, the draw distance, the
  texture filter, sharp textures, the modern effects and the frame counter, which the launcher and the in-game menu
  already had.

### Fixed

- **The final fight with Doc Ock with a fixed time of day**: it could start and go straight back to the flight to the
  warehouse. A fixed time of day was written into the game's clock every frame, also after a mission had set its own
  hour and timed itself on that clock. Now the clock is the mission's from the moment a script sets the time, until a
  save is loaded or the setting changes. Not tried in that fight: no save before it was at hand.
- **Back to the game's own time of day**: choosing Game again after a fixed hour left the clock at that hour, so a
  night stayed day. The game's hour from before is put back.
- **Crash on entering the city with the draw distance above 200%** (issue 9): the game keeps the objects near the
  camera in a list of 750, made for the Xbox's distances; with more of them in view the list ran past its end into the
  game's own memory, and it crashed at the start or soon after. The list now holds 4096, and 300% plays.
- **The window mode in the settings after F11 or Alt+Enter**: the row showed the mode in `sm2port.ini`, not the
  window's own, so after F11 the in-game menu could say "Window" in full screen. It now shows the window as it is.
  The settings also say that full screen has no border, which it always had: Alt+Tab is instant.

## 0.2.0

### New

- **Draw distance** (Settings, Graphics, 100% by default): how far away cars, pedestrians and the buildings' full
  detail are drawn, up to three times the game's own. On the Xbox, objects fade out between 20 and 280 metres
  depending on their size, and city blocks lose their detail beyond 250 metres at street level. Traffic and pedestrians also appear
  further away instead of popping up close. It costs frames: in tests at 60 fps, 200% took about a tenth of them.
- **21:9**: a third aspect ratio next to 4:3 and 16:9 in the launcher's and the in-game menu's settings. On a screen
  wider than 16:9 (21:9, 32:9) the game takes your screen's shape and sees more to the sides, as it does in 16:9
  compared with 4:3; the health ring and the minimap stay round. Films and loading screens are made for 16:9 and keep
  bars at the sides. The game's menus and texts, and the web counter, come out wider than they should.
- **120 frames per second**: a third choice next to 30 and 60 in the launcher's and the in-game menu's settings, as in
  mc3port. The game runs at the same speed, with up to twice the frames of 60. It needs a fast processor and a screen
  of 120 Hz or more; where the PC cannot keep up, fewer frames are shown and the game keeps its speed.
- **Sharp textures** (in the settings, next to Sharp HUD pictures, off by default), as in mc3port: the textures of streets, buildings and people
  enlarged two times with their edges kept, as Sharp HUD pictures does for the HUD. It shows from close; from afar the
  picture is the game's own. The textures sharpen a few seconds after a place loads, and take more graphics memory.
- **VSync** (Settings, Display, off by default), as in mc3port: frames are shown at once, and the picture can tear in fast
  camera moves. With VSync on it never tears, but a PC that cannot keep up may skip frames. It applies at the next start.

### Improved

- **More natural light and colour from the modern effects**: the glow, contrast, saturation and vignette now apply to
  the 3D scene before the HUD is drawn, so the HUD keeps its own colours. Contrast follows a curve that keeps
  detail in the darkest and brightest parts (before, dark corners lost their detail and bright surfaces burnt
  out), and the glow around lights is softer and wider.
- **The launcher's buttons**, as in mc3port: Report a problem and Support the project sit above the bottom bar, on
  every page and in the in-game menu, and a speaker at the left of the bar turns the launcher's music off and on
  (`launcher_mute` in `sm2port.ini`; the game's volume stays in the settings). The hints' keys are drawn as keyboard
  keys. Support goes to Ko-fi only.
- **First start**: after the disc and the quality the launcher opens its menu, with every page, instead of starting
  the game. The disc's row no longer cuts its text short.
- **A shorter black screen at start**: the game's loader reads its first 18 MB in 151 pieces and rested 30 ms after
  each one, waiting for the main thread too. It now rests 1 ms, and the Activision logo comes after about 4 to 6
  seconds instead of 7 to 11. The same thread loads the city while you play, now with less waiting between pieces.
- **Drawing on two threads**, as in mc3port: one reads the game's drawing commands, the other makes the picture with
  Direct3D. Here the gain is small (59 frames a second instead of 54 at 120 fps in the city, the two ways taking turns
  in the same flight), since that thread is not what limits Spider-Man 2. It needs four processor threads or more;
  `render_thread = 0` in `sm2port.ini` goes back to one thread, should the picture or the game misbehave. The two
  threads each have processor cores of their own, as in mc3port.
- **A little faster again where the PC works hardest**: for each draw the textures and the vertex state are found
  without being looked up again, a few percent less work on the thread that limits the frame rate.
- **Replacement pictures for any texture**, as in mc3port: `SM2_DUMP_TEXTURE=all` saves every texture the game
  draws with as a PNG in a folder `textures_dump` next to `sm2port.ini`. Redrawn or enlarged with a tool of your
  own and put in a folder `textures_replace` there under the same name (16 hex digits), at any size, they are
  drawn instead of the game's.
- **The launcher's new layout**, in the in-game menu too: each page sits in a column from the top of the window to the
  bottom bar, with Play at its foot. A page longer than the window scrolls (the mouse wheel, or the selection kept in
  sight with the keys and the controller). In a 4:3 or smaller window everything is larger than before (it was sized
  for 16:9 and shrank). A slider's value now sits beside its grab instead of under it, the rows of a page come in one
  after another, a light runs round Play when the game is ready and along the menu's selected item. The bottom bar no
  longer shows Enter twice on the keyboard. With the keys or a controller the selected row is lit, as in mc3port: on a
  choice already made (VSync on, say) nothing showed where the selection was. The internal resolution is a drop-down
  list, like the game language, instead of six narrow buttons.
- **Settings in five shorter tabs**: Game held nine rows. Game keeps the language, the time of day and the pause in the
  background; Display has the aspect ratio, field of view, window, frames per second, VSync and the frame counter;
  Graphics opens with the quality and the internal resolution; a new Audio tab has the two volumes. Mouse look and the
  mouse's speed are in Controls, in a Mouse tab of their own.
- **The launcher's webs** stretch towards the mouse pointer, and the spider lowers itself on its thread to the menu's
  selected item instead of going up and down by itself.

- **Fewer pauses the first time you reach a place**: the shaders a scene needs the first time are made on two helper
  threads instead of in the middle of a frame; a frame waits for them at most 8 ms, and what is still missing is
  drawn a frame or two later (as in mc3port, whose city entry ran at about 42 fps instead of 32 in tests).

- **Shadows fade out smoothly in the distance**: the game takes them from full to none within a short span of
  distance, which showed as a light arc around the camera on walls and pavements (and in 24 visible steps). The fade
  now spreads over about three times the distance, with no edge. This is a change from the original game.

- **A little lighter on the PC**: the work that turns each frame's Xbox drawing commands into Direct3D takes about 5%
  less time (more in Midnight Club 3, whose port shares this code).
- **An incomplete disc image is named in the log**, as in mc3port: when a file or folder of the image lies past its
  end (a download cut short), or a read from the disc fails, `sm2port.log` says so. Before, it showed only as a crash
  later.
- **Linux and macOS**, as in mc3port: at start the port checks that the picture it hands from one Direct3D device to
  the other really arrives, and shows it the plain way where it does not. Not yet tried on those systems.
- **Fewer antivirus false alarms**, as in mc3port. Started by RecompLauncher, `sm2.exe` no longer starts the game
  process suspended and then resumes it, which is how malware hides its code in another process: the game process
  starts as usual and joins `sm2.exe`'s job at once. `sm2.exe` and `sm2runtime.dll` also carry the checksum in
  their header, which was left empty.
- **Code signed files**: `sm2.exe`, `sm2runtime.dll`, the installer and its uninstaller are signed (Windows shows the
  publisher "Open Source Developer Gabriel Sartori"), which Windows and antivirus programs trust more than
  unsigned files. The certificate is new, so SmartScreen may still ask once until it has seen enough downloads.

### Changed

- **A new launcher music**, made the same way as the one before it and under the same licence (CC BY 4.0, see
  THIRD_PARTY.md).
- **Updating from the launcher**, as in mc3port: Update opens the new installer's link and your browser downloads it;
  you start it, as any download. The launcher no longer downloads and starts the installer itself, which antivirus
  programs take for the way malware brings in more of itself.
- **The keyboard and the mouse's buttons are read from the game's window**, no longer by asking Windows for the whole
  keyboard's state, which antivirus programs also distrust. Keys pressed in other windows no longer reach the game.
- **New default keys**, after the game's own buttons and Marvel's Spider-Man on PC: Shift sprints, Ctrl dodges and
  clings to walls, F is the web attack, Q grabs and picks up (B+Y), E is the web zip (LT+RT), R the Spider Reflexes,
  the middle mouse button recenters the camera, Tab locks it on, Esc pauses and M opens the map. The Keyboard tab names
  each key's action instead of the pad's button. Keys you changed yourself stay as they are; Restore keys gives the
  new ones.
- **Release files with names that say what they are**: `-windows-setup.exe` (the installer), `-windows-portable.zip`
  and `-linux.tar.gz`, and a "Which file?" list at the end of the release notes, right above the files. "x86" is gone from the names.

### Fixed

- **Steadier 120 fps**: the vertical blank comes from a high-resolution timer. With waits in whole milliseconds a blank
  could come late and the next one early, and a frame that needed nearly a whole blank missed it: in the city 119.6
  to 119.8 frames a second instead of 111.
- **Frame timing**: a vertical blank was at times counted just after the frame that came with it, so two frames of
  the same length were timed as one short and one long, for the game's clock and in the log.
- **Sharp HUD pictures on the HUD's smallest pictures**: those enlarged four times got one level too many, Direct3D
  refused them (`cannot create an enlarged texture` in the log) and they stayed as the game's. Now they are sharp
  like the others.
- **Pieces of the city behind the minimap moved**: a door, a sign or a piece of wall at the foot of a building that came
  out behind the minimap was shrunk towards the map's centre with it, so it seemed to slide out of place. Only the
  minimap's own pieces, drawn over the scene, are scaled now.
- **Decals on surfaces seen at an angle** (marks laid on walls and pavements) get the whole depth offset the game asks
  for: above 480 lines it was too weak, by up to 2.25 times at 1080. The offset is also switched on and off by the
  game's own setting, not by the line width.

## 0.1.6

### Improved

- **RecompLauncher** marks a disc image as verified when it is a Redump copy of a supported release: `recomplauncher.json`
  lists their size and CRC32. Other images (trimmed ones, for example) play as before.
- **The installer says what sm2port connects to**: its first page explains that the only connection is the update
  check, and a box turns it off from the first start. The README has a Privacy section and the code signing policy.

### Fixed

- **sm2port and mc3port are two programs for Windows**: their installers shared an ID, so with both installed, Installed
  apps listed only the one installed last. This version installs over the old sm2port and replaces its entry; mc3port's
  is left alone. Uninstalling also removes the whole folder.

### Changed

- **The launcher music and pictures are under CC BY 4.0**, so every part of sm2port has an open licence.

## 0.1.5

### New

- **PlayStation buttons**: with a DualSense, DualShock 4 or DualShock 3 connected (or RecompLauncher's PlayStation
  pad when none is), the launcher and the in-game menu show the cross, circle, square and triangle, L1, R1, L2, R2,
  Options and Create (Share, Select) instead of the Xbox's buttons, and the Controls rows named after a button (Y,
  Black, White) take its PlayStation name. The game's own prompts stay the Xbox's.
- **Mouse look and its speed in the launcher** (issue 4), in Settings, Game, and in the in-game menu: the speed was only
  in `sm2port.ini`.
- **Saves left in another folder are found**: the game keeps its files in `Documents\sm2port`, or next to `sm2.exe` for a
  portable copy and when RecompLauncher starts it, so after changing the way you install or start it your saves seemed
  lost. Now, before the game starts, it tells you where they are and offers to copy them (also when RecompLauncher
  starts the game directly). Saves already there are kept in `saves_backup`, the other folder stays as it is, and it
  asks only once.
- **Support the project** from the About page of the launcher and of the in-game menu: a row with the links to GitHub
  Sponsors and Ko-fi. It is never needed to play and it unlocks nothing; there are no reminders.
- **When Direct3D 11 does not start**, sm2port says so and what to do (the graphics driver on Windows, the 32-bit
  Vulkan libraries under Wine) instead of showing a black window.
- **A Linux package** (`sm2port-<version>-linux.tar.gz`), with a script that installs the game under Wine and checks
  that Direct3D 11 starts there. Not tried on Linux yet. The log says which Direct3D is in use (Windows', Wine's or
  DXVK's) and whether Steam started the game.

### Improved

- **The minimap is a little smaller** (85%), with the same shape, its "N" and its markers, around its own centre.
- Choosing a file that is not a disc image (one of sm2port's own files, for example) now says that the game is not
  included and that your own `.iso` or `.cci` is needed, instead of only "not an Xbox disc image".

### Fixed

- **Every save was "damaged" when loaded** (issue 2): the game reads a save a sector at a time and waits for each read
  to be pending, as on the Xbox; the port finished it at once and the game stopped after two sectors. Saves made with
  0.1.4 and earlier were written whole and load again.
- **A harsh, ringing sound**: a call the game makes for every voice, its volume envelope, was read as a filter, which
  put a strong peak around 8 kHz on every sound and made the limiter turn the whole mix down.
- **The display went to sleep while playing** with a controller (issue 3): Windows saw no input. sm2port now tells it a
  game is running.
- **The mouse cursor stayed on screen** with mouse look (issue 3): it was hidden only after 3 s without moving, and with
  mouse look the mouse is always moving. It is hidden at once now.
- **Mouse look turns as far as the mouse goes** (issues 3 and 4): it moves the camera through the right stick, and a
  fast swipe past the stick's full deflection was cut short, so a half turn took a swipe across the desk. What goes
  past it is now spent in the next moments (a quarter of a second at most; moving back drops it).
- With "Round HUD in 16:9" the minimap's markers were off the map: the player's arrow away from the centre, the
  objectives elsewhere. The minimap keeps the game's shape and its markers stay on it.
- **A dark half-disc beside the health ring** with "Round HUD in 16:9": the ring's dark lower half is drawn first, right
  after the 3D scene, and was the one piece of the ring left wide. It is narrowed with the rest now.
- The stencil shadows are faded with the depth of what they fall on, as the game asks: it reads the depth buffer as a
  picture and looks up a ramp with it, which the port did not do (the shadow was at full strength everywhere).
- **RecompLauncher**: `recomplauncher.json` follows the schema again (the launcher skipped it) and adds Window and
  Language; it also comes with the installer. Started by RecompLauncher, sm2port uses only the disc the launcher
  gives: no other disc in its place and no file dialog, but exit code 2, so the launcher asks for it again. Quitting
  the launcher reopened by "Back to the launcher" no longer ends with code 2.
- Started by RecompLauncher, the game's restarts (the in-game menu's restart, "Back to the launcher", moving the old
  data folder, a lost graphics card) no longer look to the launcher like the game had ended: `sm2.exe` stays as the
  process it started, starts each new one itself and ends with the game's exit code. In Big Picture mode the lost
  graphics card's message is not shown (a controller could not close it), only logged.

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
