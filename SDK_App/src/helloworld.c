
// dhq 2021 - Based on OOB design from Digilent and VDMA example


#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xv_tpg.h"

#include "xvidc.h"
#include <stdio.h>
#include "platform.h"

#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"

#define AXI_GPIO_0_BTN_CH 1
#define AXI_GPIO_0_LED_CH 2

XV_tpg 				tpg_inst;
XV_tpg_Config 		*tpg_config;


#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_assert.h"
#include "xaxivdma.h"
#include "xaxivdma_i.h"

#include "vdma_api.h"
#include "xintc.h"

/*** Global Variables ***/
unsigned int srcBuffer = (XPAR_MIG7SERIES_0_BASEADDR  + 0x1000000);

/* Instance of the Interrupt Controller */
static XIntc Intc;

/*****************************************************************************/
/*
 * Call back function for read channel
 *
 * The user can put his code that should get executed when this
 * call back happens.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void ReadCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	xil_printf("Read Call back function is called\r\n");
}

/*****************************************************************************/
/*
 * The user can put his code that should get executed when this
 * call back happens.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void ReadErrorCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	xil_printf("Read Call back Error function is called\r\n");

}

/*****************************************************************************/
/*The user can put his code that should get executed when this
 * call back happens.
 *
 *
 * This callback only clears the interrupts and updates the transfer status.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void WriteCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	xil_printf("Write Call back function is called\r\n");

}

/*****************************************************************************/
/*
* The user can put his code that should get executed when this
 * call back happens.
*
* @param	CallbackRef is the call back reference pointer
* @param	Mask is the interrupt mask passed in from the driver
*
* @return	None
*
******************************************************************************/
static void WriteErrorCallBack(void *CallbackRef, u32 Mask)
{

	/* User can add his code in this call back function */
	xil_printf("Write Call back Error function is called \r\n");

}

/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA.  This function assumes INTC component exists in the hardware system.
*
* @param	AxiDmaPtr is a pointer to the instance of the DMA engine
* @param	ReadIntrId is the read channel Interrupt ID.
* @param	WriteIntrId is the write channel Interrupt ID.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId, u16 WriteIntrId)
{
	int Status;


	XIntc *IntcInstancePtr =&Intc;

	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, XPAR_INTC_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {

		xil_printf( "Failed init intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, ReadIntrId,
	         (XInterruptHandler)XAxiVdma_ReadIntrHandler, AxiVdmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Failed read channel connect intc %d\r\n", Status);
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, WriteIntrId,
	         (XInterruptHandler)XAxiVdma_WriteIntrHandler, AxiVdmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Failed write channel connect intc %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {

		xil_printf( "Failed to start intc\r\n");
		return XST_FAILURE;
	}

	/* Enable interrupts from the hardware */
	XIntc_Enable(IntcInstancePtr, ReadIntrId);
	XIntc_Enable(IntcInstancePtr, WriteIntrId);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XIntc_InterruptHandler,
			(void *)IntcInstancePtr);

	Xil_ExceptionEnable();

	/* Register call-back functions
	 */
	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_GENERAL, ReadCallBack,(void *)AxiVdmaPtr, XAXIVDMA_READ);

	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_ERROR,ReadErrorCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);

	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_GENERAL,	WriteCallBack, (void *)AxiVdmaPtr, XAXIVDMA_WRITE);

	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_ERROR,WriteErrorCallBack, (void *)AxiVdmaPtr, XAXIVDMA_WRITE);
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function disables the interrupts
*
* @param	ReadIntrId is interrupt ID associated w/ DMA read channel
* @param	WriteIntrId is interrupt ID associated w/ DMA write channel
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DisableIntrSystem(u16 ReadIntrId, u16 WriteIntrId)
{

	/* Disconnect the interrupts for the DMA TX and RX channels */
	XIntc_Disconnect(&Intc, ReadIntrId);
	XIntc_Disconnect(&Intc, WriteIntrId);

}



int main(){

	int Status;

    init_platform();

	XGpio gpio;
	XGpio_Config *CfgPtr;
	u32 btn_last_state, btn_state;
	u32 led_state = 0b0001;
	u32 tmr = 0;
	u32 direction = 0;

	CfgPtr = XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
	XGpio_CfgInitialize(&gpio, CfgPtr, CfgPtr->BaseAddress);
	XGpio_InterruptGlobalDisable(&gpio);
	XGpio_DiscreteWrite(&gpio, AXI_GPIO_0_LED_CH, led_state);

	print("Welcome to the USB104 A7 Out-Of-Box Demo\n\r");
	btn_last_state = XGpio_DiscreteRead(&gpio, AXI_GPIO_0_BTN_CH);

    print("Hello World\n\r");

    xil_printf("2. TPG source setup!\r\n");

    /*************************************************************************************/
    /* Insert the code for the TPG here */
    print("TPG application on USB104A7 using FMC VGA\n\r");

     xil_printf("Initialize TPG.\r\n");

     Status = XV_tpg_Initialize(&tpg_inst, XPAR_V_TPG_0_DEVICE_ID);
     if(Status!= XST_SUCCESS)
     {
     	xil_printf("TPG configuration failed\r\n");
         	return(XST_FAILURE);
     }
     xil_printf("TPG Initialization Complete!\r\n");

     // Set Resolution to 800x600
     XV_tpg_Set_height(&tpg_inst, 480);
     XV_tpg_Set_width(&tpg_inst, 640);
     XV_tpg_Set_colorFormat(&tpg_inst, XVIDC_CSF_RGB);      // Set Color Space to RGB
     //Start the TPG
     XV_tpg_EnableAutoRestart(&tpg_inst);
     XV_tpg_Start(&tpg_inst);
     xil_printf("TPG started!\r\n");
     XV_tpg_Set_bckgndId(&tpg_inst, XTPG_BKGND_TARTAN_COLOR_BARS);      // Change the pattern to color bar
     //Start the TPG
     // Set Overlay to moving box
     // Set the size of the box
     XV_tpg_Set_boxSize(&tpg_inst, 100);      // Set the speed of the box
     XV_tpg_Set_motionSpeed(&tpg_inst, 2);
  	 XV_tpg_Set_boxColorR(&tpg_inst,255);
  	 XV_tpg_Set_boxColorG(&tpg_inst,128);
  	 XV_tpg_Set_boxColorB(&tpg_inst,100);
     XV_tpg_Set_ovrlayId(&tpg_inst, 1);      // Enable the moving box

	xil_printf("\n--- Entering main() --- \r\n");


	XAxiVdma InstancePtr;

	xil_printf("Starting the first VDMA \n\r");

	/* Calling the API to configure and start VDMA without frame counter interrupt */
	Status = run_triple_frame_buffer(&InstancePtr,0,640,480,srcBuffer,100,0);
	if (Status != XST_SUCCESS){
		xil_printf("Transfer of frames failed with error = %d\r\n",Status);
		return XST_FAILURE;
	}
	else
	{
		xil_printf("Transfer of frames started \r\n");
	}



	/* Infinite while loop to let it run */
	while(1)
		{
			if (tmr >= 249) {
				if (direction) {
					led_state = (led_state << 1) | ((led_state & 0b1000) >> 3);
				} else {
					led_state = (led_state >> 1) | ((led_state & 0b0001) << 3);
				}
				tmr = 0;
				XGpio_DiscreteWrite(&gpio, AXI_GPIO_0_LED_CH, led_state);
			} else {
				tmr++;
				usleep(1000);
			}
			btn_state = XGpio_DiscreteRead(&gpio, AXI_GPIO_0_BTN_CH);
			if ((btn_state & ~btn_last_state) != 0) {
				print("Button Press Detected!\n\r");
				switch (btn_state & ~btn_last_state) {
				case 0b01: direction = 0; break;
				case 0b10: direction = 1; break;
				}
				tmr++;
				usleep(1000);
			}
			btn_last_state = btn_state;

		}
	    cleanup_platform();
	    return 0;
}



