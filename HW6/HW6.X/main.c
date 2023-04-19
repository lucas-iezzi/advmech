

int main(){
	// nu32 startup
	
	// init i2c_master_setup
	
	// init the chip --> set the IODIR register for GP0 in, GP7 out
	
	
	while(1){
		//blink green or yellow LED for heartbeat
		
		/*
		//read from GP0
		if (GP0 == 1) //turn on GP7
		else //turn off GP7
		*/
		
		/*
		//turn on GP7
		delay()
		//turn off GP7
		delay()
		*/
	}
}

void turn_on_gp7(){
	//send i2c_master_start
	//send address of chip
	address = 0b01000000 //0100(A2,A1,A0,RW)
	//send register to change
	reg is 0x0A
	//send value to make register
	0b01111111 // (GP7,GP6,GP5,GP4,GP3,GP2,GP1,GP0)
	//send i2c_master_stop
}

int read_from_gp0(){
	//send the start
	//send the address of the chip with write bit
	//send the register you want to read from
	//send a restart
	//send the chip address with read bit
	//recieve from the chip (read from the chip)
	unsigned char r = i2c_master_recv()
	//ack with a 1 to tell chip you are done reading
	//send stop
	return (r&0b1) //0 or 1
	//r = 0b76543210