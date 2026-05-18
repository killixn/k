#include "../i_sound.h"
#include "../z_zone.h"
#include "../w_wad.h"

#define SAMPLECOUNT 512

char* sndserver_filename = NULL;
int lengths[NUMSFX];

static void *getsfx(const char *sfxname, int *len)
{
	char name[20];
	int sfxlump;

	sprintf(name, "ds%s", sfxname);

	if (W_CheckNumForName(name) == -1)
		sfxlump = W_GetNumForName("dspistol");
	else
		sfxlump = W_GetNumForName(name);

	int size = W_LumpLength(sfxlump);
	unsigned char *sfx = W_CacheLumpNum(sfxlump, PU_STATIC);
	int paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;
	unsigned char *paddedsfx = Z_Malloc(paddedsize + 8, PU_STATIC, 0);

	memcpy(paddedsfx, sfx, size);
	memset(paddedsfx + size, 128, paddedsize + 8 - size);

	Z_Free(sfx);

	*len = paddedsize;
	return paddedsfx + 8;
}

void I_InitSound()
{
	for (size_t i = 0; i < NUMSFX; ++i)
	{
		if (!S_sfx[i].link)
			S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
		else
		{
			S_sfx[i].data = S_sfx[i].link->data;
			lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof (sfxinfo_t)];
		}
	}
}

void I_UpdateSound(void)
{
}

void I_SubmitSound(void)
{
}

void I_ShutdownSound(void)
{
}

void I_SetChannels()
{
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
	(void) id;
	(void) vol;
	(void) sep;
	(void) pitch;
	(void) priority;

	return 0;
}

void I_StopSound(int handle)
{
	(void) handle;
}

int I_SoundIsPlaying(int handle)
{
	(void) handle;

	return 0;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	(void) handle;
	(void) vol;
	(void) sep;
	(void) pitch;
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{
}

void I_SetMusicVolume(int volume)
{
	(void) volume;
}

void I_PauseSong(int handle)
{
	(void) handle;
}

void I_ResumeSong(int handle)
{
	(void) handle;
}

int I_RegisterSong(void *data)
{
	(void) data;

	return 0;
}

void I_PlaySong(int handle, int looping)
{
	(void) handle;
	(void) looping;
}

void I_StopSong(int handle)
{
	(void) handle;
}

void I_UnRegisterSong(int handle)
{
	(void) handle;
}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
	(void) sfx;

	return 0;
}
