/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_redraw.c
 *     Functions to redraw the engine screen.
 * @par Purpose:
 *     High level redrawing routines.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     06 Nov 2010 - 03 Jul 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_redraw.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_sprfnt.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"
#include "bflib_dernc.h"

#include "player_data.h"
#include "dungeon_data.h"
#include "player_instances.h"
#include "player_states.h"
#include "kjm_input.h"
#include "gui_parchment.h"
#include "gui_draw.h"
#include "gui_boxmenu.h"
#include "gui_tooltips.h"
#include "power_hand.h"
#include "power_process.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "front_simple.h"
#include "front_easter.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_evnt.h"
#include "vidmode.h"
#include "config.h"
#include "config_strings.h"
#include "config_terrain.h"
#include "magic.h"
#include "game_merge.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_redraw_display(void);
DLLIMPORT void _DK_process_pointer_graphic(void);
DLLIMPORT void _DK_smooth_screen_area(unsigned char *a1, long a2, long a3, long a4, long a5, long a6);
DLLIMPORT void _DK_redraw_creature_view(void);
DLLIMPORT void _DK_redraw_isometric_view(void);
DLLIMPORT void _DK_redraw_frontview(void);
DLLIMPORT void _DK_draw_overlay_compass(long a1, long a2);
DLLIMPORT long _DK_map_fade_in(long a);
DLLIMPORT long _DK_map_fade_out(long a);
DLLIMPORT void _DK_message_draw(void);
DLLIMPORT void _DK_draw_sound_stuff(void);
DLLIMPORT void _DK_set_engine_view(struct PlayerInfo *player, long a2);
DLLIMPORT void _DK_set_sprite_view_3d(void);
DLLIMPORT void _DK_set_sprite_view_isometric(void);
DLLIMPORT void _DK_map_fade(unsigned char *a1, unsigned char *a2, unsigned char *a3, unsigned char *a4, unsigned char *a5, long a6, long const a7, long const a8, long a9);
DLLIMPORT void _DK_generate_map_fade_ghost_table(const char *a1, unsigned char *a2, unsigned char *a3);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void redraw_isometric_view(void);
void redraw_frontview(void);
/******************************************************************************/
void setup_engine_window(long x, long y, long width, long height)
{
    struct PlayerInfo *player;
    SYNCDBG(6,"Starting for size (%ld,%ld) at (%ld,%ld)",width,height,x,y);
    player=get_my_player();
    if (game.numfield_C & 0x20)
    {
      if (x > MyScreenWidth)
        x = MyScreenWidth;
      if (x < status_panel_width)
        x = status_panel_width;
    } else
    {
      if (x > MyScreenWidth)
        x = MyScreenWidth;
      if (x < 0)
        x = 0;
    }
    if (y > MyScreenHeight)
      y = MyScreenHeight;
    if (y < 0)
      y = 0;
    if (x+width > MyScreenWidth)
      width = MyScreenWidth-x;
    if (width < 0)
      width = 0;
    if (y+height > MyScreenHeight)
      height = MyScreenHeight-y;
    if (height < 0)
      height = 0;
    player->engine_window_x = x;
    player->engine_window_y = y;
    player->engine_window_width = width;
    player->engine_window_height = height;
}

void store_engine_window(TbGraphicsWindow *ewnd,int divider)
{
    struct PlayerInfo *player;
    player=get_my_player();
    if (divider <= 1)
    {
        ewnd->x = player->engine_window_x;
        ewnd->y = player->engine_window_y;
        ewnd->width = player->engine_window_width;
        ewnd->height = player->engine_window_height;
    } else
    {
        ewnd->x = player->engine_window_x/divider;
        ewnd->y = player->engine_window_y/divider;
        ewnd->width = player->engine_window_width/divider;
        ewnd->height = player->engine_window_height/divider;
    }
}

void load_engine_window(TbGraphicsWindow *ewnd)
{
    struct PlayerInfo *player;
    player=get_my_player();
    player->engine_window_x = ewnd->x;
    player->engine_window_y = ewnd->y;
    player->engine_window_width = ewnd->width;
    player->engine_window_height = ewnd->height;
}

void map_fade(unsigned char *a1, unsigned char *a2, unsigned char *a3, unsigned char *a4, unsigned char *a5, long a6, long const a7, long const a8, long a9)
{
    _DK_map_fade(a1, a2, a3, a4, a5, a6, a7, a8, a9); return;
}

void generate_map_fade_ghost_table(const char *a1, unsigned char *a2, unsigned char *a3)
{
    _DK_generate_map_fade_ghost_table(a1, a2, a3); return;
}

/**
 * Renders source and destination screens for map fading.
 * Stores them in given buffers.
 * @param fade_src
 * @param fade_dest
 * @param scanline Line width of the two given buffers.
 * @param height Height to be filled in given buffers.
 */
void prepare_map_fade_buffers(unsigned char *fade_src, unsigned char *fade_dest, int scanline, int height)
{
    struct PlayerInfo *player;
    player=get_my_player();
    // render the 3D screen
    if (player->field_4B5 == 2)
      redraw_isometric_view();
    else
      redraw_frontview();
    // Copy the screen to fade source temp buffer
    int i;
    int fadebuf_pos;
    fadebuf_pos = 0;
    for (i = 0; i < height; i++)
    {
        unsigned char *src;
        unsigned char *dst;
        src = lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth * i;
        dst = &fade_src[fadebuf_pos];
        fadebuf_pos += scanline;
        memcpy(dst, src, MyScreenWidth/pixel_size);
    }
    // create the parchment screen
    load_parchment_file();
    redraw_minimal_overhead_view();
    // Copy the screen to fade destination temp buffer
    fadebuf_pos = 0;
    for (i = 0; i < height; i++)
    {
        unsigned char *src;
        unsigned char *dst;
        src = lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth * i;
        dst = &fade_dest[fadebuf_pos];
        fadebuf_pos += scanline;
        memcpy(dst, src, MyScreenWidth/pixel_size);
    }
}

long map_fade_in(long a)
{
    SYNCDBG(6,"Starting");
    //return _DK_map_fade_in(a);
    if (a == 0)
    {
        map_fade_ghost_table = poly_pool;
        map_fade_src = poly_pool+0x10000;
        map_fade_dest = poly_pool+0x10000+64000;
        prepare_map_fade_buffers(map_fade_src, map_fade_dest, 320, MyScreenHeight/pixel_size);
        generate_map_fade_ghost_table("data/mapfadeg.dat", _DK_palette, map_fade_ghost_table);
    }
    map_fade(lbDisplay.WScreen, map_fade_dest, map_fade_src, pixmap.fade_tables, map_fade_ghost_table,
      a, 320, 200, lbDisplay.GraphicsScreenWidth);
    long nxamount =  a + 4;
    if (nxamount > 32)
        nxamount = 32;
    return nxamount;
}

long map_fade_out(long a)
{
    SYNCDBG(6,"Starting");
    //return _DK_map_fade_out(a);
    if (a == 32)
    {
        map_fade_ghost_table = poly_pool;
        map_fade_src = poly_pool+0x10000;
        map_fade_dest = poly_pool+0x10000+64000;
        prepare_map_fade_buffers(map_fade_src, map_fade_dest, 320, MyScreenHeight/pixel_size);
        generate_map_fade_ghost_table("data/mapfadeg.dat", _DK_palette, map_fade_ghost_table);
    }
    map_fade(lbDisplay.WScreen, map_fade_dest, map_fade_src, pixmap.fade_tables, map_fade_ghost_table,
      a, 320, 200, lbDisplay.GraphicsScreenWidth);
    long nxamount =  a - 4;
    if (a < 0)
        nxamount = 0;
    return nxamount;
}

void set_sprite_view_3d(void)
{
    _DK_set_sprite_view_3d();
}

void set_sprite_view_isometric(void)
{
    _DK_set_sprite_view_isometric();
}

long dummy_sound_line_of_sight(long a1, long a2, long a3, long a4, long a5, long a6)
{
    return 1;
}

void set_engine_view(struct PlayerInfo *player, long val)
{
    //_DK_set_engine_view(player, val);
    switch ( val )
    {
    case 1:
      player->acamera = &player->cameras[1];
      if (!is_my_player(player))
        break;
      lens_mode = 2;
      set_sprite_view_3d();
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(0);
      LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1,(MyScreenHeight/pixel_size) >> 1);
      break;
    case 2:
      player->acamera = &player->cameras[0];
      if (!is_my_player(player))
        break;
      lens_mode = 0;
      set_sprite_view_isometric();
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(1280);
      break;
    case 3:
      player->acamera = &player->cameras[2];
      if (!is_my_player(player))
        break;
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(1280);
      break;
    case 5:
      player->acamera = &player->cameras[3];
      if (!is_my_player(player))
        break;
      lens_mode = 0;
      set_sprite_view_isometric();
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(1280);
      break;
    default:
      break;
    }
    player->view_mode = val;
}

void draw_overlay_compass(long a1, long a2)
{
  _DK_draw_overlay_compass(a1, a2);
}

void message_draw(void)
{
  int i,h;
  long x,y;
  SYNCDBG(7,"Starting");
  LbTextSetFont(winfont);
  h = LbTextLineHeight();
  x = 148;
  y = 28;
  for (i=0; i < game.active_messages_count; i++)
  {
      LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
      set_flag_word(&lbDisplay.DrawFlags,Lb_TEXT_UNKNOWN0040,false);
      LbTextDraw((x+32)/pixel_size, y/pixel_size, game.messages[i].text);
      draw_gui_panel_sprite_left(x, y, 488+game.messages[i].field_40);
      y += pixel_size * h;
  }
}

void redraw_creature_view(void)
{
    SYNCDBG(6,"Starting");
    TbGraphicsWindow ewnd;
    struct PlayerInfo *player;
    struct Thing *thing;
    //_DK_redraw_creature_view(); return;
    player = get_my_player();
    if (player->field_45F != 2)
      player->field_45F = 2;
    update_explored_flags_for_power_sight(player);
    thing = thing_get(player->controlled_thing_idx);
    TRACE_THING(thing);
    if (!thing_is_invalid(thing))
      draw_creature_view(thing);
    if (smooth_on)
    {
      store_engine_window(&ewnd,pixel_size);
      smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
          ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
    }
    remove_explored_flags_for_power_sight(player);
    draw_swipe();
    if ((game.numfield_C & 0x20) != 0)
      draw_whole_status_panel();
    draw_gui();
    if ((game.numfield_C & 0x20) != 0)
      draw_overlay_compass(player->mouse_x, player->mouse_y);
    message_draw();
    gui_draw_all_boxes();
    draw_tooltip();
}

void smooth_screen_area(unsigned char *scrbuf, long x, long y, long w, long h, long scanln)
{
    SYNCDBG(7,"Starting");
    long i,k;
    unsigned char *buf;
    unsigned char *lnbuf;
    unsigned int ghpos;
    lnbuf = scrbuf + scanln*y + x;
    for (i = h-y-1; i>0; i--)
    {
      buf = lnbuf;
      for (k = w-x-1; k>0; k--)
      {
          ghpos = (buf[0] << 8) + buf[1];
          ghpos = (buf[scanln] << 8) + pixmap.ghost[ghpos];
          buf[0] = ghpos;
          buf++;
      }
      lnbuf += scanln;
    }
}

void make_camera_deviations(struct PlayerInfo *player,struct Dungeon *dungeon)
{
    long x,y;
    x = player->acamera->mappos.x.val;
    y = player->acamera->mappos.y.val;
    if (dungeon->camera_deviate_quake != 0)
    {
      x += UNSYNC_RANDOM(80) - 40;
      y += UNSYNC_RANDOM(80) - 40;
    }
    if (dungeon->camera_deviate_jump != 0)
    {
      x += ( (dungeon->camera_deviate_jump * LbSinL(player->acamera->orient_a) >> 8) >> 8);
      y += (-(dungeon->camera_deviate_jump * LbCosL(player->acamera->orient_a) >> 8) >> 8);
    }
    if ((dungeon->camera_deviate_quake != 0) || (dungeon->camera_deviate_jump != 0))
    {
      // bounding position
      if (x < 0)
      {
        x = 0;
      } else
      if (x > 65535)
      {
        x = 65535;
      }
      if (y < 0)
      {
        y = 0;
      } else
      if (y > 65535)
      {
        y = 65535;
      }
      // setting deviated position
      player->acamera->mappos.x.val = x;
      player->acamera->mappos.y.val = y;
    }
}

void redraw_isometric_view(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    TbGraphicsWindow ewnd;
    struct Coord3d pos;
    SYNCDBG(6,"Starting");
    //_DK_redraw_isometric_view(); return;

    player = get_my_player();
    if (player->acamera == NULL)
        return;
    memcpy(&pos,&player->acamera->mappos,sizeof(struct Coord3d));
    LbMemorySet(&ewnd, 0, sizeof(TbGraphicsWindow));
    if (player->field_45F != 1)
      player->field_45F = 1;
    dungeon = get_players_num_dungeon(my_player_number);
    // Camera position modifications
    make_camera_deviations(player,dungeon);
    update_explored_flags_for_power_sight(player);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
        store_engine_window(&ewnd,1);
        setup_engine_window(ewnd.x, ewnd.y, ewnd.width >> 1, ewnd.height >> 1);
    }
    engine(player,&player->cameras[0]);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
        load_engine_window(&ewnd);
    }
    if (smooth_on)
    {
        store_engine_window(&ewnd,pixel_size);
        smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
            ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
    }
    remove_explored_flags_for_power_sight(player);
    if ((game.numfield_C & 0x20) != 0)
        draw_whole_status_panel();
    draw_gui();
    if ((game.numfield_C & 0x20) != 0)
        draw_overlay_compass(player->mouse_x, player->mouse_y);
    message_draw();
    gui_draw_all_boxes();
    draw_power_hand();
    draw_tooltip();
    memcpy(&player->acamera->mappos,&pos,sizeof(struct Coord3d));
    SYNCDBG(8,"Finished");
}

void redraw_frontview(void)
{
    SYNCDBG(6,"Starting");
    //_DK_redraw_frontview(); return;
    struct PlayerInfo *player;
    long w,h;
    player = get_my_player();
    update_explored_flags_for_power_sight(player);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
      w = player->engine_window_width;
      h = player->engine_window_height;
      setup_engine_window(player->engine_window_x, player->engine_window_y, w, h >> 1);
    } else
    {
      w = 0;
      h = 0;
    }
    draw_frontview_engine(&player->cameras[3]);
    if ((game.flags_font & FFlg_unk08) != 0)
      setup_engine_window(player->engine_window_x, player->engine_window_y, w, h);
    remove_explored_flags_for_power_sight(player);
    if ((game.numfield_C & 0x20) != 0)
      draw_whole_status_panel();
    draw_gui();
    if ((game.numfield_C & 0x20) != 0)
      draw_overlay_compass(player->mouse_x, player->mouse_y);
    message_draw();
    draw_power_hand();
    draw_tooltip();
    gui_draw_all_boxes();
}

int get_place_room_pointer_graphics(RoomKind rkind)
{
    switch (rkind)
    {
    case 2:
        return 25;
    case 3:
        return 27;
    case 4:
        return 29;
    case 5:
        return 28;
    case 6:
        return 30;
    case 8:
        return 34;
    case 9:
        return 35;
    case 10:
        return 33;
    case 11:
        return 32;
    case 12:
        return 31;
    case 13:
        return 26;
    case 14:
        return 36;
    case 15:
        return 37;
    case 16:
        return 38;
    }
    return 0;
}

int get_place_trap_pointer_graphics(ThingModel trmodel)
{
    switch (trmodel)
    {
    case 1:
        return 5;
    case 2:
        return 9;
    case 3:
        return 7;
    case 4:
        return 8;
    case 5:
        return 6;
    case 6:
        return 10;
    }
    return 0;
}

int get_place_door_pointer_graphics(ThingModel drmodel)
{
    switch (drmodel)
    {
    case 1:
        return 11;
    case 2:
        return 12;
    case 3:
        return 13;
    case 4:
        return 14;
    }
    return 0;
}

void process_dungeon_top_pointer_graphic(struct PlayerInfo *player)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    long i;
    dungeon = get_dungeon(player->id_number);
    if (dungeon_invalid(dungeon))
    {
        set_pointer_graphic(0);
        return;
    }
    // During fade
    if (player->instance_num == PI_MapFadeFrom)
    {
        set_pointer_graphic(0);
        return;
    }
    // Mouse over panel map
    if (((game.numfield_C & 0x20) != 0) && mouse_is_over_small_map(player->mouse_x, player->mouse_y))
    {
        if (game.small_map_state == 2) {
            set_pointer_graphic(0);
        } else {
            set_pointer_graphic(1);
        }
        return;
    }
    // Mouse over battle message box
    if (battle_creature_over > 0)
    {
        PowerKind pwkind;
        pwkind = 0;
        if (player->work_state < PLAYER_STATES_COUNT)
            pwkind = player_state_to_spell[player->work_state];
        thing = thing_get(battle_creature_over);
        TRACE_THING(thing);
        if (can_cast_spell_on_thing(player->id_number, thing, pwkind))
        {
            draw_spell_cursor(player->work_state, battle_creature_over,
                thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        } else
        {
            set_pointer_graphic(1);
        }
        return;
    }
    // GUI action being processed
    if (game_is_busy_doing_gui())
    {
        set_pointer_graphic(1);
        return;
    }
    switch (player->work_state)
    {
    case PSt_CtrlDungeon:
        if (player->field_455)
          i = player->field_455;
        else
          i = player->field_454;
        switch (i)
        {
        case 1:
            set_pointer_graphic(2);
            break;
        case 2:
            set_pointer_graphic(39);
            break;
        case 3:
            thing = thing_get(player->thing_under_hand);
            TRACE_THING(thing);
            if ((!thing_is_invalid(thing)) && (player->field_4) && (dungeon->things_in_hand[0] != player->thing_under_hand)
                && can_thing_be_possessed(thing, player->id_number))
            {
              if (is_feature_on(Ft_BigPointer))
              {
                set_pointer_graphic(96+(game.play_gameturn%i));
              } else
              {
                set_pointer_graphic(47);
              }
              player->field_6 |= 0x01;
            } else
            if ((!thing_is_invalid(thing)) && (player->field_5) && (dungeon->things_in_hand[0] != player->thing_under_hand)
                && can_thing_be_queried(thing, player->id_number))
            {
                set_pointer_graphic(4);
                player->field_6 |= 0x01;
            } else
            {
                if ((player->field_3 & 0x02) != 0) {
                  set_pointer_graphic(2);
                } else {
                  set_pointer_graphic(0);
                }
            }
            break;
        default:
            if (player->field_10 <= game.play_gameturn)
              set_pointer_graphic(1);
            else
              set_pointer_graphic(0);
            break;
        }
        break;
    case PSt_BuildRoom:
        i = get_place_room_pointer_graphics(player->chosen_room_kind);
        set_pointer_graphic(i);
        break;
    case PSt_Unknown5:
    case PSt_Slap:
        set_pointer_graphic(0);
        break;
    case PSt_CallToArms:
    case PSt_CaveIn:
    case PSt_SightOfEvil:
    case PSt_CtrlPassngr:
    case PSt_CtrlDirect:
    case PSt_Lightning:
    case PSt_SpeedUp:
    case PSt_Armour:
    case PSt_Conceal:
    case PSt_Heal:
    case PSt_CreateDigger:
    case PSt_DestroyWalls:
    case PSt_CastDisease:
    case PSt_TurnChicken:
        draw_spell_cursor(player->work_state, 0, game.pos_14C006.x.stl.num, game.pos_14C006.y.stl.num);
        break;
    case PSt_Unknown12:
    case PSt_Unknown15:
        set_pointer_graphic(4);
        break;
    case PSt_PlaceTrap:
        i = get_place_trap_pointer_graphics(player->chosen_trap_kind);
        set_pointer_graphic(i);
        break;
    case PSt_PlaceDoor:
        i = get_place_door_pointer_graphics(player->chosen_door_kind);
        set_pointer_graphic(i);
        break;
    case PSt_Sell:
        set_pointer_graphic(3);
        break;
    default:
        set_pointer_graphic(1);
        break;
    }
}

void process_pointer_graphic(void)
{
    struct PlayerInfo *player;
    //_DK_process_pointer_graphic(); return;
    player = get_my_player();
    SYNCDBG(6,"Starting for view %d, player state %d, instance %d",(int)player->view_type,(int)player->work_state,(int)player->instance_num);
    switch (player->view_type)
    {
    case PVT_DungeonTop:
        // This case is complicated
        process_dungeon_top_pointer_graphic(player);
        break;
    case PVT_CreatureContrl:
    case PVT_CreaturePasngr:
        if ((game.numfield_D & 0x08) != 0)
          set_pointer_graphic(1);
        else
          set_pointer_graphic(0);
        break;
    case PVT_MapScreen:
    case PVT_MapFadeIn:
    case PVT_MapFadeOut:
        set_pointer_graphic(1);
        break;
    case PVT_None:
        set_pointer_graphic_none();
        break;
    default:
        WARNLOG("Unsupported view type");
        set_pointer_graphic_none();
        break;
    }
}

void redraw_display(void)
{
    //_DK_redraw_display();return;
    const char *text;
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    player = get_my_player();
    set_flag_byte(&player->field_6,0x01,false);
    if (game.game_kind == GKind_Unknown1)
      return;
    if (game.small_map_state == 2)
      set_pointer_graphic_none();
    else
      process_pointer_graphic();
    switch (player->view_mode)
    {
    case PVM_EmptyView:
        break;
    case PVM_CreatureView:
        redraw_creature_view();
        parchment_loaded = 0;
        break;
    case PVM_IsometricView:
        redraw_isometric_view();
        parchment_loaded = 0;
        break;
    case PVM_ParchmentView:
        redraw_parchment_view();
        break;
    case PVM_FrontView:
        redraw_frontview();
        parchment_loaded = 0;
        break;
    case PVM_ParchFadeIn:
        parchment_loaded = 0;
        player->field_4BD = map_fade_in(player->field_4BD);
        break;
    case PVM_ParchFadeOut:
        parchment_loaded = 0;
        player->field_4BD = map_fade_out(player->field_4BD);
        break;
    default:
        ERRORLOG("Unsupported drawing state, %d",(int)player->view_mode);
        break;
    }
    //LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    LbTextSetFont(winfont);
    lbDisplay.DrawFlags &= ~Lb_TEXT_UNKNOWN0040;
    LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    if ((player->field_0 & 0x04) != 0)
    {
        text = buf_sprintf( ">%s_", player->mp_message_text);
        LbTextDraw(148/pixel_size, 8/pixel_size, text);
    }
    if ( draw_spell_cost )
    {
        long pos_x,pos_y;
        unsigned short drwflags_mem;
        drwflags_mem = lbDisplay.DrawFlags;
        LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
        lbDisplay.DrawFlags = 0;
        LbTextSetFont(winfont);
        text = buf_sprintf("%d", draw_spell_cost);
        pos_y = GetMouseY() - pixel_size*LbTextStringHeight(text)/2 - 2;
        pos_x = GetMouseX() - pixel_size*LbTextStringWidth(text)/2;
        LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, text);
        lbDisplay.DrawFlags = drwflags_mem;
        draw_spell_cost = 0;
    }
    if (bonus_timer_enabled())
        draw_bonus_timer();
    if (((game.numfield_C & 0x01) != 0) && ((game.numfield_C & 0x80) == 0))
    {
          LbTextSetFont(winfont);
          text = gui_string(GUIStr_PausedMsg);
          long pos_x,pos_y;
          long w,h;
          int i;
          i = LbTextCharWidth(' ');
          w = pixel_size * (LbTextStringWidth(text) + 2*i);
          i = player->view_mode;
          if ((i == 2) || (i == 5) || (i == 1))
            pos_x = player->engine_window_x + (MyScreenWidth-w-player->engine_window_x)/2;
          else
            pos_x = (MyScreenWidth-w)/2;
          pos_y=16;
          i = LbTextLineHeight();
          lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
          h = pixel_size*i + pixel_size*i/2;
          LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
          draw_slab64k(pos_x, pos_y, w, h);
          LbTextDraw(0/pixel_size, 0/pixel_size, text);
          LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    }
    if (game.field_150356 != 0)
    {
      long pos_x,pos_y;
      long w,h;
      int i;
      if (game.armageddon.count_down+game.field_150356 <= game.play_gameturn)
      {
        i = 0;
        if ( game.field_15035A - game.armageddon.duration <= game.play_gameturn )
          i = game.field_15035A - game.play_gameturn;
      } else
      {
        i = game.play_gameturn - game.field_150356 - game.armageddon.count_down;
      }
      LbTextSetFont(winfont);
      text = buf_sprintf(" %s %03d", gui_string(646), i/2); // Armageddon message
      i = LbTextCharWidth(' ');
      w = pixel_size*LbTextStringWidth(text) + 6*i;
      pos_x = MyScreenWidth - w - 16;
      pos_y = 16;
      i = LbTextLineHeight();
      lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
      h = pixel_size*i + pixel_size*i/2;
      LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
      draw_slab64k(pos_x, pos_y, w, h);
      LbTextDraw(0/pixel_size, 0/pixel_size, text);
      LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    }
    draw_eastegg();
  //show_onscreen_msg(8, "Physical(%d,%d) Graphics(%d,%d) Lens(%d,%d)", (int)lbDisplay.PhysicalScreenWidth, (int)lbDisplay.PhysicalScreenHeight, (int)lbDisplay.GraphicsScreenWidth, (int)lbDisplay.GraphicsScreenHeight, (int)eye_lens_width, (int)eye_lens_height);
    SYNCDBG(7,"Finished");
}

/**
 * Redraws the game display buffer.
 */
TbBool keeper_screen_redraw(void)
{
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    player = get_my_player();
    LbScreenClear(0);
    if (LbScreenLock() == Lb_SUCCESS)
    {
        setup_engine_window(player->engine_window_x, player->engine_window_y,
            player->engine_window_width, player->engine_window_height);
        redraw_display();
        LbScreenUnlock();
        return true;
    }
    return false;
}
/******************************************************************************/
