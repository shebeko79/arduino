/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                   *
 *---------------------------------------------------------------------
 * FileName      :   app_demo_gpio.c
 * version       :   0.10
 * Description   :   
 * Authors       :   fangshengchang
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2012-09-24, fangshengchang, create originally.
 *0.20  2013-03-26, maobin, modify the PIN definition, to adapt to the SIM800W and SIM800V
 *
 *--------------------------------------------------------------------
 * File Description
 * AT+CEAT=parma1,param2
 * param1 param2 
 *   1      1    Write the EAT_GPIO_TEST1,EAT_GPIO_TEST2 pin LEVEL_LOW 
 *   1      2    Write the EAT_GPIO_TEST1,EAT_GPIO_TEST2 pin LEVEL_HIGH
 *   1      3    Read the EAT_GPIO_TEST1,EAT_GPIO_TEST2 pin 
 *
 *   2      1     eat_lcd_light_sw(EAT_TRUE); eat_kpled_sw(EAT_TRUE);
 *   2      2     eat_lcd_light_sw(EAT_FALSE);eat_kpled_sw(EAT_FALSE);
 
 *--------------------------------------------------------------------
 ********************************************************************/
/********************************************************************
 * Include Files
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include "eat_modem.h"
#include "eat_interface.h"
#include "eat_periphery.h"
#include "eat_uart.h"

#include "eat_clib_define.h" //only in main.c
/********************************************************************
 * Macros
 ********************************************************************/

/********************************************************************
* Types
 ********************************************************************/
typedef void (*app_user_func)(void*);
/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/
 
/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/

/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_func_ext1(void *data);
/********************************************************************
 * Local Function
 ********************************************************************/
#pragma arm section rodata = "APP_CFG"
APP_ENTRY_FLAG 
#pragma arm section rodata

#pragma arm section rodata="APPENTRY"
	const EatEntry_st AppEntry = 
	{
		app_main,
		app_func_ext1,
		(app_user_func)EAT_NULL,//app_user1,
		(app_user_func)EAT_NULL,//app_user2,
		(app_user_func)EAT_NULL,//app_user3,
		(app_user_func)EAT_NULL,//app_user4,
		(app_user_func)EAT_NULL,//app_user5,
		(app_user_func)EAT_NULL,//app_user6,
		(app_user_func)EAT_NULL,//app_user7,
		(app_user_func)EAT_NULL,//app_user8,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL
	};
#pragma arm section rodata


u32 g_pin_set_result = 0;
void app_func_ext1(void *data)
{
   u32 ret_val=0;
	/*This function can be called before Task running ,configure the GPIO,uart and etc.
	   Only these api can be used:
		 eat_uart_set_debug: set debug port
		 eat_pin_set_mode: set GPIO mode
		 eat_uart_set_at_port: set AT port
	*/
    eat_uart_set_debug(EAT_UART_2);
    eat_uart_set_at_port(EAT_UART_1);// UART1 is as AT PORT
    
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN3_GPIO1, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN4_STATUS, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN5_BPI_BUSI, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN20_COL0, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN21_COL3, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN22_COL2, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN23_ROW3, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN25_COL1, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN26_PWM, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN27_GPIO2, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN28_GPIO3, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN29_PCM_CLK, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN30_PCM_OUT, EAT_PIN_MODE_GPIO);
//    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN33_CTS, EAT_PIN_MODE_UART);
//    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN34_RTS, EAT_PIN_MODE_UART);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN60_ROW1, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN61_ROW2, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN62_ROW0, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN63_ROW4 , EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN64_NETLIGHT, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN65_PCM_SYNC, EAT_PIN_MODE_GPIO); 
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN66_PCM_IN, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN68_UART1_RI , EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN69_UART1_DTR, EAT_PIN_MODE_GPIO); 
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN70_UART1_DCD, EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN74_SCL , EAT_PIN_MODE_GPIO);
    ret_val =ret_val<<1||eat_pin_set_mode(EAT_PIN75_SDA , EAT_PIN_MODE_GPIO);
   
    g_pin_set_result = ret_val;
 }
eat_bool eat_modem_data_parse(u8* buffer, u16 len, u8* param1, u8* param2)
{
    eat_bool ret_val = EAT_FALSE;
    u8* buf_ptr = NULL;
    /*param:%d,extern_param:%d*/
     buf_ptr = (u8*)strstr((const char *)buffer,"param");
    if( buf_ptr != NULL)
    {
        sscanf((const char *)buf_ptr, "param:%d,extern_param:%d",(int*)param1, (int*)param2);
        eat_trace("data parse param1:%d param2:%d",*param1, *param2);
        ret_val = EAT_TRUE;
    }
    return ret_val;
}

void eat_app_usb_eint(eat_bool en,EAT_USB_MODE mode)
{
    if(en)
    {
        eat_trace(" eat_app_usb_eint ENTRY--TEST+++++++++++++++++"); 
        eat_trace(" eat_app_usb_eint USB PLUG IN-mode=%d",mode); 
    }
    else
    {
        eat_trace(" eat_app_usb_eint ENTRY--TEST-------------------"); 
    
    }
}

/****************************************************
 * GPIO testing module
 *****************************************************/
eat_bool eat_module_test_gpio(u8 param1, u8 param2)
{
    /***************************************
     * example 1
     * used eat_gpio_write test EAT_GPIO_TEST1,EAT_GPIO_TEST2
     ***************************************/
    eat_trace("GPIO test param1=%d,param2=%d",param1,param2);
    if( 1 == param1 )
    {

        if( 1 == param2 )
        {
            eat_trace("GPIO test 1,1 start");
            eat_gpio_setup(EAT_PIN3_GPIO1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN4_STATUS, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN5_BPI_BUSI, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN20_COL0, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN21_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN22_COL2, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN23_ROW3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN24_COL4, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN25_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN26_PWM, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN27_GPIO2, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN28_GPIO3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN29_PCM_CLK, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN30_PCM_OUT, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN60_ROW1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN61_ROW2, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN62_ROW0, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN63_ROW4 , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN64_NETLIGHT, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN65_PCM_SYNC, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW); 
            eat_gpio_setup(EAT_PIN66_PCM_IN, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN68_UART1_RI , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN69_UART1_DTR, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW); 
            eat_gpio_setup(EAT_PIN70_UART1_DCD, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN74_SCL , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN75_SDA , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
            eat_trace("GPIO test 1,1 end");
           
        }else if( 2 == param2)
        {                   
            eat_trace("GPIO test 1,2 start");
            eat_gpio_setup(EAT_PIN3_GPIO1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN4_STATUS, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN5_BPI_BUSI, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN20_COL0, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN21_COL3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN22_COL2, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN23_ROW3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN24_COL4, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN25_COL1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN26_PWM, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN27_GPIO2, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN28_GPIO3, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN29_PCM_CLK, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN30_PCM_OUT, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN60_ROW1, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN61_ROW2, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN62_ROW0, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN63_ROW4 , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN64_NETLIGHT, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN65_PCM_SYNC, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH); 
            eat_gpio_setup(EAT_PIN66_PCM_IN, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN68_UART1_RI , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN69_UART1_DTR, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH); 
            eat_gpio_setup(EAT_PIN70_UART1_DCD, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN74_SCL , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_setup(EAT_PIN75_SDA , EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_HIGH);
            eat_trace("GPIO test 1,2 end");
           
        }else if(3 == param2)
        {
            eat_trace("GPIO test 1,3 start");
           eat_gpio_setup(EAT_PIN3_GPIO1, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN4_STATUS, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN5_BPI_BUSI, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN20_COL0, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN21_COL3, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN22_COL2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN23_ROW3, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN24_COL4, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN25_COL1, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN26_PWM, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN27_GPIO2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN28_GPIO3, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN29_PCM_CLK, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN30_PCM_OUT, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN60_ROW1, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN61_ROW2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN62_ROW0, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN63_ROW4 , EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN64_NETLIGHT, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN65_PCM_SYNC, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH); 
           eat_gpio_setup(EAT_PIN66_PCM_IN, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN68_UART1_RI , EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN69_UART1_DTR, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH); 
           eat_gpio_setup(EAT_PIN70_UART1_DCD, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN74_SCL , EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_gpio_setup(EAT_PIN75_SDA , EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);
           eat_trace("GPIO test 1,3 end");
           
           eat_trace("EAT_PIN3_GPIO1= %d",eat_gpio_read(EAT_PIN3_GPIO1));
           eat_trace("EAT_PIN4_STATUS= %d",eat_gpio_read(EAT_PIN4_STATUS));
           eat_trace("EAT_PIN5_BPI_BUSI= %d",eat_gpio_read(EAT_PIN5_BPI_BUSI));
           eat_trace("EAT_PIN20_COL0= %d",eat_gpio_read(EAT_PIN20_COL0));
           eat_trace("EAT_PIN21_COL3= %d",eat_gpio_read(EAT_PIN21_COL3));
           
           eat_trace("EAT_PIN22_COL2= %d",eat_gpio_read(EAT_PIN22_COL2));
           eat_trace("EAT_PIN23_ROW3= %d",eat_gpio_read(EAT_PIN23_ROW3));
           eat_trace("EAT_PIN24_COL4= %d",eat_gpio_read(EAT_PIN24_COL4));
           eat_trace("EAT_PIN25_COL1= %d",eat_gpio_read(EAT_PIN25_COL1));
           eat_trace("EAT_PIN26_PWM= %d",eat_gpio_read(EAT_PIN26_PWM));
           
           eat_trace("EAT_PIN27_GPIO2= %d",eat_gpio_read(EAT_PIN27_GPIO2));
           eat_trace("EAT_PIN28_GPIO3= %d",eat_gpio_read(EAT_PIN28_GPIO3));
           eat_trace("EAT_PIN29_PCM_CLK= %d",eat_gpio_read(EAT_PIN29_PCM_CLK));
           eat_trace("EAT_PIN30_PCM_OUT= %d",eat_gpio_read(EAT_PIN30_PCM_OUT));
           eat_trace("EAT_PIN60_ROW1= %d",eat_gpio_read(EAT_PIN60_ROW1));
           eat_trace("EAT_PIN61_ROW2= %d",eat_gpio_read(EAT_PIN61_ROW2));
           eat_trace("EAT_PIN62_ROW0= %d",eat_gpio_read(EAT_PIN62_ROW0));
           
           eat_trace("EAT_PIN63_ROW4= %d",eat_gpio_read(EAT_PIN63_ROW4 ));
           eat_trace("EAT_PIN64_NETLIGHT= %d",eat_gpio_read(EAT_PIN64_NETLIGHT));
           eat_trace("EAT_PIN65_PCM_SYNC= %d",eat_gpio_read(EAT_PIN65_PCM_SYNC)); 
           eat_trace("EAT_PIN66_PCM_IN= %d",eat_gpio_read(EAT_PIN66_PCM_IN));
           eat_trace("EAT_PIN68_UART1_RI= %d",eat_gpio_read(EAT_PIN68_UART1_RI) );
           
           eat_trace("EAT_PIN69_UART1_DTR= %d",eat_gpio_read(EAT_PIN69_UART1_DTR)); 
           eat_trace("EAT_PIN70_UART1_DCD= %d",eat_gpio_read(EAT_PIN70_UART1_DCD));
           eat_trace("EAT_PIN74_SCL= %d",eat_gpio_read(EAT_PIN74_SCL ));
           eat_trace("EAT_PIN75_SDA= %d",eat_gpio_read(EAT_PIN75_SDA ));
        }else if(4 == param2)
        {
            eat_pin_set_mode(EAT_PIN25_COL1, EAT_PIN_MODE_GPIO);
            eat_pin_set_mode(EAT_PIN22_COL2, EAT_PIN_MODE_GPIO);
            eat_pin_set_mode(EAT_PIN21_COL3, EAT_PIN_MODE_GPIO);
            eat_pin_set_mode(EAT_PIN24_COL4, EAT_PIN_MODE_GPIO);

            eat_gpio_setup(EAT_PIN25_COL1, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN22_COL2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN21_COL3, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN24_COL4, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);

            eat_gpio_setup(EAT_PIN26_PWM, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_HIGH);

            eat_gpio_set_pull(EAT_PIN25_COL1, EAT_TRUE, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_set_pull(EAT_PIN22_COL2, EAT_TRUE, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_set_pull(EAT_PIN21_COL3, EAT_TRUE, EAT_GPIO_LEVEL_HIGH);
            eat_gpio_set_pull(EAT_PIN24_COL4, EAT_TRUE, EAT_GPIO_LEVEL_HIGH);

            eat_gpio_set_pull(EAT_PIN26_PWM, EAT_TRUE, EAT_GPIO_LEVEL_HIGH);
        }else if(5 == param2)
        {
		    eat_gpio_setup(EAT_PIN25_COL1, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN22_COL2, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN21_COL3, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
            eat_gpio_setup(EAT_PIN24_COL4, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);
			
            eat_gpio_setup(EAT_PIN26_PWM, EAT_GPIO_DIR_INPUT, EAT_GPIO_LEVEL_LOW);

            eat_gpio_set_pull(EAT_PIN25_COL1, EAT_TRUE, EAT_GPIO_LEVEL_LOW);
            eat_gpio_set_pull(EAT_PIN22_COL2, EAT_TRUE, EAT_GPIO_LEVEL_LOW);
            eat_gpio_set_pull(EAT_PIN21_COL3, EAT_TRUE, EAT_GPIO_LEVEL_LOW);
            eat_gpio_set_pull(EAT_PIN24_COL4, EAT_TRUE, EAT_GPIO_LEVEL_LOW);

            eat_gpio_set_pull(EAT_PIN26_PWM, EAT_TRUE, EAT_GPIO_LEVEL_LOW);
        }

    }else if(param1 == 2)
     {
         if( 1== param2 )
          {
              eat_lcd_light_sw(EAT_TRUE, EAT_BL_STEP_20_MA );
              eat_kpled_sw(EAT_TRUE);
          }else if( 2 == param2 )
          {
              eat_lcd_light_sw(EAT_FALSE, 0);
              eat_kpled_sw(EAT_FALSE);
          }
    }else if(param1 == 3)
    {
        if( 1== param2 )
        {
            eat_gpio_write(EAT_PIN64_NETLIGHT, EAT_GPIO_LEVEL_HIGH);
        }else if( 2 == param2 )
        {
            eat_gpio_write(EAT_PIN64_NETLIGHT, EAT_GPIO_LEVEL_LOW);
        }
    }
    else if(param1 == 4)
    {
        if( 1== param2 )
        {
            eat_usb_eint_register(eat_app_usb_eint);
        }
        else if( 2 == param2 )
        {
            eat_usb_eint_register(NULL);
        }
    }
    return EAT_TRUE;
}
void app_main(void *data)
{
    EatEvent_st event;
    u8 buf[2048];
    u16 len = 0;

    APP_InitRegions();//Init app RAM, first step
    APP_init_clib(); //C library initialize, second step

    eat_trace(" app_main ENTRY");
    eat_modem_write("AT+CNETLIGHT=0\r\n",16);
    while(EAT_TRUE)
    {
        eat_get_event(&event);
        eat_trace("MSG id%x", event.event);
        switch(event.event)
        {
            case EAT_EVENT_MDM_READY_RD:
                {
                    u8 param1,param2;
                    len = 0;
                    len = eat_modem_read(buf, 2048);
                    if(len > 0)
                    {
                        //Get the testing parameter
                        if(eat_modem_data_parse(buf,len,&param1,&param2))
                        {
                            //Entry gpio test module
                            eat_module_test_gpio(param1, param2);
                        }
                        else
                        {
                            eat_trace("From Mdm:%s",buf);
                        }
                    }

                }
                break;
            case EAT_EVENT_MDM_READY_WR:
            case EAT_EVENT_UART_READY_RD:
                break;
            case EAT_EVENT_UART_SEND_COMPLETE :
                break;
            default:
                break;
        }

    }

}


