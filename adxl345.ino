#include <Wire.h>
#include "LTask.h"
#include "vmtimer.h"
#include <LGPS.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LBattery.h>


#define WIFI_AP "AndroidAP"
#define WIFI_PASSWORD "00000000"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "api.mediatek.com"

#define FILTER_N 10

LWiFiClient content;

gpsSentenceInfoStruct info;

int pin = 8;

int I2C_Address = 0xA7 >> 1; // ADXL345 的 I2C 地址

float oldAccx,oldAccy,oldAccz,oldDot,oldAcc;

int stepNumber;

char buff[256];
char nors[2], eorw[2];
String latitude_str, longitude_str;

void setup(){
    LWiFi.begin();
    Serial.begin(115200);
    Serial.println("Start : \n------------------------------------");
    
    Wire.begin();  //初始化 I2C
    setReg(0x2D, 0xA); // (打開電源, 設定輸出資料速度為 100 Hz)  
    StepReset();

    //gps setup
    Serial.println("Connecting to AP");
    while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
    {
      delay(1000);
    }
     while (0 == content.connect(SITE_URL, 80))
    {
      Serial.println("Re-Connecting to WebSite");
      delay(1000);
    }
    LGPS.powerOn();
    Serial.println("LGPS Power on, and waiting ..."); 
    delay(3000);
}

void loop(){
  
  //計步
  stepCounter();
  uploadStep();
  
  //gps
  LGPS.getData(&info);
  Serial.println((char*)info.GPGGA); 
  parseGPGGA((const char*)info.GPGGA);

  //電池
  uploadBattery();
  
   delay(500);
}

/* setReg(reg,data)：寫入佔存器
 * 參數：
 *     reg → 指定佔存器位址
 *     data → 要寫入的資料
 */
void setReg(int reg, int data){
    Wire.beginTransmission(I2C_Address);
    Wire.write(reg); // 指定佔存器
    Wire.write(data); // 寫入資料
    Wire.endTransmission();
}

/* getData(reg)：取得佔存器裡的資料
 * 參數：reg → 佔存器位址
 */
int getData(int reg){
    Wire.beginTransmission(I2C_Address);
    Wire.write(reg);
    Wire.endTransmission();
    
    Wire.requestFrom(I2C_Address,1);
    
    if(Wire.available()<=1){
        return Wire.read();
    }
}


// 0:x 1:y 2:z
float Filter(int s){
  int low_b,high_b;
  for(int i=0; i < FILTER_N; i++)
  {
    if(s == 0){
      low_b = getData(0x32); // 取得 X 軸 低位元資料
      high_b  = getData(0x33); // 取得 X 軸 高位元資料
    }
    else if(s == 1){
      low_b = getData(0x34); // 取得 Y 軸 低位元資料
      high_b = getData(0x35); // 取得 Y 軸 高位元資料
    }
    else if(s == 2){
      low_b = getData(0x36); // 取得 Z 軸 低位元資料
      high_b = getData(0x37); // 取得 Z 軸 高位元資料
    }
    delay(5);
  }
  return  ((high_b << 8)  + low_b);
}

void StepReset(){
  stepNumber = 0;
}

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  double latitude;
  double longitude;
  int tmp, hour, minute, second, num ;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
    Serial.println(buff);
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(3, GPGGAstr);
    strncpy(nors,  &GPGGAstr[tmp], 1);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(5, GPGGAstr);
    strncpy(eorw,  &GPGGAstr[tmp], 1);
    convertCoords(latitude, longitude, nors, eorw);
    
    sprintf(buff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
    Serial.println(buff); 
    Serial.println("(after) latitudet = " + latitude_str + " longitude = " + longitude_str);
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "satellites number = %d", num);
    Serial.println(buff); 
    if(num!=-1){
      uploadGPS(latitude_str,longitude_str,"1.0");
    }
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

void convertCoords(float tmplat, float tmplong, const char* n_or_s, const char* e_or_w) { 
  /*
  Latitude  5213.2930,N --> 52d 13.2930' N
  52 degrees 13.2930 minutes NORTH
  52 + (13.2930 / 60) = 52.22155
  Because it is north of the equator, the number remains positive.
  +52.22155
  */
  
  float lat_return, lon_return;
  int lat_deg_int = int(tmplat / 100); //extract the first 2 chars to get the latitudinal degrees
  float latitude_float = tmplat - (lat_deg_int * 100); //remove the degrees part of the coordinates - so we are left with only minutes-seconds part of the coordinates
  lat_return = lat_deg_int + latitude_float / 60; //add back on the degrees part, so it is decimal degrees
  
  //Check if it is N or S, S will turn the value negative
  
  if (*n_or_s == 'S'){
    Serial.println("is South");
    lat_return *= -1;
  }

  sprintf(buff, "%.7f", lat_return);   
  latitude_str = buff;
  
  /*
  Longitude  00004.5337,W  --> 00d 04.5337' W
  00 degrees 4.5337 minutes WEST
  00 + (4.5337 / 60) = 0.0755616
  Because it is West, the number becomes negative.
  -0.0755616
  */
  
  int lon_deg_int = int(tmplong / 100);
  float longitude_float = tmplong - lon_deg_int * 100;
  lon_return = lon_deg_int + longitude_float / 60;  //add back on the degrees part, so it is decimal degrees
  if (*e_or_w == 'W'){
    Serial.println("is West");
    lon_return *= -1;
  }
  sprintf(buff, "%.7f", lon_return); 
  longitude_str = buff;
}

void stepCounter(){
   float X = Filter(0);
    float Y = Filter(1);
    float Z = Filter(2);

    float dot = (oldAccx * X)+(oldAccy * Y)+(oldAccz * Z);
    float oldAcc = abs(sqrt(oldAccx * oldAccx + oldAccy * oldAccy + oldAccz * oldAccz));
    float newAcc = abs(sqrt(X * X + Y * Y + Z * Z));
    dot /= (oldAcc * newAcc);  //計算加速度變量

    if(abs(dot - oldDot) >= 0.05) {
        stepNumber += 1;
        Serial.println(stepNumber);
    }
    
    oldAccx = X;
    oldAccy = Y;
    oldAccz = Z;
    oldDot = dot;

    //上傳
    uploadStep();
}


  //gps 資料上傳
void uploadGPS(String lat,String lon,String high){
      
 Serial.println("GPStest");

    //確保連線正常
    
 while (0 == content.connect(SITE_URL, 80))
    {
      Serial.println("Re-Connecting to WebSite");
      delay(1000);
    }
  
//資料上傳
  content.println("POST /mcs/v2/devices/DMWq5R7i/datapoints.csv HTTP/1.1");


String data = "2,,"+lat+","+lon + "," + high;

int dataLength = data.length();

content.println("Host: api.mediatek.com");
content.println("deviceKey: 2XlQ1UH0qUW3CIJn");
content.print("Content-Length: ");
content.println(dataLength);
content.println("Content-Type: text/csv");
content.println("Connection: close");
content.println();
content.println(data);

//  while (content)
//  {
//    int v = content.read();
//    if (v != -1)
//    {
//      Serial.print((char)v);
//    }
//    else
//    {
//      Serial.println("no more content, disconnect");
//      while (1)
//      {
//        delay(1);
//      }
//    }
//  }

  
}


//電池資料上傳
void uploadBattery(){
  
  //確保連線正常
   
 while (0 == content.connect(SITE_URL, 80))
    {
      Serial.println("Re-Connecting to WebSite");
      delay(1000);
    }
  
//資料上傳
 content.println("POST /mcs/v2/devices/DMWq5R7i/datapoints.csv HTTP/1.1");

 
String data = "3,,"+String(LBattery.level());


int dataLength = data.length();

content.println("Host: api.mediatek.com");
content.println("deviceKey: 2XlQ1UH0qUW3CIJn");
content.print("Content-Length: ");
content.println(dataLength);
content.println("Content-Type: text/csv");
content.println("Connection: close");
content.println();
content.println(data);

}

//計步資料上傳
void uploadStep(){
  //確保連線正常
   
  while (0 == content.connect(SITE_URL, 80))
    {
      Serial.println("Re-Connecting to WebSite");
      delay(1000);
    }
  
   //資料上傳
   content.println("POST /mcs/v2/devices/DMWq5R7i/datapoints.csv HTTP/1.1");
   
   String data = "1,,"+ String(stepNumber);

  int dataLength = data.length();
    
   content.println("Host: api.mediatek.com");
   content.println("deviceKey: 2XlQ1UH0qUW3CIJn");
   content.print("Content-Length: ");
   content.println(dataLength);
   content.println("Content-Type: text/csv");
   content.println("Connection: close");
   content.println();
   content.println(data);
}
