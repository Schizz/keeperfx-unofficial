/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_basics.c
 *     Basic definitions and global routines for all library files.
 * @par Purpose:
 *     Integrates all elements of the library with a common toolkit.
 * @par Comment:
 *     Only simple, basic functions which can be used in every library file.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 22 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_basics.h"
#include "globals.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "bflib_datetm.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char *log_file_name="keeperfx.log";

void error(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
}

short error_dialog(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
  MessageBox(NULL, message, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
  return 0;
}

short error_dialog_fatal(const char *codefile,const int ecode,const char *message)
{
  static char msg_text[2048];
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
  sprintf(msg_text,"%s This error in '%s' makes the program unable to continue. See '%s' for details.",message,codefile,log_file_name);
  MessageBox(NULL, msg_text, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
  return 0;
}

/******************************************************************************/
bool error_log_initialised=false;
TbLog error_log;
/******************************************************************************/
int LbLog(TbLog *log, const char *fmt_str, va_list arg);
/******************************************************************************/

int LbErrorLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Error: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbWarnLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Warning: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbNetLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Net: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbSyncLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Sync: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int __fastcall LbErrorLogSetup(const char *directory, const char *filename, uchar flag)
{
  if ( error_log_initialised )
    return -1;
  const char *fixed_fname;
  if ( (filename!=NULL)&&(filename[0]!='\0') )
    fixed_fname = filename;
  else
    fixed_fname = "ERROR.LOG";
  char log_filename[DISKPATH_SIZE];
  int result;
  int flags;
  if ( LbFileMakeFullPath(true,directory,fixed_fname,log_filename,DISKPATH_SIZE) != 1 )
    return -1;
  flags = (flag==0)+53;
  if ( LbLogSetup(&error_log, log_filename, flags) == 1 )
  {
      error_log_initialised = 1;
      result = 1;
  } else
  {
    result = -1;
  }
  return result;
}

int __fastcall LbErrorLogClose()
{
    if (!error_log_initialised)
        return -1;
    return LbLogClose(&error_log);
}

int LbLog(TbLog *log, const char *fmt_str, va_list arg)
{
  enum Header {
        NONE   = 0,
        CREATE = 1,
        APPEND = 2,
  };
//  printf(fmt_str, arg);
  if ( !log->Initialised )
    return -1;
  FILE *file;
  bool need_initial_newline;
  char header;
  if ( log->Suspended )
    return 1;
  header = NONE;
  need_initial_newline = false;
  if ( !log->Created )
  {
      if ( (!(log->Flags & 4)) || LbFileExists(log->Filename) )
      {
        if ( (log->Flags & 1) && (log->Flags & 4) )
        {
          header = CREATE;
        } else
        if ( (log->Flags & 2) && (log->Flags & 8) )
        {
          need_initial_newline = true;
          header = APPEND;
        }
      } else
      {
        header = CREATE;
      }
  }
   const char *accmode;
    if ( (log->Created) || !(log->Flags & 1) )
      accmode = "a";
    else
      accmode = "w";
    file = fopen(log->Filename, accmode);
    if ( file==NULL )
    {
      return -1;
    }
    log->Created = true;
    if ( header != NONE )
    {
      if ( need_initial_newline )
        fprintf(file, "\n");
      const char *actn;
      if ( header == CREATE )
        actn = "CREATED";
      else
        actn = "APPENDED";
      fprintf(file, "LOG %s", actn);
      bool at_used;
      at_used = 0;
      if ( log->Flags & 0x20 )
      {
        TbTime curr_time;
        LbTime(&curr_time);
        fprintf(file, "  @ %02d:%02d:%02d",
            curr_time.Hour,curr_time.Minute,curr_time.Second);
        at_used = 1;
      }
      if ( log->Flags & 0x10 )
      {
        TbDate curr_date;
        LbDate(&curr_date);
        const char *sep;
        if ( at_used )
          sep = " ";
        else
          sep = "  @ ";
        fprintf(file," %s%d-%02d-%d",sep,curr_date.Day,curr_date.Month,curr_date.Year);
      }
      fprintf(file, "\n\n");
    }
    if ( log->Flags & 0x40 )
    {
        TbDate curr_date;
        LbDate(&curr_date);
        fprintf(file,"%02d-%02d-%d ",curr_date.Day,curr_date.Month,curr_date.Year);
    }
    if ( log->Flags & 0x80 )
    {
        TbTime curr_time;
        LbTime(&curr_time);
        fprintf(file, "%02d:%02d:%02d ",
            curr_time.Hour,curr_time.Minute,curr_time.Second);
    }
    if ( log->Prefix[0] != '\0' )
      fprintf(file, log->Prefix);
    vfprintf(file, fmt_str, arg);
  fclose(file);
  return 1;
}

int __fastcall LbLogSetPrefix(TbLog *log, const char *prefix)
{
  if ( !log->Initialised )
    return -1;
  if (prefix)
  {
    LbStringCopy(log->Prefix, prefix, LOG_PREFIX_LEN);
  } else
  {
    LbMemorySet(log->Prefix, 0, LOG_PREFIX_LEN);
  }
  return 1;
}

int __fastcall LbLogSetup(TbLog *log, const char *filename, int flags)
{
  log->Initialised = false;
  LbMemorySet(log->Filename, 0, DISKPATH_SIZE);
  LbMemorySet(log->Prefix, 0, LOG_PREFIX_LEN);
  log->Initialised=false;
  log->Created=false;
  log->Suspended=false;
  if (LbStringLength(filename)>DISKPATH_SIZE)
    return -1;
  LbStringCopy(log->Filename, filename, DISKPATH_SIZE);
  log->Flags = flags;
  log->Initialised = true;
  return 1;
}

int __fastcall LbLogClose(TbLog *log)
{
  if ( !log->Initialised )
    return -1;
  LbMemorySet(log->Filename, 0, DISKPATH_SIZE);
  LbMemorySet(log->Prefix, 0, LOG_PREFIX_LEN);
  log->Flags=0;
  log->Initialised=false;
  log->Created=false;
  log->Suspended=false;
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
