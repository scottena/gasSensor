#include "uart_functions.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "cli.h"

void cli_intro(){

  uart_puts("\r\n*** Starting cli ***\r\n\r\n");
  cli_logo();
  cli_welcome();
  cli_help();
}

void cli_help(){

  uart_puts("Type “cli” to exit cli mode.\r\n");
  uart_puts("Type “all” to view all outputs.\r\n\r\n");
  uart_puts(" value      char\r\n-----------------\r\n");
  uart_puts(" PPM SO2     p  \r\n"); 
  uart_puts(" PPM IAQ     r  \r\n"); 
  uart_puts(" int SO2     j  \r\n"); 
  uart_puts(" int IAQ     y  \r\n"); 
  uart_puts(" ADC1(IAQ)   n  \r\n"); 
  uart_puts(" ADC2(SO2)   i  \r\n"); 
  uart_puts(" Temp(C)     t  \r\n"); 
  uart_puts("\r\n\r\n");
  uart_putc('>');
}  

void cli_logo(){

  uart_puts("     ___   ___  _  _ \r\n");
  uart_puts("   /    // ___// // /\r\n");
  uart_puts("  / // //__  // // /\r\n");
  uart_puts(" /__  //____/| __ /\r\n");

}

void cli_welcome(){

  uart_puts("Welcome to the Timble gas sensor cli mode!\r\n");
  uart_puts("Enter the characters listed to see the corresponding values.\r\n");
  uart_puts("Multiple characters can be entered at a time.\r\n\r\n");

}
