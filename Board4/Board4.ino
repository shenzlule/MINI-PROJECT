#include "SoftwareSerial.h"
#include <EEPROM.h>

// Define pins for LEDs and buzzer
#define BOARD_1_LED 9
#define BOARD_2_LED 8
#define BOARD_3_LED 7
#define BUZZER 4

// Thresholds for pH, NTU, and temperature
const float PH_THRESHOLD = 7.5;       // Example threshold for pH
const float NTU_THRESHOLD = 50.0;    // Example threshold for turbidity
const float TEMP_THRESHOLD = 30.0;   // Example threshold for temperature

String incomingData = "";  // From master (9600 baud, for pH)
String incomingData2 = ""; // From b2 (9600 baud, for NTU)
String incomingData3 = ""; // From b3 (9600 baud, for temperature)

bool dataReady = false;
bool dataReady_b2 = false;
bool dataReady_b3 = false;

unsigned long startb3 = 0;
unsigned long startb2 = 0;
unsigned long led_ph_time = 0;
unsigned long led_tmp_time = 0;
unsigned long led_tb_time = 0;
unsigned long alertStartTime = 0;
bool alertActive = false;
bool ledtmp = false;
bool ledtb = false;
bool ledph = false;

// Define SoftwareSerial pins
SoftwareSerial b2(10, 11); // TB sensor
SoftwareSerial b3(2, 3);  // TMP sensor

void setup() {
  // Set up serial communications
  Serial.begin(9600);
  b2.begin(9600);
  b3.begin(9600);

  // Set up LEDs and buzzer
  pinMode(BOARD_1_LED, OUTPUT);
  pinMode(BOARD_2_LED, OUTPUT);
  pinMode(BOARD_3_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
   led_ph_time=millis();
   led_tb_time=millis();
   led_tmp_time=millis();
  Serial.println("Master Ready");
}

void loop() {
  // Read from master serial
  startb3 = millis();
  startb2 = millis();
  readMasterData();
  readB2Data();
  readB3Data();

  // Process data from master (pH)
  if (dataReady) {
    digitalWrite(BOARD_1_LED, HIGH);

    processPHData();
    ledph=true;
  }

  // Process data from b2 (TB sensor)
  if (dataReady_b2) {
    digitalWrite(BOARD_2_LED, HIGH);
    processB2Data();
   ledtb=true;
  }

  // Process data from b3 (TMP sensor)
  if (dataReady_b3) {
    digitalWrite(BOARD_3_LED, HIGH);
    processB3Data();
   ledtmp=true;
  }


   if(millis()-led_tmp_time >100 && ledtmp){
     
        digitalWrite(BOARD_3_LED, LOW);
        ledtmp=false;
        led_tmp_time=millis();

   }
   
   if(millis()-led_tb_time >100 && ledtb){
     
        digitalWrite(BOARD_2_LED, LOW);
          led_tb_time=millis();
        ledtb=false;

   }
   
   if(millis()-led_ph_time >100 && ledph){
     
        digitalWrite(BOARD_1_LED, LOW);
        led_ph_time=millis();
        ledph=false;

   }
  // Handle buzzer alerts without delay
  handleAlert();
}

void readMasterData() {
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar == '\n') {
      dataReady = true;
      break;
    }
    incomingData += incomingChar;
  }
}

void readB2Data() {
  b2.listen();
  while (millis() - startb2 < 100) {
    while (b2.available() > 0) {
      char incomingChar = b2.read();
      if (incomingChar == '\n') {
        dataReady_b2 = true;
        break;
      }
      incomingData2 += incomingChar;
    }
  }
}

void readB3Data() {
  b3.listen();
  while (millis() - startb3 < 100) {
    while (b3.available() > 0) {
      char incomingChar = b3.read();
      if (incomingChar == '\n') {
        dataReady_b3 = true;
        break;
      }
      incomingData3 += incomingChar;
    }
  }
}

void processPHData() {
  float pH = incomingData.toFloat();
  Serial.print("pH Level: ");
  Serial.println(pH);
  if (pH > PH_THRESHOLD) {
    activateAlert();
    EEPROM.write(2, 1); // Save pH alert to EEPROM
  }
  incomingData = "";
  dataReady = false;
}

void processB2Data() {
  float turbidity = incomingData2.toFloat();
  Serial.print("Turbidity: ");
  Serial.println(turbidity);
  if (turbidity > NTU_THRESHOLD) {
    activateAlert();
    EEPROM.write(0, 1); // Save NTU alert to EEPROM
  }
  incomingData2 = "";
  dataReady_b2 = false;
}

void processB3Data() {
  float temperature = incomingData3.toFloat();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  if (temperature > TEMP_THRESHOLD) {
    activateAlert();
    EEPROM.write(1, 1); // Save temperature alert to EEPROM
  }
  incomingData3 = "";
  dataReady_b3 = false;
}

void activateAlert() {
  alertActive = true;
  alertStartTime = millis();
  Serial.println("Active buzzer");
  digitalWrite(BUZZER, HIGH);
}

void handleAlert() {
  if (alertActive && (millis() - alertStartTime >= 400)) {
    digitalWrite(BUZZER, LOW);
    Serial.println("OFF buzzer");
    alertActive = false;
  }
}
