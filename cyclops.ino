// Определения всех портов находятся в отдельном файле
#include "definitions.h"

// библиотека управления ИК датчиком (Ссылку надо указать)
#include <IRremote.hpp>

// Небольшой класс, для удобной работы с двигателями
#include "motor.h"

motor rightWheel(1);// low - вперёд high - назад
motor leftWheel(0);// low - назад high - вперёд

#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб



int lastAA = -1;

// Общее значение яркости в помещении на момент включения
int dark;

//  я хз
bool fl = true, povFL = true, flfl = false, fl2 = true;

//  
uint32_t counterTurnovers = 0;

uint32_t speakerTime = -1, ttime = 0;

void setup() {

  // Открытие серийного порта
  Serial.begin(9600);
if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // hold in infinite loop
  }
 radio.begin();              // активировать модуль
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(1);   // размер пакета, в байтах

  radio.openWritingPipe(address[0]);  // мы - труба 0, открываем канал для передачи данных
  radio.openReadingPipe(1, address[1]);

  radio.powerUp();        // начать работу
  radio.printDetails();

  radio.setChannel(0x65);

  radio.stopListening();

  // Запуск отбработчика ик сигнала
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK, FEEDBACK_LED_PIN);
  
 
  // Бинд всех портов  
  pinMode(fotoRez,INPUT_PULLUP);
  pinMode(speaker, OUTPUT);
  pinMode(dat,INPUT_PULLUP);
  pinMode(tax1, INPUT_PULLUP);
  pinMode(tax2, INPUT_PULLUP);

  // стоковое колличество "яркости" в комнате
  dark = analogRead(fotoRez) + 10;

  // Serial.println(dark);

  // если слишком ярко, будет пищать спикер
  if(dark < maxBrightness){
    while(true){
      for(int i = 1; i <= 6; i++){
        digitalWrite(speaker,bool(i/2));
        delay(250);
      }
      delay(3000);
    }
  }
  //digitalWrite(speaker,LOW);
}

uint32_t k = millis(), vrema = millis(), ism = 0, trm12 = 0, t =0;

uint32_t trm13 = 0;

// переделать
bool backFlak = false, forvardFlak = false, leftFlak = false, rightFlak = false, stopFlak = true, startFlak = true, help = false;;
void loop() {

  // Яркость вокруг в данный момент времени
  int AA = analogRead(fotoRez);

  // Работаем со значениями только с dAA большем 7
  if (AA != lastAA && abs(AA - lastAA) > 7){

    lastAA = AA;
    //Serial.println(AA);
    
       
      if (dark > AA){
        // Если светло - едем к свету
        forvard();
        
        if (povFL){
            counterTurnovers++;
            povFL = false;
        }

        fl = false;
        
        rightWheel.speed(100);
        leftWheel.speed(100);
        
      }else{
        //Serial.println("non"); 
        // if(!fl){
          rightWheel.speed(0);
          leftWheel.speed(0);
          rightWheel.go();
          leftWheel.go(); 
          digitalWrite(speaker,HIGH);
          speakerTime = millis();
        // }
        
        // Если темно - крутимся, чтобы найти свет
        if(counterTurnovers % 4 == 1 || counterTurnovers % 4 == 2){
          leftG();
          povFL = true;
        }else if (counterTurnovers % 4 == 3 || counterTurnovers % 4 == 0){
          rightG();
          povFL = true;
        }
        if (counterTurnovers == 4){
          counterTurnovers = 0;
        }
        rightWheel.speed(60);
        leftWheel.speed(60);
        fl = true;
      }
    
  }

  // Если "яркость" на придельном значении и больше мы будем стоять (Банально чтобы не снести лампочку) 
  if (AA < maxBrightness){
      rightWheel.speed(0);
      leftWheel.speed(0);
  } 

// Кусок код отвечающий за работу тахометров
bool rig = digitalRead(tax1), ta2 = digitalRead(tax2); 

if (rig){
    flfl = true;
}else{
    if (flfl){
      flfl = false;
      byte ob = 15000/(millis()-ttime);
      if(radio.write(&ob, 1)){
        Serial.println("pass");
      }
      Serial.println(ob);
      ttime = millis();
    }
}

// Если колесо не крутится, едем назад 
if (millis()-ttime > 400){
      t = millis();
      help = true;
      ttime = millis();
}

if (millis() - t < 250){
  back();
}
if (millis() - t < 500 && millis() - t > 250){
  leftG();
}
  // Пищалка пищит пол секунды и перестаёт
  if (millis() - speakerTime > 100){
    digitalWrite(speaker,LOW);
  } 
  
  // Работа с ИК датчиком
  if (IrReceiver.decode()) {

        //IrReceiver.printIRResultShort(&Serial);

        IrReceiver.resume(); 
        
        if (IrReceiver.decodedIRData.command == 0xC) {
            trm12 = millis();
            backFlak = true;
        } else if (IrReceiver.decodedIRData.command == 0x18) {
            trm12 = millis();
            rightFlak = true;
        } else if (IrReceiver.decodedIRData.command == 0x5E) {
            trm12 = millis();
            leftFlak = true;
        } else if (IrReceiver.decodedIRData.command == 0x16) {
            trm12 = millis();
            forvardFlak = true;
        } else if (IrReceiver.decodedIRData.command == 0x9) {
          if (stopFlak && millis() - trm13 >= 200){
            trm13 = millis();
            stopFlak = false;
          }else{
            stopFlak = true;
          }
            
        }
    }
if (stopFlak){
    startFlak = true;
    rightWheel.speed(0);
    leftWheel.speed(0);
  } 

else if(startFlak){
    rightWheel.speed(100);
    leftWheel.speed(100);
    startFlak = false;
  }
  
  if (backFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(100);
      leftWheel.speed(100);
      back();
    }else{
      forvard();
      backFlak = false;
    }
    
  }
  
  if (forvardFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(100);
      leftWheel.speed(100);
      forvard();
    }else{
      forvard();
      forvardFlak = false;
    }

  }
  
  if (leftFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(100);
      leftWheel.speed(100);
      leftG();
    }else{
      forvard();
      leftFlak = false;
    }

  }

  if (rightFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(100);
      leftWheel.speed(100);
      rightG();
    }else{
      forvard();
      rightFlak = false;
    }

  }
  
  // Крутим колёса
  rightWheel.go();
  leftWheel.go(); 
}



void forvard(){
  rightWheel.derection(LOW);
  leftWheel.derection(HIGH);
}

void back(){
  rightWheel.derection(HIGH);
  leftWheel.derection(LOW);
}

void leftG(){
  rightWheel.derection(HIGH);
  leftWheel.derection(HIGH);
}

void rightG(){
  rightWheel.derection(LOW);
  leftWheel.derection(LOW);
}
