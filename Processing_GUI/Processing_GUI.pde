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
App fuer Buerothermometer mit Farbwechsel
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

// 16.04.2020 V3.0

import processing.serial.*;                               // Fuer serielle Schnittstelle
Serial mySerialPort;                                      // Fuer serielle Schnittstelle
final int LICHT = 0;                                      // Licht Positionen im seriellen Telegramm
final int FEUCHTE = 1;                                    // Feuchtigkeit Positionen im seriellen Telegramm
final int TEMPERATUR = 2;                                 // Temperatur Positionen im seriellen Telegramm
final int ALARME = 3;                                     // Alarm Positionen im seriellen Telegramm              
float t = 0, t_balken;                                    // Temperatur und Temperaturmapping fuer Balken und Graph
int f = 0;                                                // Feuchte
int l = 0;                                                // Lichtstaerke
int a = 0;                                                // Alarme
int[] XPosYMesswert_f;                                    // Feuchtewert y auf x-Achsenposition
int[] XPosYMesswert_t;                                    // Temperaturwert y auf x-Achsenposition
PFont font;                                               // Variable fuer Schrift
PImage bg;                                                // Variable fuer Bild
int intervall_sek= 1;                                     // Intervall fuer Messwertaktualisierung nach Programmstart, 1-5 Sekunden.
int R = 230;                                              // Hintergrundfarbe RGB Rot
int G = 253;                                              // Hintergrundfarbe RGB Gruen
int B = 183;                                              // Hintergrundfarbe RGB Blau
int bs = 15;                                              // Balkenstrichstaerke
int YPos_Balken = 200;                                    // Balkenanfang Y Position

/******************
*    S E T U P    *
******************/
void setup(){
  size(240,400);                                          // Fenstergroeße Fix!!
  smooth();                                               // Kantenglaettung
  bg = loadImage("bg.jpg");                               // Hintergrundbild
  font = createFont("Arial Bold", 12, true);
  mySerialPort = new Serial(this, Serial.list()[1], 9600);// Port und Baudrate
  mySerialPort.bufferUntil('\n');                         // Zeile bis 'New Line' 
  XPosYMesswert_f = new int[width];                       // Array fuer f erzeugen
  XPosYMesswert_t = new int[width];                       // Array fuer t erzeugen               
  for (int i = 0; i < width; i++){                        // Arrays mit Anfangsposition fuellen             
    XPosYMesswert_f[i] = 0; 
    XPosYMesswert_t[i] = 0;
  }
}

/****************************************
*    Intervall per Mausklick aendern    *
*****************************************/
void mousePressed(){                                        // Wenn Maustaste (links oder rechts) gedrueckt
  if (mouseY > height/2){                                   // Wenn Maus im Bereich der Skala
    fill(180,100,100);
    text("Intervall wird geändert",width/4 * 4 - width/2, height/2 + height/8 -1);
    fill(100,100,180);
    text("Intervall wird geändert",width/4 * 4 - width/2, height/2 + height/8*3 -1);
    for (int i = 0; i < width; i++){                        // Arrays mit Anfangsposition fuellen             
    XPosYMesswert_f[i] = 0;                                 // Werte auf 0
    XPosYMesswert_t[i] = 0;
    }
    intervall_sek++;                                        // Abfrageintervall um 1 Min. hoch
    if (intervall_sek >=6){                                 // Nach 5 Minuten-Einstellung wieder auf 1 Minute
      intervall_sek=1;
    }
  }
}

/****************
*    D R A W    *
****************/
void draw(){
  background(R,G,B);
  image(bg, 0, 0);
  
  /***************
  * Werte-Balken *
  ****************/
  stroke(240,240,0);
  strokeWeight(bs);
  line(width/4,YPos_Balken,width/4,YPos_Balken-l);                // Balken fuer Licht
  stroke(0,0,255);
  line(width/2,YPos_Balken,width/4*2,YPos_Balken-f);              // Balken fuer Feuchte
  stroke(255,0,0);
  line(width*3/4,YPos_Balken,width*3/4,YPos_Balken-t_balken);     // Balken fuer Temperatur
  fill(R,G,B); stroke(R,G,B);
  strokeWeight(0);
  rect(0,200,width, bs); 
  
  /****************************************
  * Weitere Texte und Lichtschwellenregel *
  *****************************************/
  fill(0);
  textAlign(CENTER);
  textFont(font, 11);
  text("Licht(L,M,H),  Feuchte(%),  Temperatur(°C)",width/2,70);  // Infozeile
  textFont(font, 20);
  text("Sensordaten:",width/2,50);                                // Ueberschrift
  String ll;
  if (l<34){                                                      // Lichtschwellenregel
    ll = "L"; // schwach
  }
  else if (l > 33 && l < 67){                                     // Lichtschwellenregel
    ll ="M"; // mittel
  }
  else {                                                          // Lichtschwellenregel
    ll ="H"; // hoch
  }
  textFont(font, 11);
  text("Temperaturalarme: "+str(a), width/2, 20);
  
  /**************
  * Balkentexte *
  **************/
  textFont(font, 16);
  text(ll,width/4,YPos_Balken-l-14);                               // Text Lichtstaerke
  text(str(f)+"%",width/2,YPos_Balken-f-14);                       // Text Feuchte
  text(str(t)+"°",width/4*3,YPos_Balken-t_balken-14);              // Text Temperatur
  
  /********************
  * Graph-Hintergrund *
  ********************/
  noStroke();
  fill(R,G,B);
  rect( 0, height/2, width, height/2);
  
  /*************
  * Graph-Netz *
  **************/
  strokeWeight(1);
  stroke(220);
  line(width/2, height/2, width/2, height);
  line(width/4, height/2, width/4, height);
  line(width/2+width/4, height/2, width/2+width/4, height);
  for(int i=1; i<=height/2/10; i++)
    line(0, height/2+10*i, width, height/2+10*i);
  
  /*************************************
  * Graph-Beschriftung und Grenzlinien *
  *************************************/
  strokeWeight(1);
  stroke(2);
  line(0,200,width,200);
  line(0,height/2,width,height/2);
  line(0,height/2+height/4,width,height/2+height/4);
  line(0,height,width,height);
  fill(0);
  textFont(font, 10);
  textAlign(RIGHT);
  text("10°C", 25,height/2+height/4-16);
  text("20°C", 25,height/2+height/4-36);
  text("30°C", 25,height/2+height/4-56);
  text("40°C", 25,height/2+height/4-76);
  text("20%", 25,height-16);
  text("40%", 25,height-36);
  text("60%", 25,height-56);
  text("80%", 25,height-76);
  textAlign(CENTER);
  fill(150);
  text("1px. = " + intervall_sek +"Sek.", width/4 * 4 - width/8, height/2 + 19);
  text("1px. = " + intervall_sek +"Sek.",width/4 * 4 - width/8, height/2 + height/4 + 19);
  text("<                 >",width/4 * 4 - width/8, height/2 + 9);
  text("<                 >",width/4 * 4 - width/8, height/2 + height/4 + 9);
  text(str((intervall_sek)*(width/4)/60)+"Min.",width/4 * 4 - width/8, height/2 + 9);
  text(str((intervall_sek)*(width/4)/60)+"Min.",width/4 * 4 - width/8, height/2 + height/4 + 9);
  
  /*****************
  * Graph ausgeben *
  ******************/ 
  strokeWeight(1);
  for (int x = 1; x < width; x++){
    XPosYMesswert_f[x-1] = XPosYMesswert_f[x];             // Belegung für noch nicht vorhandenen Werte
    XPosYMesswert_t[x-1] = XPosYMesswert_t[x];             // Belegung für noch nicht vorhandenen Werte
  }
  XPosYMesswert_f[width-1] = f;                            // f Wert in Graph übertragen
  XPosYMesswert_t[width-1] = round(t_balken);              // t_balken Wert in Graph übertragen
  for (int x = 2; x < width-2; x++){
    stroke(0,0,255);
    line(x, XPosYMesswert_f[x]*(-1)+height-1, x-1, XPosYMesswert_f[x-1]*(-1)+height-1);                        // Linie von aktuellem f Messwert zu letztem Messwert
    stroke(255,0,0);
    line(x, XPosYMesswert_t[x]*(-1)+height/2+height/4-1, x-1, XPosYMesswert_t[x-1]*(-1)+height/2+height/4-1);  // Linie von aktuellem t Messwert zu letztem Messwert
  }
  
  /************
  * Intervall *
  ************/
  delay(intervall_sek*1000);                              // Verzoegerung in ms
}

/***************
* serialEvent  *
***************/
void serialEvent(Serial mySerialPort){                    // wird von Processing selbst aufgerufen
  String portStream = mySerialPort.readString();
  String[] list = split(portStream, ';');
  t = float(list[TEMPERATUR]);                            // Temperatur als float
  if ((t<0.0) || (t>49.4)) t=0;                           // Wenn t Messwerte außerhalb Grenzen
  t_balken = map(t,0.0,50.0,0.0,100.0);                   // in Prozent fuer Balken, Sensor liefert 0-50°C
  if ((f<0) || (f>99)) f=0;                               // Wenn f Messwerte außerhalb Grenzen
  f = int(list[FEUCHTE]);                                 // Feuchte kommt in %, kein Mapping
  l = int(list[LICHT]);                                   // Licht 0-255 in Prozent
  a = int(list[ALARME]);                                  // Anzahl Alarme
  //println(t);                                           // Diagnoseausgabe
  //println(t_balken);                                    // Diagnoseausgabe
  //println(f);                                           // Diagnoseausgabe
  //println(l);                                           // Diagnoseausgabe
  //println(a);                                           // Diagnoseausgabe
}
