/* Tranmsitter code for the Arduino Radio control with PWM or PPM output
 * Install the NRF24 library to your IDE
 * Upload this code to the Arduino UNO, NANO, Pro mini (5V,16MHz)
 * Connect a NRF24 module to it:
 
    NRF Module // Arduino UNO,NANO
    
    GND    ->   GND
    Vcc    ->   3.3V
    CE     ->   D9
    CSN    ->   D10
    CLK    ->   D13
    MOSI   ->   D11
    MISO   ->   D12

This code transmits 6 channels with data from pins A0, A1, A2, A3, D3 and D4
*/



#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// NRF24L01 communication address (must match with receiver)
const uint64_t pipeOut = 0xE8E8F0F0E1LL;

// NRF24L01 radio module initialization (CE, CSN pins)
RF24 radio(9, 10);

// Structure to hold signal data for transmission
struct Signal {
  byte ch1;
  byte ch2;
  byte ch3;
  byte ch4;
  byte ch5; // Toggle switch 1
  byte ch6; // Toggle switch 2
};

// Instance of Signal struct to store data
Signal data;

// Function to reset data values to default
void ResetData() {
  data.ch1 = 127;
  data.ch2 = 127;
  data.ch3 = 0;
  data.ch4 = 127;
  data.ch5 = 0; // Toggle switch 1 default Mode
  data.ch6 = 0; // Toggle switch 2 default Mode
}

void setup() {
  // Initialize NRF24L01 radio module
  radio.begin();
  radio.openWritingPipe(pipeOut); // Set the transmission pipe
  radio.setChannel(110); // Set communication channel
  radio.setAutoAck(true); // Disable auto acknowledgment
  radio.setDataRate(RF24_1MBPS); // Set lowest data rate for stability
  radio.setPALevel(RF24_PA_MAX); // Set maximum transmission power
  radio.stopListening(); // Configure module as transmitter
  ResetData(); // Initialize signal data
}

// Function to map joystick values to appropriate PWM range
int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper); // Ensure value stays within limits
  if (val < middle)
    val = map(val, lower, middle, 0, 128); // Map lower half of range
  else
    val = map(val, middle, upper, 128, 255); // Map upper half of range
  return (reverse ? 255 - val : val); // Reverse signal if needed
}

void loop() {
  // Read joystick and switch inputs, map values to signal range
  data.ch1 = Border_Map(analogRead(A1), 0, 512, 1023, true); // CH1 (Reversed)
  data.ch2 = Border_Map(analogRead(A0), 0, 512, 1023, true); // CH2 (Reversed)
  data.ch3 = Border_Map(analogRead(A2), 560, 850, 1023, false); // CH3 (Single Side ESC)
  // data.ch3 = Border_Map(analogRead(A2), 0, 512, 1023, true); // CH3 (Bidirectional ESC)
  data.ch4 = Border_Map(analogRead(A3), 0, 512, 1023, true); // CH4 (Reversed)
  data.ch5 = digitalRead(4); // CH5 (Toggle Switch 1)
  data.ch6 = digitalRead(3); // CH6 (Toggle Switch 2)

  // Transmit signal data to receiver
  radio.write(&data, sizeof(Signal));
}
