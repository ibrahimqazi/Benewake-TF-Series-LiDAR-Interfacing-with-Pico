#include<stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"
#include "tusb.h" // this header file will handle the problem of losing initial output

// which port we want to use uart0 or uart1
#define UART_ID0 uart0
#define BAUD_RATE 115200
#define UART_ID1 uart1

// We are using pins 0 and 1 for uart0 and pins 6 and 7 for uart1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART0_TX_PIN 0 // pin-1
#define UART0_RX_PIN 1 // pin-2

#define UART1_TX_PIN 4 // pin-6
#define UART1_RX_PIN 5 // pin-7

const uint LED_PIN = 25; // also set LED from gpio.h file
static bool ret;

//****************************Structure and Union for handling LiDAR Data***********

//Dist_L Dist_H Strength_L Strength_H Temp_L Temp_H Checksum
typedef struct{
unsigned short Header;
unsigned short Dist;
unsigned short Strength;
}structLidar;

union unionLidar{
unsigned char Byte[9];
structLidar lidar;
};

unsigned char lidarCounter=0;
union unionLidar Lidar;
//****************************Structure and Union for handling LiDAR Data***********

//****************************Function to read serial data***********
int isLidar(uart_inst_t * uart, union unionLidar * lidar)
{
  int loop;
  int checksum;
  unsigned char serialChar;

  while(uart_is_readable(uart))
  {
   if(lidarCounter > 8)
    {
      lidarCounter=0;
      return 0; // something wrong
    }

   serialChar = uart_getc(uart); // Read a single character to UART.
   lidar->Byte[lidarCounter]= serialChar;

   switch(lidarCounter++)
   {
    case 0:
    case 1:
    	   if(serialChar !=0x59)
              lidarCounter=0;
           break;
    case 8: // checksum
            checksum=0;
            lidarCounter=0;
            for(loop=0;loop<8;loop++)
                checksum+= lidar->Byte[loop];
            if((checksum &0xff) == serialChar)
                {
                  //printf("checksum ok\n");
                  lidar->lidar.Dist = lidar->Byte[2] | lidar->Byte[3] << 8;
                  lidar->lidar.Strength = lidar->Byte[4] | lidar->Byte[5] << 8;
                  return 1;
                }
	    //printf("bad checksum %02x != %02x\n",checksum & 0xff, serialChar);
    }
   }
    return 0;
}
//****************************Function to read serial data***********

//****************************Main Function**************************
int main(){

//******************************************************************
    // add some binary info
    // we need to add pico/binary_info.h for this.
    bi_decl(bi_program_description("This is a program to read from UART!"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    // add binary info for uart0 and uart1
    bi_decl(bi_1pin_with_name(UART0_TX_PIN, "pin-0 for uart0 TX"));
    bi_decl(bi_1pin_with_name(UART0_RX_PIN, "pin-1 for uart0 RX"));
    bi_decl(bi_1pin_with_name(UART1_TX_PIN, "pin-5 for uart1 TX"));
    bi_decl(bi_1pin_with_name(UART1_RX_PIN, "pin-6 for uart1 RX"));
//******************************************************************

// Enable UART so we can print status output
stdio_init_all();

gpio_init(LED_PIN); // initialize pin-25
gpio_set_dir(LED_PIN, GPIO_OUT); // set pin-25 in output mode

// Set up our UARTs with the required speed.
uart_init(UART_ID0, BAUD_RATE);
uart_init(UART_ID1, BAUD_RATE);

// Set the TX and RX pins by using the function
// Look at the datasheet for more information on function select
gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

//************************************************************
cdcd_init();
printf("waiting for usb host");
while (!tud_cdc_connected()) {
  printf(".");
  sleep_ms(500);
}
printf("\nusb host detected!\n");
//************************************************************
// In a default system, printf will also output via the default UART
sleep_ms(5000);
ret = uart_is_enabled (uart1); // pass UART_ID1 or uart1 both are okay
    if(ret == true){
        printf("UART-1 is enabled\n");
    }
  printf("Ready to read data from Benewake LiDAR\n");
  while(true){
    gpio_put(LED_PIN, 0);
    sleep_ms(100);
    gpio_put(LED_PIN, 1);
   if(isLidar(UART_ID1,&Lidar))
       {
        // ok we got valid data
        // Here we utilized the Union
        printf("Dist:%u Strength:%u \n",\
               Lidar.lidar.Dist,\
               Lidar.lidar.Strength);
       }
   }
return 0;
}