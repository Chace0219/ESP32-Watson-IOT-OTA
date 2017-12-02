/*
 * JC
 * Cloud.ino
 * This file contains the functions that publish and subscribe to IoT Watson
 */



char buf[2048];          //Message buffer to send to IoT Watson

/*
 * serializeData
 * Desc: Takes the sensor data and formats it into a JSON payload 
 * Input: Colour sensor data
 * Output: JSON formated message
 */
String serializeData(SensorData data){

  //StaticJsonBuffer<JSON_OBJECT_SIZE(16)> jsonBuffer;  //  allow for a few extra json fields that actually being used at the moment
  StaticJsonBuffer<JSON_OBJECT_SIZE(22)> jsonBuffer;  //  allow for a few extra json fields that actually being used at the moment
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& d = root.createNestedObject("d");

  d["Utc"] = GetISODateTime();

  d["Temperature"] = data.Temperature;
  d["Humidity"] = data.Humidity;
  d["Squal"] = data.Squal;
  d["UV_U"] = data.UV_U;
  d["UV_L_UVH"] = data.UV_L_UVH;
  d["UV_L_UVL"] = data.UV_L_UVL;
  
  d["RED_U_UV"] = data.RED_U_UV;
  d["GREEN_U_UV"] = data.GREEN_U_UV;
  d["BLUE_U_UV"] = data.BLUE_U_UV;
    
  d["CLEAR_U_IR"] = data.CLEAR_U_IR;
  d["CLEAR_L_IR"] = data.CLEAR_L_IR;
  
  d["RED_U_W"] = data.RED_U_W;
  d["GREEN_U_W"] = data.GREEN_U_W;
  d["BLUE_U_W"] = data.BLUE_U_W;
  
  d["RED_L_W"] = data.RED_L_W;
  d["GREEN_L_W"] = data.GREEN_L_W;
  d["BLUE_L_W"] = data.BLUE_L_W;
  
  d["CLEAR_U_W"] = data.CLEAR_U_W;
  d["CLEAR_L_W"] = data.CLEAR_L_W;

  root.printTo(buf, sizeof(buf));
  return (String) buf; 
}

/*
 * publishData
 * Desc: Send the color data to IoT Watson
 * Input: Raw sensor color data
 * Output: void
 */
void publishData(SensorData data) {
  memset(buf, 0, strlen(buf));
   String payload = serializeData(data);
   int count = 0;
   while (!client->publish(publishTopic, (char*) payload.c_str()) && count < 3) {
        Serial.println(F("Color Publish FAILED"));
        //printStatus();
        delay(2000);
        count ++;
        mqttConnect();
   }
   checkSubscriptionTopics();
}

/*
 * publishVibData
 * Desc: Takes the vibration data, splits it up and forms 5 JSON messages to send to bluemix
 * Input: Buffer containing the vibration data
 * Output: void
 */
void publishVibData(volatile uint8_t * v_buf){
   //Send 142 x,y,z samples per messages therefore 681 total samples / 142 = 4.7 -> send 5 messages
   for (int i = 0; i < 5; i ++) { 
    sprintf(buf, "{\"d\":{\"T\":\"{%4d-%02d-%02dT%02d:%02d:%02d},", year(), month(), day(), hour(), minute(), second());
    int e = i * 142 + 142;
    
    for (int kv = i * 142; kv < e && kv < 681; kv++){  //681 samples fill 2kB buffer
        sprintf(buf + strlen(buf), "{%d,%d,%d}", (int8_t)v_buf[3*kv], (int8_t)v_buf[3*kv+1], (int8_t)v_buf[3*kv+2]);
        if (kv < e - 1 && kv < 680) sprintf(buf + strlen(buf), ",");  
    }

     sprintf(buf + strlen(buf), "\"}}");
     //Serial.println(buf);
     printStatus();
     int count = 0;
     //After 3 failed attemps give up trying to send the message
     while (!client->publish(publishTopic, buf) && count < 3) {
        Serial.println(F("VIB Publish FAILED"));
        //printStatus();
        delay(2000);
        count ++;
        mqttConnect();
     }
     checkSubscriptionTopics();
     printStatus();
     delay(1000);
  }
}

/*
 * initManagedDevice
 * Desc: Used to configure the ESP as a managed device and subscrive to IoT Watson topics
 * Input: void
 * Output: void
 */
void initManagedDevice() {
	//Subscribe to REBOOT
	if (client->subscribe(rebootTopic, 1)) 
	{
		Serial.println(F("subscribe to reboot OK"));
	} 
	else 
	{
		Serial.println(F("subscribe to reboot FAILED"));
	}
	checkSubscriptionTopics();

	//Prepare device management request
	StaticJsonBuffer<300> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	JsonObject& d = root.createNestedObject("d");
	JsonObject& metadata = d.createNestedObject("metadata");
	metadata["publishInterval"] = PUBLISH_INTERVAL;
	JsonObject& supports = d.createNestedObject("supports");
	supports["deviceActions"] = true;
	supports["firmwareActions"] = true;
	supports["mike"] = true;

	char buff[300];
	root.printTo(buff, sizeof(buff));
	Serial.println(F("publishing device metadata:")); Serial.println(buff);
	if (client->publish(manageTopic, buff)) 
	{
		Serial.println(F("device Publish ok"));
      
		delay(1000);
		//Subscribe to mike topic if the device management request was succesful
		if (client->subscribe(mikeTopic, 1)) {
			Serial.println(F("subscribe to mike OK"));
		} else {
			Serial.println(F("subscribe to mike FAILED"));
		}
		checkSubscriptionTopics();

	} 
	else 
	{
		Serial.print(F("device Publish failed:"));
	}
}

/*
 * callback
 * Desc: Called when a subscription message is receieved from IoT Watson
 * Input: Topic the message was received from, payload and size of the payload
 * Output: void
 */
void callback(char* topic, byte* payload, unsigned int payloadLength) {
   //Print the topic the message came from
   Serial.println(F("callback invoked for topic: ")); Serial.println(topic); Serial.println(F("----------------------------------"));
   String resp =  (String((char *) payload));
   int s1 = resp.indexOf("reqId");

   
   if (strcmp (mikeTopic, topic) == 0) {    //mike TOPIC
       //Get paramters from mike command
       int s2 = resp.indexOf("\",\"d\":", s1);
       String reqId = resp.substring(s1 + 8, s2);
       s1 = resp.indexOf("URL") + 14;
       String url = resp.substring(s1, resp.indexOf("\"}", s1));
       s1 = resp.indexOf("Version") + 18;
       String verNew = resp.substring(s1, resp.indexOf("\"}", s1));
       
       //Compare ver # to see if update required
       if(updateToNewVersion(verNew))
	   {
          //Make changes to EEPROM to reflect update
          //String verOld = buildStringLoc(FIRMWARE_LEN, FIRMWARE_LOC);
		  String verOld = buildStringLoc(FIRMWARE);
          Serial.println(verOld + " to " + verNew);

		  writeStringLoc(FIRMWARE, verNew);
		  writeStringLoc(REQ_ID, reqId);
          
		  
          //t_httpUpdate_return ret = ESPhttpUpdate.update(url);

		  //If we get to this line, update has failed - must undo EEPROM changes
          writeStringLoc(FIRMWARE, verOld);
          writeStringLoc(REQ_ID, "");

	   }
       //Send update failed/not needed response
       dmActionResponse(reqId, 500);
   }
   
   if (strcmp (rebootTopic, topic) == 0) {        //REBOOT TOPIC
      String resp =  (String((char *) payload));
      int s1 = resp.indexOf("reqId");
      int s2 = resp.indexOf("}", s1);
      String reqId = resp.substring(s1 + 8, s2 - 1);
      dmActionResponse(reqId,202);
      
      Serial.println(F("Rebooting..."));
      for (int i = 0; i < 5; i ++){
        delay(1000);
        Serial.print(".");
      }
      
      ESP.restart();
   }
}

/*
 * dmActionResponse
 * Desc: Sends a generic acknowledgement to IoT Watson confirming a message was received
 * Input: Request ID of the receieved message, response code
 * Output: void
 */
void dmActionResponse(String reqId, int code) {
      String resp = "{\"reqId\":\"" + reqId + "\",\"rc\":" + String(code) + "}";
      Serial.println(resp);
      if (client->publish("iotdevice-1/response", (char*) resp.c_str())) {
        Serial.println(F("Publish OK"));
      } else {
        Serial.println(F("Publish FAILED"));
      }
}

/*
 * updateToNewVersion
 * Desc: On a firmware update, compares the firmware version to the current firmware version to
 *  detrermine if an update is required
 * Input: Firmaware update version
 * Output: True is update is required, false otherwise
 */
bool updateToNewVersion(String verNew) {
  int o1, o2, o3;
  int n1, n2, n3;
  //String verOld = buildStringLoc(FIRMWARE_LEN, FIRMWARE_LOC);
  String verOld = buildStringLoc(FIRMWARE);
  int s1, s2;
  s2 = verOld.indexOf(".", 0);
  o1 = verOld.substring(0, s2).toInt();

  s1 = s2 + 1;
  s2 = verOld.indexOf(".", s1);
  o2 = verOld.substring(s1, s2).toInt();

  s1 = s2 + 1;
  o3 = verOld.substring(s1).toInt();

  s2 = verNew.indexOf(".", 0);
  n1 = verNew.substring(0, s2).toInt();

  s1 = s2 + 1;
  s2 = verNew.indexOf(".", s1);
  n2 = verNew.substring(s1, s2).toInt();

  s1 = s2 + 1;
  n3 = verNew.substring(s1).toInt();
  
  
//  s1 = sscanf(verOld.c_str(), "%d.%d.%d", &o1, &o2, &o3);
//  s1 = sscanf(verNew.c_str(), "%d.%d.%d", &n1, &n2, &n3);

  if(o1 < n1 || (o1 <= n1 && o2 <=n2 && o3 < n3) || (o1 <=n1 && o2 < n2))
    return true;
  return false;  
}
