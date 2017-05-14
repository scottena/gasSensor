//Tempature Senor display
//1/9/2017
//Adam Scotten 
//Trimble Gas Sensor

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "adc_functions.h"
#include "lm92_functions.h"
#include "uart_functions.h"
#include "twi_master.h"
#include "cli.h"
/*************************************************************
		Temperature read format
    ------------------------------------------------------
    |Sb|B11|B10|B9|B8|B7|B6|B5|B4|B3|B2|B1|B0|ST2|ST1|ST0|	          		  	
    ------------------------------------------------------

Sb      = sign bit (1 = negative)
1 LSB = 0.0625
ST2-ST0 = Status bits

*************************************************************/
//gas 1 H2O
//gas 2 CO

/*
uint8_t g1_res = 
uint8_t g2_res = 

float g1_nA_p_ppm = 187.78
float g2_nA_p_ppm = 3.3

uint8_t g1_ind = 1
uint8_t g2_ind = 0 

 
*/
extern uint8_t adc_wr_buf;
extern uint8_t adc_rd_buf[2];

extern uint8_t lm92_wr_buf[2];
extern uint8_t lm92_rd_buf[2];
uint8_t rx_char;
float g1_int, g2_int;
float temp_f=0,adc1_f=0, adc2_f=0;
char temp_uart[8];
char adc1_uart[6];
char adc2_uart[6];
char gas1_uart[12];
char gas2_uart[12];
char g2_int_uart[12];
char g1_int_uart[12];
char uart_buf[8];


void LM92_read(void){
  uint16_t lm92_val;
  float dec;
  uint8_t hold=0,i=0,j=0, neg;
  twi_start_wr(0x96, lm92_wr_buf, 0x01);
  twi_start_rd(0x96, lm92_rd_buf, 0x02);
  lm92_val = lm92_rd_buf[0];//read byte 1
  lm92_val = lm92_val << 8;//shift to read byte 2
  lm92_val |= lm92_rd_buf[1];//read byte 2 
  lm92_val = lm92_val >> 3; // remove status bits
  if ((lm92_val >> 12) == 1){
      //negative number
    lm92_val = (~(lm92_val)+1);
    neg = 1;  
  }
  else {neg =0;}
  dec = lm92_val & 0x0F;
  dec = dec * 625;
  temp_f = (lm92_val*0.0625);
  temp_f = temp_f - 2.6;
  hold = (int)temp_f;
  dec = (temp_f - hold);
  dec = dec * 10000;

  itoa(hold, temp_uart, 10);  
  temp_uart[2]='.';
  i=3; 
  if (dec<1000){temp_uart[i]='0';i++;}
  itoa(dec, uart_buf, 10); //dec-ASII for dec
  for (j=0; j<3; j++)
    temp_uart[i+j]=uart_buf[j];
}

uint16_t ADC1_read(void){
  uint16_t adc_val=0;
  adc_wr_buf=0x88;

  twi_start_wr(0x10, &adc_wr_buf, 0x01);
  //_delay_ms(2);
  twi_start_rd(0x10, adc_rd_buf, 0x02); 
  adc_val = adc_rd_buf[0];//read byte 1
  adc_val = adc_val << 8;//shift to read byte 2
  adc_val |= adc_rd_buf[1];//read byte 2 
  adc_val = adc_val >> 4; // remove status bits
   
  adc1_f = (adc_val*.001);
  return adc_val;
}

uint16_t ADC2_read(void){
  uint16_t adc_val=0;
  adc_wr_buf=0xC8;
   
  twi_start_wr(0x10, &adc_wr_buf, 0x01);
  //_delay_ms(2);
  twi_start_rd(0x10, adc_rd_buf, 0x02);
  adc_val = adc_rd_buf[0];//read byte 1
  adc_val = adc_val << 8;//shift to read byte 2
  adc_val |= adc_rd_buf[1];//read byte 2 
  adc_val = adc_val >> 4; // remove status bits
   
  adc2_f = (adc_val*.001);
  return adc_val;
}

float ScfLookUp(uint8_t sensor){
  uint8_t Tks=25;
  float Scf=0;

  if (temp_f < Tks){
    switch (sensor){
      case 1:
        Scf = 0;
        break;
      case 2:
        Scf = 1.2;
        break;
      default:
        Scf = 0;
    }
  }
  else{
    switch (sensor){
      case 1:
        Scf = 0;
        break;
      case 2:
        Scf = 0.5;
        break;
      default:
        Scf = 0;
    }
  }
  return Scf;
}

float ZcfLookUp(uint8_t sensor){
  uint8_t Tks=25;
  float Zcf=0;

  if (temp_f < Tks){
    switch (sensor){
      case 1:
        Zcf = 0;
        break;
      case 2:
        Zcf = 0.5;
        break;
      default:
        Zcf = 0;
    }
  }
  else{
    switch (sensor){
      case 1:
        Zcf = 0;
        break;
      case 2:
        Zcf = 15;
        break;
      default:
        Zcf = 0;
    }
  }
  return Zcf;
}

float PPMcalc(float val, uint8_t sensor, float sens_code){
  float ppm, nA, Zc, Sc, Scf=0, Zcf=0;
  uint16_t nA_p_ppm_x100;
  uint8_t Tks = 25;
 
  nA_p_ppm_x100 = (100*sens_code);
  
  Scf = ScfLookUp(sensor);
  Zcf = ZcfLookUp(sensor); 
  
  if (val<2.56){return 0;}
  
  if (sensor == 2){
    nA = (((val-2.56)/30)*1E5);
  }
  if (sensor == 1){
    nA = (((val-2.56)/30)*1E5);
  } 
  Zc= nA-(Zcf*(temp_f-Tks));
  Sc = 1-((Scf/100)*(temp_f-Tks));
  ppm = (Zc*(Sc/nA_p_ppm_x100)*1E5);

  if (sensor == 2){
    g2_int =( ((val-2.560)*1E4)/95);
    if (g2_int<0){ g2_int = 0;} 
  }
  else if (sensor == 1){
    g1_int =(((val-2.560)*1E4)/33);
    if (g1_int<0){g1_int = 0;} 
  }
  if(ppm<0){return 0;}

  return ppm;
}


ISR(USART_RX_vect){
  static uint8_t i=0, j=0, cli_en=0;
  static char uart_buff[8];
  static char uart_all [8] = "printjy\r";
  uint8_t new_line=0;
  rx_char = UDR0;
  uart_buff[i++]=rx_char;  
  if (cli_en) {uart_putc(rx_char);}
  if((rx_char == '\r')||(i>7)){
    i=0;
    if (strstr(uart_buff, "all")){ 
      for (j=0; j<8; j++){uart_buff[j] = uart_all[j];}
    }
    if ((cli_en)&&(uart_buff[0]!='\r')){ uart_puts("\r\n\r\n");}
    if (strstr(uart_buff,"help")){cli_help();}
    else if (strstr(uart_buff,"cli")){
      cli_en =((~cli_en) & 0x01);
      if (cli_en){cli_intro();}
      else {uart_puts("*** Ending cli ***\r\n\r\n");}
      for(j=0; j<strlen(uart_buff); j++){uart_buff[j]=' ';}    
      
    }
    else {
    for(j=0; j<strlen(uart_buff); j++){    
        
      if(uart_buff[j] == 't'){
        if (cli_en){uart_puts("Temp: ");}
        uart_puts(temp_uart);
        uart_buff[j]=' ';
        new_line=1;
      }
      else if(uart_buff[j] == 'i'){
        if (cli_en) {uart_puts("ADC1(IAQ): ");}
        uart_puts(adc1_uart);
        uart_buff[j]=' ';
        new_line=1;
      }
      else if(uart_buff[j] == 'n'){
        if(cli_en) {uart_puts("ADC2(SO2): ");}
        uart_puts(adc2_uart);
        uart_buff[j]=' ';
        new_line=1;
      }
      else if(uart_buff[j] == 'r'){
        if (cli_en){uart_puts("PPM(IAQ): ");}
        uart_puts(gas1_uart);
        uart_buff[j]=' ';
        new_line=1;
      }
      else if(uart_buff[j] == 'p'){
        if (cli_en){ uart_puts("PPM(SO2): ");}
        uart_puts(gas2_uart);
        uart_buff[j]=' ';
        new_line=1;
      }
      else if(uart_buff[j] == 'j'){
        if (cli_en){ uart_puts("int(SO2): ");}
        uart_puts(g2_int_uart);
        uart_buff[j]=' ';
        new_line=1;
      }
      else if(uart_buff[j] == 'y'){
        if (cli_en){ uart_puts("int(IAQ): ");}
        uart_puts(g1_int_uart);
        uart_buff[j]=' ';
        new_line=1;
      }
      else if(uart_buff[j]=='\r'){
        //end of command char
        uart_putc('\r');
        uart_putc('\n');
        uart_buff[j]=' ';
        if (cli_en){uart_putc('>');} 
      }
      else {uart_buff[j]=' ';}
      
      if(new_line==1){
        new_line=0;  
        uart_putc('\r');
        uart_putc('\n');
      }
    } 
    }
  }
}


int main(){
  uint8_t i=0, count=0, mpr=0, j=0;
  uint16_t gas_buf, adc_val;
  float  adc2_f_avg=0, adc1_f_avg=0;
  uint8_t  adc2_count=0, adc1_count=0;
  uart_init();
  init_twi();
  sei();
  
  while(1){
    _delay_ms(120);
    
    count++;
    if (count == 9){
      LM92_read();          
      _delay_ms(2);
      count = 0;
    }
    //ADC2 read SO2
    adc_val=ADC2_read(); 
    _delay_ms(2);
    adc2_f_avg += adc2_f;
    adc2_count++;
    if (adc2_count == 9){ // average of 10 sample voltages
      adc2_f_avg = adc2_f_avg/10;
      itoa((adc2_f_avg*1000), adc2_uart, 10);
      gas_buf = PPMcalc(adc2_f_avg, 2, 31.76);
      itoa(gas_buf, gas2_uart, 10);
      itoa(g2_int, g2_int_uart, 10);
      mpr=strlen(gas2_uart);
      for (j=0;j<3;j++)
        gas2_uart[mpr-j]=gas2_uart[mpr-(j+1)];
      gas2_uart[mpr-3]='.';
      gas2_uart[mpr+1]=' ';
      adc2_count = 0;
    }      
    
    //ADC1 read IAQ
    adc_val=ADC1_read(); 
    _delay_ms(2);
    adc1_f_avg += adc1_f;
    adc1_count++;
    if (adc1_count == 9){ //average of 10 sample voltages
      adc1_f_avg = adc1_f_avg/10;
      itoa((adc1_f_avg*1000), adc1_uart, 10);
      gas_buf = PPMcalc(adc1_f_avg, 1, 11.15);
      itoa(gas_buf, gas1_uart, 10);
      itoa(g1_int, g1_int_uart, 10);
      mpr=strlen(gas1_uart);
      for (j=0;j<3;j++)
        gas1_uart[mpr-j]=gas1_uart[mpr-(j+1)];
      gas1_uart[mpr-3]='.';
      gas1_uart[mpr+1]=' ';
      adc1_count = 0;
    }
  }//while
}//main

