# Arduino Battery Monitor

Button click triggers external interrupt which toggles an OLED between displaying:
1. Temperature and humidity (Component: DHT11)
2. Voltage and percentage of battery (Components: Resistors for voltage division, relay for short sample intervals (<1sec) each 30 sec, to avoid power drain)
