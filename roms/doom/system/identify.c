#include <kstd.h>
#undef KEY_F1
// Avoid conflict between kstd.h and doomdef.h

#include "../doomdef.h"
#include "../doomstat.h"
#include "../d_main.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define arr_sz(arr) (sizeof (arr) / sizeof (*arr))

static struct rom {
	const char *name;
	const char *file;
	GameMode_t gm;
} const roms[] = {
	{ "Doom", "doom.wad", registered },
	{ "Doom II", "doom2.wad", commercial },
	{ "Final Doom - TNT: Evolution", "tnt.wad", commercial },
	{ "Final Doom - The plutonia experiment", "plutonia.wad", commercial },
	{ "Doom - SHAREWARE", "doom1.wad", shareware },
	{ "Doom U", "doomu.wad", retail }, // Untested

	/* Not working yet... */

	{ "Heretic", "heretic.wad", indetermined },
	{ "Hexen", "hexen.wad", indetermined },
	{ "Hexen: Deathkings of the Dark Citadel", "hexdd.wad", indetermined },
	{ "Strife", "strife1.wad", indetermined },
};

static const struct rom *select_game(const struct rom *present[], size_t sz)
{
	if (!sz)
		return NULL;
	if (sz == 1)
		return present[0];
	printf("Please select a game:\n");
	for (size_t i = 0; i < sz; ++i) {
		printf("\t%u - '%s'\n", i + 1, present[i]->name);
	}
	while (1) {
		int key = getkey();
		if (key-- == -EAGAIN)
			continue;
		if (key > 0 && (size_t)key <= sz) {
			return present[key - 1];
		}
		printf("Invalid choice\n", key);
	}
}

void IdentifyVersion(void)
{
	char *doomwaddir = getenv("DOOMWADDIR");
	if (!doomwaddir)
		doomwaddir = ".";

	char path[256];

	size_t dirlen = strlen(doomwaddir);
	if (dirlen++ > sizeof (path) - 32) {
		printf("'%s' too long\n", doomwaddir);
		return;
	}

	size_t nbroms = 0;
	const struct rom *present_roms[arr_sz(roms)];

	sprintf(path, "%s/", doomwaddir);
	for (size_t i = 0; i < arr_sz(roms); ++i) {
		assert(strlen(roms[i].file) < 30);
		strcpy(path + dirlen, roms[i].file);
		if (!access(path, R_OK))
			present_roms[nbroms++] = &roms[i];
	}
	const struct rom *selected_game = select_game(present_roms, nbroms);
	if (!selected_game) {
		gamemode = indetermined;
		return;
	}
	gamemode = selected_game->gm;
	strcpy(path + dirlen, selected_game->file);
	D_AddFile(path);
}
