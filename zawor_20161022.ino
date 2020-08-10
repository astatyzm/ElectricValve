//TRANSMITTER
#include <VirtualWire.h>
#include <OneWire.h>

  int odczyt=0;
  int stan_poprzedni = 0;
  int licznik = 0;
  int pin = 2; // pin przycisku
  int program = 0; // wybór programu, 1- 45 st, 0 - 55 stopni C. 
  int temp_przelaczenia = 0;

OneWire  ds(10);

void setup()
{
   Serial.begin(9600);
   // vw_set_ptt_inverted(true);  // Required by the RF module
    vw_setup(2000);            // bps connection speed
    vw_set_tx_pin(3);         // Arduino pin to connect the receiver data pin
     pinMode(9, OUTPUT); //pin od elektrozaworu, dający 12V 
     pinMode(2, INPUT);  //pin wyboru sterownia 45/55 stopni - przycisk
     pinMode(8, OUTPUT); //dioda niebieska
     pinMode(7, OUTPUT); //dioda zielona
     
}
void loop()
{
  
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
 
      /*Jak na monitorze portu szeregowego nie ma danych o temperaturze, tzn
    że pin jest źle podpięty na płytce stykowej!!!*/
    if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  
  /*musi byc, bez tego nie mierzy temp*/
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();

  }
 
  
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  Serial.print("  Temperatura = ");
  Serial.print(celsius);
  Serial.print(" *Celciusza");



  // * * * G U Z I K * * * 
  //obsługa przycisku
  odczyt = digitalRead (2);
  Serial.println ("odczyt");
  Serial.print (odczyt);
   Serial.println();
  delay(50);
   
  if (odczyt != stan_poprzedni)
  {
    if (odczyt == HIGH)
      {
       licznik ++ ;
       Serial.print ("licznik   ");
       Serial.print (licznik);
       Serial.println();
      }
     }
  stan_poprzedni = odczyt;
  
   if ( licznik % 2 == 0)
     { 
   //    program = 1;
       temp_przelaczenia = 55 ;
       digitalWrite (13, HIGH);
       digitalWrite(8, HIGH);
       digitalWrite(7, LOW);
       }
  else { 
    //     program = 2;
         temp_przelaczenia = 45;
         digitalWrite (13, LOW);
         digitalWrite(8, LOW);
         digitalWrite(7, HIGH); 
         }
      
  // koniec guzika.
    if(celsius >= temp_przelaczenia)
      {
       digitalWrite(9, HIGH); //otwiera zawór
        }
    
        if(celsius < temp_przelaczenia)
      {
         digitalWrite(9, LOW); //zamyka zawór
        }
  
 
  
  /****************************************************/
   //Message to send:
   char pomiar[6];
   dtostrf(celsius, 6, 2, pomiar);
   vw_send((uint8_t *)pomiar, strlen(pomiar));
   vw_wait_tx();        // We wait to finish sending the message
   delay(200);         // We wait to send the message again               
}

