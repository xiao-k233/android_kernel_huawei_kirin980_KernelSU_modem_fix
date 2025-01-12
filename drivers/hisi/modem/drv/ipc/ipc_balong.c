/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*lint --e{537,607,701,713,718,732,746}*/
#include <linux/version.h>
#include <linux/hwspinlock.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/syscore_ops.h>
#include <soc_interrupts_app.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "securec.h"

#define DRIVER_NAME "v7r2_ipc_device"

#include <osl_bio.h>
#include <osl_types.h>
#include <osl_module.h>
#include <soc_clk.h>
#include <bsp_memmap.h>
#include <bsp_ipc.h>
#include <bsp_slice.h>
#include <bsp_reset.h>
#include "ipc_balong.h"
#include <bsp_pm_om.h>

#undef THIS_MODU
#define THIS_MODU mod_ipc


spinlock_t ipcm_lock[IPC_SEM_BUTTOM];

/*lint --e{129, 63, 64, 409, 49, 52, 502} */
static struct ipc_control		ipc_ctrl = {0};
static struct ipc_debug_s		ipc_debug = {0};

static u32 ipc_ccore_int_reset_flag=0;
static u8 modem_reset_flag = 0;
static u32 ipc_resume_int_stat = 0;
static phys_addr_t ipc_base_addr_phy = 0;
/*lint -save -e550*/
static s32 bsp_ipc_int_enable_noirq (IPC_INT_LEV_E ulLvl)
{
    u32 u32IntMask = 0;
    if (ulLvl >= INTSRC_NUM)
    {
	   ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,ulLvl);
           return MDRV_ERROR; 
     } 
    /*д�ж����μĴ���*/
    u32IntMask = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));
    u32IntMask |= (u32)1 << ulLvl;/* [false alarm]:�� */
    writel(u32IntMask,(volatile void *)(ipc_ctrl.ipc_base+BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));    
    return MDRV_OK;
}

s32 bsp_ipc_int_enable (IPC_INT_LEV_E ulLvl)
{
    unsigned long flags=0;
    s32 ret = 0;
     if (ulLvl >= INTSRC_NUM)
    {
	   ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,ulLvl);
           return MDRV_ERROR; 
     } 
    /*д�ж����μĴ���*/
    spin_lock_irqsave(&ipc_ctrl.lock,flags);
    ret = bsp_ipc_int_enable_noirq(ulLvl);
    spin_unlock_irqrestore(&ipc_ctrl.lock,flags);
    return ret;
}

static s32 bsp_ipc_int_disable_noirq(IPC_INT_LEV_E ulLvl)
{
	u32 u32IntMask = 0;
	 if (ulLvl >= INTSRC_NUM)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,ulLvl);
		return MDRV_ERROR; 
	 } 
	/*д�ж����μĴ���*/
	u32IntMask = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));
    u32IntMask = u32IntMask & (~((u32)1 << ulLvl));/* [false alarm]:�� */
	writel(u32IntMask,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));
	return MDRV_OK;
}

s32 bsp_ipc_int_disable(IPC_INT_LEV_E ulLvl)
{
	unsigned long flags=0;
	s32 ret = 0;
	 if (ulLvl >= INTSRC_NUM)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,ulLvl);
		return MDRV_ERROR; 
	 } 
	/*д�ж����μĴ���*/
	spin_lock_irqsave(&ipc_ctrl.lock,flags);
	ret = bsp_ipc_int_disable_noirq(ulLvl);
	spin_unlock_irqrestore(&ipc_ctrl.lock,flags);
	return ret;
}

s32 bsp_ipc_int_connect(IPC_INT_LEV_E ulLvl, voidfuncptr routine, u32 parameter)
{
	 unsigned long flags=0;
	 if (ulLvl >= INTSRC_NUM)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,ulLvl);
		return MDRV_ERROR; 
	 } 
	 spin_lock_irqsave(&ipc_ctrl.lock,flags);
	 ipc_ctrl.ipc_int_table[ulLvl].routine = routine;
	 ipc_ctrl.ipc_int_table[ulLvl].arg = parameter;
	 spin_unlock_irqrestore(&ipc_ctrl.lock,flags);
	 return MDRV_OK;
}

 
s32 bsp_ipc_int_disconnect(IPC_INT_LEV_E ulLvl)
{
	unsigned long flags = 0;
	if (ulLvl >= INTSRC_NUM)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,ulLvl);
		return MDRV_ERROR; 
	 } 
	spin_lock_irqsave(&ipc_ctrl.lock,flags);
	ipc_ctrl.ipc_int_table[ulLvl].routine = NULL;
	ipc_ctrl.ipc_int_table[ulLvl].arg = 0;
	spin_unlock_irqrestore(&ipc_ctrl.lock,flags);
	return MDRV_OK;
 }

void bsp_ipc_int_mask_status_dump(void)
{
	u32 u32IntStat1=readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));
	u32 u32IntStat2=readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_STAT(ipc_ctrl.core_num)));
	ipc_print_error("bsp_ipc_int_mask = 0x%x\n",u32IntStat1);
	ipc_print_error("bsp_ipc_int_stat = 0x%x\n",u32IntStat2);
}

OSL_IRQ_FUNC(irqreturn_t,ipc_int_handler,irq,dev_id)
{
	u32 i = 0;
	u32 u32IntStat = 0,begin = 0,end = 0;
	u32 u32Date = 0x1;
	u32 u32BitValue = 0;
	u32IntStat=readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_STAT(ipc_ctrl.core_num)));
	/*���ж�*/
	writel(u32IntStat,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_CLR(ipc_ctrl.core_num)));
	/* ����32���ж� */
	for (i = 0; i < INTSRC_NUM; i++)
	{
		if(0!=i)
		{
			u32Date <<= 1;   
		} 
		u32BitValue = u32IntStat & u32Date;
		/* ������ж� ,����ö�Ӧ�жϴ������� */
		if (0 != u32BitValue)
		{  
			/*����ע����жϴ�������*/
			if (NULL !=  ipc_ctrl.ipc_int_table[i].routine)
			{
				begin = bsp_get_slice_value();
				ipc_ctrl.last_int_cb_addr = (unsigned long)(ipc_ctrl.ipc_int_table[i].routine);
				ipc_ctrl.ipc_int_table[i].routine(ipc_ctrl.ipc_int_table[i].arg);
				end = bsp_get_slice_value();
				ipc_debug.u32IntTimeDelta[i] = get_timer_slice_delta(begin,end);
			}
			else
			{
				ipc_print_error("BSP_DRV_IpcIntHandler:No IntConnect,MDRV_ERROR!.int num =%d\n",i);
			}
			ipc_debug.u32IntHandleTimes[i]++;
		}
	}
	return IRQ_HANDLED;
}


s32 bsp_ipc_int_send(IPC_INT_CORE_E enDstCore, IPC_INT_LEV_E ulLvl)
{
	unsigned long flags = 0;
	 if (ulLvl >= INTSRC_NUM)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,ulLvl);
		return MDRV_ERROR; 
	 } 
	 if (enDstCore >= IPC_CORE_BUTTOM)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,enDstCore);
		return MDRV_ERROR; 
	 } 
	
 	if(modem_reset_flag && (IPC_INT_LEV_E)IPC_CCPU_INT_SRC_ACPU_RESET != ulLvl) /* �˼���Ϣ�����Խ��� */
	{
		return IPC_ERR_MODEM_RESETING;
	}
	/*дԭʼ�жϼĴ���,�����ж�*/
	spin_lock_irqsave(&ipc_ctrl.lock,flags);
	writel((u32)1 << ulLvl,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_RAW_INT(enDstCore)));
	
	spin_unlock_irqrestore(&ipc_ctrl.lock,flags);
	ipc_debug.u32RecvIntCore = enDstCore;
	ipc_debug.u32IntSendTimes[enDstCore][ulLvl]++;
	return MDRV_OK;
}

static void  mask_int(u32 u32SignalNum)
{
	u32 u32IntMask = 0;
	unsigned long flags=0;
	spin_lock_irqsave(&ipc_ctrl.lock,flags);
	u32IntMask = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(ipc_ctrl.core_num)));
	u32IntMask = u32IntMask & (~((u32)1 << u32SignalNum)); /* [false alarm]:�� */
	writel(u32IntMask,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(ipc_ctrl.core_num)));
	spin_unlock_irqrestore(&ipc_ctrl.lock,flags);
}

 s32 bsp_ipc_sem_create(u32 u32SignalNum)
 {
	  if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 
	 if(true != ipc_ctrl.sem_exist[u32SignalNum])/*����ͬһ���ź�����û��ɾ��������´������*/
	 {
	 	osl_sem_init(SEM_EMPTY,&(ipc_ctrl.sem_ipc_task[u32SignalNum]));
		ipc_ctrl.sem_exist[u32SignalNum] = true;
	 	return MDRV_OK;
	 }
	 else
	 {
	 	return MDRV_OK;
	 }
	
 }
 
 s32 bsp_ipc_sem_delete(u32 u32SignalNum)
 {
	if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 
	if(false == ipc_ctrl.sem_exist[u32SignalNum] )
	{
		ipc_print_error("semphore not exists,may be deleted already.\n");
		return MDRV_ERROR;
	}
	else
	{
		if (osl_sema_delete(&(ipc_ctrl.sem_ipc_task[u32SignalNum])))
		{
			ipc_print_error("Delete semphore failed.\n");
			return MDRV_ERROR;
		}
		ipc_ctrl.sem_exist[u32SignalNum] = false;
		return MDRV_OK;
	}
 }

 int bsp_ipc_sem_take(u32 u32SignalNum, int s32timeout)
 {
	u32 u32IntMask = 0,ret = 0;    
	/*�������*/
	if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 
	 /*��������ź�����Ӧ���ͷ��ж�����*/
	writel((u32)1<<u32SignalNum, (volatile void *)(ipc_ctrl.ipc_base+BSP_IPC_SEM_INT_CLR(ipc_ctrl.core_num)));
	ret =  readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(ipc_ctrl.core_num, u32SignalNum)));
	if(0 == ret)
	{
		mask_int(u32SignalNum);
		ipc_debug.u32SemTakeTimes[u32SignalNum]++;
		ipc_debug.u32SemId = u32SignalNum;
		return MDRV_OK;
	}
	else
	{
		if(false == ipc_ctrl.sem_exist[u32SignalNum])
		{
			ipc_print_error("need call ipc_sem_create to create this sem before call ipc_sem_take!\n");
			return MDRV_ERROR;
		}
		if(0 != s32timeout)
		{
			/*ʹ���ź����ͷ��ж�*/
			u32IntMask = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(ipc_ctrl.core_num)));
			u32IntMask = u32IntMask | ((u32)1 << u32SignalNum);/* [false alarm]:�� */
			writel(u32IntMask,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(ipc_ctrl.core_num)));
			 if (MDRV_OK != osl_sem_downtimeout(&(ipc_ctrl.sem_ipc_task[u32SignalNum]), s32timeout))  
			{
				mask_int(u32SignalNum);
				ipc_print_error("semTake timeout!\n");
				ipc_debug.u32SemTakeFailTimes[u32SignalNum]++;
				return MDRV_ERROR;
			}
			 else
			 {
				mask_int(u32SignalNum);
				ipc_debug.u32SemTakeTimes[u32SignalNum]++;
				ipc_debug.u32SemId = u32SignalNum;
				return MDRV_OK;
			 }
		}
		else
		{
			return MDRV_ERROR;
		}
	}
	
}
 
s32 bsp_ipc_sem_give(u32 u32SignalNum)
{
	 if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 
	ipc_debug.u32SemGiveTimes[u32SignalNum]++;
	/*���ź�������Ĵ���д0*/
	writel(0,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(ipc_ctrl.core_num, u32SignalNum)));
	return MDRV_OK;
 }
 static s32 ffSLsb(s32 args)
 {
	 int num = 0; /*lint !e34 */
	 s32 s32ImpVal = args;
	 if(0 == args)
	 {
	 	return 0;
	}
	 for(;;)
	 {
		 num++;
		 if (0x1 == (s32ImpVal & 0x1))
		 {
			 break;
		 }
		 s32ImpVal = (s32)((u32)s32ImpVal >> 1);
	 }
	 return num;
 }

 /*****************************************************************************
 * �� �� ��      : ipc_sem_int_handler
 *
 * ��������  : �ź����ͷ��жϴ�������
 *
 * �������  : ��  
 * �������  : ��
 *
 * �� �� ֵ      : ��
 *
 * �޸ļ�¼  :  2013��1��9�� lixiaojie 
 *****************************************************************************/
OSL_IRQ_FUNC(irqreturn_t,ipc_sem_int_handler,irq,dev_id)
{
	u32 u32IntStat = 0,u32HsCtrl=0,u32SNum=0, i = 32;
	u32IntStat = readl((const volatile void *)(ipc_ctrl.ipc_base+BSP_IPC_SEM_INT_STAT(ipc_ctrl.core_num)));
	u32SNum = ffSLsb(u32IntStat);
	if( u32SNum != 0)
	{
		do
		{
			 /*������ź����ͷ��жϣ�������ж�*/
			writel((u32)1<<--u32SNum, (volatile void *)(ipc_ctrl.ipc_base+BSP_IPC_SEM_INT_CLR(ipc_ctrl.core_num)));
			u32HsCtrl = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(ipc_ctrl.core_num, u32SNum)));
			if (0 == u32HsCtrl)
			{
				osl_sem_up(&(ipc_ctrl.sem_ipc_task[u32SNum]));
			}
			else
			{
				ipc_debug.u32SemTakeFailTimes[u32SNum]++;
			}
			u32IntStat = readl((const volatile void *)(ipc_ctrl.ipc_base+BSP_IPC_SEM_INT_STAT(ipc_ctrl.core_num)));
			u32SNum = ffSLsb(u32IntStat);
			i--;
		}while((u32SNum != 0) && (i  > 0));
	}
	else
	{
		return  IRQ_NONE;
	}
	return  IRQ_HANDLED;
}

s32 bsp_ipc_spin_lock(u32 u32SignalNum)
{
	u32 u32HsCtrl = 0;
        if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 
	for(;;)
	{
		u32HsCtrl = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(ipc_ctrl.core_num, u32SignalNum)));
		if (0 == u32HsCtrl)
		{
			ipc_debug.u32SemTakeTimes[u32SignalNum]++;
			ipc_debug.u32SemId = u32SignalNum;
			break;
		}
	}
	return MDRV_OK;
}

s32 bsp_ipc_spin_lock_timeout(u32 u32SignalNum, u32 TimeoutMs)
{
	u32 u32HsCtrl = 0;
	u32 start_time = 0, end_time = 0, elapsed = 0;
         if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 

	start_time = bsp_get_slice_value();
	elapsed = TimeoutMs * bsp_get_slice_freq() / 1000; /*lint !e647 */

	/* coverity[no_escape] */
	for(;;)
	{
		u32HsCtrl = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(ipc_ctrl.core_num, u32SignalNum)));
		if (0 == u32HsCtrl)
		{
			ipc_debug.u32SemTakeTimes[u32SignalNum]++;
			ipc_debug.u32SemId = u32SignalNum;
			break;
		}

		end_time = bsp_get_slice_value();
		if(get_timer_slice_delta(start_time, end_time) > elapsed)
		{
			ipc_debug.u32SemCore = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_STAT(ipc_ctrl.core_num, u32SignalNum)));
			return MDRV_ERROR;
		}
	}
	return MDRV_OK;
}

s32 bsp_ipc_spin_lock_timeout_irqsave(unsigned int u32SignalNum, unsigned int TimeoutMs, unsigned long *flags)
{
	int ret = MDRV_OK;

	spin_lock_irqsave(&ipcm_lock[u32SignalNum], *flags);
	ret = bsp_ipc_spin_lock_timeout(u32SignalNum, TimeoutMs);
	if(ret == MDRV_ERROR) {
		spin_unlock_irqrestore(&ipcm_lock[u32SignalNum], *flags);
		return ret;
	}
	return ret;
}

s32 bsp_ipc_spin_trylock(u32 u32SignalNum)
{
	u32 u32HsCtrl = 0;
	if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 
	u32HsCtrl = readl(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(ipc_ctrl.core_num, u32SignalNum));
	if (0 == u32HsCtrl)
	{
		ipc_debug.u32SemTakeTimes[u32SignalNum]++;
		ipc_debug.u32SemId = u32SignalNum;
		return MDRV_OK;
	}
	else
		return MDRV_ERROR;
}


s32 bsp_ipc_spin_unlock (u32 u32SignalNum)
{
         if (u32SignalNum >= IPC_SEM_BUTTOM_ACORE)
	{
		ipc_print_error("[%s]Wrong para , line:%d,para = %d\n",__FUNCTION__, __LINE__,u32SignalNum);
		return MDRV_ERROR; 
	 } 
	writel(0,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(ipc_ctrl.core_num, u32SignalNum)));
	ipc_debug.u32SemGiveTimes[u32SignalNum]++;
	return MDRV_OK;
}
extern void run_icc_pm_debug_callback(void);
extern void icc_wakeup_flag_set(void);
static void get_ipc_int_stat(void)
{
	u32 temp=0x1;
	pr_info("%s +\n", __func__);
	ipc_resume_int_stat=readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_STAT(ipc_ctrl.core_num)));
	if(ipc_resume_int_stat)
	{
		ipc_print_error("[C SR]ipc_resume_int_stat = 0x%x\n",ipc_resume_int_stat);
		if(ipc_resume_int_stat&(temp<<IPC_ACPU_INT_SRC_CCPU_ICC)){
			icc_wakeup_flag_set();
			run_icc_pm_debug_callback();
		}
		else if (ipc_resume_int_stat&(temp<<IPC_ACPU_INT_SRC_CCPU_PM_OM))
		{
			pm_om_wakeup_stat();
		}
	}
	pr_info("%s -\n", __func__);
	return ;
}
static struct syscore_ops ipc_dpm_ops = {
	.resume = get_ipc_int_stat,
	};
phys_addr_t bsp_get_ipc_base_addr(void)
{
	return ipc_base_addr_phy;
}

static void ipc_free_sem_taked(u32 core_id)
{
    u32 i = 0;
    u32 ret = 0;

    for(i = 0; i < 32; i++)
    {
        /*�ж���Դ��ռ�ã����ռ�ã����ͷ�*/
        ret = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_STAT(core_id, i)));
        if (ret == ((1 << 3) | core_id))
        {
            writel(0, (volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_CTRL(core_id, i)));
        }
    }
}

static struct ipc_msg g_ipc_msg = {0};
static int ipc_msg_init(void)
{
    struct device_node *ptr_device_node = NULL;
    const char *compatible_name = "hisilicon,ipc_v230_app";
    int ret = 0;

    ptr_device_node = of_find_compatible_node(NULL, NULL, compatible_name);/*lint !e838*/
    if (!ptr_device_node)
    {
        ipc_print_error( "ipc msg of find fail\n");
        return BSP_ERROR;
    }

    g_ipc_msg.base_addr = of_iomap(ptr_device_node, 0);
    if (!g_ipc_msg.base_addr)
    {
        ipc_print_error( "ipc msg base fail\n");
        return BSP_ERROR;
    }

    ret = of_property_read_u32(ptr_device_node, "src_id", &g_ipc_msg.src_id);/*lint !e838*/
    if(ret)
    {
        ipc_print_error( "ipc src_id fail\n");
        return BSP_ERROR;
    }

    ret = of_property_read_u32(ptr_device_node, "recv_mbx_id", &g_ipc_msg.recv_mbx);
    if(ret)
    {
        ipc_print_error( "ipc recv_mbx_id fail\n");
        return BSP_ERROR;
    }

    ipc_print_error("ipc msg init success\n");
    return BSP_OK;
}

static s32  bsp_ipc_probe(struct platform_device *dev)
{
	s32 ret = 0,i = 0; /*lint !e34 */
	u32 array_size=0;
	void *p_iomap_ret = NULL;
	struct device_node *node = NULL;
	int len = 0;
	int na = 0;
	int ns = 0;
	const __be32 *prop = NULL;
	phys_addr_t ipc_phy_base = 0;
	if (BSP_ERROR == ipc_msg_init())
	{
		ipc_print_error( "ipc msg init fail\n");
		return BSP_ERROR;
	}

	node = of_find_compatible_node(NULL, NULL, "hisilicon,ipc_balong_app");
	if (!node)
	{
		ipc_print_error( "ipc of find fail\n");
		return MDRV_ERROR;
	}

	na = of_n_addr_cells(node);
	ns = of_n_size_cells(node);

	prop = of_get_property(node, "reg", &len);
	if (!prop || len < (na + ns) * sizeof(*prop)) /*lint !e574 */
	{
		ipc_print_error( "ipc of_get_property reg fail\n");
		return MDRV_ERROR;
	}

	ipc_phy_base = of_read_number(prop, na);
	ipc_base_addr_phy = ipc_phy_base;

	ipc_ctrl.core_num = IPC_CORE_ACORE;

	/* �ڴ�ӳ�䣬��û�ַ */
	p_iomap_ret = of_iomap(node, 0);
	if (NULL == p_iomap_ret)
	{
		ipc_print_error( "acore ipc iomap fail\n");
		return MDRV_ERROR;
	}

	
	for(i = 0;i< INTSRC_NUM;i++ )
	{
		ipc_ctrl.sem_exist[i] = false;
	}
	array_size = sizeof(struct ipc_entry)*INTSRC_NUM;
	/* coverity[secure_coding] */
	memset_s((void*)(ipc_ctrl.ipc_int_table),array_size,0x0,array_size);
	ipc_ctrl.ipc_base = p_iomap_ret;

	writel(0x0,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));
	writel(0x0,(volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_SEM_INT_MASK(ipc_ctrl.core_num)));
	spin_lock_init(&ipc_ctrl.lock);

	/* ��ȡ�жϺ� */
	ipc_ctrl.irq_int_no = irq_of_parse_and_map(node, 0);
	ret = request_irq(ipc_ctrl.irq_int_no, ipc_int_handler, IRQF_NO_SUSPEND, "ipc_irq",(void*) NULL);
	if (ret )
	{
		ipc_print_error("ipc int handler error,init failed\n");
		return MDRV_ERROR;
	}

	ipc_ctrl.irq_sem_no = irq_of_parse_and_map(node, 1);
	ret = request_irq(ipc_ctrl.irq_sem_no, ipc_sem_int_handler, IRQF_NO_SUSPEND, "ipc_sem",(void*) NULL);
	if (ret )
	{
		ipc_print_error("ipc sem handler error,init failed\n");
		return MDRV_ERROR;
	}

	register_syscore_ops(&ipc_dpm_ops);
	ipc_print_error("ipc probe success\n");
	return MDRV_OK;    /*lint !e438*/
}
static s32  bsp_ipc_remove(struct platform_device *dev)
{
	free_irq(ipc_ctrl.irq_sem_no,NULL);
	free_irq(ipc_ctrl.irq_int_no,NULL);
	return MDRV_OK;
}
/*lint -restore +e550*/
void ipc_modem_reset_cb(DRV_RESET_CB_MOMENT_E stage, int userdata)
{
	unsigned long flags=0;
	spin_lock_irqsave(&ipc_ctrl.lock,flags);
	if (MDRV_RESET_CB_BEFORE == stage)
	{
        ipc_free_sem_taked((u32)IPC_CORE_CCORE);
		modem_reset_flag = 1;
	}
	if (MDRV_RESET_RESETTING == stage)
	{
		modem_reset_flag = 0;
	}
	spin_unlock_irqrestore(&ipc_ctrl.lock,flags);
	return;
}
void disable_ipc_irq(void)
{
     disable_irq(ipc_ctrl.irq_int_no);
     disable_irq(ipc_ctrl.irq_sem_no);
     return ;
}
void enable_ipc_irq(void)
{
     enable_irq(ipc_ctrl.irq_int_no);
     enable_irq(ipc_ctrl.irq_sem_no);
     return ;
}
void ccore_ipc_disable(void)
{
	unsigned long flag = 0;
	spin_lock_irqsave(&ipc_ctrl.lock,flag);
	ipc_ccore_int_reset_flag = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));

    writel(0x0,(volatile void *)(ipc_ctrl.ipc_base+BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));  
	
	/*������λ�����⴦����������*/
	(void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_CCPU_RESET_IDLE);
	(void)bsp_ipc_int_enable_noirq(IPC_ACPU_INT_SRC_CCPU_RESET_SUCC);

	spin_unlock_irqrestore(&ipc_ctrl.lock ,flag);
}
void ccore_ipc_enable(void)
{
	unsigned long flag = 0;
	spin_lock_irqsave(&ipc_ctrl.lock,flag);
    writel(ipc_ccore_int_reset_flag,(volatile void *)(ipc_ctrl.ipc_base+BSP_IPC_CPU_INT_MASK(ipc_ctrl.core_num)));    
	spin_unlock_irqrestore(&ipc_ctrl.lock,flag);
}


static void ipc_dpm_complete(struct device *dev)
{
	if(ipc_resume_int_stat)
	{
		bsp_pm_log((unsigned int)PM_OM_AIPC, sizeof(ipc_resume_int_stat), (void *)&ipc_resume_int_stat);
		pm_om_wakeup_log();
	}

    return ;
}

static s32 ipc_suspend_noirq(struct device *dev)
{
	u32 i = 0,ret = 0;
	pr_info("%s +\n", __func__);
	for(i=0;i<32;i++)
	{
		ret = readl((const volatile void *)(ipc_ctrl.ipc_base + BSP_IPC_HS_STAT(ipc_ctrl.core_num,i)));
		if(ret==0x8)
		{
			ipc_print_error("signum id = %d is occupied\n",i);
			pr_info("%s -\n", __func__);
			return MDRV_ERROR;
		}
	}
	pr_info("%s -\n", __func__);
	return MDRV_OK;
}
static const struct dev_pm_ops balong_ipc_pm_ops ={
	.suspend_noirq = ipc_suspend_noirq,
	.complete = ipc_dpm_complete,
};

#define BALONG_DEV_PM_OPS (&balong_ipc_pm_ops)


static struct platform_driver balong_ipc_driver = {
	.probe = bsp_ipc_probe,
	.remove = bsp_ipc_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner  = THIS_MODULE,
		.pm     = BALONG_DEV_PM_OPS,
	},
};

static struct platform_device balong_ipc_device =
{
    .name = DRIVER_NAME,
    .id       = -1,
    .num_resources = 0,
};
int __init hi_ipc_init(void)
{
	s32 ret = 0;

	ret = platform_device_register(&balong_ipc_device);
	if(ret)
	{
		ipc_print_error("Platform ipc device register is failed!\n");
        	return ret;
	}
	ret = platform_driver_register(&balong_ipc_driver);
	if (ret)
	{
		ipc_print_error("Platform ipc deriver register is failed!\n");
		platform_device_unregister(&balong_ipc_device);
		return ret;
	}
	return ret;
}
/*
static void  bsp_ipc_exit(void)
{
	platform_driver_unregister(&balong_ipc_driver);
 	platform_device_unregister(&balong_ipc_device);
}
*/

void bsp_ipc_debug_show(void)
{
	u32 i = 0;
	ipc_print_error("\ncurrent sem ID       : \t%d\n", ipc_debug.u32SemId);
	ipc_print_error("\ncurrent sem core     : \t%d\n", ipc_debug.u32SemCore);
	ipc_print_error("current recv int coreid: \t%d\n", ipc_debug.u32RecvIntCore);
	for(i = 0; i < INTSRC_NUM; i++)
	{
		ipc_print_error("sem%2d take count      : \t%d\n", i,ipc_debug.u32SemTakeTimes[i]);
		ipc_print_error("sem%2d give count      : \t%d\n", i,ipc_debug.u32SemGiveTimes[i]);
		ipc_print_error("int%2d recv count      : \t%d\n",i, ipc_debug.u32IntHandleTimes[i]);
		ipc_print_error("int%2d cost time       : \t%d us\n",i, ipc_debug.u32IntTimeDelta[i]*1000000/HI_TCXO_CLK);
	}
}

void bsp_int_send_info(void)
{
	u32 i = 0,j = 0;
	for(i = 0;i <IPC_CORE_BUTTOM;i++)
	{
		for(j=0;j<INTSRC_NUM;j++)
		{
			ipc_print_error("int%2d send to core%d count: \t%d\n",j,i, ipc_debug.u32IntSendTimes[i][j]);
		}
	}
		
}
/*lint -save -e19*/

EXPORT_SYMBOL(bsp_ipc_int_enable);
EXPORT_SYMBOL(bsp_ipc_int_disable);
EXPORT_SYMBOL(bsp_ipc_int_connect);
EXPORT_SYMBOL(bsp_ipc_int_disconnect);
EXPORT_SYMBOL(bsp_ipc_int_send);
EXPORT_SYMBOL(bsp_ipc_sem_create);
EXPORT_SYMBOL(bsp_ipc_sem_take);
EXPORT_SYMBOL(bsp_ipc_sem_give);
EXPORT_SYMBOL(bsp_ipc_spin_lock);
EXPORT_SYMBOL(bsp_ipc_spin_unlock);
EXPORT_SYMBOL(bsp_ipc_debug_show);
EXPORT_SYMBOL(bsp_int_send_info);
EXPORT_SYMBOL(bsp_ipc_sem_delete);
EXPORT_SYMBOL(bsp_ipc_spin_trylock);
EXPORT_SYMBOL(enable_ipc_irq);
EXPORT_SYMBOL(disable_ipc_irq);
EXPORT_SYMBOL(ccore_ipc_enable);
EXPORT_SYMBOL(ccore_ipc_disable);
EXPORT_SYMBOL(ipc_modem_reset_cb);
EXPORT_SYMBOL(bsp_ipc_spin_lock_timeout);
EXPORT_SYMBOL(bsp_ipc_int_mask_status_dump);

/*lint -restore +e19*/


