// Кодировка данных, принимаемых ИК датчиком
#define DECODE_NEC 
#define IR_RECEIVE_PIN 2
#define FEEDBACK_LED_PIN 0

// Определение портов датчиков
#define PHOTORES_PIN A3
#define SPEAKER_PIN A4
#define dat A1
#define TACHOMETER_PIN 1
#define CE_RADIO_PIN 9
#define CSN_RADIO_PIN 10
// Не работает татчик, исправить
#define tax2 8 

// Адреса труб радио модуля
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

// Значение "яркости" при котором робот останавливается
#define maxBrightness 40

// Пины двигателей
#define SPEED_1      5 
#define DIR_1        4
 
#define SPEED_2      6
#define DIR_2        7