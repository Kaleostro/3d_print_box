
#include "GyverButton.h"
//#include "param_ferm.h"
#include <EEPROM.h>  
#include <OLED_I2C.h>                               // Подключаем библиотеку OLED_I2C для работы со шрифтами и графикой

OLED  myOLED(20, 21, 20);                           // Определяем пины I2C интерфейса: UNO и NANO -- SDA - пин A4, SCL - пин A5; MEGA -- SDA - пин 20, SCL - пин 21

extern uint8_t RusFont[];                           // Подключаем русский шрифт
extern uint8_t SmallFont[];                         // Подключаем латинский шрифтом
extern uint8_t SmallFontRus[];

const int PIN_Y = 9;
const int PIN_B = 10;
const int PIN_R = 11;

const int REL_OPEN  = LOW;
const int REL_CLOSE = HIGH;

int RELE_1      = 5;
int RELE_1_st   = REL_CLOSE;

//par_f       ter1(0);

class Line {
  public:

  Line(int _type = 3, String _name="name", int _ivalue = 0, bool _bvalue = false, bool _check = false ){
      f_type       = _type;
      f_name       = _name;
      f_check      = _check;  
      f_int_value  = _ivalue;
      f_bool_value = _bvalue;        
  };
  
 String gettext(){
    String text = f_name+": ";
   
   switch(f_type)
    {
      case 1:
        text += ""; 
      break; 
      case 2: 
        text +=f_int_value; 
      break; 
      case 3:
        text +=(f_bool_value?"on":"off");
      break;
      default:
        text = text +"";
    }
    return text;
  }; 

  bool getCheck(){
    return f_check;
  };
  
  void setValue(int _value){
    switch(f_type)
     {
       case 2: 
         setIntValue(_value);
       break; 
       case 3:
         setBoolValue(_value==1);
       break;
     }
  };

  int getValue(){
    int res;
    switch(f_type)
     {       
       case 2: 
         res = getIntValue();
       break; 
       case 3:
         res = (getBoolValue()?1:0);
       break;
     }
     return res;
  };

  int getIntValue(){
    return f_int_value;  
  };
  bool getBoolValue(){
    return f_bool_value;  
  };  
  void setCheck(bool _check){
    f_check = _check;
  };
  void setIntValue(int _value){
    f_int_value = _value;
  };
  void setBoolValue(bool _value){
    f_bool_value = _value;
  };
  void setName(String _name){
    f_name = _name;
  };
  void setType(int _type){
    f_type = _type;
  };
  void setParams(String _name, int _type){
    f_name = _name;
    f_type = _type; 
  };
  void setUp(){
   switch(f_type)
    {
      case 2: 
        f_int_value++; 
      break; 
      case 3:
        f_bool_value = !f_bool_value;
      break;      
    }
  };  
  void setDown(){
   switch(f_type)
    {
      case 2: 
        f_int_value--; 
      break; 
      case 3:
        f_bool_value = !f_bool_value;
      break;      
    }
  };  
  private:
    
  String f_name;
  bool   f_check = false;  
  int    f_int_value = 0;
  int    f_type = 1; // 1:множество; 2:число; 3:бинарное; 
  bool   f_bool_value = false;  
 
};

int pos = 1;
bool write_chk = false;

GButton butt_1(PIN_Y);
GButton butt_2(PIN_B);
GButton butt_3(PIN_R);

boolean draw = true;
boolean buttonState_y = false;
boolean buttonState_b = false;
boolean buttonState_r = false;

int ex_vent = 0;
int ex_temp = 25;
int ex_door = 0;
int ex_ligh = 0;
int ex_auto = 0;

Line lines[6];

byte def_set;

void readAllROMValue(int val_count){
  for(int i = 1; i <= val_count; i++){     
    int val = EEPROM.read(i);
    lines[i-1].setValue(val);
  };
};

int readROMValue(int indx){
  if(indx>=1)
    lines[indx-1].setValue(EEPROM.read(indx));
};

void writeAllROMValue(int val_count){
  for(int i = 1; i <= val_count; i++){
    EEPROM.update(i, lines[i-1].getValue());
    }
};

void writeROMValue(int indx, int _value){
  EEPROM.update(indx,_value);
};

void writeROMValue(int indx, bool _value){
  EEPROM.update(indx,_value?1:0);
};

void setup()
{
  Serial.begin(9600);

  pinMode(RELE_1, OUTPUT); 
  //digitalWrite(RELE_1, RELE_1_st);
  
  butt_1.setTimeout(300);
  butt_1.setDebounce(500);       

  butt_2.setTimeout(300);
  butt_2.setDebounce(500); 
  
  butt_3.setTimeout(300);
  butt_3.setDebounce(500); 
     
  lines[0].setParams("ligth", 3);
  lines[1].setParams(" auto", 3);
  lines[2].setParams(" vent", 3);
  lines[3].setParams(" heat", 3);
  lines[4].setParams(" door", 3);
  lines[5].setParams(" temp", 2);
  
  def_set = EEPROM.read(0);

  int indx = sizeof(lines)/sizeof(lines[0]);
  
  if(def_set==1){
    readAllROMValue(indx);      
    }
  else {
    
    lines[0].setValue(false);
    lines[1].setValue(false);
    lines[2].setValue(false);
    lines[3].setValue(false);
    lines[4].setValue(false);
    lines[5].setValue(25);    
    writeROMValue(0, 1);
    writeAllROMValue(indx);
   
  };     
     
  //pinMode(PIN_LED, OUTPUT);
  myOLED.begin();

   
};

void loop()
{

  buttonTick();
  if (draw == true) {
   
    int indx = sizeof(lines)/sizeof(lines[0]);

    if (buttonState_r){
      buttonState_r  = false;
      write_chk     =! write_chk;           
    }

    if(!write_chk)
    {
    
      if (buttonState_y) {
        pos++;
        buttonState_y=false;
      };
    
      if (buttonState_b) {
        pos--;
        buttonState_b=false;
      };
  
      if (pos<1){
        pos = indx;
      };
  
      if (pos>indx){
        pos = 1;
      };
      
    }
    else {
      
      if (buttonState_y) {
        lines[pos-1].setDown();
        buttonState_y=false;
      };
    
      if (buttonState_b) {
        lines[pos-1].setUp();
        buttonState_b=false;
      }; 
                
    };

    writeAllROMValue(indx);
    readAllROMValue(indx);
    /*for(int i=0;i<=(indx-1);i++){
     lines[i].gettext(), LEFT, i*10+2);
    };*/
        
    //Serial.println(sizeof(lines)/sizeof(lines[0])-1);

    //-----------------------------------------------------------
    myOLED.clrScr();
    myOLED.setFont(SmallFont);         
      
    for(int i=0;i<=(indx-1);i++){
            
      myOLED.invertText(pos == i + 1);
      
      if (write_chk && (pos == i + 1)){              
        myOLED.print(lines[i].gettext()+" *", LEFT, i*10+2);
      }
      else{
        myOLED.print(lines[i].gettext(), LEFT, i*10+2);
      };
      
      //delay(200);
      
    };
    myOLED.update();     
   
    draw = false;
    
  }

  engine();
  
  //Serial.println(buttonState_c_y);
  delay(50);
  /*Serial.println("Y: " + buttonState_c_y);
  Serial.println("B: " + buttonState_c_b);
  Serial.println("R: " + buttonState_c_r);?
  Serial.println("--------------------");
  delay(5000);
/*
  if(buttonState_c_y <> buttonState_y) || (buttonState_c_b <> buttonState_b) || (buttonState_c_r <> buttonState_r)*/
  /*myOLED.clrScr();                                    // Стираем все с экрана
  myOLED.setFont(RusFont);                            // Инициализируем русский шрифт
  myOLED.print("Heccrbq ihban", RIGHT, 0);           // Выводим надпись "Русский язык"
  myOLED.update();                                    // Обновляем информацию на дисплее
  delay(500);                                        // Пауза 3 секунды
  */
}

void buttonTick() {
  
  butt_1.tick();
  if (butt_1.isSingle() || butt_1.isHolded()) { 
    buttonState_y = true; 
    draw = true; 
    Serial.println("1");  
  }
  
  butt_2.tick();
  if (butt_2.isSingle() || butt_2.isHolded()) { 
    buttonState_b = true; 
    draw = true;   
    Serial.println("2");  
  }
  
  butt_3.tick();
  if (butt_3.isSingle() || butt_3.isHolded()) { 
    buttonState_r = true; 
    draw = true;   
    Serial.println("3");  
  }
  
}

void engine(){
  
  // -- свет --
  if ((lines[0].getValue() == 1) && (RELE_1_st == REL_CLOSE)) {
    digitalWrite(RELE_1, REL_OPEN); 
    RELE_1_st = REL_OPEN;
    Serial.println("СВЕТ ВКЛЮЧЕН"); 
  } else 
  if ((lines[0].getValue() == 0) && (RELE_1_st == REL_OPEN)) { 
    digitalWrite(RELE_1, REL_CLOSE);  
    RELE_1_st = REL_CLOSE;
    Serial.println("СВЕТ ВЫКЛЮЧЕН"); 
  }; 
  
}
