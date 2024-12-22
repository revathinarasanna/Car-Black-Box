/*
 * File:   function.c
 * Author: N Revathi
 * Created on 13 November, 2024, 10:19 AM
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

extern unsigned char screen_status,event_flag,log_flag,menu_flag;
unsigned char time[7] = {"000000"};
unsigned char Event[9][3] = {"ON","GN","G1","G2","G3","G4","G5","GR","C "};
unsigned char speed[3];
unsigned char menu[4][17] = {"VIEW LOG        ", "CLEAR LOG       ", "DOWNLOAD LOG    ", "SET TIME        "};
unsigned char address;
unsigned int event_count = 0; 
unsigned char event_index = 0,menu_index = 0,field_index  = 0,log_index = 0,blink_flag = 1,flag = 0,star = 0;  
unsigned char logs[LOG_SIZE][LOG_LENGTH];  

void dashboard_screen(unsigned char Event[9][3],unsigned char speed[3])
{ 
    clcd_print("  TIME     EV SP", LINE1(0));         
    get_time();                                                             // Retrieve the current time and store it in the `time` array 
    clcd_putch(time[0], LINE2(2)); 
    clcd_putch(time[1], LINE2(3)); 
    clcd_putch(':', LINE2(4));     
    clcd_putch(time[2], LINE2(5)); 
    clcd_putch(time[3], LINE2(6));
    clcd_putch(':', LINE2(7));     
    clcd_putch(time[4], LINE2(8)); 
    clcd_putch(time[5], LINE2(9)); 
    clcd_print(Event[event_index],LINE2(11));                                //display event
    display_speed();                                                         //call function to read speed
}

static void get_time(void)
{
    unsigned char clock_reg[3];                                   // Array to store the current time values read from RTC
    clock_reg[0] = read_ds1307(HOUR_ADDR);                        // Read hours from the RTC 
    clock_reg[1] = read_ds1307(MIN_ADDR);                         // Read minutes from the RTC
    clock_reg[2] = read_ds1307(SEC_ADDR);                         // Read seconds from the RTC
   
    if(clock_reg[0] & 0x40)                                       // Convert hour to string for 12-hour format
    {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01); 
        time[1] = '0' + (clock_reg[0] & 0x0F);        
    }
    else                                                          // Convert hour to string for 24-hour format      
    {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03); 
        time[1] = '0' + (clock_reg[0] & 0x0F);        
    }
    time[2] = '0' + ((clock_reg[1] >> 4) & 0x0F);                 // Convert minutes and seconds to string
    time[3] = '0' + (clock_reg[1] & 0x0F);            
    time[4] = '0' + ((clock_reg[2] >> 4) & 0x0F);     
    time[5] = '0' + (clock_reg[2] & 0x0F);            
    time[6] = '\0';                                   
}

void change_event(void)
{
    if(event_flag == COLLISION)                        //collision
    {
        event_index = 8;
    }
    else if(event_flag == INCREMENT_EVENT)             //increment gear based on index
    {
        if(event_index < 7)
        {
            event_index++;   
        }
        else if(event_index == 8)                      //once collision then continue operation
        {
            event_index = 1;   
        }
    }
    else if(event_flag == DECREMENT_EVENT)             //decrement gear based on index
    {
        if(event_index <= 7 && event_index > 1)
        {
            event_index--; 
        }
        else if(event_index == 8)                      //once collision then continue operation
        {
            event_index = 1;   
        }
    }
    else                                               //when board is reset it indicates 'ON'
    {
        event_index = 0; 
    }
    flag = 0;                                          //to view flag 
    event_store();                                     //store the event in external EEPROM
}

void event_store(void)
{
    address =(event_count % LOG_SIZE) * LOG_LENGTH;      //to reinitialize address after every 10 logs
    for(int i = 0;i < 6;i++)
    {
        write_eeprom(address,time[i]);                     //store time in external EEPROM
        address++;
    }
    for(int j = 0;j < 2;j++)
    {
        write_eeprom(address,Event[event_index][j]);       //store event in external EEPROM
        address++;
    }
    for(int k = 0;k < 2;k++)
    {
        write_eeprom(address,speed[k]);                    //store speed in external EEPROM
        address++;
    }
    event_count++;                                     //increase event count on storing every logs 
}

void display_speed(void)
{
    unsigned int speed_value = 0, adc_value = 0;                 //initialize speed and adc value '0'
    if(event_index == 0)                                       
    {
        speed_value = 00;                                        //make initially '0'
    }
    else
    {
        adc_value = read_adc(CHANNEL4);                          // Read ADC value from Potentiometer
        speed_value = (unsigned int)(adc_value / 10.23);         // Calculate speed 
        if(speed_value > 99)    
        {
            speed_value = 99;                                    //set max speed 99
        }                                   
    }
    speed[0] = (unsigned char)((speed_value / 10) + '0');        // Convert speed to string
    speed[1] = (unsigned char)((speed_value % 10) + '0'); 
    speed[2] = '\0';   
    clcd_putch(speed[0], LINE2(14));                             // Display speed on the CLCD
    clcd_putch(speed[1], LINE2(15)); 
}

void menu_screen(void)
{
    if(menu_index < 3)
    {
        clcd_print("->", LINE1(0));  // Highlight top line
        clcd_print(menu[menu_index], LINE1(2));             // Print current menu item
        clcd_print(menu[menu_index + 1], LINE2(2));         // Print next menu item
    }
    else 
    {
        clcd_print("->", LINE2(0));   // Highlight bottom line
        clcd_print(menu[menu_index - 1], LINE1(2));          // Print previous menu item
        clcd_print(menu[menu_index], LINE2(2));              // Print current menu item
    }
}

void change_menu(void)
{
    CLEAR_DISP_SCREEN;
    if(menu_flag == DECREMENT_MENU)
    {
        if(menu_index > 0)        // Scroll up logic 
        {
            menu_index--;         //decrement the menu index   
        }   
    }
    else if(menu_flag == INCREMENT_MENU)
    {
        if(menu_index < 3)        // Scroll down logic 
        {
            menu_index++;         //increment the menu index  
        }
    }
}

void event_read(void)
{
    if(flag == 0)                                   //if logs present
    {
        if( event_count > LOG_SIZE)
        {
            event_count = LOG_SIZE ;
        }
        unsigned int r_address = 0 ; 
        for(int log_index = 0; log_index < event_count; log_index++)
        {
            r_address = log_index * LOG_LENGTH;
            logs[log_index][0] = (unsigned char)('0' + (unsigned int)log_index); 
            for(int j = 1; j < LOG_LENGTH; j++)                  // Read each character from EEPROM and form as string
            {     
                if(j == 4 || j == 7)                             // "HH:MM:SS"
                {
                    logs[log_index][j] = ':';
                }
                else if(j == 1 || j == 10 || j == 13)            // Spaces after time,event
                {
                    logs[log_index][j] = ' ';
                }
                else                                             // Read character from EEPROM
                {
                    logs[log_index][j] = read_eeprom(r_address);   
                    r_address++;
                }
            }
            logs[log_index][LOG_LENGTH - 1] = '\0';              // Null-terminate the log string
        }
    }
    else                                            //if no logs present
    {
        for(int log_index = 0; log_index < event_count; log_index++)
        {   
            for(int j = 1; j < LOG_LENGTH; j++)                  // Read each character from EEPROM and form as string
            {     
                logs[log_index][LOG_LENGTH - 1] = '\0';           // Null-terminate the log string
            } 
        }
        
    }
}

void change_log(void)
{
    if(log_flag == DECREMENT_LOG)
    {
        if(log_index > 0)                  // Scroll up logic 
        {
            log_index--;                   //decrement the log index   
        }
    }
    else if(log_flag == INCREMENT_LOG)
    {
        if(log_index < event_count - 1)       // Scroll down logic 
        {
            log_index++;                   //increment the log index  
        }
    }
    display_selected_menu(0);
}

void display_selected_menu(unsigned int menu_index)
{
    switch(menu_index)
    {
        case 0:
            screen_status = VIEW_LOG;               //set screen as view log  screen
            CLEAR_DISP_SCREEN;
            event_read();                          //read logs from external EEPROM
            if(flag == 0)                       
            {
                clcd_print("# TIME     E  SP", LINE1(0));
                clcd_print(logs[log_index], LINE2(0));   //print log based on index    
            }
            else                                    //if no logs found
            {
                clcd_print("# TIME     E  SP", LINE1(0));
                clcd_print("  NO LOGS FOUND ", LINE2(0)); 
            }
            
            break;
        case 1:
            flag = 1;                               //flag to make view log 
            event_count = 0;                        //to clear the already stored event
            address = 0;
            event_read();                           //read logs from external EEPROM
            clcd_print("  LOGS CLEARED  ", LINE1(0)); 
            clcd_print("  SUCCESSFULLY  ", LINE2(0)); 
            manual_delay(1000);
            CLEAR_DISP_SCREEN;
            menu_screen();                          // Return to the menu screen
            break;
        case 2:
            download_log();    
            manual_delay(1000); 
            CLEAR_DISP_SCREEN;
            menu_screen();                           // Return to the menu screen
            break;
        case 3:
            screen_status = SET_TIME;                //set screen as set time screen
            CLEAR_DISP_SCREEN;
            break;
    }
}

void download_log(void)
{
    clcd_print("   Download in  ", LINE1(0));  // before downloading
    clcd_print("    progress    ", LINE2(0));
    puts("Car Black Box.Press any key to download log\n\r");
    getch();                                                           //if acknowledge received it shows
    puts("# TIME     E  SP\n\r");
    event_read();                                                      // Read the log
    for(int i = 0;i < event_count;i++)                                    // download all logs 
    {
        for(int j = 0; j < LOG_LENGTH;j++)                             // Send the log data directly from logs array
        {
            if(logs[i][j] != '\0')                                     // Skip null characters
            {
                putch(logs[i][j]);                                     // Send each character via UART
            }
        }
        puts("\n\r");                                                  // Send newline after each log
    }
    clcd_print("    Download    ", LINE1(0));   // after completion
    clcd_print("    Completed   ", LINE2(0));
}

void change_field(void) 
{
    field_index = (field_index + 1) % 3;               // Change the field index
}

void display_set_time(unsigned int field_index) 
{
    manual_delay(90);                                 //to make delay to visible blink flag
    blink_flag = !blink_flag;                         //toggle blink flag
    clcd_print((const unsigned char *)"    HH:MM:SS    ", LINE1(0));
    if(field_index == HOUR_FIELD && blink_flag)         // Display hours
    {
        clcd_putch(255, LINE2(4)); 
        clcd_putch(255, LINE2(5)); 
    }
    else
    {
        clcd_putch(time[0], LINE2(4)); 
        clcd_putch(time[1], LINE2(5)); 
    }
    clcd_putch(':', LINE2(6)); 
    if(field_index == MIN_FIELD && blink_flag)         // Display minutes
    {
        clcd_putch(255, LINE2(7)); 
        clcd_putch(255, LINE2(8)); 
    }
    else
    {
        clcd_putch(time[2], LINE2(7)); 
        clcd_putch(time[3], LINE2(8)); 
    }
    clcd_putch(':', LINE2(9)); 
    if(field_index == SEC_FIELD && blink_flag)         // Display seconds
    {
        clcd_putch(255, LINE2(10)); 
        clcd_putch(255, LINE2(11)); 
    }
    else
    {
        clcd_putch(time[4], LINE2(10)); 
        clcd_putch(time[5], LINE2(11)); 
    }   
}

void inc_time(unsigned int field_index)
{
    unsigned char hours = (time[0] - '0') * 10 + (time[1] - '0');      // Extract current values from the time array
    unsigned char minutes = (time[2] - '0') * 10 + (time[3] - '0');
    unsigned char seconds = (time[4] - '0') * 10 + (time[5] - '0');  
    switch(field_index)
    {
        case HOUR_FIELD:                            // Increment hours
            hours = (hours + 1) % 24;               // Wrap around at 24
            break;
        case MIN_FIELD:                             // Increment minutes
            minutes = (minutes + 1) % 60;           // Wrap around at 60
            break;
        case SEC_FIELD:                             // Increment seconds
            seconds = (seconds + 1) % 60;           // Wrap around at 60
            break;
    }
    time[0] = (unsigned char)((hours / 10) + '0');    // Update the time array
    time[1] = (unsigned char)((hours % 10) + '0');
    time[2] = (unsigned char)((minutes / 10) + '0');
    time[3] = (unsigned char)((minutes % 10) + '0');
    time[4] = (unsigned char)((seconds / 10) + '0');
    time[5] = (unsigned char)((seconds % 10) + '0');
}

void set_time(void)     
{                                                    // Write the updated time array to the RTC in BCD format
    write_ds1307(HOUR_ADDR,(unsigned char)(((time[0] - '0') << 4) | (time[1] - '0')));
    write_ds1307(MIN_ADDR,(unsigned char)(((time[2] - '0') << 4) | (time[3] - '0')));
    write_ds1307(SEC_ADDR,(unsigned char)(((time[4] - '0') << 4) | (time[5] - '0')));
}

void manual_delay(unsigned int delay)     
{
    for(int i = 0; i < delay; i++)       
    {
        for(int j = 0; j < 1000; j++);
    }
}
