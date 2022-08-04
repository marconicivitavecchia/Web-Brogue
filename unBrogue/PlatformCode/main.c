#include "platform.h"

#ifdef BROGUE_TCOD
#include "libtcod.h"
TCOD_renderer_t renderer=TCOD_RENDERER_SDL; // the sdl renderer is more reliable
short brogueFontSize = -1;
#endif

extern playerCharacter rogue;
struct brogueConsole currentConsole;

boolean serverMode = false;
boolean noMenu = false;
boolean noRestart = false;
boolean noScores = false;
boolean noRecording = false;
boolean noSaves = false;

unsigned long int firstSeed = 0;

void dumpScores();

static boolean endswith(const char *str, const char *ending)
{
	int str_len = strlen(str), ending_len = strlen(ending);
	if (str_len < ending_len) return false;
	return strcmp(str + str_len - ending_len, ending) == 0 ? true : false;
}

static void append(char *str, char *ending, int bufsize) {
	int str_len = strlen(str), ending_len = strlen(ending);
	if (str_len + ending_len + 1 > bufsize) return;
	strcpy(str + str_len, ending);
}

int main(int argc, char *argv[])
{
#ifdef BROGUE_TCOD
		currentConsole = tcodConsole;
#elif BROGUE_WEB
        currentConsole = webConsole;
#elif BROGUE_CURSES
		currentConsole = cursesConsole;
#endif

	rogue.nextGame = NG_NOTHING;
	rogue.nextGamePath[0] = '\0';
	rogue.nextGameSeed = 0;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--scores") == 0) {
			// just dump the scores and quit!
			dumpScores();
			return 0;
		}

		if (strcmp(argv[i], "--seed") == 0 || strcmp(argv[i], "-s") == 0) {
			// pick a seed!
			if (i + 1 < argc) {
				int seed = atoi(argv[i + 1]);
				if (seed != 0) {
					i++;
					rogue.nextGameSeed = seed;
					rogue.nextGame = NG_NEW_GAME_WITH_SEED;
					continue;
				}
			}
		}

		if(strcmp(argv[i], "-n") == 0) {
			if (rogue.nextGameSeed == 0) {
				rogue.nextGame = NG_NEW_GAME;
			} else {
				rogue.nextGame = NG_NEW_GAME_WITH_SEED;
			}
			continue;
		}

		if(strcmp(argv[i], "--no-menu") == 0 || strcmp(argv[i], "-M") == 0) {
			rogue.nextGame = NG_NEW_GAME;
			noMenu = true;
			continue;
		}

		if(strcmp(argv[i], "--no-scores") == 0) {
		  noScores = true;
		  continue;
		}

		if(strcmp(argv[i], "--no-restart") == 0) {
		  noRestart = true;
		  continue;
		}

		if(strcmp(argv[i], "--no-recording") == 0) {
		  noRecording = true;
		  continue;
		}

		if(strcmp(argv[i], "--no-saves") == 0) {
		  noSaves = true;
		  continue;
		}

		if (strcmp(argv[i], "--hide-seed") == 0) {
            rogue.hideSeed = true;
            continue;
        }
		
		if(strcmp(argv[i], "--noteye-hack") == 0) {
			serverMode = true;
			continue;
		}

		if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--open") == 0) {
			if (i + 1 < argc) {
				strncpy(rogue.nextGamePath, argv[i + 1], BROGUE_FILENAME_MAX);
				rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
				rogue.nextGame = NG_OPEN_GAME;

				if (!endswith(rogue.nextGamePath, GAME_SUFFIX)) {
					append(rogue.nextGamePath, GAME_SUFFIX, BROGUE_FILENAME_MAX);
				}

				i++;
				continue;
			}
		}

		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--view") == 0) {
			if (i + 1 < argc) {
				strncpy(rogue.nextGamePath, argv[i + 1], BROGUE_FILENAME_MAX);
				rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
				rogue.nextGame = NG_VIEW_RECORDING;

				if (!endswith(rogue.nextGamePath, RECORDING_SUFFIX)) {
					append(rogue.nextGamePath, RECORDING_SUFFIX, BROGUE_FILENAME_MAX);
				}

				i++;
				continue;
			}
		}

#ifdef BROGUE_TCOD
		if (strcmp(argv[i], "--SDL") == 0) {
			renderer = TCOD_RENDERER_SDL;
			currentConsole = tcodConsole;
			continue;
		}
		if (strcmp(argv[i], "--opengl") == 0 || strcmp(argv[i], "-gl") == 0) {
			renderer = TCOD_RENDERER_OPENGL;
			currentConsole = tcodConsole;
			continue;
		}
		if (strcmp(argv[i], "--size") == 0) {
			// pick a font size
			int size = atoi(argv[i + 1]);
			if (size != 0) {
				i++;
				brogueFontSize = size;
				continue;
			}
		}
#endif
#ifdef BROGUE_CURSES
		if (strcmp(argv[i], "--term") == 0 || strcmp(argv[i], "-t") == 0) {
			currentConsole = cursesConsole;
			continue;
		}
#endif

		// maybe it ends with .broguesave or .broguerec, then?
		if (endswith(argv[i], GAME_SUFFIX)) {
			strncpy(rogue.nextGamePath, argv[i], BROGUE_FILENAME_MAX);
			rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
			rogue.nextGame = NG_OPEN_GAME;
			continue;
		}

		if (endswith(argv[i], RECORDING_SUFFIX)) {
			strncpy(rogue.nextGamePath, argv[i], BROGUE_FILENAME_MAX);
			rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
			rogue.nextGame = NG_VIEW_RECORDING;
			continue;
		}
	}
	
	loadKeymap();
	currentConsole.gameLoop();
	
	return 0;
}

