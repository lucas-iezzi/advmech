#include "nu32dip.h" // constants, functions for startup and UART
#include "ws2812b.h"
#include <stdio.h>

int main(void) {
    NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
    ws2812b_setup();
    
    int hue;
    int hue0;
    int t;
        
    wsColor lights[8];
    
    while (1) {
        for(int i=0; i<8; i++){
            if ((hue0+i*10) > 360){
                hue =(hue0+i*10) - 360;
            }
            else{
                hue = (hue0+i*10);
            }
            lights[i] = HSBtoRGB(hue, 1, 1);
        }
        hue0 = hue0 + 1;
        if (hue0 > 360){
            hue0 = 0;
        }

        ws2812b_setColor(lights, 8);
        t = _CP0_GET_COUNT(); // should really check for overflow here
        while(_CP0_GET_COUNT() < t + 12000*10){}
    }
}