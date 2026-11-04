/*
 * Creato da Federico Cobianchi
 */

#include <EEPROM.h>
#include <TinyOzOLED.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>

#if __AVR_ATtiny84__
#else     
  #error "Questo programma è stato ideato per l'ATtiny84"
#endif

#define oled OzOled

#define cella1 1
#define vbat 2
#define selezione2 3
#define avanti 7
#define selezione1 9

#define versione_software 3.0
#define versione_hardware 2.0

// posizione dati nella EEPROM //

#define gioco1 0 // 0
#define gioco2 2 // 1
#define gioco3 4 // 2
#define gioco4 6 // 3
#define gioco5 8 // 4
#define numerosalvataggio 10
#define numerodisplay 11
#define numerotensione 12

/////////////////////////////////

#define salvataggio1 120000 // 2 min
#define salvataggio2 300000 // 5 min
#define salvataggio3 600000 // 10 min

#define updatedisplay1 500  // 5 sec
#define updatedisplay2 1000 // 1 sec 
#define updatedisplay3 3000 // 3 sec

#define letturatensione1 10000 // 10sec -> 40 secondi
#define letturatensione2 20000 // 20sec -> 1.2 minuti
#define letturatensione3 30000 // 30sec -> 2 minuti

#define conferma 1500 // 1.5 sec
#define DifferenzaMassima 0.2  // differenza massima di tensione fra le due celle
#define NumeroCampioni 4.00

byte eepromsalvataggio = 3, eepromdisplay = 3, eepromtensione = 3, i = 0, m = 1;
byte indicesalvataggio = 3, indicedisplay = 3, indicetensione = 3, numeroerrori = 0;
long pallini = 0, lastpallini = 2;
unsigned int partite[5] = {0, 0, 0, 0, 0}, avvertenza = 10000;
long temposalvataggio[3] = {salvataggio1, salvataggio2, salvataggio3}, percentualeassoluta = 0, lastpercentualeassoluta = 0, percentualerelativa = 0, lastpercentualerelativa = 0;
long tempodisplay[3] = {updatedisplay1, updatedisplay2, updatedisplay3}, tempotensione[3] = {letturatensione1, letturatensione2, letturatensione3};
unsigned long tempod = 0, t = 0, tempoe = 0, tempoavviso = 0;
float CL1[4] = {0, 0, 0, 0}, Vbat[4] = {0, 0, 0, 0}, x = 0, y = 0;
bool selettore = false, resetdisplay = false, mostrapartite = false, evitare = false, nuovapartita = false, avviso = false; 
char stato = 'a';

ISR(EXT_INT0_vect){
  // Per AtTiny44/84  EXT_INT0_vect
  // Per AtTiny45/85  INT0_vect
  pallini++;
  if(avviso == true) avviso = false;
  if(mostrapartite == true) mostrapartite = false;
}

void boot(){
    oled.init();
    delay(500);
    oled.setCursorXY(1,2);
    oled.printString("Conta pallini");
    oled.setCursorXY(6,4);
    oled.printString("2.0");
    delay(2000);

    int k = 0;
    partite[0] = EEPROM.get(gioco1, k);
    k = 0;
    delay(50);
    partite[1] = EEPROM.get(gioco2, k);
    k = 0;
    delay(50);
    partite[2] = EEPROM.get(gioco3, k);
    k = 0;
    delay(50);
    partite[3] = EEPROM.get(gioco4, k);
    k = 0;
    delay(50);
    partite[4] = EEPROM.get(gioco5, k);
    delay(50);    
    eepromsalvataggio = EEPROM.read(numerosalvataggio); // c
    delay(50);
    eepromdisplay = EEPROM.read(numerodisplay);  // b
    delay(50);
    eepromtensione = EEPROM.read(numerotensione); // a

    indicesalvataggio = eepromsalvataggio;
    indicedisplay = eepromdisplay;
    indicetensione = eepromtensione;

    delay(1000);
    tempod = millis();
    t = 0;
    x = (4.20 * analogRead(cella1)) / 817.53479;
    y = (8.40 * analogRead(vbat)) / 789.97765;
    percentualeassoluta = (100 * y) / 8.40;
    percentualerelativa = (100 * (y - 6.20)) / 2.20;
    if(percentualeassoluta > 100) percentualeassoluta = 100;
    if(percentualerelativa > 100) percentualerelativa = 100;
    oled.clearDisplay();
}

void printDecimal(float float_num, byte prec){
  byte num_int = 0;
  byte num_frac = 0;
  byte num_extra = 0;
  long d = float_num; // get the integer part
  float f = float_num - d; // get the fractional part
  if (d == 0 && f < 0.0){ 
    oled.printChar('-');
    num_extra++;
    oled.printChar('0');
    num_extra++;
    f *= -1;   
  }
  else if (d < 0 && f < 0.0){  
    num_int = oled.printNumber(d); // count how many digits in integer part
    f *= -1;   
  }
  else{ 
    num_int = oled.printNumber(d); // count how many digits in integer part  
  }
  if((f > 0.00) && (f < 0.01)){
    oled.printChar('.');
    oled.printChar('0');
    oled.printChar('0');
  }
  else if((f > 0.01) && (f < 0.1)){
    oled.printChar('.');
    oled.printChar('0');
    num_extra++;  
    long f_shift = 1;       
    for (byte j=0; j<prec; j++){
      f_shift *= 10;
    }
    num_frac = oled.printNumber((long)(f*f_shift)); // count how many digits in fractional part    
  }
  else{
      oled.printChar('.');
    num_extra++;  
    long f_shift = 1;       
    for (byte j=0; j<prec; j++){
      f_shift *= 10;
    }
    num_frac = oled.printNumber((long)(f*f_shift)); // count how many digits in fractional part    
  }
  return num_int + num_frac + num_extra;
}


void setup(){  
  GIMSK = 0b01000000; // interrupt 0
  MCUCR = 0b00000010; // falling edge
  sei(); // abilita gli interrupt
  ADMUX = 0b00000000; // Vcc (3.3V) = voltage reference
  ADCSRA = 0b10000110; //Prescaler clock ADC = 64
  //ADMUX = (0 << ADLAR); 
   
  pinMode(cella1, INPUT); 
  pinMode(vbat, INPUT);  
  pinMode(selezione2, INPUT);  
  pinMode(avanti, INPUT);    
  pinMode(10, INPUT); // conferma
  pinMode(9, INPUT); // selezione1
  
  boot();   
}

void loop(){
    if((digitalRead(selezione1) == HIGH) && (mostrapartite == false) && (avviso == false)){
      if(selettore == true){
        selettore = false;
        if((eepromsalvataggio != indicesalvataggio) || (eepromdisplay != indicedisplay) || (eepromtensione != indicetensione)){
          EEPROM.write(numerotensione, indicetensione);
          EEPROM.write(numerodisplay, indicedisplay);
          EEPROM.write(numerosalvataggio, indicesalvataggio);
          eepromsalvataggio = indicesalvataggio;
          eepromdisplay = indicedisplay;
          eepromtensione = indicetensione;
        }
        oled.clearDisplay();
      }
      if(digitalRead(avanti) == HIGH){
        mostrapartite = true;
        evitare = true;
      }
      if(millis() - tempod > tempodisplay[eepromdisplay - 1]){
        oled.setCursorXY(0,0);
        oled.printString("Pallini: ");
        oled.printNumber(pallini);
        oled.setCursorXY(0,2);
        oled.printString("Vbat: ");
        printDecimal(y,2);
        oled.printString("V");
        oled.setCursorXY(2,4);
        printDecimal(x,2);
        oled.printString("V");
        oled.printString(" / ");
        printDecimal(y-x,2);
        oled.printString("V");
        oled.setCursorXY(0,6);
        oled.printString("Batt. ass: ");
        oled.printNumber(percentualeassoluta);
        oled.printString("%");
        oled.setCursorXY(0,7);
        oled.printString("Batt. rel: ");
        oled.printNumber(percentualerelativa);
        oled.printString("%");
        tempod = millis();
     }
    }
    else if(digitalRead(selezione2) == HIGH){
      if(selettore == false){
        selettore = true;
        m = 1;
        if(mostrapartite == true) mostrapartite = false;
        oled.clearDisplay();
      }
        oled.setCursorXY(0,0);
        oled.printString("Impostazioni: ");
        oled.printNumber((long)(m));
        oled.setCursorXY(0,2);
        oled.printString("Tensione: ");
        oled.printNumber((long)(tempotensione[indicetensione - 1]) / 1000);
        oled.printString(" s");
        oled.setCursorXY(0,4);
        oled.printString("Display: ");
        printDecimal((float)(tempodisplay[indicedisplay - 1]) / 1000, 1);
        oled.printString(" s");
        oled.setCursorXY(0,6);
        oled.printString("EEPROM: ");
        oled.printNumber((long)(temposalvataggio[indicesalvataggio - 1]) / 60000);
        oled.printString(" m");
      
        if(digitalRead(avanti) == HIGH){
        if(m < 3) m++;
        else{
          m = 1;
          //selettore = false;
        }
        delay(100);
      }
      if(digitalRead(10) == HIGH){
        switch(m){
          case 1:
                 if(indicetensione < 3) indicetensione++;
                else{
                  indicetensione = 1;
                  oled.clearDisplay();
                }
                delay(100);
                break;
         case 2:
                if(indicedisplay < 3) indicedisplay++;
                else{
                  indicedisplay = 1;
                  oled.clearDisplay();
                }
                delay(100);
                break;
         case 3:
                if(indicesalvataggio < 3) indicesalvataggio++;
                else{
                  indicesalvataggio = 1;
                  oled.clearDisplay();
                }
                delay(100);
                break;
        }
      }
    }

    if(mostrapartite == true){
      if(evitare == true){
        evitare = false;
        oled.clearDisplay();
      }
      oled.setCursorXY(0,0);
      oled.printString("Partita 1: ");
      oled.printNumber((long)(partite[0]));
      oled.setCursorXY(0,1);
      oled.printString("Partita 2: ");
      oled.printNumber((long)(partite[1]));
      oled.setCursorXY(0,2);
      oled.printString("Partita 3: ");
      oled.printNumber((long)(partite[2]));
      oled.setCursorXY(0,3);
      oled.printString("Partita 4: ");
      oled.printNumber((long)(partite[3]));
      oled.setCursorXY(0,4);
      oled.printString("Partita 5: ");
      oled.printNumber((long)(partite[4]));
      if(digitalRead(10) == HIGH){
        mostrapartite = false;
        oled.clearDisplay();
      }
    }
    
    if(millis() - t > tempotensione[eepromtensione - 1]){
      float semimedia1 = (4.20 * analogRead(cella1)) / 817.53479;
      delay(10);
      float semimedia2 = (8.40 * analogRead(vbat)) / 789.97765;
      delay(10);
      if(i >= NumeroCampioni){
        x = 0;
        y = 0;
        percentualeassoluta = 0;
        percentualerelativa = 0;
        for(int k = 0; k < NumeroCampioni; k++){
          x += CL1[k];
          y += Vbat[k];
        }
        x = (x / NumeroCampioni) * 1.00;
        y = (y / NumeroCampioni) * 1.00;
        percentualeassoluta = (100 * y) / 8.40;
        percentualerelativa = (100 * (y - 6.20)) / 2.20;
        if((lastpercentualeassoluta == 100) && (percentualeassoluta < 100)){
          oled.clearDisplay();
          lastpercentualeassoluta = percentualeassoluta;
        }
        else  lastpercentualeassoluta = percentualeassoluta;
        if((lastpercentualerelativa == 100) && (percentualerelativa < 100)){
          oled.clearDisplay();
          lastpercentualerelativa = percentualerelativa;
        }
        else  lastpercentualerelativa = percentualerelativa;
        if((lastpercentualerelativa >= 10) && (percentualerelativa < 10)){
          oled.clearDisplay();
          lastpercentualerelativa = percentualerelativa;
        }
        else  lastpercentualerelativa = percentualerelativa;
        errori(x, y-x, percentualerelativa);
        i = 0;
      }
      CL1[i] = semimedia1;
      Vbat[i] = semimedia2;
      i++;
      t = millis();
    }
    if(millis() - tempoe > temposalvataggio[eepromsalvataggio - 1]){
      if(lastpallini != pallini){
        if(nuovapartita == false){
          nuovapartita = true;
          partite[4] = partite[3];
          partite[3] = partite[2];
          partite[2] = partite[1];
          partite[1] = partite[0];
          EEPROM.put(gioco5, partite[4]);
          EEPROM.put(gioco4, partite[3]);
          EEPROM.put(gioco3, partite[2]);
          EEPROM.put(gioco2, partite[1]);
        }
        partite[0] = pallini;
        lastpallini = pallini;
        EEPROM.put(gioco1, partite[0]);
      } 
    }
   if(avviso == true){
    switch(numeroerrori){
      case 1:
              oled.setCursorXY(0,3);
              oled.printString("Differenza > 0.2");
              break;
      case 2:
              oled.setCursorXY(0,2);
              oled.printString("Differenza > 0.2");
              oled.setCursorXY(0,4);
              oled.printString("Percentuale < 20");
              break;
    }   
    if(millis() - tempoavviso > avvertenza){
      avviso = false;
      numeroerrori = 0;
      oled.clearDisplay();
      oled.setNormalDisplay();
    }
    if((digitalRead(avanti) == true) || (digitalRead(10) == HIGH)){
      avviso = false;
      numeroerrori = 0;
      oled.clearDisplay();
      oled.setNormalDisplay();
    }
   }
}

void errori(float Vcella1, float Vcella2, long percentuale){  
  if(abs(Vcella1 - Vcella2) > DifferenzaMassima){
    avviso = true;
    oled.clearDisplay();
    oled.setInverseDisplay();
    numeroerrori++;
    tempoavviso = millis();
  }
  if(percentuale < 25){
    avvertenza = (tempotensione[eepromtensione - 1] * 4) * 0.3;
    numeroerrori++;
  }
}
