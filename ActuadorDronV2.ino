#include <Servo.h>
#include "LoRa_E22.h"
#include <avr/wdt.h> 

#define PIN_RX 9
#define PIN_TX 10
#define AUX 8
#define M0 12
#define M1 11

LoRa_E22 e22ttl(PIN_RX,PIN_TX,AUX,M0,M1);
Servo dosificado_1;
Servo dosificado_2;

#define DOSI_1  5
#define DOSI_2  4
#define ledPin  13
#define marti1  15
#define marti2  16

int val= LOW;
bool send_wait = false;
 int temp_1 = LOW;
 int temp_2 = LOW;
uint8_t i=0,j=0;
uint8_t pestillo2 = 0;
ResponseContainer rc;
char datoo;
unsigned long  watchdog_time;
unsigned long  send_state_time;
unsigned long  mover_time;

char estado = 'w';
void(* resetFunc) (void) = 0; //declare reset function @ address 0
void setup() {

  pinMode(marti1,OUTPUT);
  pinMode(marti2,OUTPUT);
  pinMode(ledPin,OUTPUT);
  dosificado_1.attach(DOSI_1);
  dosificado_2.attach(DOSI_2);
  dosificado_1.write(90); 
  dosificado_2.write(90); 
  Serial.begin(115200);
  delay(500);
  e22ttl.begin();
  Serial.println("setup>Dron LoRa");
  delay(500);
  watchdog_time = millis();
  send_state_time = millis();
  
}

void loop(){
  
  //actualizar "datoo" al llegar nuevo dato  
  if (e22ttl.available()>1){
    Serial.println("Dato Recibido!");
    rc = e22ttl.receiveMessage();
    Serial.println(rc.data);
    datoo = rc.data[0];
    val = ! val;
    digitalWrite(ledPin,val);
    watchdog_time = millis();
    //e22ttl.sendMessage("w");
  }

  //Enviar el estado independientemente de lo que suceda cada segundo
  if(millis() - send_state_time > 1000){
    Serial.println("Enviar estado");
    send_state_time = millis();
    e22ttl.sendMessage(String(estado));
  }

  //Realizar accionamiento cada cierto tiempo
  if((millis()- mover_time) > 200){
    Serial.println("Accionamiento");
    mover_time = millis();
    mover(datoo);//accion dura un tiempo considerable
  }


  if((millis()-watchdog_time) > 20000){
    Serial.println("No ha recibido nada por 20s ");
    watchdog_time = millis();
    dosificado_1.write(90); 
    dosificado_2.write(90); 
    digitalWrite(marti1,LOW);
    digitalWrite(marti2,LOW);
    i=0;
    j=0;
    delay(500);
    resetFunc();  //call reset
    //reset();
  }
}

void rele1()
{
  temp_1 =! temp_1;
  digitalWrite(marti1,temp_1);
}

void rele2()
{
  temp_2 =! temp_2;
  digitalWrite(marti2,temp_2);
}


void mover(char dat){
   switch (dat){
    case 'A':
      dosificado_1.write(15);    // giro horario
      i = 1;
      estado = 'X';
      //e22ttl.sendMessage("X");
    break;
    
    case 'B':
      dosificado_1.write(90);//cerrado
      i=0;
      digitalWrite(marti1,LOW);
      estado = 'Y';
     //e22ttl.sendMessage("Y");
    break;  
    
    case 'C':
      dosificado_2.write(15);   // giro antihorario
      //Serial.println("dosificado 2> abierto");
      j = 1;
      estado = 'Z';
      //e22ttl.sendMessage("Z");
      //Serial.println("dosificado 1> cerrado");
    break;
    
    case 'D':
      dosificado_2.write(90);   // giro antihorario
      //Serial.println("dosificado 2> abierto");
      
      digitalWrite(marti2,LOW);
      j=0;
      //e22ttl.sendMessage("W");
      estado = 'W';
      //Serial.println("dosificado 1> cerrado");
    break;
    
    case 'g':
      dosificado_1.write(90); 
      dosificado_2.write(90); 
      digitalWrite(marti1,LOW);
      digitalWrite(marti2,LOW);
      i=0;
      j=0;
      estado = 'w';
  }

  if(i == 1)   rele1();
  if(j == 1)   rele2();

}