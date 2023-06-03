#include "nu32dip.h" // constants, functions for startup and UART
#include "i2c_master_noint.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include "ws2812b.h"
#include "font.h"
#include <stdio.h>
#include <math.h>

void drawChar(char c, unsigned char x, unsigned char y); // draw character to screen
void drawString(char * m, unsigned char x, unsigned char y); // draw string to screen
void setServo(int pos); // set the servo to a position between 0 and 6000
void setMotors(int speedA, int speedB); //motor positions between 0 and 16999
unsigned char readSerial(void); //reads from UART2
void writeSerial(unsigned char data); //writes to UART2
void config_UART2(void); //configure UART2
void setupPWM(void); // setup PWM pins for motor controller //and servo
void wait(float time); //waits in a loop for a given time in s
float calibrate(char * str, unsigned char * data); //calibrates gyro with rotation from motors

int main(void) {
    NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
    init_mpu6050();
    ssd1306_setup();
    ws2812b_setup();
    

    wsColor lights[8];
    for(int i=0; i<8; i++){
        lights[i] = HSBtoRGB(0, 1, 1);
    }
    ws2812b_setColor(lights, 8);
	
	unsigned char data[100]; // char array for the raw data
    char str[100]; // string for printing
    
    unsigned int lastBeat = _CP0_GET_COUNT();
    unsigned int loopStart = _CP0_GET_COUNT();
    unsigned int delay = _CP0_GET_COUNT();
    unsigned int LEDon  = 0;
    unsigned char line_data;
    unsigned char msg;
    int right;
    int left;
    float zG;
    float error_heading;
    float target_heading;
    float actual_heading;
    float speed;
    int correction;
    int speed_range = (1999 - 650);
    
    config_UART2();
    
    setupPWM(); //sets up the PWM pins for motor driver and servo
    
	uint8_t whoAmI = whoami();// read whoami
    sprintf(str,"%#x\r\n",whoAmI);
    NU32DIP_WriteUART1(str);// print whoami
    while (whoAmI != 0x68){	// if whoami is not 0x68, stuck in loop with LEDs on
        NU32DIP_GREEN = 1;
        NU32DIP_YELLOW = 1;
    }
    NU32DIP_GREEN = 0;
    NU32DIP_YELLOW = 0;
    
    _CP0_SET_COUNT(0); //start clock for loop
    
    delay = _CP0_GET_COUNT();
    while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 2)); //wait 3s before calibrating
    
    //calibrate stationary gyro
    float zero_zG = 0;
    for (int i=0;i<5;i++) {
        burst_read_mpu6050(data);
        zG = conv_zG(data); // convert z-gyro rate to dps
        zero_zG = zero_zG + zG;
        delay = _CP0_GET_COUNT();
    while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 0.25)); //wait 0.25s
    }
    zero_zG = zero_zG / 5;
    //calibrate max gyro
    setMotors(1999,0);
    float max_zG = 0;
    for (int i=0;i<5;i++) {
        burst_read_mpu6050(data);
        zG = conv_zG(data); // convert z-gyro rate to dps
        max_zG = max_zG + zG;
        delay = _CP0_GET_COUNT();
        while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 0.25)); //wait 0.25s
    }
    max_zG = max_zG / 5;
    setMotors(0,0);
    
//    sprintf(str, "Max Spin = %f        ", max_zG);
//    drawString(str,0,0);
//    sprintf(str, "No Spin = %f         ", zero_zG);
//    drawString(str,0,10);
//    ssd1306_update();
    
    delay = _CP0_GET_COUNT();
       while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 3)); //wait 3s
   
//    while (NU32DIP_USER); //wait for user button press
    
    ssd1306_clear();
    
    while (1) {
        loopStart = _CP0_GET_COUNT();
        
		// use core timer for exactly 100Hz loop
        if ((_CP0_GET_COUNT() - lastBeat) > ((48000000 / 2)*0.25)){
            lastBeat = _CP0_GET_COUNT();
            if (LEDon==0){
                NU32DIP_YELLOW = 1;
                NU32DIP_GREEN = 0;
                LEDon = 1;
            }
            else{
                NU32DIP_YELLOW = 0;
                NU32DIP_GREEN = 1;
                LEDon = 0;
            }
        }
        
//        sprintf(str, "r");
//        msg = 0b00000101;
//        writeSerial(msg);
//        NU32DIP_WriteUART1(str);
//        delay = _CP0_GET_COUNT();
//        while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 0.05)); //wait 0.05s
//        line_data = readSerial();
//        sprintf(str, "%d", line_data);
//        NU32DIP_WriteUART1(str);
//        drawString(str,0,20); 

        // read IMU
        burst_read_mpu6050(data);
        zG = conv_zG(data); // convert z-gyro rate to dps
        
        target_heading = 0; //heading from 0 to 1
        actual_heading = (zG - zero_zG) / (max_zG - zero_zG);
        error_heading = actual_heading - target_heading; //calculates how far off the actual is from target
        correction = speed_range * error_heading; //calculates correction to achieve target
        
        sprintf(str, "Actual Heading = %f         ", actual_heading);
        drawString(str,0,10); 
        
        speed = 0;
        
        if (correction>0){
            right = speed * (speed_range - correction) + 650;
        }
        else if (correction<=0){
            left = speed * (speed_range - correction) + 650;
        }
        
        setMotors(left, right);
        
        //print speed to screen
        sprintf(str, "Left = %d   Right = %d          ", left, right);
        drawString(str,0,0);
        
        ssd1306_update();
        
        while ((_CP0_GET_COUNT() - loopStart)< ((48000000 / 2)*0.25)); //limit the loop to max 4Hz, 0.25s period
    }
}

//print char to screen in location
void drawChar(char c, unsigned char x, unsigned char y){
    for(int j=0; j<5; j++){
        char col = ASCII[c-0x20][j];
        for(int i=0; i<8; i++){
            ssd1306_drawPixel(x+j,y+i,(col>>i)&0b1);
        }
    }
}

void drawString(char * m, unsigned char x, unsigned char y){
    int k=0;
    while(m[k]!='\0'){
        drawChar(m[k],x+5*k,y);
        k++;
    }
}

void setServo(int pos){ //between 0 and 6000
    //TMR3 = 0;
    unsigned int min = 1500; //0.000333 ms)*4500 = 1.5ms
    unsigned int max = 7500; //(0.000333 ms)*7500 = 2.5ms
    OC2CONbits.ON = 0;       // turn off OC2
    //OC2RS = min + pos;             // duty cycle = OC2RS/(PR2+1) = 1/180
    OC2R = min + pos;              // initialize before turning OC2 on; afterward it is read-only
    OC2CONbits.ON = 1;       // turn on OC2
}

void setMotors(int speedA, int speedB){ //motor positions between 0 and 16999
    LATAbits.LATA1 = 1;
    LATBbits.LATB1 = 1;
    OC3CONbits.ON = 0;       // turn off OC3
    OC4CONbits.ON = 0;       // turn off OC4
    OC3RS = speedA;             // duty cycle = OC3RS/(PR3+1) = 0.05%
    OC4RS = speedB;
    OC3CONbits.ON = 1;      // turn on OC3
    OC4CONbits.ON = 1;      // turn on OC4
}

unsigned char readSerial(void) {
    unsigned char data;
    int complete = 0;
    // loop until you get a '\r' or '\n'
    while (!complete) {
        if (U2STAbits.URXDA) { // if data is available
            data = U2RXREG; // read the data
            complete = 1;
        }
    }
    return data;
}

// Write a character array using UART1

void writeSerial(unsigned char data) {
    while (U2STAbits.UTXBF) {
        ; // wait until tx buffer isn't full
    }
    U2TXREG = data;
}

void config_UART2(void){
    __builtin_disable_interrupts();
    
    //UART2 pins
    U2RXRbits.U2RXR = 0b0011; // Set B11 to U2RX
    RPB10Rbits.RPB10R = 0b0010; // Set B10 to U2TX

    // turn on UART2 without an interrupt
    U2MODEbits.BRGH = 0; // set baud to NU32_DESIRED_BAUD
    U2BRG = ((NU32DIP_SYS_FREQ / NU32DIP_DESIRED_BAUD) / 16) - 1;

    // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
    U2MODEbits.PDSEL = 0;
    U2MODEbits.STSEL = 0;

    // configure TX & RX pins as output & input pins
    U2STAbits.UTXEN = 1;
    U2STAbits.URXEN = 1;
    // configure without hardware flow control
    U2MODEbits.UEN = 0;

    // enable the uart
    U2MODEbits.ON = 1;

    __builtin_enable_interrupts();
    
    NU32DIP_GREEN = 0;
    NU32DIP_YELLOW = 0;
    
}

void setupPWM(void){
//Set DIR pins for motors
    TRISACLR = 0b10; // A1 is output to ADIR
    TRISBCLR = 0b10; // B1 is output to BDIR
    LATAbits.LATA1 = 1; //Set ADIR Forward
    LATBbits.LATB1 = 1; //Set BDIR Forward
    //Set PWM pins for motors
    RPA3Rbits.RPA3R = 0b0101; //Set A3 to OC3
    RPB2Rbits.RPB2R = 0b0101; //Set B2 to OC5
    //setup PWM pin for Motors on OC3 and OC5
    T3CONbits.TCKPS = 0b010;     // Timer3 prescaler N=4 (1:4)
    PR3 = 1999;              // period = (PR3+1) * N * (1/48000000) = 2.4 kHz
    TMR3 = 0;                // initial TMR3 count is 0
    OC3CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC3CON bits are defaults
    OC3CONbits.OCTSEL = 1;   // Use timer3
    OC4CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC4CON bits are defaults
    OC4CONbits.OCTSEL = 1;   // Use timer3
    OC3RS = 0;             // duty cycle = OC3RS/(PR3+1) = 0.05%
    OC3R = 0;              // initialize before turning OC1 on; afterward it is read-only
    OC4RS = 0;             // duty cycle = OC4RS/(PR3+1) = 0.05%
    OC4R = 0;              // initialize before turning OC1 on; afterward it is read-only
    T3CONbits.ON = 1;        // turn on Timer3
    OC3CONbits.ON = 1;       // turn on OC3
    OC4CONbits.ON = 1;       // turn on OC4
    
//    //set PWM pin for servo
//    RPB11Rbits.RPB11R = 0b0101; //Set B11 to OC2
//    //setup PWM pin for servo on OC2
//    T2CONbits.TCKPS = 0b100;     // Timer2 prescaler N=16 (1:16)
//    PR2 = 59999;              // period = (PR2+1) * N * (1/48000000) = 50 Hz = 20 ms
//    TMR2 = 0;                // initial TMR2 count is 0
//    OC2CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC2CON bits are defaults
//    OC2CONbits.OCTSEL = 0;   // Use timer2
//    OC2RS = 4500;             // duty cycle = OC2RS/(PR2+1) = (1.5 ms)/(0.00033 ms per)
//    OC2R = 4500;              // initialize before turning OC2 on; afterward it is read-only
//    T2CONbits.ON = 1;        // turn on Timer2
//    OC2CONbits.ON = 1;       // turn on OC2
}