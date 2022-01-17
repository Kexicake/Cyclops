// Кодировка данных, принимаемых ИК датчиком
#define DECODE_NEC 
#define IR_RECEIVE_PIN 2
#define FEEDBACK_LED_PIN 0

// Определение портов датчиков
#define fotoRez A3
#define speaker A4
#define dat A1
#define tax1 1
#define tax2 8

// Значение "яркости" при котором робот останавливается
#define maxBrightness 40

// библиотека управления ИК датчиком 
#include <IRremote.hpp>

// Небольшой класс, для удобной работы с двигателями
#include "motor.h"

motor rightWheel(1);// low - вперёд high - назад
motor leftWheel(0);// low - назад high - вперёд

#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб


// Общее значение яркости в помещении на момент включения
int dark;

//  флажки для работы глаза и поворотов
bool fl = true, povFL = true, flfl = false, fl2 = true, speakerWork = true;

//  Основные временные пременные
uint32_t counterTurnovers = 0, lastAA = -1;
uint32_t speakerTime = -1, ttime = 0;
uint32_t k = millis(), vrema = millis(), ism = 0, trm12 = 0, t =0;
uint32_t trm13 = 0;

// Стандартная скорость вращения колёс
int speed = 100;

// Флажки необходимые для работы пульта
bool backFlak = false, forvardFlak = false, leftFlak = false, rightFlak = false, stopFlak = true, startFlak = true;

void setup() {

  // Открытие серийного порта
  Serial.begin(9600);
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // Если радиомодуль не исправен мы не запустимся
  }
 radio.begin();              // активировать модуль
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(1);   // размер пакета, в байтах

  radio.openWritingPipe(address[0]);  //открываем канал для передачи данных
  radio.openReadingPipe(1, address[1]); // Канал для приём ответа

  radio.powerUp();        // начать работу nRF2401+
  

  radio.setChannel(0x65);
  
  radio.stopListening(); // Прекрпщаем слушать эфир мы - передатчик

  // Запуск отбработчика ик сигнала
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK, FEEDBACK_LED_PIN);
  
 
  // Бинд всех портов  
  pinMode(fotoRez,INPUT_PULLUP);
  pinMode(speaker, OUTPUT);
  pinMode(dat,INPUT_PULLUP);
  pinMode(tax1, INPUT_PULLUP);
  pinMode(tax2, INPUT_PULLUP);

  // стоковое колличество "яркости" в комнате (чем больше значение, тем дальше он заедет в темноту)
  dark = analogRead(fotoRez) + 10;

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
  digitalWrite(speaker,LOW);
}

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
        
        rightWheel.speed(speed);
        leftWheel.speed(speed);
        
      }else{
          rightWheel.speed(0);
          leftWheel.speed(0);
          rightWheel.go();
          leftWheel.go(); 
          if (speakerWork){
            digitalWrite(speaker,HIGH);
          }
          
          speakerTime = millis();
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
        rightWheel.speed(speed - 40);
        leftWheel.speed(speed - 40);
        fl = true;
      }
    
  }

  // Если "яркость" на придельном значении и больше мы будем стоять (Банально чтобы не снести лампочку) 
  if (AA < maxBrightness){
      rightWheel.speed(0);
      leftWheel.speed(0);
  } 

// Кусок код отвечающий за работу тахометров
bool ta1 = digitalRead(tax1), ta2 = digitalRead(tax2); 

if (ta1){
    flfl = true;
}else{
    if (flfl){
      flfl = false;
      byte ob = 15000/(millis()-ttime);
      radio.write(&ob, 1);
      ttime = millis();
    }
}

// Если колесо не крутится, едем назад ~256мс надо чтобы определить крутится ли колесо
if (millis() - ttime > 300){
      t = millis();
      ttime = millis();
}
// ОБработка остановки колёс
if (millis() - t < 500){
  if (millis() - t > 250){
    leftG();
  }else{
    back();
  } 
}

  // Пищалка пищит 1/10 секунды и перестаёт
  if ((millis() - speakerTime) >= 100 && speakerWork){
    digitalWrite(speaker,LOW);
  } else if (!speakerWork){
    digitalWrite(speaker, LOW);
  }
  
  // Работа с ИК датчиком
  if (IrReceiver.decode()) {
    // Пульт кнопки
    // 0x45 0x46 0x47
    // 0x44 0x40 0x43
    // 0x7 0x15 0x9
    // 0x16 0x19 0xD
    // 0xC 0x18 0x5E
    // 0x8 0x1C 0x5A
    // 0x42 0x52 0x4A
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.resume(); 
        switch (IrReceiver.decodedIRData.command)
        {
        case 0x15:
          if (speed < 250){
            speed = speed + 10;
          }
          break;
        case 0x7:
          if (speed > 0 ){
            speed = speed - 10;
          }
          break;
        case 0x52:
          trm12 = millis();
          backFlak = true;
          break;

        case 0x8:
          trm12 = millis();
          rightFlak = true;
          break;
          
        case 0x5A:
          trm12 = millis();
          leftFlak = true;
          break;

        case 0x18:
          trm12 = millis();
          forvardFlak = true;
          break;
        case 0x9:
          speakerWork = !speakerWork;
          break;
        case 0x43:
          if (stopFlak && millis() - trm13 >= 200){
            trm13 = millis();
            stopFlak = false;
          }else{
            stopFlak = true;
          } 
          break;

        default:
          break;
        }
    }

if (stopFlak){
    startFlak = true;
    rightWheel.speed(0);
    leftWheel.speed(0);
  } 

else if(startFlak){
    rightWheel.speed(speed);
    leftWheel.speed(speed);
    startFlak = false;
  }
  
  if (backFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
      back();
    }else{
      forvard();
      backFlak = false;
    }
    
  }
  
  if (forvardFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
      forvard();
    }else{
      forvard();
      forvardFlak = false;
    }

  }
  
  if (leftFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
      leftG();
    }else{
      forvard();
      leftFlak = false;
    }

  }

  if (rightFlak){
    if (millis() - trm12 <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
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
