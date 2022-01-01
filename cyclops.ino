// Кодировка данных, принимаемых ИК датчиком
#define DECODE_NEC 
#define IR_RECEIVE_PIN 2
#define FEEDBACK_LED_PIN 13

// библиотека управления ИК датчиком
#include <IRremote.hpp>

// мои собственные классы, для облегчения работы с кодом
// #include "button.h"
#include "motor.h"


// Тестовое управдение двигателями посредством 4-х кнопок
// button up(11); 
// button right(10);
// button left(12);
// button down(13);

motor rightWheel(1);// low - вперёд high - назад
motor leftWheel(0);// low - назад high - вперёд

int lastAA = -1;

// Общее значение яркости в помещении на момент включения
int dark;

// Значение "яркости" при котором робот останавливается
#define maxBrightness 40

//  я хз
bool fl = true, povFL = true,flfl = false;

//  
uint32_t pov = 0;

uint32_t dtime = -1, ttime = 0;

// Определение портов датчиков
#define fotoRez A5
#define signa 13
#define dat A1
#define tax1 10
#define tax2 8

int edet = 0;

void setup() {

  // Открытие серийного порта
  Serial.begin(115200);

  // Запуск отбработчика ик сигнала
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK, FEEDBACK_LED_PIN);
  
  // Бинд всех портов  
  pinMode(fotoRez,INPUT_PULLUP);
  pinMode(signa, OUTPUT);
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
        digitalWrite(signa,bool(i/2));
        delay(250);
      }
      delay(3000);
    }
  }
  //digitalWrite(signa,LOW);
}

uint32_t k = millis(), vrema = millis(), ism = 0, trm12 = 0, t =0;

uint32_t trm13 = 0;

// переделать
bool backFlak = false, forvardFlak = false, leftFlak = false, rightFlak = false, stopFlak = true, startFlak = true, help = false;;

void loop() {

  //Serial.println(analogRead(dat));

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
            pov++;
            povFL = false;
          }
          
        rightWheel.speed(100);
        leftWheel.speed(100);
        
      }else{
        //Serial.println("non"); 
        if(!fl){
          
          
          rightWheel.speed(0);
          leftWheel.speed(0);
          rightWheel.go();
          leftWheel.go(); 
          digitalWrite(signa,HIGH);
          dtime = millis();
        }
        
        // Если темно - крутимся, чтобы найти свет
        if(pov % 4 == 1 || pov % 4 == 2){
          leftG();
          povFL = true;
        }else if (pov % 4 == 3 || pov % 4 == 0){
          rightG();
          povFL = true;
        }
        if (pov == 4){
          pov = 0;
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

bool rig = digitalRead(tax1), ta2 = digitalRead(tax2); 
  if (rig){
    flfl = true;
  }else{
    if (flfl){
      flfl = false;
      Serial.println(60000/(millis()-ttime));
    ttime = millis();
    }
  }
  
if (millis()-ttime > 500){
      t = millis();
      help = true;
      ttime = millis();
}

if (millis() - t < 100){
  back();
}
  // Пищалка пищит пол секунды и перестаёт
  if (millis() - dtime > 500){
    digitalWrite(signa,LOW);
  }
  
  // Работа с ИК датчиком
  if (IrReceiver.decode()) {

        //IrReceiver.printIRResultShort(&Serial);

        IrReceiver.resume(); // Enable receiving of the next value
        
        // подписать все кнопки пульта
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
  } 

else if(startFlak){
    rightWheel.speed(50);
    leftWheel.speed(50);
    startFlak = false;
  }
  
  if (backFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(60);
      leftWheel.speed(60);
      back();
    }else{
      forvard();
      backFlak = false;
    }
    
  }
  
  if (forvardFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(60);
      leftWheel.speed(60);
      forvard();
    }else{
      forvard();
      forvardFlak = false;
    }

  }
  
  if (leftFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(60);
      leftWheel.speed(60);
      leftG();
    }else{
      forvard();
      leftFlak = false;
    }

  }

  if (rightFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(60);
      leftWheel.speed(60);
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
