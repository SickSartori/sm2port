# Sviluppo

Note tecniche per chi compila e modifica sm2port (in italiano). La guida per i giocatori è il [README](../README.md) in inglese.


## Come funziona (PC)

- `sm2.exe` è un launcher minuscolo linkato all'indirizzo `0x10000`, lo stesso dell'XBE: così Windows riserva
  l'intervallo di memoria che il gioco si aspetta.
- `sm2runtime.dll` legge `default.xbe` direttamente dalla ISO, lo copia sopra il launcher, sostituisce gli accessi
  `fs:` del kernel Xbox, collega le 154 funzioni del kernel e salta all'entry point del gioco.
- Le funzioni del kernel non ancora scritte fermano il gioco e scrivono in `sm2port.log` quale funzione serve,
  da dove è stata chiamata e con quali argomenti.

## Requisiti

Visual Studio 2022 o più recente con il carico di lavoro **"Sviluppo di applicazioni desktop con C++"**, comprensivo di:

- strumenti di compilazione MSVC x64/x86
- Windows 11 SDK
- strumenti CMake C++ per Windows

Python 3 solo per gli strumenti in `tools/`.

## Compilazione

Da una "Developer PowerShell for VS", nella cartella del progetto (il preset usa il Visual Studio installato):

```
cmake --preset pc-x86
cmake --build --preset pc-x86-debug
```

Pacchetto della release (zip e SHA256SUMS in `dist/`): `powershell -File tools/package.ps1`.

## Branch e rilasci

- `main`: solo versioni pubblicate. Ogni commit su `main` corrisponde a un tag `vx.y.z` e a una release.
- `develop`: il lavoro di tutti i giorni. I commit hanno il titolo `X.Y.Z · Area: Title` (la versione in cui esce la modifica) e un corpo breve.
- Modifiche più grandi: un branch `feature/<nome>` da `develop`, unito in `develop` quando è provato.

Non c'è un workflow di rilascio: i rilasci si fanno a mano.

Pubblicare una versione: su `develop` aggiornare `project(... VERSION x.y.z)` in `CMakeLists.txt` e aggiungere la
sezione `## x.y.z` in `CHANGELOG.md`; `powershell -File tools/package.ps1` crea zip, installer e `SHA256SUMS` in `dist/`;
unire `develop` in `main` con `--no-ff`, inviare il tag `vx.y.z` e creare la release su GitHub con le note prese dal
CHANGELOG (`tools/release_notes.ps1`) e i file di `dist/`.

## Struttura del repository

| Cartella | Contenuto |
|---|---|
| `external/sick-xport` | libreria privata condivisa con mc3port (sottomodulo git): `src/core` (ISO, XBE, export del kernel) e la parte di `src/pc` che non dipende dal gioco (kernel, memoria, thread, registri e texture della GPU, orologi, controller, percorsi, log, crash, profilo); `xport.cmake` |
| `src/pc` | la parte di Spider-Man 2: patch del gioco, input e audio, renderer D3D11 e front end della GPU, dischi, launcher |
| `third_party` | Dear ImGui e i font Barlow, con le loro licenze; la licenza di SDL3, che CMake scarica alla configurazione |
| `res` | risorse Win32: icona, musica del launcher |
| `tools` | script di build, pacchetto, note di rilascio e analisi dell'XBE |
| `docs` | questo documento e il logo |
| `tests` | prove automatiche, senza file del gioco |

## Avvio

```
build\pc-x86\Debug\sm2.exe "percorso\del\gioco.iso"
```

Con un'ISO come argomento il launcher viene saltato (anche con `SM2_SKIP_LAUNCHER=1`). Senza argomenti si apre il launcher, che usa l'ISO ricordata in `sm2port.ini`, o la prima `.iso` accanto a `sm2.exe` o nella cartella corrente, o la fa scegliere.

I dati dell'utente stanno in `Documents\sm2port` (`paths.cpp`; fino alla 0.1.1 in `%LOCALAPPDATA%\sm2port`, dove restano in uso finché il launcher non li sposta, su richiesta, con `SM2_MOVE_OLD_DATA`): l'hard disk dell'Xbox (salvataggi in `UDATA`,
dati del titolo in `TDATA`, cache `Z:`) è la cartella `xbox_hdd`, i log sono in `logs` (`sm2port.log` e i due avvii prima, `sm2port.1.log` e `.2.log`, più
il minidump e lo zip di un crash), una cartella `mods` facoltativa sostituisce file del disco con lo stesso percorso, e lì
stanno anche `sm2port.ini` e `shader_cache`. Se accanto a `sm2.exe` c'è un file `portable.txt` o un `sm2port.ini`,
tutto resta invece accanto a `sm2.exe` (modalità portatile): è il caso delle build in `build\pc-x86\<config>`.

Controlli: i controller (Xbox, PlayStation, Switch e la maggior parte degli altri, tramite SDL3) sono le porte 1-4; Bianco e Nero diventano LB e RB. Senza
controller, la porta 1 è la tastiera (solo quando la finestra del gioco è in primo piano):

| Tasto | Pulsante Xbox |
|---|---|
| Frecce | croce direzionale (mappa, idrauliche, musica) |
| Invio / Backspace | START (pausa) / BACK (navigatore) |
| Spazio / Esc / Z / C | A (freno a mano) / B (trasferimento di peso) / X (nitro) / Y (guarda indietro) |
| F | clic della levetta sinistra (abilità speciale: Zone, Agro, Roar) |
| W / S | grilletto destro (gas) / sinistro (freno) |
| A / D | levetta sinistra (sterzo) |
| I / J / K / L | levetta destra (telecamera) |
| Shift sinistro / Ctrl sinistro | Nero (telecamera) / Bianco (fari) |

I tasti della tastiera si cambiano nella pagina Controlli del launcher (voce `keyboard` di `sm2port.ini`).

Alt+Invio o F11: schermo intero (senza bordi) e ritorno alla finestra.
F10: accende e spegne gli effetti moderni, per confrontarli con l'immagine originale.

## Impostazioni

Al primo avvio viene creato `sm2port.ini` nella cartella dei dati, con una spiegazione per ogni voce: schermo
panoramico 16:9 (`widescreen`), calcolo dei vertici sulla scheda video (`gpu_vertices`),
risoluzione interna (antialiasing), filtro anisotropico, effetti moderni (nitidezza, bagliore delle luci,
saturazione, contrasto, vignettatura, ombreggiatura ambientale e suo raggio). Le voci nuove mancanti in un file già
esistente valgono col loro valore predefinito.

Con `widescreen` il gioco crede di essere collegato a un televisore panoramico e usa la sua modalità 16:9: più
campo visivo ai lati, niente bande nere né immagine stirata. Con `gpu_vertices` i vertex program del gioco vengono
tradotti in shader: la prima volta si compilano in sottofondo (intanto li esegue il processore) e restano salvati
nella cartella `shader_cache`, nella cartella dei dati.

Il file si modifica con un editor di testo e vale dal
riavvio successivo. Ogni voce si può dare anche come variabile d'ambiente `SM2_<NOME>` (per esempio `SM2_BLOOM=0`),
che ha la precedenza sul file.

Variabili d'ambiente:

| Variabile | Effetto |
|---|---|
| `SM2_<VOCE>` | qualunque voce di `sm2port.ini` (per esempio `SM2_RENDER_SCALE=1`, `SM2_AMBIENT_OCCLUSION=0`) |
| `SM2_DUMP_WINDOW=1` | con `SM2_DUMP_FRAMES`, salva anche l'immagine della finestra con gli effetti (`window-NNNNN.bmp`) |
| `SM2_AO_VIEW=1` | mostra solo l'ombreggiatura ambientale, in grigio, al posto della scena (per regolarla) |
| `SM2_LANGUAGE` | lingua del gioco: 1 inglese, 3 tedesco, 4 francese, 5 spagnolo, 6 italiano (altrimenti `game_language`, poi la lingua di Windows) |
| `SM2_MULTICORE=1` | usa tutti i core; per default il gioco gira su un core solo, come sull'Xbox |
| `SM2_NO_DIALOG=1` | niente finestre di dialogo e niente launcher (per i test automatici) |
| `SM2_SKIP_LAUNCHER=1` | parte direttamente il gioco |
| `SM2_LAUNCHER_AUTOPLAY=<s>` | il launcher preme GIOCA dopo `s` secondi (test) |
| `SM2_LAUNCHER_PAGE=settings\|controls\|about` | il launcher si apre su quella pagina (catture di prova) |
| `SM2_ANY_VERSION=1` | accetta anche dischi diversi dalle versioni supportate |
| `SM2_DUMP_FRAMES=<n>` | salva un frame presentato ogni `n` come `frame-NNNNN.bmp` nella cartella di lavoro |
| `SM2_DEBUG_DRAWS=<n>` | descrive nel log i primi `n` draw (vertici, combiner, texture, stati); con `SM2_DEBUG_DRAWS_FF=1` solo quelli fixed-function |
| `SM2_DUMP_TARGET=<indirizzo hex>` | con ogni frame salvato, anche la render target a quell'indirizzo fisico (`target-NNNNN.bmp`) |
| `SM2_DEBUG_FRAME_AT=<s>[+<durata>]` | una riga per draw (e per clear) durante 1 s, o `durata` secondi, a partire da `s` secondi dopo il primo draw (rettangolo a schermo, coordinate di texture, stati) |
| `SM2_DEBUG_PROBE=<x>,<y>` | con il precedente, registra il pixel (x, y) ogni volta che un draw lo cambia |
| `SM2_DEBUG_HUD=<n>` | a ogni F7, i successivi `n` draw piatti nel log con la decisione del renderer (HUD stretto, nascosto, bande nere) |
| `SM2_DEBUG_FULLSCREEN=1` | con `SM2_DEBUG_FRAME_AT`, solo i draw che coprono quasi tutta la superficie (gli effetti a schermo intero), così la finestra può durare a lungo |
| `SM2_DEBUG_ADDITIVE=1` | con `SM2_DEBUG_FRAME_AT`, solo i draw che aggiungono luce (fusione con destinazione ONE: aloni, scie, bagliori) |
| `SM2_DUMP_TEXTURE=<indirizzo hex>` | salva le texture decodificate di quella pagina di 4 KB come `texture-<indirizzo>.bmp` (con l'alfa) |
| `SM2_DUMP_2D=1` | salva ogni texture dei draw piatti dell'HUD come `texture-<indirizzo>-<hash>.bmp` (le compresse a blocchi così come sono, in `.dds`); l'hash è il nome da dare all'immagine in `textures_replace` |
| `SM2_GPU_STATS=1` | ogni 10 s, i metodi GPU più usati |
| `SM2_PROFILE=1` | ogni 10 s, oltre alla riga `profile:` sempre presente (fps del gioco, vblank per fotogramma, carico del core del gioco e del thread della GPU): tempo CPU e attese di ogni thread, registri della GPU letti o scritti dal gioco tramite eccezioni, con l'indirizzo del codice |
| `SM2_INPUT_SCRIPT=<s>:<pulsante>,<da>-<a>:<pulsante>,...` | preme (0,2 s) o tiene premuti pulsanti del controller 1 a quei secondi dall'avvio, per i test (`start back a b x y black white up down left right lt rt`, `sl`/`sr` per sterzare). Con `@<file>[#n]+<s>[~<durata>]:<pulsante>` il tempo conta da quando il gioco apre un file con quel nome (per esempio `@bg_main+5:down`). Con il prefisso `p2`...`p4` (`p2start`, `p2rt`) i pulsanti vanno ai controller 2-4, che compaiono collegati anche senza un controller vero; con un controller vero sulla porta 1 le pressioni si sommano alle sue |
| `SM2_DUMP_BUFFER=<byte>` | salva i dati grezzi dei buffer audio di quella dimensione in `buffer-<byte>-<indirizzo>.bin` (cartella di lavoro) |
| `SM2_TRACE_SAVES=1` | nel log, tutti i file del disco (dimensione e hash del primo MB) e cosa il gioco chiede ai file system con la risposta e l'indirizzo che chiede (`saves: from ...`, fino a 3000 righe) |
| `SM2_TRACE_WAITS=1` | registra le attese del kernel che durano più di 5 s, con gli ultimi eventi segnalati |
| `SM2_AUDIO_DUMP=<s>` | i primi `s` secondi dell'audio in uscita in `audio.wav` (cartella di lavoro) |
| `SM2_AUDIO_VOICES=1` | ogni 10 s, l'elenco delle voci audio attive con formato, volume, tono, regioni, età e uscite |
| `SM2_SKIP=<texture>:<superficie>:<fusione>` / `SM2_DUMP_DRAW=...` | salta, o salva prima e dopo, i draw corrispondenti (per isolare un difetto grafico) |
| `SM2_WATCHDOG=<s>` | ogni `s` secondi, dove si trovano i thread del gioco (cambia la temporizzazione) |

## Prove automatiche

```bash
ctest --test-dir build/pc-x86 -C RelWithDebInfo --output-on-failure
```

`tests/` non contiene e non richiede file del gioco: dischi ed eseguibili sono creati dalle prove. Coprono XBE e immagini
del disco, il controllo del disco, lo zip del crash (riaperto con il `tar` di Windows: da PowerShell, non dalla shell di
Git), i log, i punti della modalità poligono POINT, le texture e il loro ingrandimento. `disc_real` controlla il disco
vero se `SM2_TEST_DISC` indica una propria immagine, altrimenti risulta saltata. `tools/package.ps1` si ferma se una prova
fallisce.

## Strumenti

- `python -I tools/xiso_inspect.py <iso> [default.xbe di output]`: elenca i file del disco, analizza l'XBE ed eventualmente lo estrae.
- `python -I tools/check_xbe.py <default.xbe>`: verifica la tabella del kernel e la scansione `fs:` del loader sull'XBE reale.
- `python -I tools/xbe_to_pe.py <default.xbe> <out.exe>`: impacchetta l'XBE come PE per disassemblarlo agli indirizzi reali, per esempio con `dumpbin /disasm /range:0x13600,0x13700 out.exe`.
