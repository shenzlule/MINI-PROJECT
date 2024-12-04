// This board reads temperature and conductivity data using ADC and sends them via serial communication.
// ADMUX selects the ADC channel for temperature and conductivity sensors and sets the reference voltage.
// ADCSRA enables the ADC and configures the conversion speed with a prescaler.
// TCCR1B configures Timer1 in CTC mode with a 1-second interval.
// OCR1A sets the compare value to trigger the interrupt every second.
// TIMSK1 enables the compare match interrupt to handle periodic tasks without using delays.
// Inside the ISR, the ADC reads both sensors, and their results are sent serially.
// UCSR0B enables the UART transmitter, and UDR0 holds data to be sent byte by byte.
// The loop remains empty, relying on the ISR for regular operation.
// sei() ensures the timer interrupt system is activated and ready to use.
// Board 3: Temperature and Conductivity Sensor
#define TEMP_SENSOR_CHANNEL 2
#define CONDUCTIVITY_SENSOR_CHANNEL 3
#define BOARD_ID 3 // Unique ID for the Temperature and Conductivity board


void setupADC() {
  ADMUX = (1 << REFS0);
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


void sendSerial(uint8_t boardID, uint16_t data1, uint16_t data2) {
  // Wait until the USART Data Register is empty (ready to transmit)
  while (!(UCSR0A & (1 << UDRE0)));

  // Send the Board ID as the first byte
  UDR0 = boardID;

  // Wait until the USART Data Register is empty again
  while (!(UCSR0A & (1 << UDRE0)));

  // Send the high byte of the first data value (data1)
  UDR0 = (data1 >> 8);

  // Wait until the USART Data Register is empty
  while (!(UCSR0A & (1 << UDRE0)));

  // Send the low byte of the first data value (data1)
  UDR0 = (data1 & 0xFF);

  // Wait until the USART Data Register is empty
  while (!(UCSR0A & (1 << UDRE0)));

  // Send the high byte of the second data value (data2)
  UDR0 = (data2 >> 8);

  // Wait again for the USART Data Register to be empty
  while (!(UCSR0A & (1 << UDRE0)));

  // Send the low byte of the second data value (data2)
  UDR0 = (data2 & 0xFF);
}



void setupTimer() {
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  OCR1A = 15625;
  TIMSK1 = (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
  uint16_t tempValue = readADC(TEMP_SENSOR_CHANNEL);
  uint16_t conductivityValue = readADC(CONDUCTIVITY_SENSOR_CHANNEL);
  sendSerial(BOARD_ID,tempValue, conductivityValue);
}

void setup() {
  setupADC();
  setupSerial();
  setupTimer();
  sei();
}

void loop() {
  // Main loop intentionally empty
}
