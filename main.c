//******************************************************//
//                                                      //
// Empty template for TM4C123GXL Projects using FreeRTOS//
//                                                      //
//******************************************************//

//******************************************************//
//                                                      //
// SHELL ECO Autonomous team                            //
//                                                      //
// Electric Control Sub-team                            //
//                                                      //
//******************************************************//



#include "Initializations.h"
#include "Defines.h"



//freeRTOS includes

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>


//CCS Default includes

#include <stdint.h>
#include <stdbool.h>


//TivaWARE minimal includes

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

#include "inc/hw_uart.h"




#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

#include "driverlib/uart.h"
#include "driverlib/usb.h"



#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"


#include "USB_tasks.h"
#include "usb_serial_structs.h"
uint8_t flag=0;
char read_char;

void tx_app (void)
{
    flag+=1;
}

void rx_app (void)
{
   USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,"R",1);
   USBBufferRead((tUSBBuffer *)&g_sRxBuffer,&read_char,1);
   USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,&read_char,1);
}

int main(void)
{

    vInit_USBTasks (tx_app,rx_app);
    // Prototype for xTaskCreate:
        //
        //  BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
        //                          const char * const pcName,
        //                          uint16_t usStackDepth,
        //                          void *pvParameters,
        //                          UBaseType_t uxPriority,
        //                          TaskHandle_t *pvCreatedTask);

    vTaskStartScheduler();  // Start FreeRTOS!

    // Should never get here since the RTOS should never "exit".
    while(1) ;
    return 0;


}




