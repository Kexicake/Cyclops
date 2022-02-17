// Все пины и определения вынесенны в отдельный файл
#include "definitions.h"

// библиотека управления ИК датчиком 
#include <IRremote.hpp>

// Небольшой класс, для удобной работы с двигателями
#include "motor.h"
// Создания экземпляров класса работы с колёсами 
motor rightWheel(1);// low - вперёд high - назад
motor leftWheel(0);// low - назад high - вперёд

// библиотека для работы с шиной SPI
#include <SPI.h>

// библиотеки радиомодуля         
#include "nRF24L01.h"     
#include "RF24.h"               

// Создания экземпляра класса радио модуля 
RF24 radio(CE_RADIO_PIN, CSN_RADIO_PIN);

// Массив передоваемой информации передатчиком
int data[3]= {0,0,0};  
 
// Общее значение яркости в помещении на момент включения
int dark; 

//  флажки для работы глаза и поворотов
bool povFL = true, flfl = false, fl2 = true, speakerWork = true, flfl2 = false;

// Переменная, отвечающая за пеменное кручение робота во время нахождения темноты/припятствия
uint32_t counterTurnovers = 0;

// Последнее обработанное значение яркости поверхности
uint32_t lastBrightness = -1;

// Основные временные переменные
uint32_t speakerTime = -1, tachometerTime = 0, IRdriveTime = 0, backTime = 0, startScriptTime = 0;
uint32_t test = -1;
// Стандартная скорость вращения колёс
int speed = 100;

// Флажки необходимые для работы пульта
bool backFlak = false, forvardFlak = false, leftFlak = false, rightFlak = false, stopFlak = true, startFlak = true;

// Функции движения 
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

void setup() {
  // Открытие серийного порта
  Serial.begin(9600);
  radio.begin();              // активировать модуль
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.setChannel(0x65);     // Канал, на котором будет работать датчик
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(3);   // размер пакета, в байтах
  radio.openWritingPipe(address[0]);  //открываем канал для передачи данных
  radio.openReadingPipe(1, address[1]); // Канал для приём ответа
  radio.powerUp();        // начать работу nRF2401+
  
  radio.stopListening(); // слушать эфир 

  // Запуск отбработчика ик сигнала
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK, FEEDBACK_LED_PIN);
  
 
  // Бинд всех портов  
  pinMode(PHOTORES_PIN,INPUT_PULLUP);
  // pinMode(dat,INPUT_PULLUP);
  pinMode(TACHOMETER_PIN, INPUT_PULLUP);
  // pinMode(tax2, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);
  
  // Иначе начинается бесконечная перезагрузка
  digitalWrite(RESTART_PIN,1);
  pinMode(RESTART_PIN, OUTPUT);
  // стоковое колличество "яркости" в комнате (чем больше значение, тем дальше он заедет в темноту)
  dark = analogRead(PHOTORES_PIN) + 10;

  // если слишком ярко, будет пищать спикер
  if(dark < maxBrightness){
    while(true){
      for(int i = 1; i <= 6; i++){
        digitalWrite(SPEAKER_PIN,bool(i/2));
        delay(250);
      }
      delay(3000);
    }
  }
  digitalWrite(SPEAKER_PIN,LOW);
}

void loop() {
if(!stopFlak){
  // Яркость вокруг в данный момент времени
  int brightness = analogRead(PHOTORES_PIN);

  // Работаем с изменением яркости больше 7
  if (brightness != lastBrightness && abs(brightness - lastBrightness) > 7){

    lastBrightness = brightness;

    //Serial.println(brightness);

      // Если светло - едем к свету, иначе - крутимся
      if (dark > brightness){
        // Едем вперёд
        forvard();
        
        // Это условие отвечает за переменное вращение в разные стороны при поиске света
        if (povFL){
            counterTurnovers++;
            povFL = false;
        }
        rightWheel.speed(speed);
        leftWheel.speed(speed);
      }else{
          rightWheel.speed(0);
          leftWheel.speed(0);
          rightWheel.go();
          leftWheel.go(); 
          // Это условие отвечает за работу спикера, который пищит, если мы натыкаемся наприпятствие
          if (speakerWork){
            digitalWrite(SPEAKER_PIN,HIGH);
          }
          speakerTime = millis();
          
        // Если темно - крутимся, чтобы найти свет
        if(counterTurnovers % 4 == 1 || counterTurnovers % 4 == 2){
          leftG();
          povFL = true;
        }else{
          rightG();
          povFL = true;
        }
        // Обнуляем переменную, чтобы она не занимала место в памяти
        if (counterTurnovers == 4){
          counterTurnovers = 0;
        }
        rightWheel.speed(speed - 40);
        leftWheel.speed(speed - 40);
      }
    
  }

  // Если "яркость" на придельном значении и больше мы будем стоять (Банально чтобы не снести лампочку) 
  if (brightness < maxBrightness){
      rightWheel.speed(0);
      leftWheel.speed(0);
  } 

// Кусок код отвечающий за работу тахометров
bool ta1 = digitalRead(TACHOMETER_PIN), ta2 = digitalRead(tax2); 

if (ta1){
    flfl = true;
}else{
    if (flfl){
      flfl = false;
      data[2] = 15000/(millis()-tachometerTime);
      // radio.write(&ob, 1);
      tachometerTime = millis();
    }
}
// if (ta2){
//     flfl2 = true;
// }else{
//     if (flfl2){
//       flfl2 = false;
//       data[3] =  15000/(millis()-ttime2);
//       // radio.write(&ob, 1);
//       ttime2 = millis();
//     }
// }
// Если колесо не крутится, едем назад ~256мс надо чтобы определить крутится ли колесо
if (millis() - tachometerTime > 300){
      backTime = millis();
      tachometerTime = millis();
}

// ОБработка остановки колёс
if (millis() - backTime < 1000){
  if (millis() - backTime > 700){
    leftG();
  }else{
    back();
  } 
}

  // Пищалка пищит 1/10 секунды и перестаёт
  if ((millis() - speakerTime) >= 100 && speakerWork){
    digitalWrite(SPEAKER_PIN,LOW);
  } else if (!speakerWork){
    digitalWrite(SPEAKER_PIN, LOW);
  }
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
    // IrReceiver.printIRResultShort(&Serial);
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
          IRdriveTime = millis();
          backFlak = true;
          break;

        case 0x8:
          IRdriveTime = millis();
          rightFlak = true;
          break;
          
        case 0x5A:
          IRdriveTime = millis();
          leftFlak = true;
          break;

        case 0x18:
          IRdriveTime = millis();
          forvardFlak = true;
          break;
        case 0x9:
          speakerWork = !speakerWork;
          break;
        case 0x43:
          if (stopFlak && millis() - startScriptTime >= 200){
            startScriptTime = millis();
            stopFlak = false;
          }else{
            stopFlak = true;
          } 
          break;
        case 0x46:
          dark = analogRead(PHOTORES_PIN) + 10;
          break;
        case 0x16:
          digitalWrite(RESTART_PIN,0);
          break;
        default:
          break;
        }
}

if (stopFlak){
    startFlak = true;
    digitalWrite(SPEAKER_PIN,LOW);
    rightWheel.speed(0);
    leftWheel.speed(0);
  } 

else if(startFlak){
    rightWheel.speed(speed);
    leftWheel.speed(speed);
    startFlak = false;
  }
  
  if (backFlak){
    if (millis() - IRdriveTime <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
      back();
    }else{
      forvard();
      backFlak = false;
    }
    
  }
  
  if (forvardFlak){
    if (millis() - IRdriveTime <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
      forvard();
    }else{
      forvard();
      forvardFlak = false;
    }

  }
  
  if (leftFlak){
    if (millis() - IRdriveTime <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
      leftG();
    }else{
      forvard();
      leftFlak = false;
    }

  }

  if (rightFlak){
    if (millis() - IRdriveTime <= 100){
      rightWheel.speed(speed);
      leftWheel.speed(speed);
      rightG();
    }else{
      forvard();
      rightFlak = false;
    }

  }
  
  // Функция отправки информации  
  radio.write(&data, 3);

  // Крутим колёса
  rightWheel.go();
  leftWheel.go(); 
}