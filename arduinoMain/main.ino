#include <Servo.h>
#include <Stepper.h>
#include "HX711.h" 

#define VACUUM_PIN 13
#define EXHALE_PIN 7
#define VIBRATION_PIN 30

// loadcell_2
#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3

// loadcell_2
#define LOADCELL_DOUT_PIN2 5 
#define LOADCELL_SCK_PIN2 6 

// ModuleServo
#define MODULE_SERVO_PIN 8

HX711 scale1;
HX711 scale2;

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

  public:
    boolean move(int move_num) {

      this -> move_num = move_num;
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

void vacuum(int pwm) {
  analogWrite(VACUUM_PIN, pwm);
  Serial.print("vacuum value : ");
  Serial.println(pwm);
}

void exhale(int pwm) {
  analogWrite(EXHALE_PIN, pwm);
  Serial.print("exhale value : ");
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

// double getWeight() {

  // double sumWeight = scale1.get_units() + scale2.get_units();

  // return sumWeight;
// }

// LoadCell calibration
int calibration_factor = -480;
int calibration_factor2 = -480; // hx -711 두개를 사용 시 더 추가

Stepper myStepper1(200, 12, 11, 10, 9);      
Servo servo1;


//////////////////////////////////////////////////////////////////////////////////
void setup() {
  // Serial setting
  Serial.begin(19200);          

  // Module servoMotor setting
  servo1.attach(MODULE_SERVO_PIN);
  pinMode(MODULE_SERVO_PIN, OUTPUT);
  pinMode(EXHALE_PIN, OUTPUT);
  pinMode(VACUUM_PIN, OUTPUT);

  // StepMotor setting
  myStepper1.setSpeed(70); // 분당 스텝수 설정  --> 스텝모터의 속도를 결정한다. 
  
  
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

// myStepper1.step(-200/6);
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
// vucuum, exhale motor
// vibration motor
// ----------------------------
// cover servoMotor
// 

void loop() {

  scale1.set_scale(calibration_factor);
  scale2.set_scale(calibration_factor2);
  
  customServo moduleServo = customServo(70, 120, servo1);
  moveCartridge movecartridge ;
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
      exhale(255);
    } else {
      Serial.println("not Liquid");
      vibrationMotor(255);
    }

    // reset
    double Weight = 0;
    scale1.tare();
    scale2.tare();
    Serial.println("Measuring the weight.");
    while(Weight < goal_weight[i]) {
       Weight = scale1.get_units() + scale2.get_units();
      //  Serial.println(Weight);
    }
    Serial.print(Weight);
    Serial.println("g");
    Serial.print("Measuring is over.");

    if (isLiquid[i]) {
      vacuum(255);
      exhale(0);
      coverMotor("close");
      vacuum(0);
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

