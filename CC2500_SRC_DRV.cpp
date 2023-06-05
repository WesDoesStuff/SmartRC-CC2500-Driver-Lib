/*
  Based on:
            CC2500.cpp module library
            Copyright (c) 2010 Michael.
              Author: Michael, <www.elechouse.com>
              Version: November 12, 2010

  This library is designed to use CC2500 module on Arduino platform.
  CC2500 module is an useful wireless module. Using the functions of the 
  library, you can easily send and receive data by the CC2500 module. 
  Just have fun!
  For the details, please refer to the datasheet of CC2500.

----------------------------------------------------------------------------------------------------------------
cc2500 Driver. Wesley Smoyak [Feat. Little Satan and Wilson Shen (ELECHOUSE).]
----------------------------------------------------------------------------------------------------------------
*/
#include <SPI.h>
#include "CC2500_SRC_DRV.h"
#include <Arduino.h>
#include <math.h>

#ifdef CC2500_DEBUG


#endif

/****************************************************************/
#define   WRITE_SINGLE      0x00
#define   WRITE_BURST       0x40            //write burst
#define   READ_SINGLE       0x80            //read single
#define   READ_BURST        0xC0            //read burst
#define   READ_STATUS       0xC0

#define   BYTES_IN_RXFIFO   0x7F            //byte number in RXfifo
#define   max_modul 6

byte modulation = 2;
byte frend0;
byte chan = 0;
int pa = 1;
byte last_pa;
byte SCK_PIN;
byte MISO_PIN;
byte MOSI_PIN;
byte SS_PIN;
byte GDO0;
byte GDO2;
byte SCK_PIN_M[max_modul];
byte MISO_PIN_M[max_modul];
byte MOSI_PIN_M[max_modul];
byte SS_PIN_M[max_modul];
byte GDO0_M[max_modul];
byte GDO2_M[max_modul];
byte gdo_set=0;
bool spi = 0;
bool ccmode = 0;
float MHz = 2400.000;
byte m4RxBw = 0;
byte m4DaRa;
byte m2DCOFF;
byte m2MODFM;
byte m2MANCH;
byte m2SYNCM;
byte m1FEC;
byte m1PRE;
byte m1CHSP;
byte mc0XOSC_FORCE_ON;
byte mc0PIN_CTRL_EN;
byte mc0PO_TIMEOUT;
byte mc0FS_AUTOCAL;
byte pc1PQT;
byte pc1CRC_AF;
byte pc1APP_ST;
byte pc1ADRCHK;
byte pc0WDATA;
byte pc0PktForm;
byte pc0CC2400_EN;
byte pc0CRC_EN;
byte pc0LenConf;
byte trxstate = 0;
float XOSC = 26; // Crystal frequency in MHz
float synthINC = (XOSC / 65536);  // XOSC / 2^16
float frequency_band[2]{2400.000, 2483.500};
byte clb1[2]= {24,28};
byte clb2[2]= {31,38};
byte clb3[2]= {65,76};
byte clb4[2]= {77,79};

/****************************************************************/
uint8_t PA_TABLE[8]     {0x00,0x46,0x55,0x97,0xA9,0xBB,0xFE,0xFF};
//                       -55  -20  -16  -10   -4   -2    0   +1
uint8_t PA_WRITE[8]     {0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00};

/****************************************************************
*FUNCTION NAME:SpiStart
*FUNCTION     :spi communication start
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::SpiStart(void){
  // initialize the SPI pins
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(SS_PIN, OUTPUT);

  // enable SPI
  #ifdef ESP32
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  #else
  SPI.begin();
  #endif
}
/****************************************************************
*FUNCTION NAME:SpiEnd
*FUNCTION     :spi communication disable
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::SpiEnd(void){
  // disable SPI
  SPI.endTransaction();
  SPI.end();
}
/****************************************************************
*FUNCTION NAME: GDO_Set()
*FUNCTION     : set GDO0,GDO2 pin for serial pinmode.
*INPUT        : none
*OUTPUT       : none
****************************************************************/
void CC2500::GDO_Set (void){
	pinMode(GDO0, OUTPUT);
	pinMode(GDO2, INPUT);
}
/****************************************************************
*FUNCTION NAME: GDO_Set()
*FUNCTION     : set GDO0 for internal transmission mode.
*INPUT        : none
*OUTPUT       : none
****************************************************************/
void CC2500::GDO0_Set (void){
  pinMode(GDO0, INPUT);
}
/****************************************************************
*FUNCTION NAME:Reset
*FUNCTION     :CC2500 reset //details refer datasheet of CC2500
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Reset (void){
  digitalWrite(SS_PIN, LOW);
  delay(1);
  digitalWrite(SS_PIN, HIGH);
  delay(1);
  digitalWrite(SS_PIN, LOW);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  while(digitalRead(MISO_PIN));
  SPI.transfer(CC2500_SRES);
  while(digitalRead(MISO_PIN));
  digitalWrite(SS_PIN, HIGH);
}
/****************************************************************
*FUNCTION NAME:Init
*FUNCTION     :CC2500 initialization
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Init(void){
  setSpi();
  SpiStart();                   //spi initialization
  digitalWrite(SS_PIN, HIGH);
  digitalWrite(SCK_PIN, HIGH);
  digitalWrite(MOSI_PIN, LOW);
  Reset();                      //CC2500 reset
  RegConfigSettings();          //CC2500 register config
  SpiEnd();
}
/****************************************************************
*FUNCTION NAME:SpiWriteReg
*FUNCTION     :CC2500 write data to register
*INPUT        :addr: register address; value: register value
*OUTPUT       :none
****************************************************************/
void CC2500::SpiWriteReg(byte addr, byte value){
  SpiStart();
  digitalWrite(SS_PIN, LOW);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  while(digitalRead(MISO_PIN));
  SPI.transfer(addr);
  SPI.transfer(value); 
  digitalWrite(SS_PIN, HIGH);
  SpiEnd();
}
/****************************************************************
*FUNCTION NAME:SpiWriteBurstReg
*FUNCTION     :CC2500 write burst data to register
*INPUT        :addr: register address; buffer:register value array; num:number to write
*OUTPUT       :none
****************************************************************/
void CC2500::SpiWriteBurstReg(byte addr, byte *buffer, byte num){
  byte i, temp;
  SpiStart();
  temp = addr | WRITE_BURST;
  digitalWrite(SS_PIN, LOW);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  while(digitalRead(MISO_PIN));
  SPI.transfer(temp);
  for (i = 0; i < num; i++){
    SPI.transfer(buffer[i]);
  }
  digitalWrite(SS_PIN, HIGH);
  SpiEnd();
}
/****************************************************************
*FUNCTION NAME:SpiStrobe
*FUNCTION     :CC2500 Strobe
*INPUT        :strobe: command; //refer define in CC2500.h//
*OUTPUT       :none
****************************************************************/
void CC2500::SpiStrobe(byte strobe){
  SpiStart();
  digitalWrite(SS_PIN, LOW);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  while(digitalRead(MISO_PIN));
  SPI.transfer(strobe);
  digitalWrite(SS_PIN, HIGH);
  SpiEnd();
}
/****************************************************************
*FUNCTION NAME:SpiReadReg
*FUNCTION     :CC2500 read data from register
*INPUT        :addr: register address
*OUTPUT       :register value
****************************************************************/
byte CC2500::SpiReadReg(byte addr){
  byte temp, value;
  SpiStart();
  temp = addr| READ_SINGLE;
  digitalWrite(SS_PIN, LOW);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  while(digitalRead(MISO_PIN));
  SPI.transfer(temp);
  value=SPI.transfer(0);
  digitalWrite(SS_PIN, HIGH);
  SpiEnd();
  return value;
}

/****************************************************************
*FUNCTION NAME:SpiReadBurstReg
*FUNCTION     :CC2500 read burst data from register
*INPUT        :addr: register address; buffer:array to store register value; num: number to read
*OUTPUT       :none
****************************************************************/
void CC2500::SpiReadBurstReg(byte addr, byte *buffer, byte num){
  byte i,temp;
  SpiStart();
  temp = addr | READ_BURST;
  digitalWrite(SS_PIN, LOW);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  while(digitalRead(MISO_PIN));
  SPI.transfer(temp);
  for(i=0;i<num;i++){
    buffer[i]=SPI.transfer(0);
  }
  digitalWrite(SS_PIN, HIGH);
  SpiEnd();
}

/****************************************************************
*FUNCTION NAME:SpiReadStatus
*FUNCTION     :CC2500 read status register
*INPUT        :addr: register address
*OUTPUT       :status value
****************************************************************/
byte CC2500::SpiReadStatus(byte addr){
  byte value,temp;
  SpiStart();
  temp = addr | READ_BURST;
  digitalWrite(SS_PIN, LOW);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  while(digitalRead(MISO_PIN));
  SPI.transfer(temp);
  value=SPI.transfer(0);
  digitalWrite(SS_PIN, HIGH);
  SpiEnd();
  return value;
}
/****************************************************************
*FUNCTION NAME:SPI pin Settings
*FUNCTION     :Set Spi pins
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setSpi(void){
  if (spi == 0){
    #if defined __AVR_ATmega168__ || defined __AVR_ATmega328P__
      SCK_PIN = 13; MISO_PIN = 12; MOSI_PIN = 11; SS_PIN = 10;
    #elif defined __AVR_ATmega1280__ || defined __AVR_ATmega2560__
      SCK_PIN = 52; MISO_PIN = 50; MOSI_PIN = 51; SS_PIN = 53;
    #elif ESP8266
      SCK_PIN = 14; MISO_PIN = 12; MOSI_PIN = 13; SS_PIN = 15;
    #elif ESP32
      SCK_PIN = 18; MISO_PIN = 19; MOSI_PIN = 23; SS_PIN = 5;
    #else
      SCK_PIN = 13; MISO_PIN = 12; MOSI_PIN = 11; SS_PIN = 10;
    #endif
  }
}
/****************************************************************
*FUNCTION NAME:COSTUM SPI
*FUNCTION     :set costum spi pins.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setSpiPin(byte sck, byte miso, byte mosi, byte ss){
  spi = 1;
  SCK_PIN = sck;
  MISO_PIN = miso;
  MOSI_PIN = mosi;
  SS_PIN = ss;
}
/****************************************************************
*FUNCTION NAME:COSTUM SPI
*FUNCTION     :set costum spi pins.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::addSpiPin(byte sck, byte miso, byte mosi, byte ss, byte modul){
  spi = 1;
  SCK_PIN_M[modul] = sck;
  MISO_PIN_M[modul] = miso;
  MOSI_PIN_M[modul] = mosi;
  SS_PIN_M[modul] = ss;
}
/****************************************************************
*FUNCTION NAME:GDO Pin settings
*FUNCTION     :set GDO Pins
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setGDO(byte gdo0, byte gdo2){
  GDO0 = gdo0;
  GDO2 = gdo2;  
  GDO_Set();
}
/****************************************************************
*FUNCTION NAME:GDO0 Pin setting
*FUNCTION     :set GDO0 Pin
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setGDO0(byte gdo0){
  GDO0 = gdo0;
  GDO0_Set();
}
/****************************************************************
*FUNCTION NAME:GDO Pin settings
*FUNCTION     :add GDO Pins
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::addGDO(byte gdo0, byte gdo2, byte modul){
  GDO0_M[modul] = gdo0;
  GDO2_M[modul] = gdo2;  
  gdo_set=2;
  GDO_Set();
}
/****************************************************************
*FUNCTION NAME:add GDO0 Pin
*FUNCTION     :add GDO0 Pin
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::addGDO0(byte gdo0, byte modul){
  GDO0_M[modul] = gdo0;
  gdo_set=1;
  GDO0_Set();
}
/****************************************************************
*FUNCTION NAME:set Modul
*FUNCTION     :change modul
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setModul(byte modul){
  SCK_PIN = SCK_PIN_M[modul];
  MISO_PIN = MISO_PIN_M[modul];
  MOSI_PIN = MOSI_PIN_M[modul];
  SS_PIN = SS_PIN_M[modul];
  if (gdo_set==1){
    GDO0 = GDO0_M[modul];
  }else if (gdo_set==2){
    GDO0 = GDO0_M[modul];
    GDO2 = GDO2_M[modul];
  }
}
/****************************************************************
*FUNCTION NAME:CCMode (not used now. maybe later? idk)
*FUNCTION     :Format of RX and TX data
*INPUT        :none
*OUTPUT       :none
****************************************************************/
/*
void CC2500::setCCMode(bool s){
  ccmode = s;
  if (ccmode == 1){
    SpiWriteReg(CC1101_IOCFG2,      0x0B);
    SpiWriteReg(CC1101_IOCFG0,      0x06);
    SpiWriteReg(CC1101_PKTCTRL0,    0x05);
    SpiWriteReg(CC1101_MDMCFG3,     0xF8);
    SpiWriteReg(CC1101_MDMCFG4,11+m4RxBw);
  }else{
    SpiWriteReg(CC1101_IOCFG2,      0x0D);
    SpiWriteReg(CC1101_IOCFG0,      0x0D);
    SpiWriteReg(CC1101_PKTCTRL0,    0x32);
    SpiWriteReg(CC1101_MDMCFG3,     0x93);
    SpiWriteReg(CC1101_MDMCFG4, 7+m4RxBw);
  }
  setModulation(modulation);
}
*/
/****************************************************************
*FUNCTION NAME:Modulation
*FUNCTION     :set CC2500 Modulation 
*INPUT        :none
*OUTPUT       :none
*NOTES        :(Available Modulations are 2-FSK, GFSK, ASK/OOK, MSK) also supports FHSS
****************************************************************/
void CC2500::setModulation(byte m){
  if (m>3){
    m=3;
  }
  modulation = m;
  Split_MDMCFG2();
  switch (m){
    case 0:         // 2-FSK
      m2MODFM=0x02;
      frend0=0x10;
      break;
    case 1:         // GFSK
      m2MODFM=0x12;
      frend0=0x10;
      break;
    case 2:         // ASK/OOK
      m2MODFM=0x32;
      frend0=0x11;
      break;
//    case 3: m2MODFM=0x42; frend0=0x10; break; // 4-FSK (no 4-FSK for CC2500
    case 3:         // MSK
      m2MODFM=0x72;
      frend0=0x10;
      break;
  }
  SpiWriteReg(CC2500_MDMCFG2, m2DCOFF+m2MODFM+m2MANCH+m2SYNCM);
  SpiWriteReg(CC2500_FREND0,   frend0);
  setPA(pa);
}
/****************************************************************
*FUNCTION NAME:PA Power
*FUNCTION     :set CC2500 PA Power 
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setPA(int p){
  int a;
  pa = p;

  if (pa <= -55){
    a = PA_TABLE[0];
  }else if (pa > -55 && pa <= -20){
    a = PA_TABLE[1];
  }else if (pa > -20 && pa <= -16){
    a = PA_TABLE[2];
  }else if (pa > -16 && pa <= -10){
    a = PA_TABLE[3];
  }else if (pa > -10 && pa <= -4){
    a = PA_TABLE[4];
  }else if (pa > -4 && pa <= -2){
    a = PA_TABLE[5];
  }else if (pa > -2 && pa <= 0){
    a = PA_TABLE[6];
  }else if (pa > 0){
    a = PA_TABLE[7];
  }
  last_pa = 1;

  if (modulation == 2){
    PA_WRITE[0] = 0;  
    PA_WRITE[1] = a;
  }else{
    PA_WRITE[0] = a;  
    PA_WRITE[1] = 0; 
  }
  
  SpiWriteBurstReg(CC2500_PATABLE,PA_TABLE,8);
}
/****************************************************************
*FUNCTION NAME:Frequency Calculator
*FUNCTION     :Calculate the basic frequency. 
*INPUT        :none
*OUTPUT       :none
*NOTE         :Frequency =  (XOSC/2^16) * FREQ[23:0]
      (assuing 26MHz Crystal) 26/65536 = 13/32768
               FREQ[23:0] = Frequency / (XOSC/2^16)

              freq2 = FREQ[23:16]
              freq1 = FREQ[15:8]
              freq0 = FREQ[7:0]

****************************************************************/
void CC2500::setMHZ(float mhz){
  uint8_t freq[3] = {0, 0, 0};
  uint32_t freqWord;
  float frequency = 0;
  
  if(mhz < frequency_band[0]){
    mhz = frequency_band[0];
  }else if(mhz > frequency_band[1]){
    mhz = frequency_band[1];
  }
  
  frequency = ((uint32)(mhz / synthINC))*synthINC;
  MHz = frequency;
  freqWord = round(frequency/synthINC);
  
  freq[2] =  (freqWord >> 16) & 0xff;
  freq[1] =  (freqWord >> 8) & 0xff;
  freq[0] =   freqWord & 0xff;
  
  #ifdef CC2500_DEBUG
  Serial.print("Requested:\t");
  Serial.print(mhz, 6);
  Serial.print(" MHz\nActual:\t");
  Serial.print(frequency, 6);
  Serial.println(" MHz");
  
  for(int i=0;i<3;i++){
    Serial.print("freq");
    Serial.print(i);
    Serial.print(":\t");
    Serial.println(freq[i], HEX);
  }
  
  #endif
  
  SpiWriteReg(CC2500_FREQ2, freq[2]);
  SpiWriteReg(CC2500_FREQ1, freq[1]);
  SpiWriteReg(CC2500_FREQ0, freq[0]);
  
  Calibrate();
}
/****************************************************************
*FUNCTION NAME:Set Frequency Band
*FUNCTION     :Allows you to set a spacific band o frequencies within the supported range. Default is 2400 – 2483.5 MHz
*INPUT        :none
*OUTPUT       :none
*NOTE         :Sets the frequency to the lowest of the new band.
****************************************************************/
void CC2500::setBand(float mhz_l, float mhz_h){
  
  byte freq[3] = {0, 0, 0};
  if(mhz_l < 2400.000 || mhz_l > 2483.500){
    frequency_band[0] = 2400.000;
  }else{
    frequency_band[0] = mhz_l;
  }
  if(mhz_h < 2400.000 || mhz_h > 2483.500){
    frequency_band[1] = 2483.500;
  }else{
    frequency_band[1] = mhz_h;
  }
  
  setMHZ(mhz_l);
  
}
/****************************************************************
*FUNCTION NAME:Calibrate
*FUNCTION     :Calibrate frequency
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Calibrate(void){
  
  SpiStrobe(CC2500_SCAL); // manual calibration
  
}

/*
/****************************************************************
*FUNCTION NAME:Calibration offset  (not used at the moment)
*FUNCTION     :Set calibration offset
*INPUT        :none
*OUTPUT       :none
****************************************************************
void CC2500::setClb(byte b, byte s, byte e){
  if (b == 1){
    clb1[0]=s;
    clb1[1]=e;  
  }else if (b == 2){
    clb2[0]=s;
    clb2[1]=e;  
  }else if (b == 3){
    clb3[0]=s;
    clb3[1]=e;  
  }else if (b == 4){
    clb4[0]=s;
    clb4[1]=e;  
  }
}
*/

/****************************************************************
*FUNCTION NAME:getCC2500
*FUNCTION     :Test Spi connection and return 1 when true.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
bool CC2500::getCC2500(void){
  setSpi();
  if (SpiReadStatus(CC2500_VERSION) == 0x3){
    return 1;
  }else{
    return 0;
  }
}
/****************************************************************
*FUNCTION NAME:getMode
*FUNCTION     :Return the Mode. Sidle = 0, TX = 1, Rx = 2.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
byte CC2500::getMode(void){
  return trxstate;
}
/****************************************************************
*FUNCTION NAME:Set Sync_Word
*FUNCTION     :Sync Word
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setSyncWord(byte sh, byte sl){
  SpiWriteReg(CC2500_SYNC1, sh);
  SpiWriteReg(CC2500_SYNC0, sl);
}
/****************************************************************
*FUNCTION NAME:Set ADDR
*FUNCTION     :Address used for packet filtration. Optional broadcast addresses are 0 (0x00) and 255 (0xFF).
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setAddr(byte v){
  SpiWriteReg(CC2500_ADDR, v);
}
/****************************************************************
*FUNCTION NAME:Set PQT
*FUNCTION     :Preamble quality estimator threshold
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setPQT(byte v){
  Split_PKTCTRL1();
  pc1PQT = 0;
  if (v>7){
    v=7;
  }
  pc1PQT = v*32;
  SpiWriteReg(CC2500_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set CRC_AUTOFLUSH
*FUNCTION     :Enable automatic flush of RX FIFO when CRC is not OK
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setCRC_AF(bool v){
  Split_PKTCTRL1();
  pc1CRC_AF = 0;
  if (v==1){
    pc1CRC_AF=8;
  }
  SpiWriteReg(CC2500_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set APPEND_STATUS
*FUNCTION     :When enabled, two status bytes will be appended to the payload of the packet
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setAppendStatus(bool v){
  Split_PKTCTRL1();
  pc1APP_ST = 0;
  if (v==1){
    pc1APP_ST=4;
  }
  SpiWriteReg(CC2500_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set ADR_CHK
*FUNCTION     :Controls address check configuration of received packages
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setAdrChk(byte v){
  Split_PKTCTRL1();
  pc1ADRCHK = 0;
  if (v>3){
    v=3;
  }
  pc1ADRCHK = v;
  SpiWriteReg(CC2500_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set WHITE_DATA
*FUNCTION     :Turn data whitening on / off.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setWhiteData(bool v){
  Split_PKTCTRL0();
  pc0WDATA = 0;
  if (v == 1){
    pc0WDATA=64;
  }
  SpiWriteReg(CC2500_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CC2400_EN+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set PKT_FORMAT
*FUNCTION     :Format of RX and TX data
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setPktFormat(byte v){
  Split_PKTCTRL0();
  pc0PktForm = 0;
  if (v>3){
    v=3;
  }
  pc0PktForm = v*16;
  SpiWriteReg(CC2500_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CC2400_EN+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set CC2400_EN
*FUNCTION     :Enable CC2400 support. Use same CRC implementation as CC2400.
*INPUT        :none
*OUTPUT       :none
NOTE          :If enabled,
                PKTCTRL1.CRC_AUTOFLUSH must be 0
                PKTCTRL0.WHITE_DATA must be 0
                So this function sets those settings when enabled but does not reset them when you disable.
****************************************************************/
void CC2500::setCC2400_EN(bool v){
  Split_PKTCTRL0();
  pc0CRC_EN = 0;
  if (v==1){
    setWhiteData(0);
    setCRC_AF(0);
    pc0CRC_EN=4;
  }
  SpiWriteReg(CC2500_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CC2400_EN+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set CRC
*FUNCTION     :CRC calculation in TX and CRC check in RX
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setCrc(bool v){
  Split_PKTCTRL0();
  pc0CRC_EN = 0;
  if (v==1){
    pc0CRC_EN=4;
  }
  SpiWriteReg(CC2500_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CC2400_EN+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set LENGTH_CONFIG
*FUNCTION     :Configure the packet length
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setLengthConfig(byte v){
  Split_PKTCTRL0();
  pc0LenConf = 0;
  if (v>3){
    v=3;
  }
  pc0LenConf = v;
  SpiWriteReg(CC2500_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CC2400_EN+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set Auto Calibration settings
*FUNCTION     :change setting for frequency auto calibration
*INPUT        :[0],1,2,3
*OUTPUT       :none
*NOTE         :FS_AUTOCAL (bit 4-5 [default  0x10]) Automatically calibrate when going to RX or TX, or back to IDLE
****************************************************************/
void CC2500::setAutoCal(byte v){
  Split_MCSM0();
  mc0FS_AUTOCAL = 0;
  if(v > 3){
    v = 3;
  }
  mc0FS_AUTOCAL = v*16;
  SpiWriteReg(CC2500_MCSM0, mc0FS_AUTOCAL+mc0PO_TIMEOUT+mc0PIN_CTRL_EN+mc0XOSC_FORCE_ON);
}
/****************************************************************
*FUNCTION NAME:Set PO Timeout
*FUNCTION     :Programs the number of times the six-bit ripple counter must expire after XOSC has stabilized before CHP_RDYn goes low.
*INPUT        :0,[1],2,3
*OUTPUT       :none
****************************************************************/
void CC2500::setPOTimeout(byte v){
  Split_MCSM0();
  mc0PO_TIMEOUT = 0;
  if(v > 3){
    v = 3;
  }
  mc0PO_TIMEOUT = v*4;
  SpiWriteReg(CC2500_MCSM0, mc0FS_AUTOCAL+mc0PO_TIMEOUT+mc0PIN_CTRL_EN+mc0XOSC_FORCE_ON);
}
/****************************************************************
*FUNCTION NAME:PIN_CTRL_EN
*FUNCTION     :Enables the pin radio control option
*INPUT        :[0],1
*OUTPUT       :none
****************************************************************/
void CC2500::setPinCtrl(bool v){
  Split_MCSM0();
  mc0PIN_CTRL_EN = 0;
  if(v == 1){
    mc0PIN_CTRL_EN = 2; 
  }
  SpiWriteReg(CC2500_MCSM0, mc0FS_AUTOCAL+mc0PO_TIMEOUT+mc0PIN_CTRL_EN+mc0XOSC_FORCE_ON);
}
/****************************************************************
*FUNCTION NAME:XOSC_FORCE_ON
*FUNCTION     :Force the XOSC to stay on in the SLEEP state.
*INPUT        :[0],1
*OUTPUT       :none
****************************************************************/
void CC2500::setXOSCForceOn(bool v){
  Split_MCSM0();
  mc0XOSC_FORCE_ON = 0;
  if(v == 1){
    mc0XOSC_FORCE_ON = 1;
  }
  SpiWriteReg(CC2500_MCSM0, mc0FS_AUTOCAL+mc0PO_TIMEOUT+mc0PIN_CTRL_EN+mc0XOSC_FORCE_ON);
}
/****************************************************************
*FUNCTION NAME:Set PACKET_LENGTH
*FUNCTION     :Indicates the packet length
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setPacketLength(byte v){
  SpiWriteReg(CC2500_PKTLEN, v);
}
/****************************************************************
*FUNCTION NAME:Set DCFILT_OFF
*FUNCTION     :Disable digital DC blocking filter before demodulator
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setDcFilterOff(bool v){
  Split_MDMCFG2();
  m2DCOFF = 0;
  if (v==1){
    m2DCOFF=128;
  }
  SpiWriteReg(CC2500_MDMCFG2, m2DCOFF+m2MODFM+m2MANCH+m2SYNCM);
}
/****************************************************************
*FUNCTION NAME:Set MANCHESTER
*FUNCTION     :Enables Manchester encoding/decoding
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setManchester(bool v){
  Split_MDMCFG2();
  m2MANCH = 0;
  if (v==1){
    m2MANCH=8;
  }
  SpiWriteReg(CC2500_MDMCFG2, m2DCOFF+m2MODFM+m2MANCH+m2SYNCM);
}
/****************************************************************
*FUNCTION NAME:Set SYNC_MODE
*FUNCTION     :Combined sync-word qualifier mode
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setSyncMode(byte v){
  Split_MDMCFG2();
  m2SYNCM = 0;
  if (v>7){
    v=7;
  }
  m2SYNCM=v;
  SpiWriteReg(CC2500_MDMCFG2, m2DCOFF+m2MODFM+m2MANCH+m2SYNCM);
}
/****************************************************************
*FUNCTION NAME:Set FEC
*FUNCTION     :Enable Forward Error Correction (FEC)
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setFEC(bool v){
  Split_MDMCFG1();
  m1FEC=0;
  if (v==1){
    m1FEC=128;
  }
  SpiWriteReg(CC2500_MDMCFG1, m1FEC+m1PRE+m1CHSP);
}
/****************************************************************
*FUNCTION NAME:Set PRE
*FUNCTION     :Sets the minimum number of preamble bytes to be transmitted.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setPRE(byte v){
  Split_MDMCFG1();
  m1PRE=0;
  if (v>7){
    v=7;
  }
  m1PRE = v*16;
  SpiWriteReg(CC2500_MDMCFG1, m1FEC+m1PRE+m1CHSP);
}
/****************************************************************
*FUNCTION NAME:Set Channel
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setChannel(byte ch){
  chan = ch;
  SpiWriteReg(CC2500_CHANNR,   chan);
}
/****************************************************************
*FUNCTION NAME:Set Channel spacing
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
*NOTE         :carrier = (XOSC / 2^16) * ((FREQ + CHAN) * ((256 + CHANSPC_M) * 2^(CHANSPC_E-2))
               MAX  = 405.456543 max channel spacing
               MIN  = 25.390625 min channel spacing
****************************************************************/
void CC2500::setChsp(float f){
  Split_MDMCFG1();
  byte MDMCFG0 = 0;
  m1CHSP = 0;
  if (f > 405.456543){
    f = 405.456543;
  }
  if (f < 25.390625){
    f = 25.390625;
  }
  for (int i = 0; i<5; i++){
    if (f <= 50.682068){
      f -= 25.390625;
      f /= 0.0991825;
      MDMCFG0 = f;
      float s1 = (f - MDMCFG0) *10;
      if (s1 >= 5){
        MDMCFG0++;
      }
      i = 5;
    }else{
      m1CHSP++;
      f/=2;
    }
  }
  SpiWriteReg(CC2500_MDMCFG1, m1CHSP+m1FEC+m1PRE);
  SpiWriteReg(CC2500_MDMCFG0, MDMCFG0);
}
/****************************************************************
*FUNCTION NAME:Set Receive bandwidth
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
*NOTE         :BWchannel = XOSC/(8 * (4 + CHANBW_M)*2^CHANBW_E
               MAX  = 13/.016 = 812.50 kHz
               MIN  = 13/.224 =  58.03 kHz
               BW Filters are limited to the following
               812.500, 650.000, 541.667, 464.286,406.250,325.000,270.833,232.143,203.125, 162.500, 135.416, 116.071, 101.563,  81.250,  67.708,  58.036
   Frequency   1625/2, 1625/2.5, 1625/3, 1625/3.5, 1625/4, 1625/5, 1625/6, 1625/7, 1625/8, 1625/10, 1625/12, 1625/14, 1625/16, 1625/20, 1625/24, 1625/28
m4RxBw Value    0x00    0x10      0x20    0x30      0x40    0x50    0x60    0x70    0x80    0x90     0xA0     0xB0     0xC0     0xD0     0xE0     0xF0
              The origial function is so elegant in it's aproximation that I almost hate to change it however, because the RX bandwidth can only be one of 16 values, 
              when you set the bandwith, you were not getting the closest bandwith to what you had requested. This function is not as elegant and is probably less efficent however, 
              it will get you a RX bandwidth closer to the one you requested. The array consists of the mid-point frequencies between the possible ones. There is also a pattern to 
              the frequencies but I couln't figure out an easy way from A to B without going into polynomials. 
****************************************************************/
void CC2500::setRxBW(float f){
  uint32_t bw = (f*1000000);
  Split_MDMCFG4();
  byte em = 0;
  uint32_t freq[16] = {731250000,595833000,502976000,435286000,365625000,297917000,251488000,217634000,182813000,148958000,125744000,108817000,91406000,74479000,62872000,0};
  for(int i=0;i<16;i++){
    if(bw>freq[i]){
      break;
    }else if(em >= 0xF0){
      em = 0xF0;
      break;
    }else{
      em+=0x10;
    }
  }
  m4RxBw = em;
  SpiWriteReg(CC2500_MDMCFG4, m4RxBw+m4DaRa);
}
/*  
void CC2500::setRxBW(float f){
  Split_MDMCFG4();
  int s1 = 3;
  int s2 = 3;
  for (int i = 0; i<3; i++){
    if (f > 101.5625){
      f/=2;
      s1--;
    }else{
      i=3;
    }
  }
  for (int i = 0; i<3; i++){
    if (f > 58.1){
      f/=1.25;
      s2--;
    }else{
      i=3;
    }
  }
  s1 *= 64;
  s2 *= 16;
  m4RxBw = s1 + s2;
  SpiWriteReg(16,m4RxBw+m4DaRa);
}
*/
/****************************************************************
*FUNCTION NAME:Set Data Rate
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
*NOTE         :Data Rate  = (((256 + DRATE_M)*2^DRATE_E)/2^28)*XOSC
                DRATE_E   = log2((DataRate * 2^20)/XOSC)
                DRATE_M   = ((DataRate * 2^28)/(XOSC*2^DRATE_E))-256
                DRATE_M   = ((DataRate * 2^28)/(XOSC*2^(log2((DataRate * 2^20)/XOSC))))-256
                MIN       = 13000/524288    ~   0.02479553
                MAX       = 6642000/4096    ~1621.58203125
          Increment       = 13000/134217728 ~   0.00009685754776000977
****************************************************************/
void CC2500::setDRate(float d){
  
  Split_MDMCFG4();
  float max = 1621.58203125;    // Maximum Data Rate with 26MHz crystal
  float min = 0.02479553;       // Minimum Data Rate with 26MHz crystal (this is also the increment value when you increase DRATE_E)
  float inc = 13000/134217728;  // Smallest increment between Data Rates with 26MHz crystal
  float c = d;
  byte MDMCFG3 = 0;
  if (c > max){
    c = max;
  }
  if (c < min){
    c = min;
  }
  m4DaRa = 0;
  for (int i = 0; i<20; i++){
    if (c <= 0.0494942){
      c = c - min;
      c = c / inc;
      MDMCFG3 = c;
      float s1 = (c - MDMCFG3) *10;
      if (s1 >= 5){
        MDMCFG3++;
      }
      i = 20;
    }else{
      m4DaRa++;
      c = c/2;
    }
  }
  SpiWriteReg(CC2500_MDMCFG4,  m4RxBw+m4DaRa);
  SpiWriteReg(CC2500_MDMCFG3,  MDMCFG3);
}
/****************************************************************
*FUNCTION NAME:Set Devitation
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
*NOTE         :Deviation    = (XOSC/(2^17))*(8 + DEVIATN_M)*2^DEVIATN_E
                     MIN    =  13000/8192   ~  1.5869140625
                     MAX    = 195000/512    ~380.859375
               INCREMENT    =  13000/65336  ~  0.1989714705522224
\****************************************************************/
void CC2500::setDeviation(float d){
  
  float m = 195000/512;      // Maximum Deviation with 26MHz crystal
  float f = 13000/8192;       // Minimum Deviation with 26MHz crystal (this is also the increment value when you increase DEVIATN_E)
  float v = 13000/65336;      // Smallest increment between Deviations with 26MHz crystal
  int c = 0;
  if (d > m){        
    d = m;
  }
  if (d < f){
    d = f;
  }
  
  for (int i = 0; i<255; i++){
    f+=v;
    if (c==7){
      v*=2;
      c=-1;
      i+=8;
    }
    if (f>=d){
      c=i;
      i=255;
    }
  c++;
  }
  SpiWriteReg(CC2500_DEVIATN,c);
}
/****************************************************************
*FUNCTION NAME:Split PKTCTRL1
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Split_PKTCTRL1(void){
  int calc = SpiReadStatus(7);
  pc1PQT = 0;
  pc1CRC_AF = 0;
  pc1APP_ST = 0;
  pc1ADRCHK = 0;
  for (bool i = 0; i==0;){
    if (calc >= 32){
      calc-=32; 
      pc1PQT+=32;
    }else if (calc >= 8){
      calc-=8; 
      pc1CRC_AF+=8;
    }else if (calc >= 4){
      calc-=4; 
      pc1APP_ST+=4;
    }else {
      pc1ADRCHK = calc; 
      i=1;
    }
  }
}
/****************************************************************
*FUNCTION NAME:Split PKTCTRL0
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Split_PKTCTRL0(void){
  int calc = SpiReadStatus(8);
  pc0WDATA = 0;
  pc0PktForm = 0;
  pc0CC2400_EN = 0;
  pc0CRC_EN = 0;
  pc0LenConf = 0;
  for (bool i = 0; i==0;){
    if (calc >= 64){        // 1 bit
      calc-=64; 
      pc0WDATA+=64;
    }else if (calc >= 16){  // 2 bits
      calc-=16;
      pc0PktForm+=16;
    }else if (calc >= 8){   // 1 bit
      calc-=8;
      pc0CC2400_EN+=8;
    }else if (calc >= 4){   // 1 bit
      calc-=4;
      pc0CRC_EN+=4;
    }else if (calc >= 1){   // 1 bits
      calc-=1;
      pc0LenConf+= 1;
    }else{
      i=1;
    }
  }
}
/****************************************************************
*FUNCTION NAME:Split MCSM0
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Split_MCSM0(void){
  int calc = SpiReadStatus(CC2500_MCSM0);
  mc0FS_AUTOCAL     = 0;  // 0x00 off  
                          // 0x10 When going from IDLE to RX or TX
                          // 0x20 When going from RX or TX back to IDLE automatically
                          // 0x30 Every 4th time when going from RX or TX to IDLE automatically
  mc0PO_TIMEOUT     = 0;  // 0x00 1 count  0x04 16 counts  0x08 64 counts  0x0C 256 counts
  mc0PIN_CTRL_EN    = 0;  // 0x00 off  0x02 on
  mc0XOSC_FORCE_ON  = 0;  // 0x00 off  0x01 on

  for (bool i = 0; i==0;){
    if (calc >= 16){
      calc-=16;
      mc0FS_AUTOCAL+=16;
    }else if (calc >= 4){
      calc-=4;
      mc0PO_TIMEOUT+=4;
    }else if (calc >= 2){
      calc-=2;
      mc0PIN_CTRL_EN+=2;
    }else {
      mc0XOSC_FORCE_ON = calc;
      i=1;
    }
  }
}
/****************************************************************
*FUNCTION NAME:Split MDMCFG1
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Split_MDMCFG1(void){
  int calc = SpiReadStatus(19);
  m1FEC = 0;
  m1PRE = 0;
  m1CHSP = 0;
  int s2 = 0;
  for (bool i = 0; i==0;){
    if (calc >= 128){
      calc-=128;
      m1FEC+=128;
    }else if (calc >= 16){
      calc-=16;
      m1PRE+=16;
    }else {
      m1CHSP = calc;
      i=1;
    }
  }
}
/****************************************************************
*FUNCTION NAME:Split MDMCFG2
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Split_MDMCFG2(void){
  int calc = SpiReadStatus(18);
  m2DCOFF = 0;
  m2MODFM = 0;
  m2MANCH = 0;
  m2SYNCM = 0;
  for (bool i = 0; i==0;){
    if (calc >= 128){
      calc-=128;
      m2DCOFF+=128;
    }else if (calc >= 16){
      calc-=16;
      m2MODFM+=16;
    }else if (calc >= 8){
      calc-=8;
      m2MANCH+=8;
    }else{
      m2SYNCM = calc;
      i=1;
    }
  }
}
/****************************************************************
*FUNCTION NAME:Split MDMCFG4
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::Split_MDMCFG4(void){
  int calc = SpiReadStatus(16);
  m4RxBw = 0;
  m4DaRa = 0;
  for (bool i = 0; i==0;){
    if (calc >= 64){
      calc-=64;
      m4RxBw+=64;
    }else if (calc >= 16){
      calc -= 16;
      m4RxBw+=16;
    }else{
      m4DaRa = calc;
      i=1;
    }
  }
}
/****************************************************************
*FUNCTION NAME:RegConfigSettings
*FUNCTION     :CC2500 register config //details refer datasheet of CC2500//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::RegConfigSettings(void){   
    SpiWriteReg(CC2500_FSCTRL1,  0x0F);
    
    //setCCMode(ccmode);
    setMHZ(MHz);
    
    SpiWriteReg(CC2500_MDMCFG1,  0x03); //  defaulting to a channel spcing of 333 kHz so every 3 channels is ~1MHz, no FEC or preamble
    SpiWriteReg(CC2500_MDMCFG0,  0xA4); //  setting for 333. default is 248 (0xF8). The default values give 199.951 kHz channel spacing (the closest setting to 200 kHz), assuming 26.0 MHz crystal frequency.
    SpiWriteReg(CC2500_CHANNR,   chan); //  chan is set to 0. 
    SpiWriteReg(CC2500_DEVIATN,  0x63); //  139.64 kHz deviation
    SpiWriteReg(CC2500_FREND1,   0x56); //  Front End RX Configuration
    SpiWriteReg(CC2500_MCSM0 ,   0x14); //  AUTO calibrate. po timeout 16 counts
    SpiWriteReg(CC2500_FOCCFG,   0x36); //  Frequency Offset Compensation Configuration
    SpiWriteReg(CC2500_BSCFG,    0x6C); //  Bit Synchronization Configuration
    SpiWriteReg(CC2500_AGCCTRL2, 0x03); //  AGC Control
    SpiWriteReg(CC2500_AGCCTRL1, 0x40); //  AGC Control
    SpiWriteReg(CC2500_AGCCTRL0, 0x91); //  AGC Control
    SpiWriteReg(CC2500_FSCAL3,   0xA9); //  Frequency Synthesizer Calibration
    SpiWriteReg(CC2500_FSCAL2,   0x0A); //  Frequency Synthesizer Calibration
    SpiWriteReg(CC2500_FSCAL1,   0x20); //  Frequency Synthesizer Calibration
    SpiWriteReg(CC2500_FSCAL0,   0x0d); //  Frequency Synthesizer Calibration
    //SpiWriteReg(CC2500_FSTEST,   0x59);
    SpiWriteReg(CC2500_TEST2,    0x88); //  Set to 0x81 for improved sensitivity at data rates <= 100 kbaud
    SpiWriteReg(CC2500_TEST1,    0x31); //  Set to 0x35 for improved sensitivity at data rates ≤100 kBaud.
    SpiWriteReg(CC2500_TEST0,    0x0B);
    SpiWriteReg(CC2500_PKTCTRL1, 0x04);
    SpiWriteReg(CC2500_ADDR,     0x00);
    SpiWriteReg(CC2500_PKTLEN,   0xFF);
}
/****************************************************************
*FUNCTION NAME:SetTx
*FUNCTION     :set CC2500 send data
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::SetTx(void){
  SpiStrobe(CC2500_SIDLE);
  SpiStrobe(CC2500_STX);        //start send
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:SetRx
*FUNCTION     :set CC2500 to receive state
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::SetRx(void){
  SpiStrobe(CC2500_SIDLE);
  SpiStrobe(CC2500_SRX);        //start receive
  trxstate=2;
}
/****************************************************************
*FUNCTION NAME:SetTx
*FUNCTION     :set CC2500 send data and change frequency
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::SetTx(float mhz){
  SpiStrobe(CC2500_SIDLE);
  setMHZ(mhz);
  SpiStrobe(CC2500_STX);        //start send
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:SetRx
*FUNCTION     :set CC2500 to receive state and change frequency
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::SetRx(float mhz){
  SpiStrobe(CC2500_SIDLE);
  setMHZ(mhz);
  SpiStrobe(CC2500_SRX);        //start receive
  trxstate=2;
}
/****************************************************************
*FUNCTION NAME:RSSI Level
*FUNCTION     :Calculating the RSSI Level
*INPUT        :none
*OUTPUT       :none
****************************************************************/
int CC2500::getRssi(void){
  int rssi;
  rssi=SpiReadStatus(CC2500_RSSI);
  if (rssi >= 128){
    rssi = (rssi-256)/2-74;
  }else{
    rssi = (rssi/2)-74;
  }
  return rssi;
}
/****************************************************************
*FUNCTION NAME:LQI Level
*FUNCTION     :get Lqi state
*INPUT        :none
*OUTPUT       :none
****************************************************************/
byte CC2500::getLqi(void){
byte lqi;
lqi=SpiReadStatus(CC2500_LQI);
return lqi;
}
/****************************************************************
*FUNCTION NAME:SetSres
*FUNCTION     :Reset CC2500
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setSres(void){
  SpiStrobe(CC2500_SRES);
  trxstate=0;
}
/****************************************************************
*FUNCTION NAME:setSidle
*FUNCTION     :set Rx / TX Off
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::setSidle(void){
  SpiStrobe(CC2500_SIDLE);
  trxstate=0;
}
/****************************************************************
*FUNCTION NAME:goSleep
*FUNCTION     :set CC2500 Sleep on
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC2500::goSleep(void){
  trxstate=0;
  SpiStrobe(0x36);//Exit RX / TX, turn off frequency synthesizer and exit
  SpiStrobe(0x39);//Enter power down mode when CSn goes high.
}
/****************************************************************
*FUNCTION NAME:Char direct SendData
*FUNCTION     :use CC2500 send data
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void CC2500::SendData(char *txchar){
  int len = strlen(txchar);
  byte chartobyte[len];
  for (int i = 0; i<len; i++){
    chartobyte[i] = txchar[i];
  }
  SendData(chartobyte,len);
}
/****************************************************************
*FUNCTION NAME:SendData
*FUNCTION     :use CC2500 send data
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void CC2500::SendData(byte *txBuffer,byte size){
  SpiWriteReg(CC2500_TXFIFO,size);
  SpiWriteBurstReg(CC2500_TXFIFO,txBuffer,size);   //write data to send
  SpiStrobe(CC2500_SIDLE);
  SpiStrobe(CC2500_STX);                           //start send
  while (!digitalRead(GDO0));                      // Wait for GDO0 to be set -> sync transmitted  
  while (digitalRead(GDO0));                       // Wait for GDO0 to be cleared -> end of packet
  SpiStrobe(CC2500_SFTX);                          //flush TXfifo
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:Char direct SendData
*FUNCTION     :use CC2500 send data without GDO
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void CC2500::SendData(char *txchar,int t){
  int len = strlen(txchar);
  byte chartobyte[len];
  for (int i = 0; i<len; i++){
    chartobyte[i] = txchar[i];
  }
  SendData(chartobyte,len,t);
}
/****************************************************************
*FUNCTION NAME:SendData
*FUNCTION     :use CC2500 send data without GDO
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void CC2500::SendData(byte *txBuffer,byte size,int t){
  SpiWriteReg(CC2500_TXFIFO,size);
  SpiWriteBurstReg(CC2500_TXFIFO,txBuffer,size);  //write data to send
  SpiStrobe(CC2500_SIDLE);
  SpiStrobe(CC2500_STX);                          //start send
  delay(t);
  SpiStrobe(CC2500_SFTX);                         //flush TXfifo
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:Check CRC
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
bool CC2500::CheckCRC(void){
  byte lqi=SpiReadStatus(CC2500_LQI);
  bool crc_ok = bitRead(lqi,7);
  if (crc_ok == 1){
    return 1;
  }else{
    SpiStrobe(CC2500_SFRX);
    SpiStrobe(CC2500_SRX);
    return 0;
  }
}
/****************************************************************
*FUNCTION NAME:CheckRxFifo
*FUNCTION     :check receive data or not
*INPUT        :none
*OUTPUT       :flag: 0 no data; 1 receive data 
****************************************************************/
bool CC2500::CheckRxFifo(int t){
  if(trxstate!=2){SetRx();}
    if(SpiReadStatus(CC2500_RXBYTES) & BYTES_IN_RXFIFO){
      delay(t);
      return 1;
    }else{
      return 0;
  }
}
/****************************************************************
*FUNCTION NAME:CheckReceiveFlag
*FUNCTION     :check receive data or not
*INPUT        :none
*OUTPUT       :flag: 0 no data; 1 receive data 
****************************************************************/
byte CC2500::CheckReceiveFlag(void){
  if(trxstate!=2){
    SetRx();
  }
	if(digitalRead(GDO0)){			//receive data
		while (digitalRead(GDO0));
		return 1;
	}else{							// no data
		return 0;
	}
}
/****************************************************************
*FUNCTION NAME:ReceiveData
*FUNCTION     :read data received from RXfifo
*INPUT        :rxBuffer: buffer to store data
*OUTPUT       :size of data received
****************************************************************/
byte CC2500::ReceiveData(byte *rxBuffer){
	byte size;
	byte status[2];

	if(SpiReadStatus(CC2500_RXBYTES) & BYTES_IN_RXFIFO)	{
		size=SpiReadReg(CC2500_RXFIFO);
		SpiReadBurstReg(CC2500_RXFIFO,rxBuffer,size);
		SpiReadBurstReg(CC2500_RXFIFO,status,2);
		SpiStrobe(CC2500_SFRX);
    SpiStrobe(CC2500_SRX);
		return size;
	}else{
		SpiStrobe(CC2500_SFRX);
    SpiStrobe(CC2500_SRX);
 		return 0;
	}
}
CC2500 CC2500;
