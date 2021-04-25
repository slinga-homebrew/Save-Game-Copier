/*  Copyright 2006-2007,2013 Theo Berkau

    This file is part of Iapetus.

    Iapetus is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Iapetus is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Iapetus; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef CD_H
#define CD_H

enum IAPETUS_ERR
{
   IAPETUS_ERR_OK=0,                // Everything is good
   IAPETUS_ERR_COMM=-1,             // Communication error
   IAPETUS_ERR_HWNOTFOUND=-2,       // Hardware not found
   IAPETUS_ERR_SIZE=-3,             // Invalid size specified
   IAPETUS_ERR_INVALIDPOINTER=-4,   // Invalid pointer passed
   IAPETUS_ERR_INVALIDARG=-5,       // Invalid argument passed
   IAPETUS_ERR_BUSY=-6,             // Hardware is busy
   IAPETUS_ERR_UNKNOWN=-7,          // Unknown error
   IAPETUS_ERR_FILENOTFOUND=-8,     // File not found error
   IAPETUS_ERR_UNSUPPORTED=-9,      // Unsupported feature
   IAPETUS_ERR_TIMEOUT=-10,         // Operation timed out
   IAPETUS_ERR_UNEXPECTDATA=-11,    // Unexpected data
   IAPETUS_ERR_OUTOFMEMORY=-12,     // Ran out of memory
	IAPETUS_ERR_BADHEADER=-13,       // Bad Header

   // CD/MPEG Related
   IAPETUS_ERR_AUTH=-100,            // Disc/MPEG card authentication error
   IAPETUS_ERR_CMOK=-101,            // CD command ok hirq bit not set
   IAPETUS_ERR_CDNOTFOUND=-102,      // CD not found
   IAPETUS_ERR_MPEGCMD=-103,         // MPEG command hirq bit not set
};

#include "satiator-types.h"

#define HIRQ_CMOK   0x0001
#define HIRQ_DRDY   0x0002
#define HIRQ_CSCT   0x0004
#define HIRQ_BFUL   0x0008
#define HIRQ_PEND   0x0010
#define HIRQ_DCHG   0x0020
#define HIRQ_ESEL   0x0040
#define HIRQ_EHST   0x0080
#define HIRQ_ECPY   0x0100
#define HIRQ_EFLS   0x0200
#define HIRQ_SCDQ   0x0400
#define HIRQ_MPED   0x0800
#define HIRQ_MPCM   0x1000
#define HIRQ_MPST   0x2000

#define STATUS_BUSY             0x00
#define STATUS_PAUSE            0x01
#define STATUS_STANDBY          0x02
#define STATUS_PLAY             0x03
#define STATUS_SEEK             0x04
#define STATUS_SCAN             0x05
#define STATUS_OPEN             0x06
#define STATUS_NODISC           0x07
#define STATUS_RETRY            0x08
#define STATUS_ERROR            0x09
#define STATUS_FATAL            0x0a
#define STATUS_PERIODIC         0x20
#define STATUS_TRANSFER         0x40
#define STATUS_WAIT             0x80
#define STATUS_REJECT           0xff

#define CDB_REG_HIRQ        *((volatile u16 *)0x25890008)
#define CDB_REG_HIRQMASK    *((volatile u16 *)0x2589000C)
#define CDB_REG_CR1         *((volatile u16 *)0x25890018)
#define CDB_REG_CR2         *((volatile u16 *)0x2589001C)
#define CDB_REG_CR3         *((volatile u16 *)0x25890020)
#define CDB_REG_CR4         *((volatile u16 *)0x25890024)
#define CDB_REG_DATATRNS    *((volatile u32 *)0x25818000)
#define CDB_REG_DATATRNSW   *((volatile u16 *)0x25898000)

#define CD_CON_TRUE             (1 << 0)
#define CD_CON_FALSE            (1 << 1)

#define CD_NO_CHANGE 0xFF

typedef struct
{
   u16 CR1;
   u16 CR2;
   u16 CR3;
   u16 CR4;
} cd_cmd_struct;

typedef struct
{
   u8 status;
   u8 flag;
   u8 repeat_cnt;
   u8 ctrl_addr;
   u8 track;
   u8 index;
   u32 FAD;
} cd_stat_struct;

typedef struct
{
   u8 hw_flag;
   u8 hw_ver;
   u8 mpeg_ver;
   u8 drive_ver;
   u8 drive_rev;
} hw_info_struct;

#define FM_FN                    (1 << 0)
#define FM_CN                    (1 << 1)
#define FM_SM                    (1 << 2)
#define FM_CI                    (1 << 3)
#define FM_REV                   (1 << 4)
#define FM_FAD                   (1 << 6)

#define SM_EOR                   (1 << 0)
#define SM_VIDEO                 (1 << 1)
#define SM_AUDIO                 (1 << 2)
#define SM_DATA                  (1 << 3)
#define SM_TRIGGER               (1 << 4)
#define SM_FORM                  (1 << 5)
#define SM_RT                    (1 << 6)
#define SM_EOF                   (1 << 7)

typedef struct
{
   u8 channel;
   u8 sm_mask;
   u8 ci_mask;
   u8 file_id;
   u8 sm_val;
   u8 ci_val;
} cd_sh_cond_struct;

typedef struct
{
   u32 fad;
   u32 range;
} cd_range_struct;

typedef struct
{
   u8 connect_flags;
   u8 true_con;
   u8 false_con;
} cd_con_struct;

enum SECTOR_SIZE
{
   SECT_2048 = 0x0,
   SECT_2336 = 0x1,
   SECT_2340 = 0x2,
   SECT_2352 = 0x3
};

enum SUBCODE_TYPE
{
   SC_Q = 0x0,
   SC_RW = 0x1
};

#define  SCEF_PACKDATAERROR   (1 << 0)
#define  SCEF_OVERRUNERROR    (1 << 1)

/*
int cd_wait_hirq(int flag);
int cd_exec_command(u16 hirq_mask, cd_cmd_struct *cd_cmd, cd_cmd_struct *cd_cmd_rs);
int cd_debug_exec_command(font_struct *font, u16 hirq_mask, cd_cmd_struct *cd_cmd, cd_cmd_struct *cd_cmd_rs);
int cd_get_hw_info(hw_info_struct *hw_info);
int cd_get_toc(u32 *toc);
int cd_get_session_num(u8 *num);
int cd_get_session_info(u8 num, u32 *lba);
int cd_play_fad(int play_mode, int start_fad, int num_sectors);
int cd_seek_fad(int seekfad);
int cd_get_subcode(enum SUBCODE_TYPE type, u16 *data, u8 *flags);
int cd_connect_cd_to_filter(u8 filter_num);
int cd_set_filter(u8 filter_num, u8 mode, cd_sh_cond_struct *sh_cond, cd_range_struct *cd_range, cd_con_struct *cd_con);
int cd_reset_selector_all();
int cd_init();
int cd_end_transfer();
int cd_get_stat(cd_stat_struct *cd_status);
int is_cd_auth(u16 *disc_type_auth);
int cd_auth();
int cd_stop_drive();
int cd_start_drive();
int is_cd_present();
int cd_read_sector(void *buffer, u32 FAD, int sector_size, u32 num_bytes);
int play_cd_audio(u8 audio_track, u8 repeat, u8 vol_l, u8 vol_r);
int stop_cd_audio(void);
int cd_set_sector_size(int size);
int cd_abort_file(void);
int cd_end_transfer(void);
*/

int cd_stop_drive();

#endif
