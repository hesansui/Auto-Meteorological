#include <dht11.h>
#include <U8glib.h>
#include <DFRobotDFPlayerMini.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#define BMP085_ADDRESS 0x77  // I2C address of BMP085
const unsigned char OSS = 0;  // Oversampling Setting

// Calibration values
int ac1;
int ac2;
int ac3;
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1;
int b2;
int mb;
int mc;
int md;
long b5; 
dht11 DHT;
U8GLIB_SSD1306_128X64 u8g (U8G_I2C_OPT_NONE);
void GetData();
double dewPoint(double,double);
void DisplayOled();
void TopTitle();
void DisplayPressure();
void bmp085Calibration();
void DisplayAltitude();
unsigned int bmp085ReadUT();
SoftwareSerial TrumpetSerial(8,9);//RX TX
DFRobotDFPlayerMini Mp3Moudle;
float TemperatureData = 0.0;
float HumidityData = 0.0;
float DewPointData = 0.0;
float StationPressureData = 1013.4;
float AltitudeData = 0.0;
float StandardAtmosphere = 0.0;
void setup()
{
  Serial.begin(9600);
  Wire.begin();
  TrumpetSerial.begin(9600);
  Mp3Moudle.begin(TrumpetSerial,true,false);
  Mp3Moudle.volume(30);
  Mp3Moudle.playMp3Folder(1);
  bmp085Calibration();
}

void loop()
{
  Wire.beginTransmission(60);
  Wire.write(80);
  u8g.firstPage();
  do
  {
    TopTitle();
    DisplayOled();
  }while(u8g.nextPage());
  delay(4000);
  u8g.firstPage();
  do
  {
    DisplayPressure();
  }while(u8g.nextPage());
  delay(2000);

  u8g.firstPage();
  do
  {
    DisplayAltitude();
  }while(u8g.nextPage());
  delay(2000);
  GetData();
  Wire.endTransmission();
}


void GetData()
{
  DHT.read(7);
  TemperatureData = bmp085GetTemperature(bmp085ReadUT());
  HumidityData = DHT.humidity;
  DewPointData = dewPoint(TemperatureData,HumidityData);
  StationPressureData = bmp085GetPressure(bmp085ReadUP());
  AltitudeData = (calcAltitude(StationPressureData*100));
  StandardAtmosphere = StationPressureData/1013.25;
}

double dewPoint(double celsius, double humidity)
{
        double A0= 373.15/(273.15 + celsius);
        double SUM = -7.90298 * (A0-1);
        SUM += 5.02808 * log10(A0);
        SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
        SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
        SUM += log10(1013.246);
        double VP = pow(10, SUM-3) * humidity;
        double T = log(VP/0.61078);   // temp var
        return (241.88 * T) / (17.558-T);
}
void DisplayOled()
{
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(0,31,"Temper:"); 
  u8g.setPrintPos(70,31);
  u8g.print(TemperatureData);
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(105,31," C");
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(0,47,"Humidi:");
  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(70,47);
  u8g.print(HumidityData);
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(105,47," %");
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(0,63,"Dewpoi:");
  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(70,63);  
  u8g.print(DewPointData);
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(105,63," C");
}

void TopTitle()
{
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(0,10,"Auto Observatory");
  u8g.drawHLine(0,14,127);
}
void DisplayPressure()
{
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(0,35,"Station Pressure");
  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(18,58);
  u8g.print(StationPressureData);
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(76,58,"hpa");
}

void DisplayAltitude()
{
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(0,35,"Station Altitude");
  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(65,58);
  u8g.print(AltitudeData);
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(80,58,"M");
}

void DisplayStandardAtmosphere()
{
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(0,35,"Standard Atmosphere");
  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(18,58);
  u8g.print(StandardAtmosphere);
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(76,58,"hpa");
}

//Bmp180
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Calculate temperature in deg C
float bmp085GetTemperature(unsigned int ut){
  long x1, x2;

  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  float temp = ((b5 + 8)>>4);
  temp = temp /10;

  return temp;
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
float bmp085GetPressure(unsigned long up){
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  float temp = p/100.0;
  return temp;
}


// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;

  return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2);
  msb = Wire.read();
  lsb = Wire.read();

  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT(){
  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  // Wait at least 4.5ms
  delay(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long bmp085ReadUP(){

  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  msb = bmp085Read(0xF6);
  lsb = bmp085Read(0xF7);
  xlsb = bmp085Read(0xF8);

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);

  return up;
}

void writeRegister(int deviceAddress, byte address, byte val) {
  Wire.beginTransmission(deviceAddress); // start transmission to device 
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}

int readRegister(int deviceAddress, byte address){

  int v;
  Wire.beginTransmission(deviceAddress);
  Wire.write(address); // register to read
  Wire.endTransmission();

  Wire.requestFrom(deviceAddress, 1); // read a byte

  while(!Wire.available()) {
    // waiting
  }

  v = Wire.read();
  return v;
}

float calcAltitude(float pressure){

  float A = pressure/101325;
  float B = 1/5.25588;
  float C = pow(A,B);
  C = 1 - C;
  C = C /0.0000225577;

  return C;
}
