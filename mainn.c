/*
 * File:   mainn.c
 * Author: N Revathi
 *
 * Created on 13 November, 2024, 5:00 PM
 */



#include <xc.h>
#include "main.h"
#include "clcd.h"
#include "mkp.h"
#include "adc.h"
#include "external_eeprom.h"
#include "i2c.h"
#include "ds1307.h"
#include "uart.h"

unsigned char screen_status = DASHBOARD_SCREEN;
unsigned char Event[9][3]; 
unsigned char speed[3];  
unsigned char menu_index = 0,field_index = 0,log_flag = 0,menu_flag = 0;  
unsigned char event_flag = 0;
 
void init_config()
{
    init_matrix_keypad();       // Initialize matrix keypad
    init_clcd();                // Initialize CLCD
    init_adc();                 // Initialize ADC
    init_i2c();                 // Initialize I2C
	init_ds1307();              // Initialize RTC & External EEPROM
    init_uart();                // Initialize Uart
    
}

void main(void) 
{
    init_config();              //configure peripherals
    while(1)
    {
        if(screen_status == DASHBOARD_SCREEN)     
        {
            dashboard_screen(Event,speed);              //show dashboard screen   
        }
        else if(screen_status == MENU_SCREEN)
        {
            menu_screen();                             //show menu screen
        }
        else if(screen_status == SET_TIME)
        {
            display_set_time(field_index);             //show set time
        }
        
        unsigned char key = ALL_RELEASED;
        key = read_switches(STATE_CHANGE);              // Read key is pressed
        switch(key)
        {
            case MK_SW1:
                if(screen_status == DASHBOARD_SCREEN)     
                {
                    event_flag = COLLISION;                  //make event collision 
                    change_event();                          //call function to change event
                }
                else if(screen_status == MENU_SCREEN)
                {
                    display_selected_menu(menu_index);      //display the selected menu
                }
                else if(screen_status == SET_TIME)                           
                {
                    set_time();                             // Save the updated time to the RTC.
                    CLEAR_DISP_SCREEN;
                    screen_status = MENU_SCREEN;   
                }
                break;

            case MK_SW2:
                if(screen_status == DASHBOARD_SCREEN )     
                {
                    event_flag = INCREMENT_EVENT;              //increment event index 
                    change_event();                            //call function to change event
                } 
                else if(screen_status == MENU_SCREEN)
                {
                    menu_index = 0; 
                    CLEAR_DISP_SCREEN;
                    screen_status = DASHBOARD_SCREEN;          //come back to main  dashboard screen             
                }
                else if(screen_status == SET_TIME || screen_status == VIEW_LOG)                            
                {
                    CLEAR_DISP_SCREEN;
                    screen_status = MENU_SCREEN;              //come back to menu screen    
                }
                break;

            case MK_SW3:
                if(screen_status == DASHBOARD_SCREEN)   
                {
                    event_flag = DECREMENT_EVENT;             //decrement event index
                    change_event();                           //call function to change event
                }
                break;

            case MK_SW11:
                if(screen_status == DASHBOARD_SCREEN)         
                {   
                    CLEAR_DISP_SCREEN;
                    screen_status = MENU_SCREEN;              //Switch to menu screen  
                }
                else if(screen_status == MENU_SCREEN)
                {
                    menu_flag = DECREMENT_MENU;              //to move previous menu 
                    change_menu();                           //call function to change menu
                }
                else if(screen_status == SET_TIME)                          
                {
                    change_field();                          //call function to change time field
                }
                else if(screen_status == VIEW_LOG)          
                {
                    log_flag = INCREMENT_LOG;                //increment logs 
                    change_log();                            //call function to change log
                }
                break;

            case MK_SW12:
                if(screen_status == MENU_SCREEN)
                {
                    menu_flag = INCREMENT_MENU;              //to move next menu 
                    change_menu();                           //call function to change menu
                }
                else if(screen_status == SET_TIME)                          
                {
                    inc_time(field_index);                  //increase the selected field value       
                }
                else if(screen_status == VIEW_LOG)         
                {
                    log_flag = DECREMENT_LOG;              //decrement logs 
                    change_log();                          //call function to change log
                }
                break;
        }  
    }
    return;
}

    

