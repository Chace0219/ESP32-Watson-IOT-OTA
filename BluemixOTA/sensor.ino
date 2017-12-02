/*
 * JC Comment - These are all the functions corresponding to sensor operations (MH I believe you wrote these,
 * I haven't made any modifications to them)
 */

void I2C1_WriteData(byte d_address, byte r_address, byte value)
{
  Wire.beginTransmission(d_address);                               
  Wire.write(r_address);            // sends instruction byte
  Wire.write(value);            // sends instruction byte
  Wire.endTransmission();   
  delay(1);  
  }

byte  I2C1_ReadData(byte d_address, byte r_address)
{
  delay(1);
 volatile byte val=0;
  Wire.beginTransmission(d_address);           
  Wire.write(r_address); 
  Wire.requestFrom((int)d_address, (int)1);
  
 while(Wire.available())    // slave may send less than requested
  {   
  val=Wire.read();             // sends potentiometer value byte  
  }
  Wire.endTransmission();
  delay(1);
  return val;
}

byte  I2C_ReadData_RS(byte d_address, byte r_address) // i2c read with repeated start
{
  delay(1);
 volatile byte val=0;
  Wire.beginTransmission(d_address);           
  Wire.write(r_address); 
  Wire.endTransmission(false);
  Wire.requestFrom((int)d_address, (int)1);
  
 while(Wire.available())    // slave may send less than requested
  {   
  val=Wire.read();             // sends potentiometer value byte  
  }  
  Wire.endTransmission(true);
  delay(1);
  return val;
}

void mouse_setup(void)
{
    Wire.beginTransmission(mouse_SensorI2C_ADDRESS);                               
    Wire.write(byte(0x61));            // sends instruction byte
    Wire.write(byte(0xC9));            // sends instruction byte
    Wire.endTransmission();   
    delay(1);
}

void mux_reset(void)
{
    uint8_t controlr=0x00;// control register of I2C MUX 
    I2C1_WriteData(Mux_Address, controlr, 0x1); 
    delay(5); 
}

void led_driver_config()
{
    I2C1_WriteData(led_driver_address, mode_status_register, 0x2e);
    
    I2C1_WriteData(led_driver_address, 0x03, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x04, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x05, 0x7f);
    
    I2C1_WriteData(led_driver_address, 0x06, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x07, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x08, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x09, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x0a, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x0f, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x10, 0x46);//D7:IR; D2:white; D3:UV
    
    I2C1_WriteData(led_driver_address, 0x11, 0xcf);
    
    I2C1_WriteData(led_driver_address, 0x12, 0x00);
    
    I2C1_WriteData(led_driver_address, 0x13, 0x00);        
}

void color_sensor_config(uint8_t address)
{
    I2C1_WriteData(address, MODE_CONTROL3, 0x02); // 72(0111 0010) 44(0100 0100) 00000010
    mux_reset();
    delay(10);
    I2C1_WriteData(address, MODE_CONTROL2, 0x10); // 72(0111 0010) 42(0100 0010) 00010000 
}

void KX_init(void)
{
    delay(1);
    I2C1_WriteData(kx_address, 0x18, 0x00);// CNTL1 register- sensor in stansby mode
    
    delay(1);
    I2C1_WriteData(kx_address, 0x1B, 0x8D);// writing ODCNTL register- hypass filter bypass and ODR =6.4KHz for 12.8 it is 0x8E and for 25.6 it is 0x8F  
    delay(1);
    I2C1_WriteData(kx_address, 0x3A, 0xFF);// writing BUF_CNTL1 register- number of sample required for water mark int=1024
    delay(1);
    I2C1_WriteData(kx_address, 0x3B, 0x8C);// writing BUF_CNTL2 register- buffer mode active -low resolution-buffer ful interrupt disabled-FIFO mode
    delay(1); 
    I2C1_WriteData(kx_address, 0x18, 0x90);// start operating mode in low resolution and 8g acceleration range
    
    delay(1);// delay is needed here
}

void read_RHT(void)// reads Releative humidity and Temperature
{
  volatile byte val[3] = {0x0};
  int i=0;
  volatile long result;
  volatile float rh;
  volatile float te;
  Wire.beginTransmission(RH_T_SENSOR_I2C_ADDRESS); // transmit to device #30 (0x1e)                              
  Wire.write(TRIG_RH_MEASUREMENT_HM); // sends instruction byte
  delay(66);
  Wire.requestFrom(RH_T_SENSOR_I2C_ADDRESS, 3);  
 while(Wire.available())    // slave may send less than requested
  {   
  val[i]=Wire.read();             // sends potentiometer value byte  
  i++;
  }
  Wire.endTransmission(); 
  result=((val[0])<<8)|((val[1]&0xFC));
  result=result&0x0000FFFF;
  rh=result*0.0019-6;
  data.Humidity = (float)rh;
//  Serial.print("Humidity is:");
//  Serial.println(rh);

  // Measuring Temperature
  Wire.beginTransmission(RH_T_SENSOR_I2C_ADDRESS); // transmit to device #30 (0x1e)                              
  Wire.write(TRIG_TEMP_MEASUREMENT); // sends instruction byte
  delay(66);
  Wire.requestFrom(RH_T_SENSOR_I2C_ADDRESS, 3);  
 while(Wire.available())    // slave may send less than requested
  {   
  val[i]=Wire.read();             // sends potentiometer value byte  
  i++;
  }
  Wire.endTransmission(); 
  result=((val[0])<<8)|((val[1]&0xFC));
  result=result&0x0000FFFF;
  te=result*0.0027-46.85;
  data.Temperature =(float)te;
//  Serial.println("");
//  Serial.print("Temperature is: ");
//  Serial.println(te);
//  Serial.println("");    
}

void read_mouse(void)// reads squal value
{
  byte val = 0;
  Wire.beginTransmission(mouse_SensorI2C_ADDRESS);           
  Wire.write(byte(SQUAL_Reg)); 
  Wire.requestFrom(mouse_SensorI2C_ADDRESS, 1);
  delay(1);
 while(Wire.available())    // slave may send less than requested
  {   
  val=Wire.read();             // sends potentiometer value byte  
  data.Squal=val;
//  Serial.print("Squal is: ");
//  Serial.println(val);
//  Serial.println("");
  }
  Wire.endTransmission();   
}

void led_control(uint8_t select , uint8_t value )
{
               
       switch (select)
       {
       case 0x1 : //LED1        
       I2C1_WriteData(led_driver_address, Independent_Sink_Current_led1, value); 
       break;
       case 0x2 : //LED2
       I2C1_WriteData(led_driver_address, Independent_Sink_Current_led2, value); 
//       Serial.println("----------------White LED Level: ");   
//       Serial.println(value);   
       break;
        case 0x3 : //LED3
      I2C1_WriteData(led_driver_address, Independent_Sink_Current_led3, value);
//      Serial.println("----------------UV LED Level: ");
//      Serial.println(value); 
        break;
        case 0x4 : //LED4
      I2C1_WriteData(led_driver_address, Independent_Sink_Current_led4, value); 
       break;
        case 0x5 : //LED5
      I2C1_WriteData(led_driver_address, Independent_Sink_Current_led5, value); 
       break;
        case 0x6 : //LED6
      I2C1_WriteData(led_driver_address, Independent_Sink_Current_led6, value); 
       break;
        case 0x7 : //LED7
      I2C1_WriteData(led_driver_address, Independent_Sink_Current_led7, value); 
//      Serial.println("----------------IR LED Level: ");
//      Serial.println(value); 
       break;
        }
         
       delay(100);
       
}

void get_color_data (uint8_t address, uint8_t in, ColorData* rgbc)
{
   volatile byte             c3=0x0;
   volatile byte          red_data[2]={0x00};
   volatile byte          green_data[2]={0x00};
   volatile byte          blue_data[2]={0x00};
   volatile byte          clear_data[2]={0x00};
   volatile int           k=0;
   
   unsigned int result;
   delay(10);  
   while ((c3 & 0x80)!=0x80) 
   {          
     c3 = I2C1_ReadData(address, MODE_CONTROL2);
   } 
  
   if (in==RGB_level)
   {   
  delay(1);   
  red_data[0] = I2C_ReadData_RS(address, RED_DATA_H);// 70 or 72 then 0x51 
 delay(1); 
 red_data[1] = I2C_ReadData_RS(address, RED_DATA_L);// 70 or 72 then 0x50

   result=((red_data[0])<<8)|(red_data[1]);
   result=result&0x0000FFFF;

   
   rgbc->Red_level=result;
   
   if (address==(0x70>>1))// Upper color sensor
 {
//  
//   Serial.println("Upper Red Level is: ");
//   Serial.println(result);
   
 }
 else
 {
//   Serial.println("Lower Red Level is: ");
//   Serial.println(result);
  
 }
  
   green_data[1] = I2C_ReadData_RS(address, GREEN_DATA_L);
   green_data[0] = I2C_ReadData_RS(address, GREEN_DATA_H);
   result=((green_data[0])<<8)|(green_data[1]);
   result=result&0x0000FFFF;
   rgbc->Green_level=result;
    if (address==(0x70>>1))// Upper color sensor
 {
//  Serial.println("Upper Green Level is: ");
//  Serial.println(result);
 }
 else
 {
//   Serial.println("Lower Green Level is: ");  
//   Serial.println(result);
 }
    
   blue_data[1] = I2C_ReadData_RS(address, BLUE_DATA_L);  
   blue_data[0] = I2C_ReadData_RS(address, BLUE_DATA_H);
   result=((blue_data[0])<<8)|(blue_data[1]);
   result=result&0x0000FFFF;
   rgbc->Blue_level=result;
    if (address==(0x70>>1))// Upper color sensor
 {
//   Serial.println("Upper Blue Level is: ");
//   Serial.println(result);
 }
 else
 {
//   Serial.println("Lower Blue Level is: ");  
//   Serial.println(result);
 }
    
   }
   else if (in==C_level)
   {
   clear_data[1] = I2C_ReadData_RS(address, CLEAR_DATA_L);
   delay(1);
   clear_data[0] = I2C_ReadData_RS(address, CLEAR_DATA_H);
   result=((clear_data[0])<<8)|(clear_data[1]);
   result=result&0x0000FFFF;
   rgbc->Clear_level=result;
   
   if (address==(0x70>>1))// Upper color sensor
 {
//   Serial.println("Upper Clear Level is: ");
//   Serial.println(result);
 }
 else
 {
//   Serial.println("Lower Clear Level is: "); 
//   Serial.println(result); 
 }
   
   }
               
}

void KX_getdata(volatile uint8_t* buf)
{
 
  uint8_t sData=0x0;// data acquired from I2C, buffer keeps 681 8 bit samples for 3 axis - 681x3
  int i=0;
  int j=0;  
  delay(1);  
 // Testing buffer
 I2C1_WriteData(kx_address, 0x3E, 0x00);// Clearing the buffer with writting to BUFF_CLEAR register
 
 delay(300);// Wait until buffer become full with 3.2KHz sampling rate
  
 // Read data from the buffer
 for(i=0;i<2043;i++)
 {
  buf[i]=I2C_ReadData_RS(kx_address, 0x3F);// read data buf  
  }
}


