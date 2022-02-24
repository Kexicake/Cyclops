// Все пины и определения вынесенны в отдельный файл
#include "definitions.h"

// библиотека управления ИК датчиком 
#include <IRremote.hpp>

// Небольшой класс, для удобной работы с двигателями
#include "motor.h"
// Создания экземпляров класса работы с колёсами 
motor rightWheel(DIR_2,SPEED_2);// low - вперёд high - назад
motor leftWheel(DIR_1,SPEED_1);// low - назад high - вперёд

// библиотека для работы с шиной SPI
// #include <SPI.h>

// библиотеки радиомодуля         
// #include "nRF24L01.h"     
// #include "RF24.h"               

// Создания экземпляра класса радио модуля 
// RF24 radio(CE_RADIO_PIN, CSN_RADIO_PIN);

// Массив передоваемой информации передатчиком
// int data[3]= {0,0,0};  
 
// Общее значение яркости в помещении на момент включения
int dark; 

//  флажки для работы глаза и поворотов
bool povFL = true, flfl = false, fl2 = true, speakerWork = true, flfl2 = false;

// Переменная, отвечающая за попеменное кручение робота во время нахождения темноты/припятствия
uint32_t counterTurnovers = 0;

// Последнее обработанное значение яркости поверхности
uint32_t lastBrightness = -1;

// Основные временные переменные
uint32_t speakerTime = -1, tachometerTime = 0, IRdriveTime = 0, backTime = 0, startScriptTime = 0;

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

void go(){
  rightWheel.go();
  leftWheel.go(); 
}

void stop(){
  rightWheel.stop();
  leftWheel.stop();
}

void speedW(int right, int left){
    rightWheel.speed(right);
    leftWheel.speed(left);
}

void speedW(int speedW){
  rightWheel.speed(speedW);
  leftWheel.speed(speedW);
}

void nRF24L01_SETUP(){
  radio.begin();              // активировать модуль
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.setChannel(0x65);     // Канал, на котором будет работать датчик
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(3);   // размер пакета, в байтах
  radio.openWritingPipe(address[0]);  //открываем канал для передачи данных
  radio.openReadingPipe(1, address[1]); // Канал для приём ответа
  radio.powerUp();        // начать работу nRF2401+
}

void photorez_SETUP(){
  pinMode(PHOTORES_PIN,INPUT_PULLUP);
  // стоковое колличество "яркости" в комнате (чем больше значение, тем дальше он заедет в темноту)
  dark = analogRead(PHOTORES_PIN) + 10;
}

void tachometer(){
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
  if (millis() - backTime < 500){
    if (millis() - backTime > 300){
      leftG();
    }else{
      back();
    } 
  }

}

void moveControl(){
  // Яркость вокруг в данный момент времени
  int brightness = analogRead(PHOTORES_PIN);

  // Работаем с изменением яркости больше 7
  if (brightness != lastBrightness && abs(brightness - lastBrightness) > deltaBrightness){
    lastBrightness = brightness;

    // Если светло - едем к свету, иначе - крутимся
    if (dark > brightness){

      // Едем вперёд
      forvard();
      // Это условие отвечает за переменное вращение в разные стороны при поиске света
      if (povFL && !(povFL = !povFL)){
          counterTurnovers++;
      }
      speedW(speed);

    }else{

        stop();

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
      // Обнуляем переменную, чтобы она не перегружалась
      if (counterTurnovers == 4){
        counterTurnovers = 0;
      }
      speedW(speed - 40);
    }
}

  // Если "яркость" на придельном значении и больше мы будем стоять (Банально чтобы не снести лампочку) 
  if (brightness < maxBrightness){stop();} 

  // Кусок код отвечающий за работу тахометров
  tachometer();

  // Пищалка пищит 1/10 секунды и перестаёт
  speaker();
}

void speaker(){
  if ((millis() - speakerTime) >= 100 && speakerWork){
    digitalWrite(SPEAKER_PIN,LOW);
  } else if (!speakerWork){
    digitalWrite(SPEAKER_PIN, LOW);
  }
}

void moveControlIR(){

  if (stopFlak){
    startFlak = true;
    digitalWrite(SPEAKER_PIN,LOW);
    stop();
} 

else if(startFlak){
    speedW(speed);
    startFlak = false;
}
  
if (backFlak){
  if (millis() - IRdriveTime <= 100){
    speedW(speed);
    back();
  }else{
    forvard();
    backFlak = false;
  }
}

if (forvardFlak){
  if (millis() - IRdriveTime <= 100){
    speedW(speed);
    forvard();
  }else{
    forvard();
    forvardFlak = false;
  }

}

if (leftFlak){
  if (millis() - IRdriveTime <= 100){
    speedW(speed);
    leftG();
  }else{
    forvard();
    leftFlak = false;
  }

}

if (rightFlak){
  if (millis() - IRdriveTime <= 100){
    speedW(speed);
    rightG();
  }else{
    forvard();
    rightFlak = false;
  }

}
}

void IRdecoder(uint16_t command){
switch (command)
{
case 0x15:
  speed = (speed<250)?speed+10:speed;
  break;
case 0x7:
  speed = (speed>0)?speed-10:speed;
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

void ERROR(){
while(true){
    for(int i = 1; i <= 6; i++){
      digitalWrite(SPEAKER_PIN,bool(i/2));
      delay(250);
    }
    delay(3000);
  }
}

void setup() {
  // Открытие серийного порта
  Serial.begin(9600);

  // nRF24L01_SETUP();
  
  // radio.stopListening(); // не слушать эфир 

  // Запуск отбработчика ик сигнала
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK, FEEDBACK_LED_PIN);
 
  // Бинд всех портов  
  // pinMode(dat,INPUT_PULLUP);
  pinMode(TACHOMETER_PIN, INPUT_PULLUP);
  // pinMode(tax2, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);
  
  // Иначе начинается бесконечная перезагрузка
  digitalWrite(RESTART_PIN,1);
  pinMode(RESTART_PIN, OUTPUT);
  
  photorez_SETUP();

  // если слишком ярко, будет пищать спикер
  if (dark < maxBrightness){ERROR();}
  
  digitalWrite(SPEAKER_PIN,LOW);
}

void loop() {
  if(!stopFlak){moveControl();}

  // Работа с ИК датчиком
  if (IrReceiver.decode()) {
    IrReceiver.resume(); 
    IRdecoder(IrReceiver.decodedIRData.command);
  }
  
  // Управление роботом через пульт
  moveControlIR();

  // Функция отправки информации  !!!НЕ ВКЛЮЧАТЬ, ЛЕТИТ ВСЯ ПРОГРАММА ИЗ_ЗА ТОГО, ЧТО ВРЕМЯ НА ЭТУ ФУНКЦИЮ НУЖНО НЕМАЛО!!!
  // radio.write(&data, 3);
  
  // Крутим колёса
  go();
}