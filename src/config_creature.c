/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_creature.c
 *     Creature names, appearance and parameters configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for creatures list.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 03 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_creature.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"
#include "thing_creature.h"

#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_creaturetp_file[]="creature.cfg";

const struct NamedCommand creaturetype_common_commands[] = {
  {"CREATURES",              1},
  {"INSTANCESCOUNT",         2},
  {"JOBSCOUNT",              3},
  {"ANGERJOBSCOUNT",         4},
  {"ATTACKPREFERENCESCOUNT", 5},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_instance_commands[] = {
  {"NAME",            1},
  {"TIME",            2},
  {"ACTIONTIME",      3},
  {"RESETTIME",       4},
  {"FPTIME",          5},
  {"FPACTIONTIME",    6},
  {"FPRESETTIME",     7},
  {"FORCEVISIBILITY", 8},
  {NULL,              0},
  };

const struct NamedCommand angerjob_desc[] = {
  {"KILL_CREATURES",  1},
  {"DESTROY_ROOMS",   2},
  {"LEAVE_DUNGEON",   4},
  {"STEAL_GOLD",      8},
  {"DAMAGE_WALLS",   16},
  {"MAD_PSYCHO",     32},
  {"PERSUADE",       64},
  {"JOIN_ENEMY",    128},
  {"UNKNOWN1",      256},
  {NULL,              0},
  };

const struct NamedCommand creaturejob_desc[] = {
  {"NULL",               0},
  {"TUNNEL",             1},
  {"DIG",                2},
  {"RESEARCH",           4},
  {"TRAIN",              8},
  {"MANUFACTURE",       16},
  {"SCAVENGE",          32},
  {"KINKY_TORTURE",     64},
  {"FIGHT",            128},
  {"SEEK_THE_ENEMY",   256},
  {"GUARD",            512},
  {"GROUP",           1024},
  {"BARRACK",         2048},
  {"TEMPLE",          4096},
  {"FREEZE_PRISONERS",8192},
  {"EXPLORE",        16384},
  {NULL,                 0},
  };

const struct NamedCommand attackpref_desc[] = {
  {"MELEE",              1},
  {"RANGED",             2},
  {NULL,                 0},
  };

/******************************************************************************/
struct CreatureConfig crtr_conf;
struct NamedCommand creature_desc[CREATURE_TYPES_MAX];
struct NamedCommand instance_desc[INSTANCE_TYPES_MAX];
/******************************************************************************/

TbBool parse_creaturetypes_common_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  crtr_conf.kind_count = 1;
  crtr_conf.instance_count = 1;
  crtr_conf.job_count = 1;
  crtr_conf.angerjob_count = 1;
  crtr_conf.attackpref_count = 1;
  k = sizeof(crtr_conf.kind_names)/sizeof(crtr_conf.kind_names[0]);
  for (i=0; i < k; i++)
  {
    LbMemorySet(crtr_conf.kind_names[i].text, 0, COMMAND_WORD_LEN);
  }

  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    LbWarnLog("Block [%s] not found in Creature config file.\n",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // CREATURES
          while (get_conf_parameter_single(buf,&pos,len,crtr_conf.kind_names[n].text,COMMAND_WORD_LEN) > 0)
          {
            creature_desc[n].name = crtr_conf.kind_names[n].text;
            creature_desc[n].num = n+1;
            n++;
            if (n >= CREATURE_TYPES_MAX)
            {
              LbWarnLog("Too many spiecies defined with \"%s\" in [%s] block of Creature file.\n",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
              break;
            }
          }
          crtr_conf.kind_count = n;
          if (n < 1)
          {
            LbWarnLog("No spiecies defined with \"%s\" in [%s] block of Creature file.\n",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          while (n < CREATURE_TYPES_MAX)
          {
            creature_desc[n].name = NULL;
            creature_desc[n].num = 0;
            n++;
          }
          break;
      case 2: // INSTANCESCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.instance_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            LbWarnLog("Incorrect value of \"%s\" parameter in [%s] block of Creature file.\n",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 3: // JOBSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.job_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            LbWarnLog("Incorrect value of \"%s\" parameter in [%s] block of Creature file.\n",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 4: // ANGERJOBSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.angerjob_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            LbWarnLog("Incorrect value of \"%s\" parameter in [%s] block of Creature file.\n",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 5: // ATTACKPREFERENCESCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.attackpref_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            LbWarnLog("Incorrect value of \"%s\" parameter in [%s] block of Creature file.\n",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          LbWarnLog("Unrecognized command (%d) in [%s] block of Creature file, starting on byte %d.\n",
              cmd_num,block_buf,pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_creaturetype_instance_blocks(char *buf,long len)
{
  struct InstanceInfo *inst_inf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  int arr_size;
  // Initialize the array
  arr_size = sizeof(crtr_conf.instance_names)/sizeof(crtr_conf.instance_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(crtr_conf.instance_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < crtr_conf.instance_count)
    {
      instance_desc[i].name = crtr_conf.instance_names[i].text;
      instance_desc[i].num = i;
    } else
    {
      instance_desc[i].name = NULL;
      instance_desc[i].num = 0;
    }
  }
  arr_size = crtr_conf.instance_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"instance%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      LbWarnLog("Block [%s] not found in Creature Types config file.\n",block_buf);
      continue;
    }
    inst_inf = &instance_info[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_instance_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,crtr_conf.instance_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
            break;
          }
          n++;
          break;
      case 2: // TIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->time = k;
            n++;
          }
          if (n < 1)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 3: // ACTIONTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->action_time = k;
            n++;
          }
          if (n < 1)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 4: // RESETTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->reset_time = k;
            n++;
          }
          if (n < 1)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 5: // FPTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->fp_time = k;
            n++;
          }
          if (n < 1)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 6: // FPACTIONTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->fp_action_time = k;
            n++;
          }
          if (n < 1)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 7: // FPRESETTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->fp_reset_time = k;
            n++;
          }
          if (n < 1)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 8: // FORCEVISIBILITY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->force_visibility = k;
            n++;
          }
          if (n < 1)
          {
            LbWarnLog("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.\n",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          LbWarnLog("Unrecognized command (%d) in [%s] block of Creature Types file, starting on byte %d.\n",
              cmd_num,block_buf,pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool load_creaturetypes_config(const char *conf_fname,unsigned short flags)
{
  static const char *func_name="load_creaturetypes_config";
  char *fname;
  char *buf;
  long len,pos;
  int cmd_num;
  TbBool result;
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Reading Creature config file \"%s\".\n",conf_fname);
#endif
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    LbWarnLog("Creature config file \"%s\" doesn't exist or is too small.\n",conf_fname);
    return false;
  }
  if (len > 65536)
  {
    LbWarnLog("Creature config file \"%s\" is too large.\n",conf_fname);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  result = (len > 0);
  if (result)
  {
    result = parse_creaturetypes_common_blocks(buf, len);
    if (!result)
      LbWarnLog("Parsing Creature file \"%s\" common blocks failed.\n",conf_fname);
  }
  if (result)
  {
    result = parse_creaturetype_instance_blocks(buf, len);
    if (!result)
      LbWarnLog("Parsing Creature file \"%s\" instance blocks failed.\n",conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif