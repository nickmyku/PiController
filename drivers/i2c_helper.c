
//collections of helper functions for I2C bus
//Nicholas Mykulowycz
//Created May 26, 2014

//just so the full functions dont have to be written out every time
#define writeReg(dev,reg,data) wiringPiI2CWriteReg8(dev,reg,data)
#define readReg(dev,reg) wiringPiI2CReadReg8(dev,reg)


//convert integer to a boolean array, needed so register read data is easier to manipulate
int intToBits(bool* array, int number)
{
	int i;
	
	if(number>255)
	{
		for(i=7; i>=0; i--)
		{
			array[i] = true;
		}
		return -1;
	}
	
	for(i=7; i>=0; i--)
	{
		if(number >= (pow(2,i)))
		{
			number = number -pow(2,i);
			array[i] = true;
		}
		else
		{
			array[i] = false;
		}
		
		
	}
	
	
	return 0;
}

//converts a boolean array back to an integer, integer format is needed to send the dad back to the sx1509
int bitsToInt(bool array[8])
{
	int i;
	int number = 0;
	for(i=0; i<8; i++)
	{
		if(array[i])
		{
			number = number + pow(2,i);
		}	
	}
	
	return number;
}

int scaleIntensity(int num)
{
	//negative value limit is 0
	if(num<0)
	{
		return 0;
	}
	
	//positive value limit is 255
	if(num > 255)
	{
		return 255;
	}	
	
	//otherwise return the value which was input	
	return num;
}

