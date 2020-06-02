/*
Copyright (c) 2020 Christoph Goebel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

/*--------------------------------------
    Buerothermometer mit Farbwechsel
                /----\                 
                | 32 |
                | 30 |
                | 28 |
                |>26<|
                |>24<|
                | 22 |
                | 20 |
                | 18 |
                | 16 |
            ----|    |-----
           |  O       |--| |
           | Make  03/2020 |---/
           -----------------
          von Christoph Goebel
  --------------------------------------*/

// 19.03.2020 V1.0

#include "pitches.h"
#include "FastLED.h"                                    // LED-Streifen-Bibliothek
#include "DHT.h"                                        // DHT Bibliothek

#define DHTPIN 13                                       // Temperaturfuehler-Pin
#define DHTTYPE DHT11                                   // Typ des Temperaturfuehlers
#define ANZAHL_LEDS 9                                   // LED Streifen Anzahl LEDs
#define LED_PIN 2                                       // LED Streifen Pin
#define BUTTONPIN 3                                     // Knopf Pin mit Interrupt-Moeglichkeit
#define LDRPIN 0                                        // Analoger Pin fuer Foto-Widerstand
#define DECKKRAFT_LEDS 255                              // Deckkraft Min-Max 0-255
#define HSV_TEMP_LED 0                                  // Farbwert aus HSV Farbkreis 0-360° fuer Temperatur LED
#define HSV_FEU_LED 240                                 // Farbwert aus HSV Farbkreis 0-360° fuer Feuchte LED
#define MIN_LICHTSTAERKE 20                             // Minimale LED Ausleuchtung/Lichtstaerke um Farben nicht zu verfaelschen, Sinn macht ein Wert zwischen 20 und 70.
#define OFFSET_AKT_LED 30                               // Aktive LEDs etwas heller als passive LEDs ansteuern, Sinn macht ein Wert zwischen Offset 10 und 30.
#define PIEPSER 6                                       // PIEPSER-Pin
#define L_GERING 30                                     // Bis zu welchem Wert ist Lichtstaerke gering 0-255?
#define L_HOCH 120                                      // Ab welchem Wert ist Lichstaerke hoch 0-255?
#define CHECKINT 900000                                 // Einmal alle 15 Minuten (900000 Millisekunden) auf Alarmschwelle pruefen
#define T_HOCH 32.5                                     // Alarmschwelle - zu warm
#define T_NIEDRIG 15.4                                  // Alarmschwelle - zu kalt
#define ANZAHL_BIS_STUMM 4                              // Nach n Alarmen wird die Akustik deaktiviert, die Alarme lassen sich ueber die Processing-App auslesen. Mit Reset kann man die Alarmausgabe ruecksetzen.          

CRGB leds[ANZAHL_LEDS];                                 // Led Funktionsprototyp
DHT dht(DHTPIN, DHTTYPE);                               // DHT Funktionsprototyp

unsigned long prevTaster, prevAlarm;                    // fuer millis() Pruefung
unsigned short int modus=0;                             // Anzeigemodus, Feuchte oder Temperatur, globale Variable
unsigned short int HSV_PAS_LED[] = {60, 120, 300, 0};   // Alternative Farben aus HSV Farbkreis 0-365° fuer passive LEDs. Das letzte Feld (Wert egal) wird als Dummy fuer Eco(unsichtbar) genutzt.
unsigned short int PosPasFarbe = 0;                     // Position in Array der passiven Farben
bool eco = false;                                       // Passive LEDs aus, gleich nach dem Start
bool piep = true;                                       // Sound ja/nein true/false
int lichtstaerke;                                       // Lichtstaerke
float feuchte;                                          // Feuchtigkeit
float temperatur;                                       // Temperatur
unsigned short int anzahl_alarme;                       // Fuer Alarmprotokollierung


/*    
8 Toene (Noten) muessen als Startmelodie oder Alarm konfiguriert werden, '0' anstatt 'NOTE_XY' setzt den Ton aus. 
Darunter die Abspieldauer (Tonlaenge) z.B. Viertel, Achtel usw.                                                     
*/
int startsound[] = {NOTE_E5, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_A4};
int startsound_tonlaenge[] = {4, 8, 8, 4, 8, 8, 4,  4};
int alarmsound[] = {NOTE_E4, NOTE_C5, NOTE_E4, NOTE_C5, 0, 0, 0, 0};                  
int alarmsound_tonlaenge[] = {4, 4, 4, 4, 16, 16, 16, 16};


/*************************
 *     A N Z E I G E     *
 *************************/
void led_anzeige(unsigned short int ls, float f, float t){
    int p,a,lsp;                                                                        // passive LEDs, aktive LEDs, Lichtstaerke passive LEDs
    p = map(HSV_PAS_LED[PosPasFarbe],0,360,0,255);                                      // Farbe der passiven Leds von HSV-Wert auf Byte-Wert
    if (eco == true) lsp = 0;                                                           // ECO, Passive LEDs aus
    else lsp = ls;                                                                      // kein ECO, Passive LEDs normal                                       
    switch(modus){    
      case 0: // Temperatur
        a = map(HSV_TEMP_LED,0,360,0,255);
        if (t < 17.5) leds[0] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED);             // 16° LED
        else leds[0] = CHSV(p, DECKKRAFT_LEDS, lsp);  
        if (t < 19.5 && t > 16.4) leds[1] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 18° LED      
        else leds[1] = CHSV(p, DECKKRAFT_LEDS, lsp);          
        if (t < 21.5 && t > 18.4) leds[2] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 20° LED          
        else leds[2] = CHSV(p, DECKKRAFT_LEDS, lsp);          
        if (t < 23.5 && t > 20.4) leds[3] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 22° LED
        else leds[3] = CHSV(p, DECKKRAFT_LEDS, lsp);    
        if (t < 25.5 && t > 22.4) leds[4] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 24° LED
        else leds[4] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (t < 27.5 && t > 24.4) leds[5] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 26° LED
        else leds[5] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (t < 29.5 && t > 26.4) leds[6] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 28° LED
        else leds[6] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (t < 31.5 && t > 28.4) leds[7] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 30° LED
        else leds[7] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (t > 30.4) leds[8] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED);             // 32° LED
        else leds[8] = CHSV(p, DECKKRAFT_LEDS, lsp);
        FastLED.show();                                                                 // LEDs schreiben  
        break;
      case 1: // Feuchte
        a = map(HSV_FEU_LED,0,360,0,255);
        if (f < 32.5) leds[0] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED);             // 30% LED
        else leds[0] = CHSV(p, DECKKRAFT_LEDS, lsp);  
        if (f < 37.5 && f > 32.4) leds[1] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 35% LED      
        else leds[1] = CHSV(p, DECKKRAFT_LEDS, lsp);          
        if (f < 42.5 && f > 37.4) leds[2] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 40% LED          
        else leds[2] = CHSV(p, DECKKRAFT_LEDS, lsp);          
        if (f < 47.5 && f > 42.4) leds[3] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 45% LED
        else leds[3] = CHSV(p, DECKKRAFT_LEDS, lsp);    
        if (f < 52.5 && f > 47.4) leds[4] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 50% LED
        else leds[4] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (f < 57.5 && f > 52.4) leds[5] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 55% LED
        else leds[5] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (f < 62.5 && f > 57.4) leds[6] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 60% LED
        else leds[6] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (f < 67.5 && f > 62.4) leds[7] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED); // 65% LED
        else leds[7] = CHSV(p, DECKKRAFT_LEDS, lsp);
        if (f > 67.4) leds[8] = CHSV(a, DECKKRAFT_LEDS, ls+OFFSET_AKT_LED);             // 70% LED
        else leds[8] = CHSV(p, DECKKRAFT_LEDS, lsp);
        FastLED.show();                                                                 // LEDs schreiben
        delay(3000);                                                                    // n Millisekunden Feuchte anzeigen
        modus = 0;                                                                      // Modus zurueck auf Temperatur
        break;
    }
}



/***************************************
 *     S O U N D - a u s g e b e n     *
 ***************************************/
void sound_abspielen(int sound[7], int tonlaenge[7]) {
    for (int Note = 0; Note < 8; Note++) {                                              // Schleifenzaehler fuer Array
      int ad = 1000 / tonlaenge[Note];                                                  // Abspieldauer ad 
      tone(PIEPSER, sound[Note], ad);                                                   // Ton ausgeben
      int pause = ad * 1.30;                                                            // Pausenlaenge zwischen Toenen
      delay(pause);                                                                     // Pause
      noTone(PIEPSER);                                                                  // Tonausgabe beenden
    }
}


/****************************
 *     M O D U S - T / F    *
 ****************************/
void taste_losgelassen(){
  modus++;
  if (modus >=2)
    modus = 0;
}


/************************************
 *     A L A R M - p r u e f e n    *
 ************************************/
// Es wird alle CHECKINT Millisekunden geprueft, ob ein Alarm generiert werden muss.
int alarm_check(float t, float f){
  static int i = 0;                                                                 // Einmalige Initialisierung
  if (millis() - prevAlarm > CHECKINT){                                             // Pruefintervall
    if ((t <= T_NIEDRIG) or (t >= T_HOCH)){                                         // Pruefung fuer Alarm
      if (i < ANZAHL_BIS_STUMM) sound_abspielen(alarmsound, alarmsound_tonlaenge);  // Wurden n Alarme ausgegeben, schweigt der Piepser, bis zu einem Reset
    i++;                                                                            // Die Anzahl der Alarme kann ueber die Processing-App eingesehen werden
    prevAlarm = millis();                                                           // Pruefintervall ruecksetzen
     }    
    }
  return i;
  }



/************************
 *       S E T U P      *
 ************************/
void setup() {
  if (piep == true) sound_abspielen(startsound, startsound_tonlaenge);              // Startsound abspielen
  pinMode(BUTTONPIN, INPUT_PULLUP);                                                 // Internen PullUp Widerstand aktivieren
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), taste_losgelassen, CHANGE);     // Interrupt 
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, ANZAHL_LEDS);                            // LED Band konfigurieren
  Serial.begin(9600);                                                               // Bautrate der seriellen Schnittstelle
  dht.begin();                                                                      // Dht init
  prevAlarm = millis();                                                             // Zeitstempel fuer Alarmpruefung
}


/************************
 *       L O O P        *
 ************************/
void loop() {
  /* Sensordaten ermitteln */
  lichtstaerke = map(analogRead(LDRPIN), 0, 1023, 0, 255 - OFFSET_AKT_LED);          // Lichtstaerke ermitteln und auf 8Bit Wert fuer LEDs mappen (- max OFFSET) 
  if (lichtstaerke < MIN_LICHTSTAERKE) lichtstaerke = MIN_LICHTSTAERKE;              // Minimale Lichtstaerke setzen
  feuchte = dht.readHumidity();                                                      // Lesen der Luftfeuchtigkeit in % und speichern in die Variable
  temperatur = dht.readTemperature();                                                // Lesen der Temperatur in °C und speichern in die Variable

  /* Akustik */
  if (piep == true)                                                                  // Akustik generell erlaubt - true, generell nicht erlaubt - false
    anzahl_alarme = alarm_check(temperatur, feuchte);                                // Aufruf alarm_check
      
  /* Serielle Daten ausgeben */
  if (lichtstaerke <= L_GERING) Serial.print(33);                                    // Level 33 (Licht low) in Ausgabezeile
  else if ((lichtstaerke > L_GERING) and (lichtstaerke < L_HOCH)) Serial.print(66);  // Level 66 (Licht middle) in Ausgabezeile
  else Serial.print(100);                                                            // Level 100 (Licht high) in Ausgabezeile
  Serial.print(";");                                                                 // Trenner nach Level
  Serial.print(feuchte); Serial.print(";");                                          // Feuchte in Ausgabezeile
  Serial.print(temperatur); Serial.print(";");                                       // Temperatur in Ausgabezeile
  Serial.print(anzahl_alarme); Serial.print(";");                                    // Anzahl Alarme in Ausgabezeile
  Serial.print("\n");                                                                // Ende
  
  /* Anzeige */
  led_anzeige(lichtstaerke, feuchte, temperatur); 
  prevTaster = millis();
  while (digitalRead(BUTTONPIN) == 0){                                            // Solange der Knopfstatus GEDRUECKT ist
    if ((millis() - prevTaster) > 2000){                                          // wenn dieses Druecken laenger als 2 Sekunden
      PosPasFarbe++;                                                              // Farb-Zeiger Wert aendern
      if (PosPasFarbe == (sizeof(HSV_PAS_LED) / sizeof (int))-1)                  // Wenn man im letzten Feld des Arrays fuer die passiven Farben ist 
        eco = true;                                                               // dann ECO Spareinstellung aktivieren (passive LEDs aus)
      else
        eco = false;
      if (PosPasFarbe == sizeof(HSV_PAS_LED) / sizeof (int))                      // Zurueck auf erste Farbe, sobald man im letzen Farbschema ist und wieder wechseln will
        PosPasFarbe = 0;     
      prevTaster = millis();                                                      // Die verstrichene Zeit wieder auf aktuelleZeit zuruecksetzen
      led_anzeige(lichtstaerke, feuchte, temperatur);
    }
  }
}
