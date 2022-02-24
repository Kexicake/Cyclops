#define SPEED_1      5 
#define DIR_1        4
 
#define SPEED_2      6
#define DIR_2        7

// Упращение работы с моторами, улучшение читабельности кода
// Пин регулировки скорости первого двигателя - 5
// Пин направления первого двигателя - 4
// Пин регулировки скорости Второго двигателя - 6
// Пин направления второго двигателя - 7
class motor {
  public:
    motor (int direction, int speed) {
      _direction = direction;
      _speed = speed;  
      pinMode(_direction, OUTPUT);
      pinMode(_speedControl,OUTPUT);
    }
    // motor (bool i) : (i)?motor(DIR_2,SPEED_2):motor(DIR_1,SPEED_1){
    //   // if (!i){
    //   //   motor(DIR_1,SPEED_1);
    //   //   // _direction = DIR_1;
    //   //   // _speedControl = SPEED_1;  
    //   //   // pinMode(_direction, OUTPUT);
    //   //   // pinMode(_speedControl,OUTPUT);
    //   // }else{
    //   //   motor(DIR_2,SPEED_2);
    //   //   // _direction = DIR_2;
    //   //   // _speedControl = SPEED_2;  
    //   //   // pinMode(_direction, OUTPUT);
    //   //   // pinMode(_speedControl,OUTPUT);
    //   // }
    // }
    
    void speed(int speed){
      _speed = speed;
    }
    void derection(byte direction){
      digitalWrite(_direction,direction);
    }
    void go(){
      analogWrite(_speedControl, _speed);
    }
    void stop(){
      analogWrite(_speedControl, 0);
    }
    //Типо когда нажимаешь или удерживаешь возвращаеться True
  private:
    byte _direction;
    byte _speedControl;
    byte _speed;
};