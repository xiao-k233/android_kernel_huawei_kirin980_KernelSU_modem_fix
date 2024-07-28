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

#ifndef __MDRV_CLK_COMMON_H__
#define __MDRV_CLK_COMMON_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <mdrv_public.h>

/************************************************************************
 * 函 数 名 : mdrv_acc_clk_enable
 * 功能描述 : 打开时钟
 * 输入参数 : mdrv_clk_acc_type: 时钟枚举类型
 * 输出参数 : 无
 * 返 回 值 : 0，操作成功
 *           -1，操作失败
 **************************************************************************/
int mdrv_acc_clk_enable(MDRV_CLK_ACC_TYPE acc_clk_index);

/************************************************************************
 * 函 数 名 : mdrv_acc_clk_disable
 * 功能描述 : 关闭时钟
 * 输入参数 : mdrv_clk_acc_type: 时钟枚举类型
 * 输出参数 : 无
 * 返 回 值 : 无
 **************************************************************************/
void mdrv_acc_clk_disable(MDRV_CLK_ACC_TYPE acc_clk_index);

/************************************************************************
 * 函 数 名 : mdrv_acc_clk_is_enabled
 * 功能描述 : 查询时钟状态
 * 输入参数 : mdrv_clk_acc_type: 时钟枚举类型
 * 输出参数 : 无
 * 返 回 值 : 1，时钟是打开的
              0，时钟是关闭的
             -1，无该时钟节点
 **************************************************************************/
int mdrv_acc_clk_is_enabled(MDRV_CLK_ACC_TYPE acc_clk_index);

/************************************************************************
 * 函 数 名 : mdrv_clk_enable
 * 功能描述 : 打开时钟
 * 输入参数 : mdrv_clk_type: 时钟枚举类型
 * 输出参数 : 无
 * 返 回 值 : 0，操作成功
 *           -1，操作失败
 **************************************************************************/
int mdrv_clk_enable(MDRV_CLK_TYPE mdrv_clk_index);

/************************************************************************
 * 函 数 名 : mdrv_clk_disable
 * 功能描述 : 关闭时钟
 * 输入参数 : mdrv_clk_type: 时钟枚举类型
 * 输出参数 : 无
 * 返 回 值 : 无
 **************************************************************************/
void mdrv_clk_disable(MDRV_CLK_TYPE mdrv_clk_index);

/************************************************************************
 * 函 数 名 : mdrv_clk_is_enabled
 * 功能描述 : 查询时钟状态
 * 输入参数 : mdrv_clk_type: 时钟枚举类型
 * 输出参数 : 无
 * 返 回 值 : 1，时钟是打开的
              0，时钟是关闭的
             -1，无该时钟节点
 **************************************************************************/
int mdrv_clk_is_enabled(MDRV_CLK_TYPE mdrv_clk_index);

/************************************************************************
 * 函 数 名 : mdrv_clk_get_rate
 * 功能描述 : 获取时钟频率
 * 输入参数 : mdrv_clk_type: 时钟枚举类型
 * 输出参数 : 无
 * 返 回 值 : 0，获取频率异常
              非0，时钟频率
 **************************************************************************/
unsigned long mdrv_clk_get_rate(MDRV_CLK_TYPE mdrv_clk_index);

/************************************************************************
 * 函 数 名 : mdrv_clk_round_rate
 * 功能描述 :得到和期望设置的rate比较接近的频率值
 * 输入参数 : mdrv_clk_type: 时钟枚举类型,rate: 期望设置的频率值
 * 输出参数 : 无
 * 返 回 值 : 0，round频率异常
              非0，round后的时钟频率
 **************************************************************************/
unsigned long mdrv_clk_round_rate(MDRV_CLK_TYPE mdrv_clk_index, unsigned long rate);

/************************************************************************
 * 函 数 名 : mdrv_clk_set_rate
 * 功能描述 : 设置时钟频率
 * 输入参数 : mdrv_clk_type: 时钟枚举类型,rate: 设置的频率值
 * 输出参数 : 无
 * 返 回 值 : 0，设置频率成功
              非0，设置频率失败
 **************************************************************************/
int mdrv_clk_set_rate(MDRV_CLK_TYPE mdrv_clk_index, unsigned long rate);

#ifdef __cplusplus
}
#endif

#endif
