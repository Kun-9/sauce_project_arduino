#include "stdio.h"
#include <Stepper.h>
#include "HX711.h" 
#include "string.h"
#include "Servo.h"
// #include <iostream>
// #include <string>
// #include <vector>
// #include <sstream>
// #include <typeinfo>
// using namespace std;

#define SWITCH 9

#define VACUUM_PIN 13
#define AIRPUMP_PIN 7
#define VIBRATION_PIN 30

// loadcell_1
#define LOADCELL_DOUT_PIN 10  
#define LOADCELL_SCK_PIN 11

// loadcell_2
#define LOADCELL_DOUT_PIN2 12 
#define LOADCELL_SCK_PIN2 13 

// ModuleServo
#define MODULE_SERVO_PIN 2

// StepMoter
#define A 9
#define A_ 10
#define B 11
#define B_ 12
#define PUL 7
#define DIR 6
#define ENA 5

// Input String
String str = "";

/*  
* source_info[0] = Cartridge number info
* source_info[1] = Weight info
* source_info[2] = Liquid info
*/
int source_info[3][6];

// Number to Output
int N;

class customServo {
private:
  int downAngle;
  int upAngle;
  Servo servo;
  
public:

  customServo(int downAngle, int upAngle, Servo servo) {
    this->downAngle = downAngle;
    this->upAngle = upAngle;
    this->servo = servo;
  }

  void down() {
    servo.write(downAngle);
    Serial.println("ModuleServo down");
  }

  void up() {
    servo.write(upAngle);
    Serial.println("ModuleServo up");
  }
};

class moveCartridge {
  private:
    int current_num = 1;
    int move_num;
    int pul, dir, ena;
    // Stepper shaftStep;

  public:
    moveCartridge(int pul, int dir, int ena) {
      this -> pul = pul;
      this -> dir = dir;
      this -> ena = ena;
    }

    void moveMotor(int step, int speed, String direction) {
      for (int i=0; i<step; i++)    //Forward 5000 steps
      {
        if (direction == "rc") {
          digitalWrite(dir,HIGH);
        } else if(direction == "c") {
          digitalWrite(dir,LOW);
        } else {
          exit(1);
        }
        digitalWrite(ena,HIGH);
        digitalWrite(pul,HIGH);
        delayMicroseconds(speed);
        digitalWrite(pul,LOW);
        delayMicroseconds(speed);
      }
    }

    boolean move(int move_num) {

      this -> move_num = move_num;
      int tmp = move_num - current_num;

      // move
      for(int i = 0; i < tmp ; i ++) {
        moveMotor(266, 1000, "c");
      }

      /*
        move funtion
      */
      Serial.print(this -> current_num);
      Serial.print(" to ");
      Serial.println(move_num);
      this -> current_num = move_num;
      return true;
    }

    void toStartingPoint(int startCali) {
      while(1) {
        moveMotor(1, 2000, "rc");
        if(digitalRead(SWITCH) == HIGH){
          break;
        }
      }
      delay(1000);
      moveMotor(startCali, 2000, "c"); 
      delay(2000);
    }

    void sleep() {
      digitalWrite(ena,LOW);
      digitalWrite(pul,LOW);
    }
};

void vacuumMotor(int pwm) {
  analogWrite(VACUUM_PIN, pwm);
  Serial.print("vacuum Motor value : ");
  Serial.println(pwm);
}

void airpumpMotor(int pwm) {
  analogWrite(AIRPUMP_PIN, pwm);
  Serial.print("airpump Motor value : ");
  Serial.println(pwm);
}

void coverMotor(String status) {
  if (status == "open") {
    Serial.println("coverMotor status : open");
  } else {
    Serial.println("coverMotor status : close");
  }
}

void vibrationMotor(int pwm) {
  analogWrite(VIBRATION_PIN, pwm);
  Serial.print("vibrationMotor value : ");
  Serial.println(pwm);
}

void parseStr(String Str) {
  int first = str.indexOf(",");
  int second = str.indexOf(",", first+1);
  int third = str.indexOf(",", second+1);
  int StrLength = str.length();

  int N = str.substring(0, first).toInt();
  String CartNumStr = str.substring(first+1, second);
  String WeightStr = str.substring(second+1, third);
  String LiquidStr = str.substring(third+1, StrLength);

  
  // Cartrdige number Str parsing
  for (int i = 0; i < N; i++) {
  
    String tmp = (String) CartNumStr.charAt(2*i);
    source_info[0][i] = tmp.toInt();
    tmp = (String) LiquidStr.charAt(2*i);
    source_info[2][i] = tmp.toInt();
  }

  // WeightStr parsing
  for (int i = 0; i < N; i++) {
    String tmp;
    int index = WeightStr.indexOf(" ");
    StrLength = WeightStr.length();

    // 공백 전까지 읽고 저장
    tmp = WeightStr.substring(0,index);
    
    source_info[1][i] = tmp.toInt();

    // 읽은 부분 자름
    WeightStr = WeightStr.substring(index+1,StrLength);
  }

  // input info array print
  // Serial.print(N);
  // for (int i = 0; i < 3; i++) {
  //     for(int t = 0; t < N; t++) {
  //       Serial.print(source_info[i][t]);
  //     }
  //   Serial.println();
  // }
}

class LoadCell {
  private:
    int calibration_factor;
    int calibration_factor2;
    HX711 scale1;
    HX711 scale2;

  public:
    LoadCell(HX711 scale1, HX711 scale2, int calibration_factor, int calibration_factor2) {
      this -> scale1 = scale1;
      this -> scale2 = scale2;
      scale1.tare();
      scale2.tare();
    }

    void tare() {
      scale1.tare();
      scale2.tare();
    }

    double getWeight() {
      scale1.set_scale(calibration_factor);
      scale2.set_scale(calibration_factor2);
      double sumWeight = scale1.get_units() + scale2.get_units();
      Serial.print(sumWeight);
      Serial.println(" g");
      return sumWeight;
    }
};

// LoadCell calibration
int calibration_factor = -480;
int calibration_factor2 = -480;

// Stepper shaftStep = Stepper(200, 12, 11, 10, 9);      

HX711 scale1;
HX711 scale2;

// down angle, up angle
Servo servo1;
customServo moduleservo = customServo(70, 120, servo1);
moveCartridge movecartridge = moveCartridge(PUL, DIR, ENA) ;
// moveCartridge movecartridge = moveCartridge(200, B_, B, A_, A) ;


//////////////////////////////////////////////////////////////////////////////////
void setup() {
  // Serial setting
  Serial.begin(9600);          

  // Module servoMotor setting
  servo1.attach(MODULE_SERVO_PIN);
  pinMode(MODULE_SERVO_PIN, OUTPUT);
  pinMode(AIRPUMP_PIN, OUTPUT);
  pinMode(VACUUM_PIN, OUTPUT);
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(SWITCH, INPUT);
  // shaftStep.setSpeed(70);
  
  
  // LoadCell1 setting
  scale1.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale1.set_scale(calibration_factor);
  scale1.tare();
  
  // LoadCell2 setting
  scale2.begin(LOADCELL_DOUT_PIN2, LOADCELL_SCK_PIN2); 
  scale2.set_scale(calibration_factor2);
  scale2.tare();
  
  // long zero_factor = scale.read_average();
  // Serial.print("Zero factor: ");
  // Serial.println(zero_factor);
}

// shaftStep.step(-200/6);
// turn counterclockwise 60 degrees.
/*
 *
 *  1. 시리얼 입력이 있을때 까지 대기
 *  2. 입력정보 (n번 카트리지 m(g)출력 정보 6개) 해석
 *  3. 출력, 진행상황 시리얼 송신
 * 
 *  현재 카트리지에서 n번째 카트리지로 이동하려면 [ -200/6 * (이동할 카트리지 번호 - 현재 카트리지 번호) ]
 *  1 -> 2번 카트리지 -200/6 * 1 만큼 회전
 *  1 -> 3번 카트리지 -200/6 * 2 만큼 회전 
 *
 */


// 입력 개수 n개 
// 3 -> 5 -> 6 번째 칸

// *** Implemented funtions ***
// module servoMotor
// shaft stepMotor
// loadcell
// vucuum, airpumpMotor motor
// vibration motor
// ----------------------------
// cover servoMotor
// 

void loop() {

  //test servo
  /*
  while (1)
  {
    moduleservo.down();
    delay(1000);
    moduleservo.up();
    delay(1000);

  }
*/ 

  //test
  while (1)
  {
    movecartridge.toStartingPoint(47);
    movecartridge.move(3);
    
    double Weight = 0;
    scale1.tare();
    scale2.tare();
    ////////////////////
    
    // Starting weighing
    while (Weight < 10)
    {
      Serial.println("Measuring the weight.");
      Weight = scale1.get_units() + scale2.get_units();
      Serial.println(Weight);
    }

    movecartridge.move(6);

    movecartridge.sleep();

    delay(10000000);
    
  }
  

  // wait String info from rasberry pi
  while(Serial.available() == 0) {}

  str = Serial.readString();

  // input String parsing

  int first = str.indexOf(",");
  int second = str.indexOf(",", first+1);
  int third = str.indexOf(",", second+1);
  int StrLength = str.length();

  int N = str.substring(0, first).toInt();
  String CartNumStr = str.substring(first+1, second);
  String WeightStr = str.substring(second+1, third);
  String LiquidStr = str.substring(third+1, StrLength);

  
  // Cartrdige number Str parsing
  for (int i = 0; i < N; i++) {
  
    String tmp = (String) CartNumStr.charAt(2*i);
    source_info[0][i] = tmp.toInt();
    tmp = (String) LiquidStr.charAt(2*i);
    source_info[2][i] = tmp.toInt();
  }

  // WeightStr parsing
  for (int i = 0; i < N; i++) {
    String tmp;
    int index = WeightStr.indexOf(" ");
    StrLength = WeightStr.length();

    // 공백 전까지 읽고 저장
    tmp = WeightStr.substring(0,index);
    
    source_info[1][i] = tmp.toInt();

    // 읽은 부분 자름
    WeightStr = WeightStr.substring(index+1,StrLength);
  }

  // parseStr(str);  

  Serial.println(N);
  for (int i = 0; i < 3; i++) {
      for(int t = 0; t < N; t++) {
        Serial.print(source_info[i][t]);
        Serial.print(", "); 
      }
    Serial.println();
  }

  // scale1.set_scale(calibration_factor);
  // scale2.set_scale(calibration_factor2);
  
  // StepMotor setting

  // LoadCell loadcell = LoadCell(scale1, scale2, calibration_factor, calibration_factor2);

  ////////////////////////// start /////////////////////////////////////////

  ///////////////// input value ///////////////////

  // int Source_arr[3] = {3, 4, 6};
  // int goal_weight[3] = {30, 10, 50};
  // boolean isLiquid[3] = {true, true, false};

  // int n = (int)(sizeof(Source_arr) / sizeof(int));

  /////////////////////////////////////////////////
  for (int i = 0 ; i < N ; i++) {

    // move to cartridge to output
    movecartridge.move(source_info[0][i]);
    delay(1000);

    // Output module coupling
    moduleservo.down();
    delay(1000);
    
    // Cover module open
    coverMotor("open");

    // Select output method
    if (source_info[2][i] == 1) {
      Serial.println("Liquid");
      airpumpMotor(255);
    } else {
      Serial.println("not Liquid");
      vibrationMotor(255);
    }
    
    // Reset LoadCell //
    double Weight = 0;
    scale1.tare();
    scale2.tare();
    ////////////////////

    // Starting weighing
    Serial.println("Measuring the weight.");
    while(Weight <= source_info[1][i]) {
       Weight = scale1.get_units() + scale2.get_units();
      //  Serial.println(Weight);
    }

    // End of weight measurement
    Serial.print(Weight);
    Serial.println("g");
    Serial.println("Measuring is over.");

    // close cover
    if (source_info[2][i] == 1) {
      vacuumMotor(255);
      airpumpMotor(0);
      coverMotor("close");
      vacuumMotor(0);
    } else {
      vibrationMotor(0);
      coverMotor("close");
    }

    moduleservo.up();

    delay(5000);
  }

  movecartridge.move(1);

  Serial.println("End cycle");
  delay(5000);

}

