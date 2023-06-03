/* 
 * File:   function.h
 * Author: Lucas
 *
 * Created on May 30, 2023, 11:09 PM
 */

#ifndef FUNCTION_H
#define	FUNCTION_H

// Header file for miscelaneous functions towards line following car

void drawChar(char c, unsigned char x, unsigned char y); // draw character to screen
void drawString(char * m, unsigned char x, unsigned char y); // draw string to screen
void setServo(int pos); // set the servo to a position between 0 and 6000
void setMotors(int speedA, int speedB); //motor positions between 0 and 16999
void readSerial(char * message, int maxLength); //reads from UART2
void writeSerial(const char * string); //writes to UART2
void config_UART2(void); //configure UART2
void setupPWM(void); // setup PWM pins for motor controller and servo
void wait(float time); //waits in a loop for a given time in s
float calibrate(char * str, unsigned char * data); //calibrates gyro with rotation from motors

#endif

