DOOM_OBJS =	\
	am_map.o		\
	d_items.o		\
	d_main.o		\
	d_net.o			\
	doomdef.o		\
	doomstat.o		\
	dstrings.o		\
	f_finale.o		\
	f_wipe.o		\
	g_game.o		\
	hu_lib.o		\
	hu_stuff.o		\
	i_main.o		\
	info.o			\
	m_argv.o		\
	m_bbox.o		\
	m_cheat.o		\
	m_fixed.o		\
	m_menu.o		\
	m_misc.o		\
	m_random.o		\
	m_swap.o		\
	p_ceilng.o		\
	p_doors.o		\
	p_enemy.o		\
	p_floor.o		\
	p_inter.o		\
	p_lights.o		\
	p_map.o			\
	p_maputl.o		\
	p_mobj.o		\
	p_plats.o		\
	p_pspr.o		\
	p_saveg.o		\
	p_setup.o		\
	p_sight.o		\
	p_spec.o		\
	p_switch.o		\
	p_telept.o		\
	p_tick.o		\
	p_user.o		\
	r_bsp.o			\
	r_data.o		\
	r_draw.o		\
	r_main.o		\
	r_plane.o		\
	r_segs.o		\
	r_sky.o			\
	r_things.o		\
	sounds.o		\
	s_sound.o		\
	st_lib.o		\
	st_stuff.o		\
	tables.o		\
	v_video.o		\
	wi_stuff.o		\
	w_wad.o			\
	xdoom.o			\
	z_zone.o		\

COMPAT_OBJS = \
	doom.o			\
	system/compat.o		\
	system/i_net.o		\
	system/i_sound.o	\
	system/i_system.o	\
	system/i_video.o	\
	system/identify.o	\

OBJS = $(DOOM_OBJS) $(COMPAT_OBJS)

TARGET = doom
ROM_TITLE = "doom"

ROM_FILES = roms/doom1.wad

WADS = $(wildcard roms/*.wad)

ifneq ($(WADS),)
	ROM_FILES += $(filter-out roms/doom1.wad,$(WADS))
endif

include ../roms.mk

DOOM_WARNING_FLAGS = -Wno-dangling-pointer		\
                     -Wno-enum-compare			\
                     -Wno-enum-conversion		\
                     -Wno-implicit-fallthrough		\
                     -Wno-implicit-function-declaration	\
                     -Wno-implicit-int			\
                     -Wno-int-conversion		\
                     -Wno-macro-redefined		\
                     -Wno-misleading-indentation	\
                     -Wno-missing-field-initializers	\
                     -Wno-parentheses			\
                     -Wno-pointer-sign			\
                     -Wno-sequence-point		\
                     -Wno-sign-compare			\
                     -Wno-sizeof-pointer-memaccess	\
                     -Wno-uninitialized			\
                     -Wno-unknown-warning-option	\
                     -Wno-unused-but-set-parameter	\
                     -Wno-unused-but-set-variable	\
                     -Wno-unused-parameter		\
                     -Wno-unused-variable

# Reference mock headers not present in K
# Also add an header referencing symbols not present
CFLAGS += \
	  -Isystem/include			\
	  -include system/include/compat.h	\
	  -DNORMALUNIX				\

# Silence warnings from original DOOM sources.
$(DOOM_OBJS): CFLAGS += $(DOOM_WARNING_FLAGS)

roms/doom1.wad:
	mkdir -p roms
	curl -L http://distro.ibiblio.org/pub/linux/distributions/slitaz/sources/packages/d/doom1.wad > $@

install: roms/doom1.wad
