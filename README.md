# Hzg_AVR
Heizungssteuerung auf Atmega32
Ein Buderus Ölbrenner mit Warmwasserbereitung wurde gesteuert.
Sensorik: 
- Sensorbus mit Temperatursensoren DS18B20 parsitär versorg
  Aussentemperatur
  Innentemperatur
  Vorlauf
  Rücklauf
  Raumtemperatur Referenzraum
  Zimmertemperaturen andere Räume und Keller
  Warmwassertemperatur
- Abgastemperatur über NTC am A/D-Input
- Uhr über DCF77
- Eingänge für Wochenschaltuhr der Originalsteuerung (Hzg und WW)
- Poti für Raumtemperatur

Aktorik:
  Umwälzpumpe ein/aus (Relais)
  Umwälzpumpe Drehzahl (Umbau an Pumpenelektronik, Optokoppler mit PWM)
  Warmwasser-Ladepumpe ein/aus
  Brenner ein/aus (Relais)
  Umschalter Sommer/Winterbetrieb
  Ansteuerung Brauchwasserwärmepumpe über SSR
  LED ("Pulsschlag")

Regelung:
  Rücklauftemperaturregelung
  Statemachine für Heizung
  Warmwasser alternativ über Ölkessel oder Brauchwasserwärmepumpe
  Statemachine für Warmwasserbereitung Ölkessel
  Statemachine für Warmwasserbereitung Brauchwasserwärmepumpe
  Erkennung, ob Kamin heizt
  Erkennung, ob Terassentür auf ist  
  Ausgabe der Sensor- und Statuswerte über RS232
  Nutzung EEPROM, um gefilterte Temperaturwerte abzulegen

Parametrierung:
  über Defines
  per Kennfeld für außentemperaturabhängige Vorsteuerung
  empirisch ermittelte Werte, z. B. PID-Regler für Raumtemperatureinfluss, Verhalten in der Übergangszeit
 
