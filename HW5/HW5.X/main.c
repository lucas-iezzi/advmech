#include "nu32dip.h" // constants, functions for startup and UART
#include "spi.h" // spi functions for startup and sending
//CS is B2
#include <stdio.h>
#include <stdlib.h>  
#include <math.h>

#define PI 3.1416

int main() {
  
  NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
  initSPI(); //setup SPI pins and modes
  
  //Set CS High
  LATBbits.LATB2 = 1;
  
  unsigned int t;
  unsigned int time;
  
  _CP0_SET_COUNT(0);
  
  while (1) {
    time = _CP0_GET_COUNT();
	//calculate sin value to send
    float posa = 2*PI*t;
    unsigned int va = (sin(posa/100)+1)/2 * 1023; // get sin on a scale of 0-1023
    unsigned short da = va & 0b1111111111; //convert value to bits
    unsigned char ca = 0; //0 for a, 1 for b
    unsigned short pa;
    pa = (ca<<15); //shift the output bit to the last bit
    pa = pa | (0b111<<12); //add the rest of the setup bits
    pa = pa | (da<<2); // combine the voltage into the next 10 bits, skipping the last two
    
    LATAbits.LATA0 = 0;
	spi_io(pa>>8); //send high 8 bits from 16 bit num spi_io(t>>8)
    spi_io(pa); //send low 8 bits from 16 bit num spi_io(t)
	LATAbits.LATA0 = 1;
	
    //calculate triangle value to send
    unsigned int vb = 1023/200 * t; // get triangle value from 0 to 1023
    unsigned short db = vb & 0b1111111111; //convert value to bits
    unsigned char cb = 1; //0 for a, 1 for b
    unsigned short pb;
    pb = (cb<<15); //shift the output bit to the last bit
    pb = pb | (0b111<<12); //add the rest of the setup bits
    pb = pb | (db<<2); // combine the voltage into the next 10 bits, skipping the last two
    
    LATAbits.LATA0 = 0;
	spi_io(pb>>8); //send high 8 bits from 16 bit num spi_io(t>>8)
    spi_io(pb); //send low 8 bits from 16 bit num spi_io(t)
	LATAbits.LATA0 = 1;
    
	t += 1;
    if (t>199){
        t=0;
    }
    
    //delay 5ms to get 200 samples per second;
    while(_CP0_GET_COUNT() < time + (48000000 / 2)*0.005){}
  }
}
		