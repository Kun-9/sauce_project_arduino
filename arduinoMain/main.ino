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


#define VACUUM_PIN 13
#define AIRPUMP_PIN 7
#define VIBRATION_PIN 30

// loadcell_2
#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3

// loadcell_2
#define LOADCELL_DOUT_PIN2 5 
#define LOADCELL_SCK_PIN2 6 

// ModuleServo
#define MODULE_SERVO_PIN 8

// StepMoter
#define A 9
#define A_ 10
#define B 11
#define B_ 12

String str = "";


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

class moveCartridge : public Stepper {
  private:
    int current_num = 1;
    int move_num;
    // Stepper shaftStep;
  public:
    moveCartridge(int number_of_steps, int motor_pin_1, int motor_pin_2) 
                                 : Stepper(number_of_steps, motor_pin_1, motor_pin_2){}       
    moveCartridge(int number_of_steps, int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4) 
                                 : Stepper(number_of_steps, motor_pin_1, motor_pin_2, motor_pin_3, motor_pin_4){}
    moveCartridge(int number_of_steps, int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4, int motor_pin_5) 
                                 : Stepper(number_of_steps, motor_pin_1, motor_pin_2, motor_pin_3, motor_pin_4, motor_pin_5){}

    boolean move(int move_num) {

      this -> move_num = move_num;
      int tmp = move_num - current_num;

      // move
      for(int i = 0; i < tmp ; i ++) {
        step(-200/6);
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
customServo moduleServo = customServo(70, 120, servo1);
moveCartridge movecartridge = moveCartridge(200, A, A_, B, B_) ;
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

  // shaftStep.setSpeed(70);
  
  
  // LoadCell1 setting
  scale1.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale1.set_scale();
  scale1.tare();
  
  // LoadCell2 setting
  scale2.begin(LOADCELL_DOUT_PIN2, LOADCELL_SCK_PIN2); 
  scale2.set_scale();
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
  
  while(Serial.available() == 0) {
  }

//  while(Serial.available() != 0){
    str = Serial.readString();
    
//  }
  Serial.println("end");
  Serial.println(str);

  
  // int str2 = Serial.read();
  // String str= "3 5 7 0 0 0,40 50 20 0 0 0,1 0 1 0 0 0";

  

  /*
  int first = str.indexOf(",");
  int second = str.indexOf(",", first+1);
  int StrLength = str.length();

  String firstStr = str.substring(0, first);
  String SecondStr = str.substring(first+1, second);
  String LastStr = str.substring(second+1, StrLength);

  int source_info[3][6];
  

  for (int i = 0; i < 6; i++) {
  
    char tmp = firstStr.charAt(2*i);
    
    source_info[0][i] = tmp;
    Serial.println(tmp);
  }
*/
  /*
    int temp = 0;
    int point = str.indexOf(" ");
    String tok = SecondStr.substring(0,point);
    Serial.println(tok);
    Serial.println(point);
    int point2 = str.indexOf(" ", point+1);
    String tok2 = str.substring(point+1, point2);

    Serial.println(tok2);
    Serial.println(point2);

    temp = point;
  
  Serial.println(firstStr);
  Serial.println(SecondStr);
  Serial.println(LastStr);
  */
  // String str="3 5 7 0 0 0,40 50 20 0 0 0,1 0 1 0 0 0";




  scale1.set_scale(calibration_factor);
  scale2.set_scale(calibration_factor2);
  
  // StepMotor setting
  movecartridge.setSpeed(70);

  // LoadCell loadcell = LoadCell(scale1, scale2, calibration_factor, calibration_factor2);

  ////////////////////////// start /////////////////////////////////////////

  ///////////////// input value ///////////////////

  int Source_arr[3] = {3, 4, 6};
  int goal_weight[3] = {30, 10, 50};
  boolean isLiquid[3] = {true, true, false};

  int n = (int)(sizeof(Source_arr) / sizeof(int));

  /////////////////////////////////////////////////
  for (int i = 0 ; i < n ; i++) {

    movecartridge.move(Source_arr[i]);
    delay(1000);

    moduleServo.down();
    delay(1000);
    
    coverMotor("open");

    if (isLiquid[i]) {
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

    Serial.println("Measuring the weight.");
    while(Weight <= goal_weight[i]) {
       Weight = scale1.get_units() + scale2.get_units();
      //  Serial.println(Weight);
    }
    Serial.print(Weight);
    Serial.println("g");
    Serial.println("Measuring is over.");

    if (isLiquid[i]) {
      vacuumMotor(255);
      airpumpMotor(0);
      coverMotor("close");
      vacuumMotor(0);
    } else {
      vibrationMotor(0);
      coverMotor("close");
    }

    moduleServo.up();

    delay(5000);
  }

  movecartridge.move(1);

  Serial.println("End cycle");
  delay(5000);

}

