#include <Servo.h>
#include <Stepper.h>
#include "HX711.h" 

class customServo {
private:
// 멤버 변수
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
  }

  void up() {
    servo.write(upAngle);
  }
};

// loadcell_2
#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3

// loadcell_2
#define LOADCELL_DOUT_PIN2 5 
#define LOADCELL_SCK_PIN2 6 

// ModuleServo
#define MODULE_SERVO_PIN 8

HX711 scale; // hx-711 하나
HX711 scale2; // hx-711 두개

// 로드 셀 캘리브레이션
int calibration_factor =-480;
int calibration_factor2 =-480; // hx -711 두개를 사용 시 더 추가

Stepper myStepper1(200, 12, 11, 10, 9);      
Servo servo1;

void setup() {
  // 시리얼 통신 설정
  Serial.begin(19200);          

  // 모듈 서보 설정
  servo1.attach(MODULE_SERVO_PIN);
  pinMode(MODULE_SERVO_PIN, OUTPUT);

  // 스텝 모터 설정
  myStepper1.setSpeed(70); // 분당 스텝수 설정  --> 스텝모터의 속도를 결정한다. 
  
  
  // 로드셀1 설정
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare();
  
  // 로드셀2 설정
  scale2.begin(LOADCELL_DOUT_PIN2, LOADCELL_SCK_PIN2); 
  scale2.set_scale();
  scale2.tare();
  
  long zero_factor = scale.read_average();
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
}

// myStepper1.step(-200/4);
// 반시계방향 회전
/*
 *
 *  1. 시리얼 입력이 있을때 까지 대기
 *  2. 입력정보 (n번 카트리지 m(g)출력 정보 6개) 해석
 *  3. 출력, 진행상황 시리얼 송신
 * 
 */

void loop() {
  
  customServo moduleServo = customServo(70, 120, servo1);

  ////////////////////////// start /////////////////////////////////////////

  scale.set_scale(calibration_factor);
  scale2.set_scale(calibration_factor2);

  // print weight in serialmonitor
  Serial.print(" loadcell 1 : "); 
  Serial.print(scale.get_units(), 1);
  Serial.print(" g");
  double sumWeight = scale.get_units() + scale2.get_units();
  Serial.print(" loadcㅇell 2 : ");
  Serial.print(scale2.get_units(), 1);
  Serial.println(" g"); 

  Serial.print(sumWeight);
  Serial.println(" g"); 


//   if (sumWeight > 28 && sumWeight < 57) {
//     
//     delay(500);
//   } else if (sumWeight > 57) {
//     myStepper1.step(-200/62);
//     delay(500);
//   }

  if(sumWeight > 5) {
    moduleServo.up();
  } else {
      moduleServo.down();
  }

}

// void Servo_down() {

// }

