#include <stdlib.h>

#include "../doomstat.h"
#include "../i_system.h"

void I_InitNetwork(void)
{
	doomcom = calloc(1, sizeof (*doomcom));
	if (!doomcom)
		I_Error("Could not allocate doomcom");
	doomcom->numplayers = doomcom->numnodes = doomcom->ticdup = 1;
	doomcom->id = DOOMCOM_ID;
}

void I_NetCmd(void)
{
}
