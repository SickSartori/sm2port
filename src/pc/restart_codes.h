/* Started by RecompLauncher, sm2.exe stays as the process it watches and starts each game process in turn
 * (launcher.c). A game process under it (SM2_SUPERVISED) does not start the next one itself: it ends with one of
 * these codes and sm2.exe starts it, so RecompLauncher sees the game end only when the player quits. */
#pragma once

#define SM2_EXIT_RESTART_GAME 0x52530001u     /* the in-game menu's restart: straight into the game */
#define SM2_EXIT_BACK_TO_LAUNCHER 0x52530002u /* "Back to the launcher" */
#define SM2_EXIT_MOVE_OLD_DATA 0x52530003u    /* the launcher's move of the old data folder */
