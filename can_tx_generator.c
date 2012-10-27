//*****************************************************************************
//
// can_device_qs.c - Target application that runs on the CAN device board and
//                   uses the CAN controller to communicate with the main
//                   board.
//
// Copyright (c) 2007-2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 9107 of the EK-LM3S8962 Firmware Package.
//
//*****************************************************************************

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/can.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "can_common.h"

tCANMsgObject tx_msg_object;
volatile unsigned long count = 0;
unsigned char tx_buffer[8];



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void __error__(char *pcFilename, unsigned long ulLine)
{
}
#endif


// The CAN controller interrupt handler.
void CANHandler(void)
{
    unsigned long ulStatus;

    //
    // Find the cause of the interrupt, if it is a status interrupt then just
    // acknowledge the interrupt by reading the status register.
    //
    ulStatus = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    switch(ulStatus)
    {
        // This was a status interrupt so read the current status to clear the interrupt and return.
        default:
        {
            CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
            return;
        }
    }

    // Acknowledge the CAN controller interrupt has been handled.
    CANIntClear(CAN0_BASE, ulStatus);
}


//*****************************************************************************
//
// This functions sends out a button update message.
//
//*****************************************************************************
void SendMessage(unsigned char *data)
{
    tx_msg_object.ulFlags = MSG_OBJ_EXTENDED_ID;
    tx_msg_object.ulMsgID = 0x10;
    tx_msg_object.ulMsgIDMask = 0;
    tx_msg_object.ulMsgLen = 8;
    tx_msg_object.pucMsgData = data;                //allocate memory for TX buffer
    tx_msg_object.pucMsgData[0] = 2;
    tx_msg_object.pucMsgData[1] = 3;
    tx_msg_object.pucMsgData[2] = 4;
    tx_msg_object.pucMsgData[3] = 5;
    tx_msg_object.pucMsgData[4] = 6;
    tx_msg_object.pucMsgData[5] = 7;
    tx_msg_object.pucMsgData[6] = 8;
    tx_msg_object.pucMsgData[7] = 9;
    CANMessageSet(CAN0_BASE, 1, &tx_msg_object, MSG_OBJ_TYPE_TX);
}

//*****************************************************************************
//
// This is the interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void SysTickIntHandler(void)
{
    if (count < 4)
    {
     SendMessage(tx_buffer);   
    }
    count++;
}



int main(void)
{
    //
    // If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    // a workaround to allow the PLL to operate reliably.
    //
    if(REVISION_IS_A2)
    {
        SysCtlLDOSet(SYSCTL_LDO_2_75V);
    }

    
    // Set the clocking to run directly from the PLL at 25MHz.
    SysCtlClockSet(SYSCTL_SYSDIV_8 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

    
    // Configure CAN 0 Pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeCAN(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Enable the CAN controller.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    //
    // Reset the state of all the message object and the state of the CAN
    // module to a known state.
    //
    CANInit(CAN0_BASE);

    //
    // Configure the bit rate for the CAN device, the clock rate to the CAN
    // controller is fixed at 8MHz for this class of device and the bit rate is
    // set to 250000.
    //
    CANBitRateSet(CAN0_BASE, 8000000, 1000000);

    //
    // Take the CAN0 device out of INIT state.
    //
    CANEnable(CAN0_BASE);

    //
    // Enable interrups from CAN controller.
    //
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR);


    // Enable interrupts for the CAN in the NVIC.
    IntEnable(INT_CAN0);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure SysTick for a 10ms interrupt.
    //
    //  1   = 1s - 
    //  100 = 10ms
    //  200 = 5ms
    //  500 = 2ms
    // 1000 = 1ms
    //10000 = 100us
    //20000 = 50us  - this is about the time a max CAN packet (50bytes) needs to be sent at max bit rate of 1Mbps
    SysTickPeriodSet(SysCtlClockGet() / 1);
    SysTickEnable();
    SysTickIntEnable();


    //
    // Loop forever.
    //
    while(1)
    {
       
    }
}
