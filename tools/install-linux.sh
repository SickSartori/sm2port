#!/bin/sh
# Installs sm2port for the current user on Linux, through Wine: copies this folder to ~/.local/share/sm2port, makes
# a Wine prefix of its own there, and adds the "sm2port" command (~/.local/bin) and an entry in the applications
# menu. Needs a recent Wine with 32-bit support: README.md, "Linux". Run it again to update: settings and saves stay.
set -eu

here=$(cd "$(dirname "$0")" && pwd)
dest=${XDG_DATA_HOME:-$HOME/.local/share}/sm2port
bin=$HOME/.local/bin
apps=${XDG_DATA_HOME:-$HOME/.local/share}/applications
export WINEDEBUG=${WINEDEBUG:--all}

command -v wine >/dev/null 2>&1 || { echo "wine not found: install Wine first (README.md, section Linux)." >&2; exit 1; }

mkdir -p "$dest" "$bin" "$apps"
cp -f "$here/sm2.exe" "$here/sm2runtime.dll" "$here/sm2port.png" "$here"/*.md "$here/README.txt" "$here/LICENSE" "$dest/"
rm -rf "$dest/licenses" && cp -r "$here/licenses" "$dest/"
# Settings, saves, log and shader cache next to sm2.exe (README.md, "Portable"): everything in one folder.
: > "$dest/portable.txt"

export WINEPREFIX=$dest/prefix
if [ ! -f "$WINEPREFIX/system.reg" ]; then
    echo "Making the Wine prefix in $WINEPREFIX (about a minute)..."
    # No Mono and no Gecko: sm2port does not use them, and Wine would offer to download them.
    WINEDLLOVERRIDES="mscoree,mshtml=" wineboot -i
    wineserver -w
fi

cat > "$bin/sm2port" <<SCRIPT
#!/bin/sh
# Starts sm2port (installed by install.sh) in its own Wine prefix.
export WINEPREFIX="$dest/prefix"
export WINEDEBUG=\${WINEDEBUG:--all}
cd "$dest" && exec wine sm2.exe "\$@"
SCRIPT
chmod +x "$bin/sm2port"

cat > "$apps/sm2port.desktop" <<ENTRY
[Desktop Entry]
Type=Application
Name=Midnight Club 3 DUB Edition Remix (sm2port)
Comment=Unofficial native Windows port of Midnight Club 3: DUB Edition Remix, through Wine
Exec="$bin/sm2port"
Icon=$dest/sm2port.png
Terminal=false
Categories=Game;
ENTRY
update-desktop-database "$apps" >/dev/null 2>&1 || true

echo "sm2port is installed in $dest."
echo "Start it from the applications menu, or with: $bin/sm2port"
echo "(if the command is not found, add $bin to your PATH, or run it with its full path)"
echo "Your settings and saves are in $dest. To remove everything:"
echo "  rm -rf \"$dest\" \"$bin/sm2port\" \"$apps/sm2port.desktop\""
