// Упращение работы с моторами, улучшение читабельности кода
class motor {
  public:
    motor (byte direction, byte speed) {
      _direction = direction;
      _speed = speed;  
      pinMode(_direction, OUTPUT);
      pinMode(_speedControl,OUTPUT);
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
