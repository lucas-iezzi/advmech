////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
void readSerial(char * message, int maxLength); //reads from UART2
void writeSerial(const char * string); //writes to UART2
void config_UART2(void); //configure UART2
void setupPWM(void); // setup PWM pins for motor controller //and servo
void wait(float time); //waits in a loop for a given time in s
float calibrate(char * str, unsigned char * data); //calibrates gyro with rotation from motors

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void) {
//run setup functions---------------------------------------------------------------------------------------------------------------------------------------------------
    NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
    init_mpu6050();
    ssd1306_setup();
    ws2812b_setup();
    config_UART2();
    setupPWM(); //sets up the PWM pins for motor driver and servo

    wsColor lights[8];
    for(int i=0; i<8; i++){
        lights[i] = HSBtoRGB(100, 1, 1);
    }
    ws2812b_setColor(lights, 8);
	
//init all variables----------------------------------------------------------------------------------------------------------------------------------------------------
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
    int speed_range = (1999 - 500);
    int asc = 1;
    
//	uint8_t whoAmI = whoami();// read whoami
//    sprintf(str,"%#x\r\n",whoAmI);
//    NU32DIP_WriteUART1(str);// print whoami
//    while (whoAmI != 0x68){	// if whoami is not 0x68, stuck in loop with LEDs on
//        NU32DIP_GREEN = 1;
//        NU32DIP_YELLOW = 1;
//    }
//    NU32DIP_GREEN = 0;
//    NU32DIP_YELLOW = 0;
    
    _CP0_SET_COUNT(0); //start clock for loop
    
    delay = _CP0_GET_COUNT();
    while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 2)); //wait 3s before calibrating
    
//calibrate gyro--------------------------------------------------------------------------------------------------------------------------------------------------------
    float zero_zG = 0;
    for (int i=0;i<5;i++) {
        burst_read_mpu6050(data);
        zG = conv_zG(data); // convert z-gyro rate to dps
        zero_zG = zero_zG + zG;
        delay = _CP0_GET_COUNT();
    while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 0.25)); //wait 0.25s
    }
    zero_zG = zero_zG / 5;
    setMotors(1999,-1999);  //set motors to max speed to calibrate upper end
    float max_zG = 0;
    for (int i=0;i<5;i++) {
        burst_read_mpu6050(data);
        zG = conv_zG(data); // convert z-gyro rate to dps
        max_zG = max_zG + zG;
        delay = _CP0_GET_COUNT();
        while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 0.25)); //wait 0.25s
    }
    max_zG = max_zG / 5;
    setMotors(0,0);  //set motors back to stopped
    
    sprintf(str, "Max = %f        ", max_zG);
    drawString(str,0,0);
    sprintf(str, "Zero = %f         ", zero_zG);
    drawString(str,0,12);
    ssd1306_update();
    
    delay = _CP0_GET_COUNT();
    while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 3)); //wait 3s
   
    burst_read_mpu6050(data);
    zG = conv_zG(data);   
    
    while (NU32DIP_USER); //wait for user button press
    
    ssd1306_clear();
    ssd1306_update();
    
    target_heading = 0; //heading from -1(right) to 1(left)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
//start of main loop----------------------------------------------------------------------------------------------------------------------------------------------------
    while (1) {
        loopStart = _CP0_GET_COUNT();
        
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

//read desired heading over uart from pico------------------------------------------------------------------------------------------------------------------------------
        sprintf(str, "ready");
        writeSerial(str);
        NU32DIP_WriteUART1(str);
        delay = _CP0_GET_COUNT();
        while ((_CP0_GET_COUNT() - delay)< ((48000000 / 2) * 0.05)); //wait 0.05s
        readSerial(str,100);
        sscanf( str, "%f", target_heading);
        NU32DIP_WriteUART1(str);
        
//set heading and speed-------------------------------------------------------------------------------------------------------------------------------------------------
//        if (asc==1){
//            target_heading = target_heading + 0.075; 
//        }
//        else{
//            target_heading = target_heading - 0.075;
//        }
//        if (target_heading > 1){
//            target_heading = 1;
//            asc = 0;
//        }
//        else if (target_heading < -1){
//            target_heading = -1;
//            asc = 1;
//        }
        speed = 1;
        
//calculate heading and error correction--------------------------------------------------------------------------------------------------------------------------------
        burst_read_mpu6050(data); //read IMU guyro data
        zG = conv_zG(data); // convert z-gyro rate to dps
        actual_heading = (zG - zero_zG) / (max_zG - zero_zG);
        if (actual_heading > 1){
            actual_heading = 1;
        }
        else if (actual_heading < -1){
            actual_heading = -1;
        }
        error_heading = actual_heading - target_heading; //calculates how far off the actual is from target
        correction = speed_range * error_heading; //calculates correction to achieve target
        
//set the motor speeds--------------------------------------------------------------------------------------------------------------------------------------------------
        left = round(speed * speed_range) + 500;
        right = round(speed * speed_range) + 500;
        if (correction>0){
            left = round(speed * (speed_range - correction)) + 500;
        }
        else if (correction<=0){
            right = round(speed * (speed_range + correction)) + 500;
        }
        setMotors(left, right);
        
//print data to screen--------------------------------------------------------------------------------------------------------------------------------------------------
        sprintf(str, "L = %d R = %d          ", left, right);
        drawString(str,0,0);
        sprintf(str, "A = %f T = %f      ", actual_heading, target_heading);
        drawString(str,0,12);
        sprintf(str, "C = %d       ", correction);
        drawString(str,0,24);
        ssd1306_update();
        
        while ((_CP0_GET_COUNT() - loopStart)< ((48000000 / 2)*0.25)); //limit the loop to max 4Hz, 0.25s period
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//functions to print to screen------------------------------------------------------------------------------------------------------------------------------------------
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

//functions to set motor and servos-------------------------------------------------------------------------------------------------------------------------------------
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
    
    if (speedA > 0){
        LATAbits.LATA1 = 1;
    }
    else{
        LATAbits.LATA1 = 0;
    }
    if (speedB > 0){
        LATBbits.LATB1 = 1;
    }
    else{
        LATBbits.LATB1 = 0;
    }
    
    OC3CONbits.ON = 0;       // turn off OC3
    OC4CONbits.ON = 0;       // turn off OC4
    OC3RS = abs(speedA);             // duty cycle = OC3RS/(PR3+1) = 0.05%
    OC4RS = abs(speedB);
    OC3CONbits.ON = 1;      // turn on OC3
    OC4CONbits.ON = 1;      // turn on OC4
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

//functions to read and write over UART2 to pico------------------------------------------------------------------------------------------------------------------------
void readSerial(char * message, int maxLength) {
    char data = 0;
    int complete = 0, num_bytes = 0;
    // loop until you get a '\r' or '\n'
    while (!complete) {
        if (U2STAbits.URXDA) { // if data is available
            data = U2RXREG; // read the data
            if ((data == '\n') || (data == '\r')) {
                complete = 1;
            } else {
                message[num_bytes] = data;
                ++num_bytes;
                // roll over if the array is too small
                if (num_bytes >= maxLength) {
                    num_bytes = 0;
                }
            }
        }
    }
}
void writeSerial(const char * string) {
    while (*string != '\0') {
        while (U2STAbits.UTXBF) {
            ; // wait until tx buffer isn't full
        }
        U2TXREG = *string;
        ++string;
    }
}
void config_UART2(void){
    __builtin_disable_interrupts();
    
    //Disable analog pins
    ANSELA = 0;                     // Set all ports of PortA as digital
    ANSELB = 0; 
    
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////