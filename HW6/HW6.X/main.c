#include "nu32dip.h" // constants, functions for startup and UART
#include "i2c_master_noint.h" // functions for i2c
#include <stdio.h>

int main(){
	// nu32 startup
	NU32DIP_Startup();
	// init i2c_master_setup
	i2c_master_setup();
    
    unsigned char OLATreg = 0x0A; //OLAT register
    unsigned char IODIRreg = 0x00; //OLAT register
    unsigned char address = 0b0100000; //0100(A2,A1,A0)
    
	// init the chip --> set the IODIR register for GP0 in, GP7 out
    setPin(address, IODIRreg, 0b01111111); //(IO7,IO6,IO5,IO4,IO3,IO2,IO1,IO0)
    
    unsigned int lastBeat = _CP0_GET_COUNT();
    unsigned int LEDon  = 0;
	
	while(1){
		//blink green or yellow LED for heartbeat
        if ((_CP0_GET_COUNT() - lastBeat) > ((48000000 / 2)*0.25)){
            lastBeat = _CP0_GET_COUNT();
            if (LEDon==0){
                NU32DIP_YELLOW = 1;
                LEDon = 1;
            }
            else{
                NU32DIP_YELLOW = 0;
                LEDon = 0;
            }
        }
        
        set_gp7(read_gp0());
        // to write
        //setPin(address, OLATreg, 0b00000000);
	}
}

void setPin(unsigned char address, unsigned char reg, unsigned char value){ 
    //address = 0b0100(A2,A1,A0,RW);
    //reg = 0x0A //for OLAT
    //value = 0b(GP7,GP6,GP5,GP4,GP3,GP2,GP1,GP0)
    address = address << 1;
    i2c_master_start(); //send start bit
    i2c_master_send(address); //send address of chip
    i2c_master_send(reg); //send register to read from
    i2c_master_send(value); //send value to change register to
    i2c_master_stop(); //send stop bit
}
unsigned char readPin(unsigned char address, unsigned char reg){ 
    //address = 0b0100(A2,A1,A0);
    //reg = 0x0A //for OLAT
    unsigned char addressW = address << 1;
    unsigned char addressR = (address << 1) | 0b00000001;
    i2c_master_start(); //send start bit
    i2c_master_send(addressW); //send address of chip
    i2c_master_send(reg); //send register to read from
    i2c_master_restart();
    i2c_master_send(addressR);
    unsigned char value = i2c_master_recv(); //recieve value
    i2c_master_ack(1);
    i2c_master_stop(); //send stop bit
    return value;
}

void set_gp7(unsigned char value){
    unsigned char address = 0b01000000;
    unsigned char reg = 0x0A;
    value = value << 7;
    i2c_master_start(); //send start bit
    i2c_master_send(address); //send address of chip
    i2c_master_send(reg); //send register to read from
    i2c_master_send(value); //send value to change register to
    i2c_master_stop(); //send stop bit
}

int read_gp0(){
    unsigned char reg = 0x09; //for GPIO
    unsigned char addressW = 0b01000000;
    unsigned char addressR = 0b01000001;
    i2c_master_start(); //send start bit
    i2c_master_send(addressW); //send address of chip
    i2c_master_send(reg); //send register to read from
    i2c_master_restart();
    i2c_master_send(addressR);
    unsigned char value = i2c_master_recv(); //recieve value
    i2c_master_ack(1);
    i2c_master_stop(); //send stop bit
    return value ^= 0b00000001;
}