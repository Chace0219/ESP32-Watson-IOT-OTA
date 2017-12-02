/*
 * JC
 * 
 * Modified by jinzhouyun
 * Date 20170407
 * BluemixClient2.ino
 * This file is used to control the initialization of sensors, fetching data from the sensors and
 * publishing of the data to IoT Watson before the ESP enters deep sleep.
 */

//-------- Customise these values -----------
#define DEEP_SLEEP_SECONDS          30                                          //Number of seconds the ESP is in deep sleep
#define PUBLISH_INTERVAL            30000                                       //The interval between messages published to IoT Watson
//-------- Customise the above values --------

const char publishTopic[] = "iot-2/evt/status/fmt/json";                        //Topic to publish data to IoT Watson
const char responseTopic[] = "iotdm-1/response";                                //Generic subscription topic IoT Watson will respond along
const char manageTopic[] = "iotdevice-1/mgmt/manage";                           //Topic to declare a device as a managed device to IoT Watson
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";               //Reboot subscription topic
const char mikeTopic[] = "iotdm-1/mgmt/custom/mike/UpdateESP8266";  //Custom mike topic used for firmware update/download

IPAddress timeServer(203, 56, 27, 253); // NTP Server au.pool.ntp.org           //IP address of NTP time server

volatile uint8_t v_buf[2043];                                                   //Buffer to collect vibration sensor data
SensorData data;                                                                //Used to hold sensor data object
ColorData c_data;                                                               //Used to collect color data

/*
 * printStatus
 * Desc: Used for debugging to print available memory
 * Input: void
 * Output: void
 */
void printStatus() {
  //Serial.println("Flash memory:     "+String(ESP.getFreeSketchSpace())+" Bytes.");
  Serial.println("FHM: "+String(ESP.getFreeHeap())+" B");  
}


/*
 * getCurrentTime
 * Desc: Calls NTP time server to get the current time
 * Input: void
 * Output: void
 */
void getCurrentTime(){
  int ntpRetryCount = 0;
  while (timeStatus() == timeNotSet && ++ntpRetryCount < 10) { // get NTP time
    Serial.println(WiFi.localIP());
    setSyncProvider(getNtpTime);
    setSyncInterval(60 * 60);  
  }
}

/*
 * setupSensors
 * Desc: Called from WifiManagerBluemix.ino to configure the sensors,  initiate ESP as a managed device
 *      and begin getting sensor data and publishing to Watson
 * Input: void
 * Output: void
 */
void setupSensors() {

   //Connected to Bluemix therefore credentials must be correct
	preferences.putUInt(WIFI_CONNECT_BOOL, 2);

	/*
	Wire.begin();

	//Configure sensors
	mouse_setup();
	mux_reset();
	led_driver_config();
	color_sensor_config(color_sensor_address_U);
	color_sensor_config(color_sensor_address_L);
	KX_init();
    */

	//Check if need to send firmware update response
	String firmwareUpdateResp = buildStringLoc(REQ_ID);

	if (firmwareUpdateResp.length() > 0)
	{
		Serial.println("Sending Reboot Success Response");
		dmActionResponse(firmwareUpdateResp, 200);
		/*
		EEPROM.write(REQ_ID_LEN, 0);
		EEPROM.commit();*/
	}

	//Send device management request
	initManagedDevice();
	//Begin getting sensor data
	sampleAndPublish();
}

/*
 * sampleAndPublish
 * Desc: Read sensor data and send it to bluemix
 * Input: void
 * Output: void
 */
void sampleAndPublish() {
    checkSubscriptionTopics();
    
    /*  
    
    read_RHT();
    read_mouse();
    // led_control(0x03 ,0x6A);// LED 3 is UV, 0x6A--> 25mA
    // Add Upper and Lower UV sensors functions here
    led_control(0x03 ,0x16);// LED 3 is UV,0x16 --> 5mA
    delay(10);
    // color_sensor_config(color_sensor_address_U);
    // delay(500);
    checkSubscriptionTopics();
    mux_reset();
    get_color_data (color_sensor_address_U, RGB_level,&c_data); 
    data.RED_U_UV=c_data.Red_level;
    data.GREEN_U_UV=c_data.Green_level;
    data.BLUE_U_UV=c_data.Blue_level;
    // Add Lower UV sensors functions here
    led_control(0x03 ,0x0);// LED 3 is UV,0x0 --> Off
    led_control(0x07 ,0x16);// LED 7 is IR,0x16 --> 5mA 
    // color_sensor_config(color_sensor_address_U);
    // color_sensor_config(color_sensor_address_L);
    // delay(500);
    get_color_data (color_sensor_address_U, C_level,&c_data);
    data.CLEAR_U_IR=c_data.Clear_level;   
    get_color_data (color_sensor_address_L, C_level,&c_data);
    data.CLEAR_L_IR=c_data.Clear_level;
    led_control(0x07 ,0x0);// LED 7 is IR,0x0 --> Off
    led_control(0x02 ,0x6A);// LED 2 is White, 0x6A--> 25mA
    delay(500);
    get_color_data (color_sensor_address_U, RGB_level,&c_data);
    data.RED_U_W=c_data.Red_level;
    data.GREEN_U_W=c_data.Green_level;
    data.BLUE_U_W=c_data.Blue_level;
    get_color_data (color_sensor_address_U, C_level,&c_data);
    data.CLEAR_U_W=c_data.Clear_level;  
    get_color_data (color_sensor_address_L, RGB_level,&c_data);
    data.RED_L_W=c_data.Red_level;
    data.GREEN_L_W=c_data.Green_level;
    data.BLUE_L_W=c_data.Blue_level;
    get_color_data (color_sensor_address_L, C_level,&c_data);
    data.CLEAR_L_W=c_data.Clear_level; 
    led_control(0x02 ,0x0);// LED 2 is White, 0x0--> off
    KX_getdata(v_buf);*/
    

    getCurrentTime();
    checkSubscriptionTopics();

    //Publish data to IoT Watson
    publishData(data);
    publishVibData(v_buf);
    
    //Enter Deep Sleep (note the ESP will reset once it wakes up and begin executing the program from the very begining)
    //ESP.deepSleep(1000 * 1000 * DEEP_SLEEP_SECONDS); //Deep Sleep for 60 seconds
}



