/* Receiver code for the Arduino Radio control with PWM output
 * Install the NRF24 library to your IDE
 * Upload this code to the Arduino UNO, NANO, Pro mini (5V,16MHz)
 * Connect a NRF24 module to it:
 
    NRF Module --> Arduino UNO,NANO
    
    GND    ->   GND
    Vcc    ->   3.3V
    CE     ->   D9
    CSN    ->   D10
    CLK    ->   D13
    MOSI   ->   D11
    MISO   ->   D12

  This code receive 6 channels and create a PWM output for each one on D3, D4, D5, D6, D7, D8
*/


#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// NRF24L01 communication address
const uint64_t pipeIn = 0xE8E8F0F0E1LL;

// Variables to store PWM pulse widths for each channel
int ch_width_1 = 0;
int ch_width_2 = 0;
int ch_width_3 = 0;
int ch_width_4 = 0;
int ch_width_5 = 0;
int ch_width_6 = 0;

// Servo objects for each channel
Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;
Servo ch6;

// Structure to hold received signal data
struct Signal {
  byte ch1;
  byte ch2;
  byte ch3;
  byte ch4;
  byte ch5;
  byte ch6;
};

// Instance of Signal struct to store received data
Signal data;

// NRF24L01 radio module initialization
RF24 radio(9, 10);

// Function to reset received data to default values
void ResetData() {
  data.ch1 = 127;
  data.ch2 = 127;
  data.ch3 = 0;  //tottle
  data.ch4 = 127;
  data.ch5 = 0;
  data.ch6 = 0;
}

void setup() {
  // Attach each channel to a specific arduino pin
  ch1.attach(3);
  ch2.attach(4);
  ch3.attach(5);
  ch4.attach(6);
  ch5.attach(7);
  ch6.attach(8);

  // Initialize data values
  ResetData();

  // Initialize NRF24L01 radio module
  radio.begin();
  radio.openReadingPipe(1, pipeIn);
  radio.setChannel(110);            // Set communication channel
  radio.setAutoAck(true);          // Disable auto acknowledgment
  radio.setDataRate(RF24_1MBPS);  // Set lowest data rate for stability
  radio.setPALevel(RF24_PA_MAX);    // Set maximum transmission power
  radio.startListening();           // Start listening for incoming data

  // Set pin 2 as an output for additional control
  pinMode(2, OUTPUT);
}

// Variable to track last received signal time
unsigned long lastRecvTime = 0;

// Function to receive data from the transmitter
void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(Signal));  // Read received data into struct
    lastRecvTime = millis();            // Update last received time
  }
}

void loop() {
  recvData();  // Call function to receive data

  // Check if data has not been received for more than 1 second
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) {
    ResetData();  // Reset data if signal is lost
  }

  // Map received values to PWM pulse width range
  ch_width_1 = map(data.ch1, 0, 255, 700, 2400);
  ch_width_2 = map(data.ch2, 0, 255, 700, 2400);
  ch_width_3 = map(data.ch3, 0, 255, 1000, 2000);
  ch_width_4 = map(data.ch4, 0, 255, 700, 2400);
  ch_width_5 = map(data.ch5, 0, 1, 1000, 2000);
  ch_width_6 = map(data.ch6, 0, 1, 1000, 2000);

  // Send PWM signals to servo motors
  ch1.writeMicroseconds(ch_width_1);
  ch2.writeMicroseconds(ch_width_2);
  ch3.writeMicroseconds(ch_width_3);
  ch4.writeMicroseconds(ch_width_4);
  ch5.writeMicroseconds(ch_width_5);
  ch6.writeMicroseconds(ch_width_6);

  // Control digital output on pin 2 based on ch5 value
  (ch_width_5 >= 1500) ? digitalWrite(2, HIGH) : digitalWrite(2, LOW);
}
