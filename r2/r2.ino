
//Use this lib https://github.com/DzikuVx/PPMReader
#include "PPMReader.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//Setup ppm read on pin 2, 

#define ADC_MIN 0
#define ADC_MID 511
#define ADC_MAX 1023

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

uint32_t lastDisplayUpdate = 0;  // Store the last display update time
const uint32_t displayInterval = 500;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//PPMReader ppmReader(2, 0, false);

int Is_Aileron_Reverse  =1;
int Is_Elevator_Reverse =0;
int Is_Throttle_Reverse =0;
int Is_Rudder_Reverse   =0;

int i;

const int analogInPinAileron = A1;
const int analogInPinElevator = A0;
const int analogInPinThrottle = A3;
const int analogInPinRudder = A2;

byte channelAmount = 8;

int Aileron_value = 0; // values read from the pot
int Elevator_value = 0;
int Throttle_value = 0;
int Rudder_value = 0;
int previous_throttle = 191;

int AUX1_Arm = 0; // switch values read from the digital pin
int AUX2_value = 0;
int AUX3_value = 0;
int AUX4_value = 0;

long flag_conn = 0;

int Arming_Flag = 172;
int Pot_Val = 0;
//default channel order is  AETR
//#define TAER

#define RADIO_ADDRESS                  0xEA
#define ADDR_MODULE                    0xEE  //  Crossfire transmitter
#define TYPE_CHANNELS                  0x16

// internal crsf variables

#define CRSF_CHANNEL_MIN 172
#define CRSF_CHANNEL_MID 991
#define CRSF_CHANNEL_MAX 1811
#define CRSF_TIME_NEEDED_PER_FRAME_US   1100 // 700 ms + 400 ms for potential ad-hoc request
#define CRSF_TIME_BETWEEN_FRAMES_US     4000 // 4 ms 250Hz
#define CRSF_PAYLOAD_OFFSET offsetof(crsfFrameDef_t, type)
#define CRSF_MAX_CHANNEL 16
#define CRSF_FRAME_SIZE_MAX 64
#define SERIAL_BAUDRATE 460800
#define CRSF_MSP_RX_BUF_SIZE 128
#define CRSF_MSP_TX_BUF_SIZE 128
#define CRSF_PAYLOAD_SIZE_MAX   60
#define CRSF_PACKET_LENGTH 22
#define CRSF_PACKET_SIZE  26
#define CRSF_FRAME_LENGTH 24;   // length of type + payload + crc

uint8_t crsfPacket[CRSF_PACKET_SIZE];
int rcChannels[CRSF_MAX_CHANNEL];
uint32_t crsfTime = 0;

#ifdef TAER
enum chan_order{
    THROTTLE, 
    AILERON,
    ELEVATOR,
    RUDDER,
    AUX1,  // (CH5)  ARM switch for Expresslrs
    AUX2,  // (CH6)  angel / airmode change
    AUX3,  // (CH7)  flip after crash
    AUX4,  // (CH8) 
    AUX5,  // (CH9) 
    AUX6,  // (CH10) 
    AUX7,  // (CH11)
    AUX8,  // (CH12)
};
#else
enum chan_order{
    AILERON,
    ELEVATOR,
    THROTTLE, 
    RUDDER,
    AUX1,  // (CH5)  ARM switch for Expresslrs
    AUX2,  // (CH6)  angel / airmode change
    AUX3,  // (CH7)  flip after crash
    AUX4,  // (CH8) 
    AUX5,  // (CH9) 
    AUX6,  // (CH10) 
    AUX7,  // (CH11)
    AUX8,  // (CH12)
};
#endif

void setup()
{
  // analogReference(DEFAULT);
  pinMode(A6, INPUT);   //Scroll for Camera
  pinMode(4, INPUT_PULLUP);   //Switch for Arming
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  /*
  TCCR1A = 0;  //reset timer1
  TCCR1B = 0;
  TCCR1B |= (1 << CS11);  //set timer1 to increment every 0,5 us or 1us on 8MHz
  */
  for (uint8_t i = 0; i < CRSF_MAX_CHANNEL; i++) {
        rcChannels[i] = CRSF_CHANNEL_MID;
    }
    rcChannels[THROTTLE] = CRSF_CHANNEL_MIN; // Throttle
 
    delay(1000);
    Serial.begin(SERIAL_BAUDRATE);
  
    //Serial.begin(115200);
    //Serial.println("ready");

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

  drawDot(20, 10);
  drawDot(40, 10);
  drawDot(60, 10);
  drawDot(80, 10);
  drawDot(100, 10);

  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 1); 
  display.println(F("IDR R&D"));
  display.setTextSize(1); 
  display.setCursor(20, 18);
  display.println(F("For the Nation"));
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop()
{
  makenmodel();
  uint32_t currentMicros = micros();

  //main ARMING Channel [AUX1]
  AUX1_Arm = digitalRead(4);
  if(AUX1_Arm == HIGH)
  {
    Arming_Flag = CRSF_CHANNEL_MIN;
  }
  else if(AUX1_Arm == LOW)
  {
    Arming_Flag = CRSF_CHANNEL_MAX;
  }

  //Channel 6 (Mode Selection)
  AUX2_value=digitalRead(2);
  AUX3_value =digitalRead(3);
  if(AUX2_value == HIGH && AUX3_value == HIGH)
  {
    rcChannels[AUX2] = CRSF_CHANNEL_MID;
  }
  else if(AUX2_value == LOW && AUX3_value == HIGH)
  {
    rcChannels[AUX2] = CRSF_CHANNEL_MIN;
  }
  else if(AUX2_value == HIGH && AUX3_value == LOW)
  {
    rcChannels[AUX2] = CRSF_CHANNEL_MAX;
  }

  //Read Joysticks
  int analogVal=0;
  for (i=0; i<10; i++){
    analogVal=analogRead(analogInPinAileron)+analogVal;
  }
  analogVal=analogVal/10;
  if(analogVal <= 347)
  {
    Aileron_value = map(analogVal,205, 347, ADC_MIN, ADC_MID);
  }
  else
  {
    Aileron_value = map(analogVal,347, 490, ADC_MID+1, ADC_MAX);
  }
  analogVal=0;
  for(i=0;i<10;i++){
    analogVal=analogRead(analogInPinElevator)+analogVal;
  }
  analogVal=analogVal/10;
  
  if(analogVal <= 296)
  {
    Elevator_value = map(analogVal,162, 296, ADC_MIN, ADC_MID);
  }
  else
  {
    Elevator_value = map(analogVal,296, 424, ADC_MID+1, ADC_MAX);
  }

  analogVal=analogRead(analogInPinThrottle);
  Throttle_value = map(analogVal, 890, 156, ADC_MIN, ADC_MAX);

  analogVal=0;
  for(i=0;i<10;i++){
    analogVal=analogRead(analogInPinRudder)+analogVal;
  }
  analogVal=analogVal/10;
  
  if(analogVal <= 501)
  {
    Rudder_value = map(analogVal,142, 501, ADC_MIN, ADC_MID);
  }
  else
  {
    Rudder_value = map(analogVal,501, 860, ADC_MID+1, ADC_MAX);
  }
  
  //constrain
  Aileron_value  = constrain(Aileron_value,  ADC_MIN, ADC_MAX); 
  Elevator_value = constrain(Elevator_value, ADC_MIN, ADC_MAX); 
  Throttle_value = constrain(Throttle_value, ADC_MIN, ADC_MAX); 
  Rudder_value   = constrain(Rudder_value,   ADC_MIN, ADC_MAX); 

  //reverse channels if required
  if (Is_Aileron_Reverse == 1){
      Aileron_value  = 1023-Aileron_value;
  }
  if (Is_Elevator_Reverse == 1){
      Elevator_value = 1023-Elevator_value;
  }
  if (Is_Throttle_Reverse == 1){
      Throttle_value = 1023-Throttle_value;
  }
  if (Is_Rudder_Reverse == 1){
      Rudder_value   = 1023-Rudder_value;
  }

  //packatize main channels
  rcChannels[AILERON]   = map(Aileron_value,  ADC_MIN, ADC_MAX, CRSF_CHANNEL_MIN, CRSF_CHANNEL_MAX); 
  rcChannels[ELEVATOR]  = map(Elevator_value, ADC_MIN, ADC_MAX, CRSF_CHANNEL_MIN, CRSF_CHANNEL_MAX);
  rcChannels[THROTTLE]  = map(Throttle_value+121, ADC_MIN, ADC_MAX, CRSF_CHANNEL_MIN, CRSF_CHANNEL_MAX);
  rcChannels[RUDDER]    = map(Rudder_value-155,   ADC_MIN, ADC_MAX, CRSF_CHANNEL_MIN, CRSF_CHANNEL_MAX);

  rcChannels[AUX1] = Arming_Flag; // ARM Low
  rcChannels[AUX2] = CRSF_CHANNEL_MIN;
  /*
    for(int i= 0; i<10; i++){
      AUX2_value = analogRead(A6)+AUX2_value;
    }
    AUX2_value=AUX2_value/10;
    Pot_Val=map(AUX2_value, 0, 100, CRSF_CHANNEL_MIN, CRSF_CHANNEL_MAX);
    rcChannels[AUX2] = Pot_Val;
  */                                                                                                                                                           
  
    if (currentMicros > crsfTime) {
        crsfPreparePacket(crsfPacket, rcChannels);
        Serial.write(crsfPacket, CRSF_PACKET_SIZE);            
        crsfTime = currentMicros + CRSF_TIME_BETWEEN_FRAMES_US;
    }

}

// crc implementation from CRSF protocol document rev7
static uint8_t crsf_crc8tab[256] = {
    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9};

uint8_t crsf_crc8(const uint8_t *ptr, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i=0; i < len; i++) {
        crc = crsf_crc8tab[crc ^ *ptr++];
    }
    return crc;
}

void crsfPreparePacket(uint8_t packet[], int channels[]){

    static int output[CRSF_MAX_CHANNEL] = {0};
    const uint8_t crc = crsf_crc8(&packet[2], CRSF_PACKET_SIZE-3);
    /*
     * Map 1000-2000 with middle at 1500 chanel values to
     * 173-1811 with middle at 992 S.BUS protocol requires
     */
    for (uint8_t i = 0; i < CRSF_MAX_CHANNEL; i++) {
        output[i] = channels[i];
    }    
   /*    
       Serial.print("crsf  "); 
       Serial.print(output[0]); 
       Serial.print("  i"); 
       Serial.print(output[1]); 
       Serial.print("  i"); 
       Serial.print(output[2]); 
       Serial.print("  i"); 
       Serial.print(output[3]); 
       Serial.println(); */

    // packet[0] = UART_SYNC; //Header
    packet[0] = ADDR_MODULE; //Header
    packet[1] = 24;   // length of type (24) + payload + crc
    packet[2] = TYPE_CHANNELS;
    packet[3] = (uint8_t) (channels[0] & 0x07FF);
    packet[4] = (uint8_t) ((channels[0] & 0x07FF)>>8 | (channels[1] & 0x07FF)<<3);
    packet[5] = (uint8_t) ((channels[1] & 0x07FF)>>5 | (channels[2] & 0x07FF)<<6);
    packet[6] = (uint8_t) ((channels[2] & 0x07FF)>>2);
    packet[7] = (uint8_t) ((channels[2] & 0x07FF)>>10 | (channels[3] & 0x07FF)<<1);
    packet[8] = (uint8_t) ((channels[3] & 0x07FF)>>7 | (channels[4] & 0x07FF)<<4);
    packet[9] = (uint8_t) ((channels[4] & 0x07FF)>>4 | (channels[5] & 0x07FF)<<7);
    packet[10] = (uint8_t) ((channels[5] & 0x07FF)>>1);
    packet[11] = (uint8_t) ((channels[5] & 0x07FF)>>9 | (channels[6] & 0x07FF)<<2);
    packet[12] = (uint8_t) ((channels[6] & 0x07FF)>>6 | (channels[7] & 0x07FF)<<5);
    packet[13] = (uint8_t) ((channels[7] & 0x07FF)>>3);
    packet[14] = (uint8_t) ((channels[8] & 0x07FF));
    packet[15] = (uint8_t) ((channels[8] & 0x07FF)>>8 | (channels[9] & 0x07FF)<<3);
    packet[16] = (uint8_t) ((channels[9] & 0x07FF)>>5 | (channels[10] & 0x07FF)<<6);  
    packet[17] = (uint8_t) ((channels[10] & 0x07FF)>>2);
    packet[18] = (uint8_t) ((channels[10] & 0x07FF)>>10 | (channels[11] & 0x07FF)<<1);
    packet[19] = (uint8_t) ((channels[11] & 0x07FF)>>7 | (channels[12] & 0x07FF)<<4);
    packet[20] = (uint8_t) ((channels[12] & 0x07FF)>>4  | (channels[13] & 0x07FF)<<7);
    packet[21] = (uint8_t) ((channels[13] & 0x07FF)>>1);
    packet[22] = (uint8_t) ((channels[13] & 0x07FF)>>9  | (channels[14] & 0x07FF)<<2);
    packet[23] = (uint8_t) ((channels[14] & 0x07FF)>>6  | (channels[15] & 0x07FF)<<5);
    packet[24] = (uint8_t) ((channels[15] & 0x07FF)>>3);
    
    packet[25] = crsf_crc8(&packet[2], CRSF_PACKET_SIZE-3); //CRC


}

int drawDot(int x, int y)
{
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.println(F("-"));
  display.display();
  delay(500);
}


void makenmodel()
{
   uint32_t currentMillis = millis();
    if (currentMillis - lastDisplayUpdate >= displayInterval) {
        lastDisplayUpdate = currentMillis; 
  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 1); 
  display.println(F("FPV DRONE"));
  display.setCursor(5, 14); 
  display.println(F(". . . ."));
  display.setCursor(5, 12); 
  display.println(F(". . ."));
  display.setCursor(5, 10); 
  display.println(F(". ."));
  display.setTextSize(0.5);
  display.setCursor(1, 25); 
  display.println(F("IDR/RC/1_2025"));

  display.display();
    }
}
