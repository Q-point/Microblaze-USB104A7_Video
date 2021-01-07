/******************************************************************************
*
* (c) Copyright 2014-2015 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file vdma_api.c
 *
 * This file has high level API to configure and start the VDMA.The file assumes that:
 * The design has VDMA with both MM2S and S2MM path enable.
 * The API's has tested with hardware that has tow VDMA and MM2S to S2MM are back
 * to back connected for each VDMA.
 *
 * ***************************************************************************/

/******************** Include files **********************************/

/*****************************************************************************
*
* run_triple_frame_buffer API
*
* This API is the interface between application and other API. When application will call
* this API with right argument, This API will call rest of the API to configure the read
* and write path of VDMA,based on ID. After that it will start both the read and write path
* of VDMA
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @Argument
* 		- InstancePtr:		The handle to XAxiVdma data structure.
* 		- DeviceId:			The device ID of current VDMA
* 		- hsize:			The horizontal size of the frame. It will be in Pixels.
* 							The actual size of frame will be calculated by multiplying this
* 							with tdata width.
* 		-vsize:				Vertical size of the frame.
* 		-buf_base_addr:		The buffer address where frames will be written and read by VDMA
* 		-number_frame_count:If application needs interrupt on frame processing, this variable
* 							will tell after how many frames interrupt should come.
* 		-enable_frm_cnt_intr: Enable frame count interrupt.
*
******************************************************************************/

int run_triple_frame_buffer(XAxiVdma* InstancePtr, int DeviceId, int hsize,\
		int vsize, int buf_base_addr, int number_frame_count, int enable_frm_cnt_intr);
