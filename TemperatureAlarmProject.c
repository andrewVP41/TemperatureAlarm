/****************************************************
 *
 * FILE NAME  :  TemperatureAlarm.c     
 * DESCRIPTION:  This program monitores the temperature and compares it with the threshold to set the alarm off and the status of
 *               buttons to turn the alarm off
 * *************************************************/
 
//BUTTONS
#include "mraa_beaglebone_pinmap.h"

//LCD
#define LCD_ADDR 		 0x 3e

mraa_i2c_context i2cp;

void home_LCD (void)
{
    uint8_t buf[2] = {0x00,0x02};
    mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

void LCD_Print (uint8_t* str)
{
    uint8_t buf[80];
    uint8_t buf1[2]={0x00,0x80};		
    int32_t i = 0, strl, j=0;
 
    home_LCD();
    buf[i] = 0x40;  //register for display
    i++;
    strl = strlen((char*)str);
    for (j = 0; j < strl; j++)
    {
			buf[i] = str[j];
			i++;
	}
	mraa_i2c_write(i2cp, buf1, 2);
	mraa_i2c_write(i2cp, buf, i);
}

void LCD_init (void)
{
   uint8_t init1[2] = {0x00,0x38};
   uint8_t init2[8] = {0x00, 0x39, 0x14,0x74,0x54,0x6f,0x0c,0x01};
   // 2 lines 8 bit 3.3V Version
    mraa_i2c_write(i2cp, init1, 2);
    mraa_i2c_write(i2cp, init2,8);  //Function Set
}

void clear_LCD (void)
{
        uint8_t buf[2] = {0x00,0x01};
        mraa_i2c_write(i2cp, buf, 2);  //Clear Display
}

//TEMP 
mraa_gpio_context MCP3208_DIN;
mraa_gpio_context MCP3208_DOUT;
mraa_gpio_context MCP3208_CLK;
mraa_gpio_context MCP3208_CS;

int GetMCP3208 (int Channel);

int GetMCP3208 (int Channel)
{
	int i;
	int val;

	mraa_gpio_write (MCP3208_DIN, 0);
	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CS, 0);

	Channel = Channel | 0x18;
	for (i = 0; i < 5; i ++)
	{
		if (Channel & 0x10)
		{
			mraa_gpio_write (MCP3208_DIN, 1);
		}
		else
		{
			mraa_gpio_write (MCP3208_DIN, 0);
		}
		Channel <<= 1;

		mraa_gpio_write (MCP3208_CLK, 0);
		mraa_gpio_write (MCP3208_CLK, 1);
	}

	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CLK, 1);

	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CLK, 1);

	val = 0;
	for (i = 0; i < 12; i ++)
	{
		mraa_gpio_write (MCP3208_CLK, 0);
		mraa_gpio_write (MCP3208_CLK, 1);

		val = (val << 1) | ((int) mraa_gpio_read (MCP3208_DOUT));	
	}

	mraa_gpio_write (MCP3208_CS, 1);
	mraa_gpio_write (MCP3208_DIN, 0);
	mraa_gpio_write (MCP3208_CLK, 0);

	return val;
}

//PWM
#include <signal.h>
int period = 500000000; // 500ms or 2Hz cycle
int duty_cycle = 1; 

// set which pwm channel with in a pwmchip
int red_pwm_ch = 1;

char pwm_per_red[] ="/sys/class/pwm/pwm-2:1/period";
char pwm_duty_red[] ="/sys/class/pwm/pwm-2:1/duty_cycle";
char pwm_enable_red[] ="/sys/class/pwm/pwm-2:1/enable";

char pwm_chip_un0[] = "/sys/class/pwm/pwmchip0/unexport";
char pwm_chip_ex0[] = "/sys/class/pwm/pwmchip0/export";
char pwm_chip_un2[] = "/sys/class/pwm/pwmchip2/unexport";
char pwm_chip_ex2[] = "/sys/class/pwm/pwmchip2/export";

FILE *ppntr,*wpntr,*epntr;

int close_pwm()    // Disable all the PWM's otherwise they will remian active. 
		   // Initilizing active PWM pins will cause a segment fault. 
{
	epntr = fopen(pwm_enable_red,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
	
	epntr = fopen(pwm_chip_un0,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
	epntr = fopen(pwm_chip_un0,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
	epntr = fopen(pwm_chip_un0,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"1");
		fclose(epntr);
	}
       	epntr = fopen(pwm_chip_un2,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"1");
		fclose(epntr);
	}

}

int set_duty(float duty)
{
	int pulse_w;
	wpntr = fopen(pwm_duty_red,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	pulse_w = (int) period*duty;
	//printf("Duty Cycle = %3.1f \%\n",duty*100);
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);
	return MRAA_SUCCESS;
}


int set_period(int per)
{
	int pulse_w;	
	ppntr = fopen(pwm_per_red,"w");
	if (ppntr == NULL) 
	{
		printf("Can't set red pwm pin period, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(ppntr,"%d",per);
	fclose(ppntr);
	printf("Period set to %5.2f ms\n",(float)per/1000000);

	return MRAA_SUCCESS;

}

void exit_signal(int signum)
{
	printf("\nExiting PWM Program \n");
	close_pwm();
	exit(signum);
}

int main()
{
	mraa_init();
	// create a GPIO object from MRAA using it
	mraa_gpio_context B1_pin;
	mraa_gpio_context B2_pin;
	mraa_gpio_context B3_pin;
	mraa_gpio_context B4_pin;
	unsigned int value1 = 0;
	unsigned int value2 = 0;
	unsigned int value3 = 0;
	unsigned int value4 = 0;
	B1_pin = mraa_gpio_init(B1);
	B2_pin = mraa_gpio_init(B2);
	B3_pin = mraa_gpio_init(B3);
	B4_pin = mraa_gpio_init(B4);
	
	unsigned int preset = 28;
	unsigned int done = 0;

	char buf[16];
	char temp_buf[20];

	mraa_result_t status = MRAA_SUCCESS;
	char pwm_buffer[20];
	float dty_cycle;
	signal(SIGINT, exit_signal);
// Initialize Red LED PWM pwmchip2
	ppntr = fopen(pwm_per_red,"r");
	if (ppntr != NULL)
	{
		fclose(ppntr);
		printf("PWM already enabled\n");
	}
	else
	{
		epntr=fopen(pwm_chip_ex2,"w");
		printf("Enabling PWM\n");
		fprintf(epntr,"%d",red_pwm_ch);
		fclose(epntr);
	}

	sleep(1);

	set_period(period);
	set_duty(duty_cycle);


// Enable the outputs

	epntr = fopen(pwm_enable_red,"w");
	if (epntr == NULL) 
	{
		printf("Can't enable red pwm pin, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(epntr,"%d",1);
	fclose(epntr);


	



	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);

	MCP3208_DIN = mraa_gpio_init (SPI_MOSI_PIN);
	MCP3208_DOUT = mraa_gpio_init (SPI_MISO_PIN);
	MCP3208_CLK = mraa_gpio_init (SPI_CLK_PIN);
	MCP3208_CS = mraa_gpio_init (SPI_CS0_PIN);

	mraa_gpio_dir(MCP3208_DIN, MRAA_GPIO_OUT_HIGH);
	mraa_gpio_dir(MCP3208_DOUT, MRAA_GPIO_IN);
	mraa_gpio_dir(MCP3208_CLK, MRAA_GPIO_OUT);
	mraa_gpio_dir(MCP3208_CS, MRAA_GPIO_OUT);

	// create a GPIO object from MRAA using it
	mraa_gpio_context relay;
	relay = mraa_gpio_init(RELAY_PIN);
	
	if (relay == NULL) 
	{
		printf("MRAA couldn't initialize GPIO %d, exiting",RELAY_PIN);
		return MRAA_ERROR_UNSPECIFIED;
	}
	
    sleep(1); //Delay after initialization 
	// set the pin as input
	if (mraa_gpio_dir(relay, MRAA_GPIO_OUT) != MRAA_SUCCESS) 
	{
		printf("Can't set digital pin %d as output, exiting",RELAY_PIN);
		return MRAA_ERROR_UNSPECIFIED;
	};




	LCD_init ();
	LCD_Print ((uint8_t*)"AVP Technologies");
	sleep(2);
	

	




	if (B1_pin == NULL || B2_pin == NULL || B3_pin == NULL || B4_pin == NULL) 
	{
		fprintf(stderr, "MRAA couldn't initialize GPIO, exiting\n");
		return MRAA_ERROR_UNSPECIFIED;
	}
	

	// set the pin as input
	if (mraa_gpio_dir(B1_pin, MRAA_GPIO_IN) != MRAA_SUCCESS || mraa_gpio_dir(B2_pin, MRAA_GPIO_IN) != MRAA_SUCCESS|| mraa_gpio_dir(B3_pin, MRAA_GPIO_IN) != MRAA_SUCCESS || mraa_gpio_dir(B4_pin, MRAA_GPIO_IN) != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Can't set digital pin as output, exiting\n");
		return MRAA_ERROR_UNSPECIFIED;
	};
	

	int state=0;
	while(1){
		


		unsigned int time =0;
		period = 500000000;
		set_period(period);

		
		while(state == 0) {
			if(mraa_gpio_read(B1_pin) ==0 && preset <35){
				preset++;	} 
			else if(mraa_gpio_read(B2_pin) ==0 && preset >15){
				preset --; }
			else if(mraa_gpio_read(B4_pin) == 0){
				state = 1; }
			//printf("Preset value: %u\n", preset);
			sprintf(buf, "Preset: %u ", preset);
    			clear_LCD();
    			LCD_Print ((uint8_t*)buf);
			sleep(1);
		}

		while(state == 1) {
			float temp = ( ( ( ((float) GetMCP3208 (7)) * 3300 / 4096)-500)/10);
			sprintf (temp_buf, "T:%3.1f P:%u", temp, preset);
			LCD_Print (temp_buf);
			sleep (1);
			if(temp > preset){
				state = 2;
			}
			if(mraa_gpio_read(B3_pin) == 0){ set_duty(0); close_pwm(); state=0;}
		}
		
		while(state ==2){
			for (dty_cycle = 0.0;dty_cycle < 1.0;dty_cycle += 0.5){
				set_duty(dty_cycle);
				sleep(1);
				time++;
			}
			if(mraa_gpio_read(B3_pin) == 0){ set_duty(0); close_pwm(); state=0;}
			if(time == 10){state =3;}
		}
		
		period =100000000;  // 100ms or 10Hz
		set_period(period);

		while(state ==3){
			for (dty_cycle = 0.0;dty_cycle < 1.0;dty_cycle += 0.5){
				set_duty(dty_cycle);
				sleep(1);
				time++;
			}
			if(mraa_gpio_read(B3_pin) == 0){ set_duty(0); close_pwm(); state=0;}
			if(time == 20){state =4;}
		}

		while(state ==4){
			for (dty_cycle = 0.0;dty_cycle < 1.0;dty_cycle += 0.5){
				set_duty(dty_cycle);
				sleep(1);
			}
				mraa_gpio_write(relay, 0);
				sleep(2);
				mraa_gpio_write(relay, 1);
				sleep(2);
			if(mraa_gpio_read(B3_pin) == 0){ set_duty(0); close_pwm(); state=0;}
			
		}

	}

	return MRAA_SUCCESS;
}




