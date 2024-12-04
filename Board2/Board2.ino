// This board measures turbidity using ADC and sends data periodically using a timer interrupt.
// ADMUX sets the ADC reference voltage and the input channel for the turbidity sensor.
// ADCSRA configures and enables the ADC for reading sensor values.
// TCCR1B sets the timer in CTC mode with a prescaler for 1-second intervals.
// OCR1A sets the timer's compare value for precise timing.
// TIMSK1 enables the timer compare interrupt, calling an ISR when the timer reaches OCR1A.
// Inside the ISR, the ADC reads the sensor data, and the result is sent via serial communication.
// Serial communication uses UCSR0B to enable the transmitter and UDR0 to send data bytes.
// This board avoids delays in the loop by using interrupts for periodic measurements.
// Global interrupts are enabled with sei(), ensuring the ISR can execute.
// Board 2: Turbidity Sensor
#define TURBIDITY_SENSOR_CHANNEL 1
#define BOARD_ID 2 // Unique ID for the Turbidity Sensor board


void setupADC() {
  ADMUX = (1 << REFS0); // AVcc reference
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t readADC(uint8_t channel) {
  ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

void setupSerial() {
  UBRR0H = 0;
  UBRR0L = 103;
  UCSR0B = (1 << TXEN0);
}


void sendSerialWithID(uint8_t boardID, uint16_t data) {
  // Send Start Byte
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = 0x7E; // Start Byte (e.g., '~')

  // Send Board ID
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = boardID;

  // Send Data High Byte
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = (data >> 8);

  // Send Data Low Byte
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = (data & 0xFF);

  // Send End Byte
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = 0x7F; // End Byte (e.g., DEL)
}


void setupTimer() {
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode, 1024 prescaler
  OCR1A = 15625; // 1-second interval
  TIMSK1 = (1 << OCIE1A); // Enable compare interrupt
}

ISR(TIMER1_COMPA_vect) {
  uint16_t turbidityValue = readADC(TURBIDITY_SENSOR_CHANNEL);
   sendSerialWithID(BOARD_ID, turbidityValue);
}

void setup() {
  setupADC();
  setupSerial();
  setupTimer();
  sei(); // Enable global interrupts
}

void loop() {
  // Main loop intentionally empty
}
