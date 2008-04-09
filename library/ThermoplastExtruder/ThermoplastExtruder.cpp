#include "WConstants.h"
#include "ThermoplastExtruder.h"

//
// Start of temperature lookup table
//
#define NUMTEMPS  20
short temptable[NUMTEMPS][2] = {
// { adc ,  temp }
   { 1 ,  929 } ,
   { 54 ,  266 } ,
   { 107 ,  217 } ,
   { 160 ,  190 } ,
   { 213 ,  172 } ,
   { 266 ,  158 } ,
   { 319 ,  146 } ,
   { 372 ,  136 } ,
   { 425 ,  127 } ,
   { 478 ,  119 } ,
   { 531 ,  111 } ,
   { 584 ,  103 } ,
   { 637 ,  96 } ,
   { 690 ,  88 } ,
   { 743 ,  80 } ,
   { 796 ,  71 } ,
   { 849 ,  62 } ,
   { 902 ,  50 } ,
   { 955 ,  34 } ,
   { 1008 ,  2 }
};
//
// End of temperature lookup table
//

/*!
	motor_dir_pin must be a digital output.
	motor_pwm_pin and heater_pin must be PWM capable outputs.
	thermistor_pin must be an analog input.
*/
ThermoplastExtruder::ThermoplastExtruder(byte motor_dir_pin, byte motor_pwm_pin, byte heater_pin, byte cooler_pin, byte thermistor_pin)
{
	this->motor_dir_pin = motor_dir_pin;
	this->motor_pwm_pin = motor_pwm_pin;
	this->heater_pin = heater_pin;
	this->cooler_pin = cooler_pin;
	this->thermistor_pin = thermistor_pin;

	pinMode(this->motor_dir_pin, OUTPUT);
	pinMode(this->motor_pwm_pin, OUTPUT);
	pinMode(this->heater_pin, OUTPUT);

	this->getTemperature();
	this->setSpeed(0);
	this->setDirection(true);

	//init our temp stuff.
	this->target_celsius = 0;
	this->max_celsius = 0;
	this->heater_low = 0;
	this->heater_high = 0;
}

/*!
  Sets the motor speed from 0-255 (0 is off).
*/
void ThermoplastExtruder::setSpeed(byte speed)
{
	this->motor_pwm = speed;
	analogWrite(this->motor_pwm_pin, this->motor_pwm);
}

void ThermoplastExtruder::setCooler(byte speed)
{
	this->cooler_pwm = speed;
	analogWrite(this->cooler_pin, this->cooler_pwm);
}

/*!
  Sets the motor direction (true = forward, false = backward)
*/
void ThermoplastExtruder::setDirection(bool dir)
{
	this->motor_dir = dir;
	digitalWrite(this->motor_dir_pin, this->motor_dir);
}

void ThermoplastExtruder::setTemperature(int temp)
{
	this->target_celsius = temp;
	this->max_celsius = (int)((float)temp * 1.1);
}

/**
*  Samples the temperature and converts it to degrees celsius.
*  Returns degrees celsius.
*/
int ThermoplastExtruder::getTemperature()
{
	this->raw_temperature = analogRead(thermistor_pin);
	this->current_celsius = this->calculateTemperatureFromRaw(this->raw_temperature);
	
	return this->current_celsius;
}

/**
*  Samples the temperature and converts it to degrees celsius.
*  Returns degrees celsius.
*/
int ThermoplastExtruder::calculateTemperatureFromRaw(int raw)
{
	int celsius = 0;
	byte i;
	
	for (i=1; i<NUMTEMPS; i++)
	{
		if (temptable[i][0] > raw)
		{
			celsius  = temptable[i-1][1] + 
				(raw - temptable[i-1][0]) * 
				(temptable[i][1] - temptable[i-1][1]) /
				(temptable[i][0] - temptable[i-1][0]);
			
			if (celsius > 255)
				celsius = 255; 

			break;
		}
	}

	// Overflow: We just clamp to 0 degrees celsius
	if (i == NUMTEMPS)
		celsius = 0;
		
	return celsius;
}

/*!
  Manages motor and heater based on measured temperature:
  o If temp is too low, don't start the motor
  o Adjust the heater power to keep the temperature at the target
 */
void ThermoplastExtruder::manageTemperature()
{
	//make sure we know what our temp is.
	this->getTemperature();

	//put the heater into high mode if we're not at our target.
	if (current_celsius < target_celsius)
	{
		analogWrite(heater_pin, heater_high);
	}
	//put the heater on low if we're at our target.
	else if (current_celsius < max_celsius)
	{
		analogWrite(heater_pin, heater_low);
	}
	//turn the heater off if we're above our max.
	else
	{
		analogWrite(heater_pin, 0);
	}
}
