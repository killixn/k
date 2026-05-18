#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <kstd.h>

#include "../i_system.h"
#include "../i_sound.h"
#include "../d_net.h"
#include "../m_misc.h"
#include "../i_video.h"

#define HEAP_SIZE 6 * 1024 * 1024
int mb_used;

static unsigned long start_time;

void I_Init(void)
{
	I_InitSound();
	start_time = gettick();
}

void I_Quit(void)
{
	D_QuitNetGame();
	I_ShutdownSound();
	I_ShutdownMusic();
	M_SaveDefaults();
	I_ShutdownGraphics();
	exit(0);
}

byte* I_ZoneBase(int *size)
{
	mb_used = HEAP_SIZE / 1024 / 1024;
	*size = HEAP_SIZE;
	return malloc(HEAP_SIZE);
}

int I_GetTime(void)
{
	return ((gettick() - start_time) * TICRATE) / 1000;
}

ticcmd_t* I_BaseTiccmd(void)
{
	static ticcmd_t emptycmd;
	return &emptycmd;
}

byte* I_AllocLow(int length)
{
	return calloc(length, 1);
}

void I_Error(char *error, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, error);

	vsprintf(buf, error, args);
	printf("[FATAL] %s\n", buf);

	va_end(args);
	exit(-1);

}

/*
 * Unused functions in the original linuxdoom.
 */
void I_StartFrame(void) {}
void I_StartTic(void) {}
void I_Tactile(int on, int off, int total)
{
	(void) on;
	(void) off;
	(void) total;
}
