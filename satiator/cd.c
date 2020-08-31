#include "cd.h"

u32 interrupt_get_level_mask(void)
{
   u32 sr;

   asm("stc sr,%0": "=r"(sr));
   sr = (sr & 0xF0) >> 4;

   return sr;
}

void interrupt_set_level_mask(u32 imask)
{
   u32 sr;

   asm("stc sr,%0": "=r"(sr));
   sr &= 0xFFFFFF0F;
   imask <<= 4;
   sr |= imask;
   asm("ldc %0,sr": : "r" (sr));
}


void cd_write_command(cd_cmd_struct *cd_cmd)
{
   CDB_REG_CR1 = cd_cmd->CR1;
   CDB_REG_CR2 = cd_cmd->CR2;
   CDB_REG_CR3 = cd_cmd->CR3;
   CDB_REG_CR4 = cd_cmd->CR4;
}

void cd_read_return_status(cd_cmd_struct *cd_cmd_rs)
{
   cd_cmd_rs->CR1 = CDB_REG_CR1;
   cd_cmd_rs->CR2 = CDB_REG_CR2;
   cd_cmd_rs->CR3 = CDB_REG_CR3;
   cd_cmd_rs->CR4 = CDB_REG_CR4;
}

int cd_wait_hirq(int flag)
{
   int i;
   u16 hirq_temp;

   for (i = 0; i < 0x240000; i++)
   {
      hirq_temp = CDB_REG_HIRQ;
      if (hirq_temp & flag)
         return 1;
   }
   return 0;
}

int cd_exec_command(u16 hirq_mask, cd_cmd_struct *cd_cmd, cd_cmd_struct *cd_cmd_rs)
{
   int old_level_mask;
   u16 hirq_temp;
   u16 cd_status;

   // Mask any interrupts, we don't need to be interrupted
   old_level_mask = interrupt_get_level_mask();
   interrupt_set_level_mask(0xF);

   hirq_temp = CDB_REG_HIRQ;

   // Make sure CMOK flag is set, or we can't continue
   if (!(hirq_temp & HIRQ_CMOK))
      return IAPETUS_ERR_CMOK;

   // Clear CMOK and any other user-defined flags
   CDB_REG_HIRQ = ~(hirq_mask | HIRQ_CMOK);

   // Alright, time to execute the command
   cd_write_command(cd_cmd);

   // Let's wait till the command operation is finished
   if (!cd_wait_hirq(HIRQ_CMOK))
      return IAPETUS_ERR_TIMEOUT;

   // Read return data
   cd_read_return_status(cd_cmd_rs);

   cd_status = cd_cmd_rs->CR1 >> 8;

   // Was command good?
   if (cd_status == STATUS_REJECT)
      return IAPETUS_ERR_INVALIDARG;
   else if (cd_status & STATUS_WAIT)
      return IAPETUS_ERR_BUSY;

   // return interrupts back to normal
   interrupt_set_level_mask(old_level_mask);

   // It's all good
   return IAPETUS_ERR_OK;
}

int cd_get_stat(cd_stat_struct *cd_status)
{
   cd_cmd_struct cd_cmd;
   cd_cmd_struct cd_cmd_rs;
   int ret;

   cd_cmd.CR1 = 0x0000;
   cd_cmd.CR2 = 0x0000;
   cd_cmd.CR3 = 0x0000;
   cd_cmd.CR4 = 0x0000;

   if ((ret = cd_exec_command(0, &cd_cmd, &cd_cmd_rs)) != 0)
      return ret;

   cd_status->status = cd_cmd_rs.CR1 >> 8;
   cd_status->flag = (cd_cmd_rs.CR1 >> 4) & 0xF;
   cd_status->repeat_cnt = cd_cmd_rs.CR1 & 0xF;
   cd_status->ctrl_addr = cd_cmd_rs.CR2 >> 8;
   cd_status->track = cd_cmd_rs.CR2 & 0xFF;
   cd_status->index = cd_cmd_rs.CR3 >> 8;
   cd_status->FAD = ((cd_cmd_rs.CR3 & 0xFF) << 16) | cd_cmd_rs.CR4;

   return IAPETUS_ERR_OK;
}

int cd_stop_drive()
{
   int ret;
   cd_cmd_struct cd_cmd;
   cd_cmd_struct cd_cmd_rs;
   cd_stat_struct cd_status;
   int i;

   // CD Init Command
   cd_cmd.CR1 = 0x0400;
   cd_cmd.CR2 = 0x0001;
   cd_cmd.CR3 = 0x0000;
   cd_cmd.CR4 = 0x040F;

   if ((ret = cd_exec_command(0, &cd_cmd, &cd_cmd_rs)) != 0)
      return ret;

   // Wait till operation is finished(fix me)

   // Wait till drive is stopped
   for (;;)
   {
      // wait a bit
      for (i = 0; i < 100000; i++) { }

      if (cd_get_stat(&cd_status) != 0) continue;

      if (cd_status.status == STATUS_STANDBY) break;
      else if (cd_status.status == STATUS_FATAL) return IAPETUS_ERR_UNKNOWN;
   }

   return IAPETUS_ERR_OK;
}
