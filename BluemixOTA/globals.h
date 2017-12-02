#define              RH_T_SENSOR_I2C_ADDRESS                        0x40 // SHT21 7 bit address
#define              TRIG_RH_MEASUREMENT_HM                         0xF5 // no hold master trig for humidity
#define              TRIG_TEMP_MEASUREMENT                          0xF3 // no hold master trig for temperature
#define              mouse_SensorI2C_ADDRESS                        0x33 // mouse sensor 7 bit address
#define              SQUAL_Reg                                      0x05
#define              Mux_Address                                    0x70           // Address of 2 channel MUX

#define              led_driver_address                             0x2A   // 7 bit address
#define              mode_status_register                           0x01
#define              mode_status_register2                          0x02
#define              Independent_Sink_Current_led7                  0x14
#define              Independent_Sink_Current_led6                  0x15
#define              Independent_Sink_Current_led5                  0x16
#define              Independent_Sink_Current_led4                  0x17
#define              Independent_Sink_Current_led3                  0x18
#define              Independent_Sink_Current_led2                  0x19
#define              Independent_Sink_Current_led1                  0x1a

#define              color_sensor_address_U                         0x38 // for upper 
#define              color_sensor_address_L                         0x39 
#define              SYSTEM_CONTROL                                 0x40
#define              MODE_CONTROL1                                  0x41
#define              MODE_CONTROL2                                  0x42
#define              MODE_CONTROL3                                  0x44
#define              RED_DATA_L                                     0x50
#define              RED_DATA_H                                     0x51
#define              GREEN_DATA_L                                   0x52
#define              GREEN_DATA_H                                   0x53
#define              BLUE_DATA_L                                    0x54
#define              BLUE_DATA_H                                    0x55
#define              CLEAR_DATA_L                                   0x56
#define              CLEAR_DATA_H                                   0x57
#define              DINT_DATA_L                                    0x58
#define              DINT_DATA_H                                    0x59
#define              INTERRUPT_CONFIG                               0x60
#define              PERSISTENCE_SETTING                            0x61

#define              RGB_level                                      0x01 
#define              C_level                                        0x02

#define              kx_address                                     0x1E

// Preferences Key Defintion

#define WIFI_CONNECT_BOOL     "WifiConn"
#define CLOUD_CHOICE          "CloudChoice"
#define SSID_STR				  "SSID"
#define PASSWORD			  "PASSWORD"
#define ORG					  "ORG"
#define DEVTYP				  "DEVTYPE"
#define DEVTID				  "DEVTID"
#define TOKEN				  "TOK"
#define INTERVAL			  "INTERVAL"

#define REQ_ID				  "REQ" 
#define FIRMWARE			  "FIRM"

/*
//EEPROM MEM LAYOUT
#define WIFI_CONNECT_BOOL     0   //1 - credentials have been entered, 2 - credentials confirmed as correct
#define CLOUD_CHOICE          1   //Cloud choice: Azure: 0, Bluemix: 1
#define SSID_LENGTH           2
#define PASSWORD_LENGTH       3
#define SSID_LOCATION         4
#define PASSWORD_LOCATION     36
#define ORG_B_LEN             81
#define DEVTYP_B_LEN          82
#define DEVTID_B_LEN          83
#define TOK_B_LEN             84
#define ORG_B_LOC             85
#define DEVTYP_B_LOC          106
#define DEVTID_B_LOC          147
#define TOK_B_LOC             188

#define REQ_ID_LEN            229
#define REQ_ID_LOC            230 
#define FIRMWARE_LEN          280
#define FIRMWARE_LOC          281 // */

struct ColorData{
unsigned int Green_level;
unsigned int Red_level;
unsigned int Blue_level;
unsigned int Clear_level;  
};



enum CloudMode {
  IoTHub,
  EventHub
};

enum LedState {
  Off,
  On
};

enum SensorMode {
  None,
  Bmp180Mode,
  DhtShieldMode
};

enum BoardType {
  NodeMCU,
  WeMos,
  SparkfunThing,
  Other
};

enum DisplayMode {
  NoDisplay,
  LedMatrix
};

struct SensorData{
  float Temperature;
  float Humidity;
  int Squal;
  int UV_U;// UV sensor reading, upper board
  int UV_L_UVH;// UV sensor reading, lower board, UV LED high brightness
  int UV_L_UVL;// UV sensor reading, lower board, UV LED low brightness
  // RGB level of color sensor, upper board, UV led ON
  int RED_U_UV;
  int GREEN_U_UV;
  int BLUE_U_UV;  
  int CLEAR_U_IR;// CLEAR level of color sensor, upper board, IR led ON
  int CLEAR_L_IR;// CLEAR level of color sensor, lower board, IR led ON
  // RGB level of color sensor, upper board, White led ON
  int RED_U_W;
  int GREEN_U_W;
  int BLUE_U_W;  
  // RGB level of color sensor, lower board, White led ON
  int RED_L_W;
  int GREEN_L_W;
  int BLUE_L_W; 
  int CLEAR_U_W;//CLEAR level of color sensor, upper board, White led ON
  int CLEAR_L_W; //CLEAR level of color sensor, lower board, White led ON  
  int V_X;// Vibration X axis
  int V_Y;// Vibration Y axis
  int V_Z;  // Vibration Z axis
};

struct DeviceConfig {
  int WifiIndex = 0;
  unsigned long LastWifiTime = 0;
  int WiFiConnectAttempts = 0;
  int wifiPairs = 0;
  const char ** ssid;
  const char **pwd;
  BoardType boardType = Other;            // OperationMode enumeration: NodeMCU, WeMos, SparkfunThing, Other
  SensorMode sensorMode = None;           // OperationMode enumeration: DemoMode (no sensors, fakes data), Bmp180Mode, Dht11Mode
  DisplayMode displayMode = NoDisplay;    // DisplayMode enumeration: NoDisplay or LedMatrix

  unsigned int deepSleepSeconds = 0;      // Number of seconds for the ESP8266 chip to deepsleep for.  GPIO16 needs to be tied to RST to wake from deepSleep http://esp8266.github.io/Arduino/versions/2.0.0/doc/libraries.html
};
