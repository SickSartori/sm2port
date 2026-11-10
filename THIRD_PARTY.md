# Third-party components

sm2port's own code is under the MIT licence ([LICENSE](LICENSE)). These parts come from others and keep their
licences:

| Component | Where | Licence |
|---|---|---|
| Dear ImGui 1.92.9b, by Omar Cornut and contributors | `third_party/imgui` | MIT, see `third_party/imgui/LICENSE.txt` |
| SDL 3.4.20, by Sam Lantinga and contributors | downloaded by CMake while building (not in the repository); its licence is in `third_party/sdl3` | zlib, see `third_party/sdl3/LICENSE.txt` |
| Barlow Semi Condensed, by Jeremy Tribby (The Barlow Project Authors) | `third_party/fonts` | SIL Open Font License 1.1, see `third_party/fonts/OFL.txt` |
| Launcher music, generated for sm2port with ElevenLabs Music through Magnific | `res/launcher_music.mp3` | used under the generating account's commercial terms |

The Barlow fonts and the music are built into `sm2runtime.dll` as resources; SDL is linked into it.

Not included, and never distributed: anything from Spider-Man 2. The game is read from the player's own disc image at run time. It
belongs to its owners (Marvel, Activision, Treyarch).

The launcher's eight background pictures (`res/launcher_bg_*.jpg`) were generated for sm2port with Magnific. They show no people, text or logos.
