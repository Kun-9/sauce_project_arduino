#include "stdio.h"
#include "HX711.h" 
#include "string.h"
#include "Servo.h"

#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 53    // spi 통신을 위한 SS(chip select)핀 설정
#define RST_PIN 5    // 리셋 핀 설정 

// #include <iostream>
// #include <string>
// #include <vector>
// #include <sstream>
// #include <typeinfo>
// using namespace std;

#define SWITCH 9

#define MAGNET_RELAY 41

#define VACUUM_PIN_1 3
#define VACUUM_PIN_2 4
#define VACUUM_PIN_ENA 8

#define AIRPUMP_PIN_1 42
#define AIRPUMP_PIN_2 43
#define AIRPUMP_PIN_ENA 10

#define VIBRATION_PIN_1 22
#define VIBRATION_PIN_2 23
#define VIBRATION_PIN_ENA 11


// loadcell_1
#define LOADCELL_DOUT_PIN 30  
#define LOADCELL_SCK_PIN 31

// loadcell_2
#define LOADCELL_DOUT_PIN2 32 
#define LOADCELL_SCK_PIN2 33 

// ModuleServo
#define OUTPUT_SERVO_PIN 24
#define CLOSE_SERVO_PIN 44

// StepMoter
#define PUL 7
#define DIR 6
#define ENA 2

// Setting
#define START_CALIBRATION 30


#define OUTPUT_DOWN 74
#define CLOSE_CLOSE 4

#define OUTPUT_UP 124
#define CLOSE_OPEN 103

// Input String
String str = "";

/*  
* sauce_info[0] = Cartridge number info
* sauce_info[1] = Weight info
* sauce_info[2] = Liquid info
*/
int sauce_info[3][6];

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
    Serial.println("0 OutputServo down");
  }

  void up() {
    servo.write(upAngle);
    Serial.println("0 OutputServo up");
  }

  void close() {
    servo.write(downAngle);
    Serial.println("0 closeServo close");
  }

  void open() {
    servo.write(upAngle);
    Serial.println("0 closeServo open");
  }

  void detach() {
    servo.detach();
    Serial.println("0 OutputServo detach");
  }

  void attach() {
    servo.attach(OUTPUT_SERVO_PIN);
    Serial.println("0 OutputServo attach");
  }
};

class moveCartridge {
  private:
    int current_num = 1;
    int move_num;
    int pul, dir, ena;
    float errorValue;

  public:
    moveCartridge(int pul, int dir, int ena) {
      this -> pul = pul;
      this -> dir = dir;
      this -> ena = ena;
      this -> errorValue = 0;
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
      // 한 카트리지 당 266스텝 (59.85도) -0.15도 오차. 한스텝당 0.225도 만약 총 오차 합의 절댓값이 0.225가 넘으면 1스텝 더 움직임
      //  2     3     4     5     6
      // -0.15 - 0.3 -0.45 -0.6 -0.75
      this -> move_num = move_num;

      // 움직이는 거리
      int tmp = move_num - current_num;

      // 오차 값 계산      
      errorValue += tmp * 0.15;
      
      // Serial.print("0 tmp : ");
      // Serial.println(tmp);

      Serial.println("0 ////////////////////////////////");
      Serial.print("0 현재 오차값 : ");
      Serial.println(errorValue);
      

      // 오차를 0.225로 나눈 값 => 더 움직여야 할 스텝 수
      int calibration = 0;

      while (abs(errorValue - 0.255) < 0.13) {
        errorValue -= 0.255;
        Serial.print("0 오차값 : ");
        Serial.println(errorValue);

        calibration++;        
        Serial.print("0 이동 할 스텝 값 : ");
        Serial.println(calibration);
      }

      // if (errorValue > 0.225) {
      //   calibration = floor(errorValue / 0.225);
      //   Serial.print("0 <오차 캘리브레이션> ");
      //   Serial.print(calibration);
      //   Serial.println("스텝 추가 이동");

      //   // 남은 오차 값 갱신
      //   errorValue = errorValue - (calibration * 0.225);  
      //   Serial.print("0 잔여 값 : ");
      //   Serial.println(errorValue);
      //   Serial.println("0 ////////////////////////////////");
      //   Serial.println("0 ");
      // }
            
      // move
      for(int i = 0; i < tmp ; i ++) {
        moveMotor(266, 1500, "c");
      }

      for(int i = 0; i < calibration ; i ++) {
        moveMotor(1, 1500, "c");
      }
      
      Serial.print("0 ");
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
      moveMotor(startCali, 3000, "c"); 

    }

    void sleep() {
      digitalWrite(ena,LOW);
      digitalWrite(pul,LOW);
    }
    
    void resetCartNum() {
      this -> current_num = 1;
    }
};

void runVacuumMotor(int pwm) {
  digitalWrite(VACUUM_PIN_1, HIGH);
  digitalWrite(VACUUM_PIN_2, LOW);
  analogWrite(VACUUM_PIN_ENA, pwm);
  Serial.print("0 vacuum Motor value : ");
  Serial.println(pwm);
}

void runAirMotor(int pwm) {

  Serial.print("0 airpump Motor value : ");
  Serial.println(pwm);

  digitalWrite(AIRPUMP_PIN_1, HIGH);
  digitalWrite(AIRPUMP_PIN_2, LOW);
  analogWrite(AIRPUMP_PIN_ENA, pwm);
}

class Magnet {
  private:
    
  public:
    void on() {
      digitalWrite(MAGNET_RELAY, HIGH);   
      Serial.println("0 magent on");
    }

    void off() {
      digitalWrite(MAGNET_RELAY, LOW);
      Serial.println("0 magnet off"); 
  }
};

void coverMotor(String status) {
  if (status == "open") {

    Serial.println("0 coverMotor status : open");
  } else {

    Serial.println("0 coverMotor status : close");
  }
}

void runVibrationMotor(int pwm) {
  digitalWrite(VIBRATION_PIN_1, HIGH);
  digitalWrite(VIBRATION_PIN_2, LOW);
  analogWrite(VIBRATION_PIN_ENA, pwm);

  Serial.print("0 vibrationMotor value : ");
  Serial.println(pwm);
}

int parseInputString() {
  
  str = Serial.readString();

  // input String parsing
  int first = str.indexOf(",");
  int second = str.indexOf(",", first+1);
  int third = str.indexOf(",", second+1);
  int StrLength = str.length();

  Serial.print("0 ");
  Serial.print(first);
  Serial.print(", ");
  Serial.print(second);
  Serial.print(", ");
  Serial.println(third);



  int N = str.substring(0, first).toInt();

  // N이 0이라면 N을 리턴하고 종료
  if (!N) { return N; }

  String CartNumStr = str.substring(first+1, second);
  String WeightStr = str.substring(second+1, third);
  String LiquidStr = str.substring(third+1, StrLength);

  Serial.print("0 ");
  Serial.print(CartNumStr);
  Serial.print(", ");
  Serial.print(WeightStr);
  Serial.print(", ");
  Serial.println(LiquidStr);


  
  // Cartrdige number Str parsing
  for (int i = 0; i < N; i++) {
    String tmp = (String) CartNumStr.charAt(2*i);
    sauce_info[0][i] = tmp.toInt();
    tmp = (String) LiquidStr.charAt(2*i);
    sauce_info[2][i] = tmp.toInt();
  }

  // WeightStr parsing
  for (int i = 0; i < N; i++) {
    String tmp;
    int index = WeightStr.indexOf(" ");
    StrLength = WeightStr.length();

    // 공백 전까지 읽고 저장
    tmp = WeightStr.substring(0,index);
    if (tmp == " ") {
      tmp == "0";
    }
    sauce_info[1][i] = tmp.toInt();

    // 읽은 부분 자름
    WeightStr = WeightStr.substring(index+1,StrLength);
  }

  // parseStr(str);  


  // Serial.print("0 ");
  // for (int i = 0; i < 3; i++) {
  //     for(int t = 0; t < N; t++) {
  //       Serial.print(sauce_info[i][t]);
  //       Serial.print(", "); 
  //     }
  //   Serial.println();
  // }

  return N;
}

// ID to hexadecimal number
void printHex(byte *buffer, byte bufferSize) {
  Serial.print("1 ");
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// ID to decimal number
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}


MFRC522 rfid(SS_PIN, RST_PIN); // 'rfid' 이름으로 클래스 객체 선언
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];   // 카드 ID들을 저장(비교)하기 위한 배열(변수)선언

// LoadCell calibration
int calibration_factor = -480;
int calibration_factor2 = -480;

HX711 scale1;
HX711 scale2;

// down angle, up angle
Servo outputServo, closeServo;
customServo outputModule = customServo(OUTPUT_DOWN, OUTPUT_UP, outputServo);
customServo closeModule = customServo(CLOSE_OPEN, CLOSE_CLOSE, closeServo);
moveCartridge movecartridge = moveCartridge(PUL, DIR, ENA) ;
Magnet magnet;

//////////////////////////////////////////////////////////////////////////////////
void setup() {
  // Serial setting
  Serial.begin(9600);          
  SPI.begin();
  rfid.PCD_Init();
  // ID reset
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Module servoMotor setting
  outputServo.attach(OUTPUT_SERVO_PIN);
  closeServo.attach(CLOSE_SERVO_PIN);

  pinMode(OUTPUT_SERVO_PIN, OUTPUT);
  pinMode(CLOSE_SERVO_PIN, OUTPUT);
  pinMode(MAGNET_RELAY, OUTPUT);
  pinMode(AIRPUMP_PIN_1, OUTPUT);
  pinMode(AIRPUMP_PIN_2, OUTPUT);
  pinMode(AIRPUMP_PIN_ENA, OUTPUT);
  pinMode(VACUUM_PIN_1, OUTPUT);
  pinMode(VACUUM_PIN_2, OUTPUT);
  pinMode(VACUUM_PIN_ENA, OUTPUT);
  pinMode(VIBRATION_PIN_1,OUTPUT);
  pinMode(VIBRATION_PIN_2,OUTPUT); 
  pinMode(VIBRATION_PIN_ENA,OUTPUT);
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(SWITCH, INPUT);

  
  // LoadCell1 setting
  scale1.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale1.set_scale(calibration_factor);
  scale1.tare();
  
  // LoadCell2 setting
  scale2.begin(LOADCELL_DOUT_PIN2, LOADCELL_SCK_PIN2); 
  scale2.set_scale(calibration_factor2);
  scale2.tare();
  
  outputModule.up();
  closeModule.close();
  magnet.off();
  movecartridge.sleep();
  // movecartridge.toStartingPoint(START_CALIBRATION);

}



/*
 *
 *  1. 시리얼 입력이 있을때 까지 대기
 *  2. 입력정보 (n번 카트리지 m(g)출력 정보 6개) 해석
 *  3. 출력, 진행상황 시리얼 송신
 * 
 *  현재 카트리지에서 n번째 카트리지로 이동하려면 [ -1600/6 * (이동할 카트리지 번호 - 현재 카트리지 번호) ]
 *  1 -> 2번 카트리지 -1600/6 * 1 만큼 회전
 *  1 -> 3번 카트리지 -1600/6 * 2 만큼 회전 
 *
 */


/////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  

  // wait String info from rasberry pi
  while(Serial.available() == 0) {}

  str = Serial.readString();

  // input String parsing start
  int first = str.indexOf(",");
  int N = str.substring(0, first).toInt();

  // if N>0, then edit saucelist
  if (N) {
    int second = str.indexOf(",", first+1);
    int third = str.indexOf(",", second+1);
    int StrLength = str.length();

    String CartNumStr = str.substring(first+1, second);
    String WeightStr = str.substring(second+1, third);
    String LiquidStr = str.substring(third+1, StrLength);

    // Cartrdige number Str parsing
    for (int i = 0; i < N; i++) {
      String tmp = (String) CartNumStr.charAt(2*i);
      sauce_info[0][i] = tmp.toInt();
      tmp = (String) LiquidStr.charAt(2*i);
      sauce_info[2][i] = tmp.toInt();
    }

    // WeightStr parsing
    for (int i = 0; i < N; i++) {
      String tmp;
      int index = WeightStr.indexOf(" ");
      StrLength = WeightStr.length();

      // 공백 전까지 읽고 저장
      tmp = WeightStr.substring(0,index);
      if (tmp == " ") {
        tmp == "0";
      }
      sauce_info[1][i] = tmp.toInt();

      // 읽은 부분 자름
      WeightStr = WeightStr.substring(index+1,StrLength);
    }

    Serial.print("0 Arduino Input : | ");
    for (int i = 0; i < 3; i++) {
        for(int t = 0; t < N; t++) {
          Serial.print(sauce_info[i][t]);
          Serial.print(" "); 
        }
        Serial.print(" | "); 
    }
    Serial.println("");

    // MainPrintProcess
    movecartridge.toStartingPoint(START_CALIBRATION);

    for (int i = 0 ; i < N ; i++) {
      if (sauce_info[1][i] == 0) {
        continue;
      }
      // move to cartridge to output
      movecartridge.move(sauce_info[0][i]);
      delay(1000);

      // Output module coupling
      outputModule.down();
      delay(1000);

      // Output module detach and magnet on
      outputModule.detach();
      magnet.on();
      delay(1000);

      // vacuum motor on
      runVacuumMotor(255);
      delay(1000);

      // Cover module open
      closeModule.open();
      delay(1000);

      runVacuumMotor(0);

      // Select output method
      if (sauce_info[2][i] == 1) {
        Serial.println("0 Liquid");
        runAirMotor(255);
      } else {
        Serial.println("0 not Liquid");
        runVibrationMotor(255);
      }
      
      // Reset LoadCell //
      double Weight = 0;
      scale1.tare();
      scale2.tare();
      ////////////////////

      // Starting weighing
      Serial.println("0 Measuring the weight.");
      while(Weight <= sauce_info[1][i]) {
        Weight = scale1.get_units() + scale2.get_units();
        //  Serial.println(Weight);
      }

      // End of weight measurement
      Serial.print("0 ");
      Serial.print(Weight);
      Serial.println("g");
      Serial.println("0 Measuring is over.");

      // close cover
      if (sauce_info[2][i] == 1) {
        runVacuumMotor(255);
        runAirMotor(0);
        delay(1000);

        closeModule.close();
        runVacuumMotor(0);
      } else {
        runVibrationMotor(0);
        delay(1000);

        closeModule.close();
      }

      // Magnet off
      magnet.off();
      delay(1000);

      // Output module up
      outputModule.attach();
      delay(500);
      outputModule.up();

      delay(2000);
    }

    Serial.println("0 End cycle");
    movecartridge.resetCartNum();

  } else {

    // if N == 0 then, start sauce scan
    startScan();
    
  }

  delay(2000);
  movecartridge.sleep();
}


// Sauce Scan Method
void startScan() {
  String id[6][4]; 
  movecartridge.toStartingPoint(START_CALIBRATION);
  for (int i = 0; i < 6; i++) {

    movecartridge.move(i+1);
    // delay(1000);
    int check = 0;
    int cnt = 0;
    while (1)
    {
      // delay(10);
      cnt++;
      if (cnt > 5) break;

      // 새카드 접촉이 있을 때만 다음 단계로 넘어감
      if ( ! rfid.PICC_IsNewCardPresent())
        continue;
        // break;

      // 카드 읽힘이 제대로 되면 다음으로 넘어감
      if ( ! rfid.PICC_ReadCardSerial())
        continue;
        // break;
      // 현재 접촉 되는 카드 타입을 읽어와 모니터에 표시함
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.println(rfid.PICC_GetTypeName(piccType));
      // MIFARE 방식의 카드인지 확인 루틴
      if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && 
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return;
      }
      
      // 이전 인식된 카드와 다른, 혹은 새카드가 인식되면
      if (rfid.uid.uidByte[0] != nuidPICC[0] ||
        rfid.uid.uidByte[1] != nuidPICC[1] ||
        rfid.uid.uidByte[2] != nuidPICC[2] ||
        rfid.uid.uidByte[3] != nuidPICC[3] ) {
        Serial.println(F("A new card has been detected."));
        
      // 고유아이디(UID) 값을 저장한다.
        for (byte i = 0; i < 4; i++) {
          nuidPICC[i] = rfid.uid.uidByte[i];
        }
        check = 1;

        break;
      }
 
      // 연속으로 동일한 카드를 접촉하면 다른 처리 없이
      // '이미 인식된 카드'라는 메세지를 출력한다.
      else Serial.println(F("Card read previously."));
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
    Serial.print("0 end scan : ");
    Serial.println(i+1);

    if(check) {
      for (int j = 0; j < 4; j ++) {
        id[i][j] = rfid.uid.uidByte[j];
        Serial.println("scan");
      }          
    } else {
      for (int j = 0; j < 4; j ++) {
        id[i][j] = "0";
      }          
    }    
  }

  Serial.println("1");
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 4; j++) {
      Serial.print(id[i][j]);
      Serial.print(" ");
    }
    Serial.println("");
  }
}


// 2,5 6 ,5 12 ,1 1
// 3,2 5 6 ,5 5 12 ,1 1 1
// 5,2 3 4 5 6 ,5 5 5 5 5 ,1 1 1 1 1
