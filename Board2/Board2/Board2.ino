/*
  AVR Registers Explanation:
  1. ADMUX (ADC Multiplexer Selection Register):
     - Bits [7:6]: Reference voltage selection (REFS1:REFS0).
       Setting REFS0 to 1 selects AVcc as the reference voltage.
     - Bits [3:0]: Analog channel selection.
       These bits choose the ADC channel (ADC0 to ADC7).

  2. ADCSRA (ADC Control and Status Register A):
     - ADEN: ADC Enable. Set to 1 to enable the ADC.
     - ADSC: ADC Start Conversion. Writing 1 starts a conversion.
     - ADPS2:ADPS0: ADC Prescaler bits to divide system clock.
       Prescaler of 128 is set for 16 MHz systems for stable readings.

  3. ADC (Analog-to-Digital Converter Data Register):
     - A 10-bit register that holds the conversion result.

  4. millis(): Provides system uptime in milliseconds.
     Useful for time-based operations.

  5. _BV(x): A macro that returns a byte value with bit x set.
     Commonly used in register bit manipulations.

 
*/

#define SensorPin 0 // Analog channel for TB meter output

unsigned long avgValue;    // Pointer to store the average value of the sensor feedback
float voltage, turbidity;
int buf[10], temp;
unsigned long previousMillis = 0;
const unsigned long samplingInterval = 100;

void setup() {
  Serial.begin(9600);

  // Set up the ADC
  ADMUX = (1 << REFS0); // Set AVcc as the reference voltage and select ADC0
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC with a prescaler of 128
}

uint16_t analogReadDirect(uint8_t channel) {
  // Select the channel using bitwise operations
  ADMUX = (ADMUX & 0xF8) | (channel & 0x07);

  // Start the conversion
  ADCSRA |= (1 << ADSC);

  // Wait for the conversion to complete
  while (ADCSRA & (1 << ADSC));

  // Return the ADC value
  return ADC;
}

void processSamples(int *buffer, unsigned long *average) {
  // Sort the samples from smallest to largest
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer[i] > buffer[j]) {
        int temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
      }
    }
  }

  // Calculate the average of the middle 6 values
  *average = 0;
  for (int i = 2; i < 8; i++) {
    *average += buffer[i];
  }
  *average /= 6;
}

void calculateTurbidity(float *voltage, float *turbidity, unsigned long avgValue) {
  *voltage = (float)avgValue * 5.0 / 1024;
  *turbidity = -1333.33 * (*voltage) + 5333.33;
  *turbidity = max(*turbidity, 0); // Clamp negative turbidity to 0
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= samplingInterval) {
    previousMillis = currentMillis;

    // Collect 10 samples
    for (int i = 0; i < 10; i++) {
      buf[i] = analogReadDirect(SensorPin);
      delay(10);
    }

    // Process samples and calculate average
    processSamples(buf, &avgValue);

    // Calculate turbidity
    calculateTurbidity(&voltage, &turbidity, avgValue);

    // Print the result
    // Serial.print("TB:");
    Serial.println(turbidity);
  }
}
