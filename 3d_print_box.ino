#include <TroykaDHT.h>
#include "GyverButton.h"
//#include "param_ferm.h"
#include <EEPROM.h>  
#include <OLED_I2C.h>                               // Подключаем библиотеку OLED_I2C для работы со шрифтами и графикой

OLED myOLED(20, 21, 20);                           // Определяем пины I2C интерфейса: UNO и NANO -- SDA - пин A4, SCL - пин A5; MEGA -- SDA - пин 20, SCL - пин 21
DHT  dht(12, DHT11);
extern uint8_t RusFont[];                           // Подключаем русский шрифт
extern uint8_t SmallFont[];                         // Подключаем латинский шрифтом
extern uint8_t SmallFontRus[];

const bool LOG = true;
const int PIN_Y = 6;
const int PIN_B = 7;
const int PIN_R = 8;
const int MAINTICK = 50;
const double TEMP_DEL_COLD = 2.0;
const double TEMP_DEL_HEAT_1 = 1.0;
const double TEMP_DEL_HEAT_2 = 2.0;

const int REL_OPEN  = LOW;
const int REL_CLOSE = HIGH;

// свет
int RELE_1      = 2;
int RELE_1_st   = REL_CLOSE;

// подогрев
int RELE_2      = 3;
int RELE_2_st   = REL_CLOSE;

// дверь
int RELE_3      = 4;
int RELE_3_st   = REL_CLOSE;

// вентиляция
int RELE_4      = 5;
int RELE_4_st   = REL_CLOSE;

void print_log(String _value, int _type = 0){
  if (LOG){ 
    switch(_type)
    {
      case 0:
       Serial.println(_value); 
      break; 
      case 1: 
        Serial.print(_value); 
      break; 
      default:
        Serial.println(_value); 
    }
  }
};

void print_log(float _value, int _type = 0){
  char c_tmp[15];  
  dtostrf(_value,7,2,c_tmp);
  String c_str = c_tmp;
  print_log(c_str, _type);
};

void print_log(double _value, int _type = 0){
  char c_tmp[15];  
  dtostrf(_value,7,2,c_tmp);
  String c_str = c_tmp;
  print_log(c_str, _type);
};

void print_log(int _value, int _type = 0){
  char c_tmp[15];  
  dtostrf(_value,7,2,c_tmp);
  String c_str = c_tmp;
  print_log(c_str, _type);
};

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

    print_log("Параметр = ", 1);
    print_log(f_sens_dbl_value, 0);

    if (f_sens_dbl_check) 
      text = text + " | " + f_sens_dbl_value;
     
    return text;
  }; 

  bool getCheck(){
    return f_check;
  };
  
  void setValue(int _value){

    f_need_save = true;
    
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

  bool get_need_save(){
    return f_need_save;
  }

  void set_need_save(bool _value){
    f_need_save = _value;
  }
  int getIntValue(){
    return f_int_value;  
  };
  bool getBoolValue(){
    return f_bool_value;  
  };
  double get_sens_DblValue(){
    return f_sens_dbl_value;
  };   
  bool get_sens_dbl_check(){
    return f_sens_dbl_check;
  };   
  void set_sens_dbl_check(bool _value){
    f_sens_dbl_check = _value;
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
  void set_sens_DblValue(double _value){
    
    print_log("set_sens_DblValue = ",1);
    print_log(_value,0);
    
    f_sens_dbl_value = _value;
    f_sens_dbl_check = true;
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
   f_need_save = true;
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
   f_need_save = true;
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
  bool   f_need_save = false; 
  double f_sens_dbl_value = 999999.00;  
  bool f_sens_dbl_check = false;  
  
};

/*--------------------------------------------------------------------------------------------------------*/
const int TEMPMASCOUNT = 1;   // сколько хранить измерений температуры
const int TEMPAVGCOUNT = 6;     // сколько измерений брать для вычисления средней температуры
const int TEMPREFCOUNT = 10000; // интервал измерения температуры в мс 

class TTempSensor {
  public:
    
    TTempSensor(int _main_tick){
      int tmsc    = TEMPREFCOUNT;
      f_main_tick = _main_tick;
      
      if (f_main_tick >= tmsc) 
        {
          f_tick_refresh_count = 1;
        }
      else
       {
        int d = tmsc % f_main_tick;
        if (d>0) d=1;
        f_tick_refresh_count = tmsc / f_main_tick  + d;
       }
    };

    double get_temp(){
      double r = 0.0;
      if (f_tick_null)
        r = f_temp;

        return r;
    } 

    bool get_tick_null(){
      return f_tick_null;
    }

    void set_tick_null(bool _value){
      f_tick_null = _value;
    }
            
    void refresh() {
      
        double tmp_temp = 0.0;
        
        f_tick_refresh_index++;
        
        if (f_tick_refresh_index >= f_tick_refresh_count)
        {
          f_tick_refresh_index = 0;
          dht.read();         
          float f_tmp_temp = dht.getTemperatureC(); 
          
          write_c_temp(f_tmp_temp, f_tick_avg_index);
          
          f_tick_avg_index++;
          
          print_log("TMP_Temperature = ",1);
          print_log(f_tmp_temp,1);
          print_log(" C \t",0);
            
          if (f_tick_avg_index > f_tick_avg_count - 1) {
            f_tick_avg_index = 0;
            for (int i=0; i<=f_tick_avg_count-1; i++){
              tmp_temp = tmp_temp + f_temp_c_mas[i];
            };
            f_temp = tmp_temp/f_tick_avg_count;
            f_tick_null = true;
            
            print_log(" -------------------------------------------------------------------------------", 0);  
            print_log("Temperature = ", 1);
            print_log(f_temp, 1);    
            print_log(" C \t",0);
            print_log(" -------------------------------------------------------------------------------",0); 
          }
        }
        
        
        // проверяем состояние данных
        /*switch(dht.getState()) {
          // всё OK
          case DHT_OK:
            // выводим показания влажности и температуры
            Serial.print("Temperature = ");
            Serial.print(dht.getTemperatureC());
            Serial.println(" C \t");
            Serial.print("Humidity = ");
            Serial.print(dht.getHumidity());
            Serial.println(" %");
            Serial.println("------------------------------------");
            break;
          // ошибка контрольной суммы
          case DHT_ERROR_CHECKSUM:
            Serial.println("Checksum error");
            break;
          // превышение времени ожидания
          case DHT_ERROR_TIMEOUT:
            Serial.println("Time out error");
            break;
          // данных нет, датчик не реагирует или отсутствует
          case DHT_ERROR_NO_REPLY:
            Serial.println("Sensor not connected");
            break;
            */
        
    };
    
  private:
  
    double  f_temp = 0.0;
    //double  f_temp_h_mas[TEMPMASCOUNT];
    double  f_temp_c_mas[TEMPAVGCOUNT];
    int f_temp_index = 0;
    int f_temp_count = TEMPMASCOUNT;
    int f_main_tick;
    int f_tick_refresh_count;
    int f_tick_refresh_index = 0;
    int f_tick_avg_count = TEMPAVGCOUNT;
    int f_tick_avg_index = 0;
    boolean f_tick_null = false;
    
    void write_c_temp(double  _temp, int _index) {
      f_temp_c_mas[_index] = _temp;
    }
    
    void write_h_temp(double  _temp) {
      
    }
     
};
/*--------------------------------------------------------------------------------------------------------*/


int pos = 1;
bool write_chk = false;

TTempSensor tem_sens(MAINTICK);

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
      if (lines[i-1].get_need_save()){
        print_log("Save value to EEPROM", 0);    
        lines[i-1].set_need_save(false);
        EEPROM.update(i, lines[i-1].getValue());
      }
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

  dht.begin();

  pinMode(RELE_1, OUTPUT); 
  pinMode(RELE_2, OUTPUT); 
  pinMode(RELE_3, OUTPUT);
  pinMode(RELE_4, OUTPUT);
  digitalWrite(RELE_1, RELE_1_st);
  digitalWrite(RELE_2, RELE_2_st);
  digitalWrite(RELE_3, RELE_3_st);
  digitalWrite(RELE_4, RELE_4_st);
  
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

  print_log("def_set = ", 1);
  print_log(def_set, 0);
  
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
  tem_sens.refresh();
  if(tem_sens.get_tick_null()){
    lines[5].set_sens_DblValue(tem_sens.get_temp());
    tem_sens.set_tick_null(false);
    draw = true;
  }
  
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
    //readAllROMValue(indx);

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
 
  
  delay(MAINTICK);
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
    print_log("button: 1", 0);  
  }
  
  butt_2.tick();
  if (butt_2.isSingle() || butt_2.isHolded()) { 
    buttonState_b = true; 
    draw = true;   
    print_log("button: 2", 0);  
  }
  
  butt_3.tick();
  if (butt_3.isSingle() || butt_3.isHolded()) { 
    buttonState_r = true; 
    draw = true;   
    print_log("button: 3",0);  
  }
  
}

void engine(){
  
  // -- свет --
  if ((lines[0].getValue() == 1) && (RELE_1_st == REL_CLOSE)) {
    digitalWrite(RELE_1, REL_OPEN); 
    RELE_1_st = REL_OPEN;
    print_log("СВЕТ ВКЛЮЧЕН", 0); 
  } else 
  if ((lines[0].getValue() == 0) && (RELE_1_st == REL_OPEN)) { 
    digitalWrite(RELE_1, REL_CLOSE);  
    RELE_1_st = REL_CLOSE;
    print_log("СВЕТ ВЫКЛЮЧЕН", 0); 
  }; 

  // -- автоматизация
  if (lines[1].getValue() ==1 && lines[5].get_sens_dbl_check()){
    double temp_tmp_sens = lines[5].get_sens_DblValue();
    int    temp_tmp_dest = lines[5].getValue();
        
    // -- подогрев
    if ((lines[3].getValue() == 1) && (RELE_2_st == REL_CLOSE) && (temp_tmp_sens < temp_tmp_dest-TEMP_DEL_COLD)) {
      digitalWrite(RELE_2, REL_OPEN); 
      RELE_2_st = REL_OPEN;
      print_log("ПОДОГРЕВ ВКЛЮЧЕН", 0); 
    } else 
    if (((lines[3].getValue() == 0) || (temp_tmp_sens >= temp_tmp_dest)) && (RELE_2_st == REL_OPEN)) { 
      digitalWrite(RELE_2, REL_CLOSE);  
      RELE_2_st = REL_CLOSE;
      print_log("ПОДОГРЕВ ВЫКЛЮЧЕН", 0); 
    };

    // --охлаждение - дверь
    if ((lines[4].getValue() == 1) && (RELE_3_st == REL_CLOSE) && (temp_tmp_sens > temp_tmp_dest+TEMP_DEL_HEAT_1)) {
      digitalWrite(RELE_3, REL_OPEN); 
      RELE_3_st = REL_OPEN;
      print_log("ДВЕРЬ ОТКРЫТА", 0); 
    } else 
    if (((lines[4].getValue() == 0) || (temp_tmp_sens <= temp_tmp_dest)) && (RELE_3_st == REL_OPEN)) { 
      digitalWrite(RELE_3, REL_CLOSE);  
      RELE_3_st = REL_CLOSE;
      print_log("ДВЕРЬ ЗАКРЫТА", 0); 
    };    

    // --охлаждение - вентиляция
    if ((lines[2].getValue() == 1) && (RELE_4_st == REL_CLOSE) && (temp_tmp_sens > temp_tmp_dest+TEMP_DEL_HEAT_2)) {
      digitalWrite(RELE_4, REL_OPEN); 
      RELE_4_st = REL_OPEN;
      print_log("ПРИТОК ВКЛЮЧЕН", 0); 
    } else 
    if (((lines[2].getValue() == 0) || (temp_tmp_sens <= temp_tmp_dest+TEMP_DEL_HEAT_1)) && (RELE_4_st == REL_OPEN)) { 
      digitalWrite(RELE_4, REL_CLOSE);  
      RELE_4_st = REL_CLOSE;
      print_log("ПРИТОК ВЫКЛЮЧЕН", 0); 
    }; 
        
  }
  // если автоматизация выключена - закрываем все позиции
  else {
    if (RELE_2_st == REL_OPEN) { 
      digitalWrite(RELE_2, REL_CLOSE);  
      RELE_2_st = REL_CLOSE;
      print_log("ПОДОГРЕВ ВЫКЛЮЧЕН", 0); 
    };  
    if (RELE_3_st == REL_OPEN) { 
      digitalWrite(RELE_3, REL_CLOSE);  
      RELE_3_st = REL_CLOSE;
      print_log("ДВЕРЬ ЗАКРЫТА", 0);      
    };
    if (RELE_4_st == REL_OPEN) { 
      digitalWrite(RELE_4, REL_CLOSE);  
      RELE_4_st = REL_CLOSE;
      print_log("ПРИТОК ВЫКЛЮЧЕН", 0); 
    }; 
  }       
}
