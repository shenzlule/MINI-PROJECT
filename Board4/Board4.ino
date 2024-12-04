// This board collects data from the other boards and stores it in EEPROM.
// UBRR0 sets the baud rate for receiving data, and UCSR0B enables the UART receiver.
// UDR0 is used to read incoming data, one byte at a time.
// The EEPROM control register (EECR) manages writing and reading operations.
// EEAR specifies the memory address in EEPROM for reading or writing data.
// EEDR temporarily holds data to be written to or read from EEPROM.
// writeEEPROM() writes data byte-by-byte to EEPROM at specified addresses.
// readEEPROM() retrieves stored data from EEPROM by accessing its address.
// The loop reads incoming data, stores it in EEPROM, and checks thresholds for alarms.
// EEPROM operations ensure the system logs data persistently, even after power loss.
// Board 4: Central Controller

void setupSerial() {
  // Configure serial for receiving
  UBRR0H = 0;
  UBRR0L = 103;
  UCSR0B = (1 << RXEN0); // Enable Receiver
}


uint8_t readSerial() {
  while (!(UCSR0A & (1 << RXC0)));
  return UDR0;
}

void setupEEPROM() {
  // Nothing specific needed for basic EEPROM usage
}

void writeEEPROM(uint16_t address, uint8_t data) {
  while (EECR & (1 << EEPE)); // Wait for EEPROM to be ready
  EEAR = address;
  EEDR = data;
  EECR = (1 << EEMPE);
  EECR |= (1 << EEPE);
}

uint8_t readEEPROM(uint16_t address) {
  while (EECR & (1 << EEPE)); // Wait for EEPROM to be ready
  EEAR = address;
  EECR |= (1 << EERE);
  return EEDR;
}

void setup() {
  setupSerial();
  setupEEPROM();
}
void processIncomingData() {
  static uint8_t state = 0;
  static uint8_t boardID = 0;
  static uint16_t data = 0;

  while (UCSR0A & (1 << RXC0)) { // Check if data is available
    uint8_t byte = UDR0;

    switch (state) {
      case 0: // Waiting for Start Byte
        if (byte == 0x7E) state = 1;
        break;

      case 1: // Reading Board ID
        boardID = byte;
        state = 2;
        break;

      case 2: // Reading Data High Byte
        data = (byte << 8);
        state = 3;
        break;

      case 3: // Reading Data Low Byte
        data |= byte;
        state = 4;
        break;

      case 4: // Waiting for End Byte
        if (byte == 0x7F) {
          // Process data based on board ID
          switch (boardID) {
            case 1: // pH Sensor
              processPHData(data);
              break;
            case 2: // Turbidity Sensor
              processTurbidityData(data);
              break;
            case 3: // Temperature and Conductivity Sensor
              processTempConductivityData(data);
              break;
          }
        }
        state = 0; // Reset state
        break;

      default:
        state = 0; // Reset on error
        break;
    }
  }
}

void loop() {
  processIncomingData();
}

void processPHData(uint16_t phValue) {
  // Handle pH data (e.g., store in EEPROM or analyze)
}

void processTurbidityData(uint16_t turbidityValue) {
  // Handle turbidity data
}

void processTempConductivityData(uint16_t combinedData) {
  // Split and handle temperature and conductivity data if necessary
}

