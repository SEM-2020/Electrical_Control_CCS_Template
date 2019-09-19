/*
 * USB_tasks.c
 *
 *  Created on: ???/???/????
 *      Author: Tefa
 */


#include "USB_tasks.h"

/********************************************************************
 *                          Variables
 ********************************************************************/

uint8_t * uint8_USBTx = &g_pui8USBTxBuffer ;
uint8_t * uint8_USBRx = &g_pui8USBRxBuffer ;

static void (*ptr_transmitapp)(void) = NULL;
static void (*ptr_receiveapp)(void) = NULL ;
static void * vpCDCDevice = NULL ;

//*****************************************************************************
//
// Flags used to pass commands from interrupt context to the main loop.
//
//*****************************************************************************
#define COMMAND_PACKET_RECEIVED 0x00000001
#define COMMAND_STATUS_UPDATE   0x00000002

volatile uint32_t g_ui32Flags = 0;
char *g_pcStatus;

//*****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//*****************************************************************************
static volatile bool g_bUSBConfigured = false;

/*****************************************************************
 *                       Semaphores
 *****************************************************************/

SemaphoreHandle_t Sem_USBReceive ;
SemaphoreHandle_t Sem_USBTransmit ;

/********************************************************************
 *                        Private Functions
 ********************************************************************/

static void USB_HardwareConfiguration (void )
{
    // Set the clocking to run at 50 MHz from the PLL.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    ROM_IntMasterEnable();//enable global interrupt

    /* Configure the required pins for USB operation AS AN ANALOG function pins.*/
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);//enable clk to GPIOD
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);//PD4(PIN43)>>>>>(USB0D-)  PD5(PIN44)>>>>>>(USB0D+)


    /*INIT TX AND RX BUFFERS*/
    //buffer is defined in usb_serial_structs files
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    /*configure the tiva c to be device(slave) mode on the bus */
    /*so the computer will be host(master) starts handshakes process*/
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    /*configure handshaking to  start(enumeration)
     * (our device information to the USB library to inform pc its identity )
     * */
    vpCDCDevice = USBDCDCInit(0, &g_sCDCDevice);

    /*enable usb interrupt*/
    ROM_IntEnable(INT_USB0);

}

/*****************************CALLBACK ROUTINES*********************************/

uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,uint32_t ui32MsgValue, void *pvMsgData){}

//*****************************************************************************
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//*****************************************************************************
uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,void *pvMsgData)
{
    xSemaphoreGive(Sem_USBTransmit);
    return(0);
}


//*****************************************************************************
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//*******************************
uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,void *pvMsgData)
{
    xSemaphoreGive(Sem_USBReceive);
    return(0);
}


/********************************************************************
 *                        Public Functions
 ********************************************************************/

void vTASK_USBReceive (void)
{
    while (1)
    {
        xSemaphoreTake(Sem_USBReceive,portMAX_DELAY);
        USBDCDCPacketRead(vpCDCDevice,g_pui8USBRxBuffer,UART_BUFFER_SIZE,false);
        if(ptr_receiveapp != NULL)
        {
            (*ptr_receiveapp)();
        }
    }
}

void vTASK_USBTransmit (void)
{
    while(1)
    {
        xSemaphoreTake(Sem_USBTransmit,portMAX_DELAY);
        USBDCDCPacketWrite(vpCDCDevice,g_pui8USBTxBuffer,UART_BUFFER_SIZE,false);
        if(ptr_transmitapp != NULL)
        {
            (*ptr_transmitapp)();
        }

    }
}

void vInit_USBTasks(void (* Ptr_TxHandler)(void), void (* Ptr_RxHandler)(void) )
{
    /* getting hardware ready */
    USB_HardwareConfiguration();

    /* creating semaphores */
    Sem_USBReceive = xSemaphoreCreateBinary();
    Sem_USBTransmit = xSemaphoreCreateBinary();

    /* creating tasks */
    xTaskCreate(vTASK_USBReceive,NULL,USB_StackDepth,NULL,USBReceive_prio,NULL);
    xTaskCreate(vTASK_USBTransmit,NULL,USB_StackDepth,NULL,USBTransmit_prio,NULL);


    ptr_transmitapp = Ptr_TxHandler ;
    ptr_receiveapp = Ptr_RxHandler ;
}
