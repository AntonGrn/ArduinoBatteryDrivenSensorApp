#include <dht.h> // Library for DHT11 (Temperature and humidity sensor)
#include <U8glib.h> // Library for OLED screen

const byte relayPin = 2;
const byte interruptPin = 3;
const byte batteryPin = A2; //For analog read
const byte lowBatteryLEDpin = 9;
const byte temperaturePin = A0; //For analog read

dht DHT; //Create DHT object
U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE); //Create SSD1306 I2C display object with 128x64 resolution.

int temp; 
int humidity;
float batteryVolt;
float batteryPerc;

volatile boolean toggle = false; //button pressed status
volatile boolean displayTemp = true; // Display temperature OR battery power  -- NEED TO BE VOLATILE??
unsigned long lastIntervalTime = 0;
unsigned long lastDebounceTime = 0;
const long interval = 30000; // Time interval for updating sensor values
const long debounceTime = 500;

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(lowBatteryLEDpin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), interruptFunction, RISING); //Initialize ISR. Button has pullDown resistor -> Trigger on RISING edge

  lastDebounceTime = millis();

  delay(1500); 
  doUpdate();
  
}

void loop() {
  
  if (toggle) {
    displayTemp = !displayTemp; //Toggle display mode
    printToScreen();
    toggle = false;
    
  }
  if ((millis() - lastIntervalTime) > interval) { //Read analog values and update screen at certain interval
    doUpdate();
  }
}

void interruptFunction() { //The function that will be run as ISR. Short, fast.
  if ((millis() - lastDebounceTime) > debounceTime) { // Debounce
    toggle = true;
    lastDebounceTime = millis();
  }
}

void doUpdate() {
  measureTemp();
  measureBattery();
  printToScreen();
  lastIntervalTime = millis();
}

void measureTemp() {
  DHT.read11(temperaturePin);
  temp = DHT.temperature;
  humidity = DHT.humidity;
}

void measureBattery() {
  int batterySensorValue = 0;
  digitalWrite(relayPin, HIGH);// Relay: close circuit
  delay(700); 
  for(int r = 0 ; r < 10 ; r++) { // 10 analog samples
    batterySensorValue += analogRead(batteryPin); // 1024 quantization levels: 0 - 1023(5V)
    delay(10);
  }
  batterySensorValue = batterySensorValue / 10; //Calculate average 
  batteryVolt = ((batterySensorValue * 5.00) / 1023.00) * 2 * 1.06; // *2 because of the voltage dividor (20k & 20k splits voltage in half). * 1.02 is the calibrating factor to compensate for resistor precisions.
  //batteryPerc = batteryVolt / 9 * 100; // 9V battery. 0V = 0%, 9V = 100%
  batteryPerc = ((batteryVolt - 6) / 3) * 100; // 6V = 0%, 9V = 100% -> Arduino won't run properly beneith around 6.5V power supply (recommended: 7-12V)
  batteryPerc = batteryPerc > 100 ? 100 : batteryPerc; //100% is max even if voltage exceeds 9V
  batteryPerc = batteryPerc < 0 ? 0 : batteryPerc; //0% is minimum
  digitalWrite(relayPin, LOW); //Relay: open circuit
}

void printToScreen() {
  digitalWrite(lowBatteryLEDpin, batteryVolt <= 6.5 ? HIGH : LOW); // Indicate via LED if batterypower is low (Arduino supplied by 7-12V) 
  oled.firstPage();
  do {
    draw();
  } while (oled.nextPage());
}

void draw() {
   // HEADER
  oled.setFont(u8g_font_9x15);
  oled.setPrintPos((displayTemp ? 14 : 33), 10);
  oled.print(displayTemp ? "Temperature" : "Battery"); 
  // MAIN AREA
  oled.setFont(u8g_font_helvB24);
  oled.setPrintPos(29, 42);
  if (displayTemp) {
    oled.print(temp);
    oled.print("\xB0""C"); // °C    
  } else { //display battery power
    oled.print(String(batteryPerc, 0));
    oled.print(" %");
  }
  // SUB AREA
  oled.setFont(u8g_font_9x15);
  oled.setPrintPos((displayTemp ? 10 : 40), 60);
  if (displayTemp) {
    oled.print(humidity);
    oled.print("% humidity");
  } else { 
    oled.print(batteryVolt);
    oled.print("V");
  }
  
}
