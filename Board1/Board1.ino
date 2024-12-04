// This board reads pH values using the ADC module.
// The ADMUX register configures the voltage reference and selects the ADC channel.
// ADCSRA enables the ADC and sets the conversion speed using a prescaler.
// readADC() starts the conversion and waits for it to complete.
// The result is stored in the ADC data register (ADC).
// Serial communication is configured using UBRR0 for the baud rate.
// UCSR0B enables the transmitter, and UDR0 holds the data to be sent.
// The loop reads the pH sensor's analog value, formats it, and sends it serially.
// This approach uses direct register manipulation for speed and control.
// _delay_ms() ensures the sensor is read every second.
// Board 1: pH Sensor
#define PH_SENSOR_CHANNEL 0
#define BOARD_ID 1 // Unique ID for the pH Sensor board

void setupADC() {
  // Set reference voltage to AVcc and select ADC0
  ADMUX = (1 << REFS0);
  // Enable ADC and set prescaler to 128
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t readADC(uint8_t channel) {
  // Select ADC channel
  ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
  // Start conversion
  ADCSRA |= (1 << ADSC);
  // Wait for conversion to complete
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

void setupSerial() {
  // Set baud rate to 9600
  UBRR0H = 0;
  UBRR0L = 103;
  // Enable transmitter
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


void setup() {
  setupADC();
  setupSerial();
}

void loop() {
  uint16_t phValue = readADC(PH_SENSOR_CHANNEL);
  sendSerialWithID(BOARD_ID, phValue);
  _delay_ms(1000);
}
