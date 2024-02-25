// cai thu vien firebase esp8266 client
// cai thu vien arduino json barn 5x
// cai board manager esp8266 barn 2.7.4
// du lieu bom la c+data
// du lieu cua la d+data
// du lieu cam bien la ab + ....
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#define FIREBASE_HOST "https://smart-garden-5967a-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "Fgyjtuy5dmwLrXPZIxNy9KGvxxXeQOQQch3qKYM0"
const char* ssid = "OPPO F11 Pro"; //Enter SSID
const char* password = "123123123"; //Enter Password

FirebaseData firebaseData;
WiFiClient client;
String  path = "/";
FirebaseJson json;

SoftwareSerial zigbee (13, 15);

String data = "";
String data_bom = "";
uint8_t status_data_bom;
uint8_t old_status_data_bom;
String data_cua = "";
uint8_t status_data_cua;
uint8_t old_status_data_cua;
String sensor;
uint8_t pump_local;
uint8_t door_local;

String dht11_t;
String dht11_h;
String lm35;
String hum_land;
String light;

String send_pump;
String send_door;

uint8_t status_pump;;
uint8_t status_rolling_door;

uint8_t value_sensor;
uint8_t value_pump;
uint8_t value_rolling_door;
uint8_t value_light;

uint8_t e1, e2, e3, e4, e5;



void setup(void)
{
  Serial.begin(9600);
  zigbee.begin(9600);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");
  }

  Serial.println("");
  Serial.println("WiFi connection Successful");
  Serial.print("The IP Address of ESP8266 Module is: ");
  Serial.print(WiFi.localIP());// Print the IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (!Firebase.beginStream(firebaseData, path))
  {
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println();
  }
}

void loop()
{
  /********** kiem tra du lieu tu app **************/
  if (Firebase.getString(firebaseData, path + "/bom"))
  {
    data_bom = "";
    String data_bom1 = firebaseData.stringData();
    String dat = "";
    data_bom = (String)data_bom1[2] ;
    if (data_bom == "b")
    {
      status_data_bom = 1;
      dat = 'm';

    }
    else if (data_bom == "t")
    {
      dat = 'n';
      status_data_bom = 0;
    }
    if (old_status_data_bom != status_data_bom)
    {
      String transmit_data_bom = "b" + (String)status_data_bom;
      //zigbee.println(transmit_data_bom);
      zigbee.println(dat);
      delay(1000);

      Serial.println(transmit_data_bom);
      old_status_data_bom = status_data_bom;
    }
  }
  if (Firebase.getString(firebaseData, path + "/cua"))
  {
    data_cua = "";
    String dat = "";
    String data_cua1 = firebaseData.stringData();
    data_cua = (String)data_cua1[2] ;
    if (data_cua == "l")
    {
      status_data_cua = 1;
      dat = 'x';
    }
    else if (data_cua == "d")
    {
      status_data_cua = 0;
      dat = 'y';
    }
    else if (data_cua == "x")
    {
      status_data_cua = 2;
      dat = 'z';
    }
    if (old_status_data_cua != status_data_cua)
    {
      String transmit_data_cua = "c" + (String)status_data_cua;
      //zigbee.println(transmit_data_cua);
      zigbee.println(dat);
      delay(1000);

      Serial.println(transmit_data_cua);
      old_status_data_cua = status_data_cua;
    }
  }
  /*********** kiem tra du lieu tu zigbee **********/
  if (zigbee.available())
  {
    data = "";
    sensor = "";

    dht11_t = "";
    dht11_h = "";
    lm35 = "";
    hum_land = "";
    light = "";

    send_pump = "";
    send_door = "";

    data = zigbee.readString();
    Serial.println(data);
    if (value_pump == 0  )
    {
      for (int i = 0; i < data.length(); i ++)
      {
        if (data[i] == 'c')
        {
          pump_local = i;
          send_pump = data[i + 1];
          value_pump = 1;
        }
        if (data[i] == 'd')
        {
          door_local = i;
          send_door = data[i + 1];
          value_rolling_door = 1;
        }
      }
    }
    if(value_rolling_door == 0)
    {
      for (int i = 0; i < data.length(); i ++)
      {
        if (data[i] == 'd')
        {
          door_local = i;
          send_door = data[i + 1];
          value_rolling_door = 1;
        }
      }    
    }
    /********* tach du lieu cac cam bien*********/
    if (data[0] == 'a')
    {
      value_sensor = 1;
    }
  }
  
  if (value_sensor == 1)
  {

    for (int i = 1; i < 16; i++)
    {
      sensor += data[i];
    }
    //Serial.println(sensor);

    for (int i = 0; i < sensor.length(); i ++)
    {
      if (sensor[i] == 'b')
      {
        e1 = 1;
        e2 = 0;
        e3 = 0;
        e4 = 0;
        e5 = 0;
      }
      else if (sensor[i] == '+')
      {
        e1 = 0;
        e2 = 1;
        e3 = 0;
        e4 = 0;
        e5 = 0;
      }
      else if (sensor[i] == '-')
      {
        e1 = 0;
        e2 = 0;
        e3 = 1;
        e4 = 0;
        e5 = 0;
      }
      else if (sensor[i] == '*')
      {
        e1 = 0;
        e2 = 0;
        e3 = 0;
        e4 = 1;
        e5 = 0;
      }
      else if (sensor[i] == ':')
      {
        e1 = 0;
        e2 = 0;
        e3 = 0;
        e4 = 0;
        e5 = 1;
      }
      else
      {
        if (e1 == 1) dht11_t += sensor[i];
        if (e2 == 1) dht11_h += sensor[i];
        if (e3 == 1) lm35 += sensor[i];
        if (e4 == 1) hum_land += sensor[i];
        if (e5 == 1) light += sensor[i];
      }
    }
    //Serial.println(dht11_t);
    //Serial.println(dht11_h);
    //Serial.println(lm35);
    //Serial.println(hum_land);
    //Serial.println(light);
    uint8_t as = light.toInt();

    Firebase.setString(firebaseData, path + "/dht11_t", dht11_t);
    Firebase.setString(firebaseData, path + "/dht11_h", dht11_h);
    Firebase.setString(firebaseData, path + "/lm35", lm35);
    Firebase.setString(firebaseData, path + "/hum_land", hum_land);
    Firebase.setInt(firebaseData, path + "/light", as);
    value_sensor = 0;
  }
  if (value_pump == 1)
  {
    String pump = (String)send_pump[0];
    status_pump = pump.toInt();
    Serial.println(status_pump);
    /********* dieu khien bom mach dieu khien ******************/
    if (status_pump == 1) Firebase.setString(firebaseData, path + "/bom", "bat");
    else Firebase.setString(firebaseData, path + "/bom", "tat");
    value_pump = 0;
  }


  if (value_rolling_door == 1)
  {
    String rolling_door = (String)send_door[0];
    status_rolling_door = rolling_door.toInt();
    Serial.println(status_rolling_door);
    /************* dieu khien cua tu mach dieu khien *********************/
    if (status_rolling_door == 1) Firebase.setString(firebaseData, path + "/cua", "len");
    else if (status_rolling_door == 2) Firebase.setString(firebaseData, path + "/cua", "xuong");
    else if (status_rolling_door == 0) Firebase.setString(firebaseData, path + "/cua", "dung");
    value_rolling_door = 0;
  }
}
