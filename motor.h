// Упращение работы с моторами, улучшение читабельности кода
// <param = a> fa
class motor {
  public:
    motor (byte direction, byte speed) {
      _direction = direction;
      _speed = speed;  
      pinMode(_direction, OUTPUT);
      pinMode(_speedControl,OUTPUT);
    }
    motor (bool i){
      if (!i){
        _direction = DIR_1;
        _speedControl = SPEED_1;  
        pinMode(_direction, OUTPUT);
        pinMode(_speedControl,OUTPUT);
      }else{
        _direction = DIR_2;
        _speedControl = SPEED_2;  
        pinMode(_direction, OUTPUT);
        pinMode(_speedControl,OUTPUT);
      }
    }
    
    void speed(int speed){
      _speed = speed;
    }
    void derection(byte direction){
      digitalWrite(_direction,direction);
    }
    void go(){
      analogWrite(_speedControl, _speed);
    }

    //Типо когда нажимаешь или удерживаешь возвращаеться True
  private:
    byte _direction;
    byte _speedControl;
    byte _speed;
};
