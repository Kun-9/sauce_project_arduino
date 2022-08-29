//test openerServo
  // while (1)
  // {
  //   openerServo.down();
  //   delay(2000);

  //   openerServo.up();
  //   delay(2000);
  // }
  
  // digitalWrite(32, HIGH);
  // digitalWrite(33, LOW);
  //test motor
  // outputModule.down();
  // delay(1000);
  // closeModule.open();
  // delay(1000);
  // while (1)
  // {
  //   runVibrationMotor(255);
  //   delay(200);
  //   runVibrationMotor(0);
  //   delay(200);
  // }

  //test ViberationMotor
  // digitalWrite(VIBRATION_PIN_1, HIGH);            // H-Bridge Switch ON
  // digitalWrite(VIBRATION_PIN_2, LOW);             // H-Bridge Switch LOW
  // while (1)
  // {  
  //   for (int i = 0; i < 255; i += 50) {
  //   analogWrite(VIBRATION_PIN_ENA, i);   // PWM 신호를 value만큼 인가
  //   delay(1000);                      // 1초 delay
  //   }
  //   for (int i = 255; i > 0; i -= 50) {
  //     analogWrite(VIBRATION_PIN_ENA, i);   // PWM 신호를 value만큼 인가
  //     delay(1000);                      // 1초 delay
  //   }
  // }

  //test servo
  // while (1)
  // {
  //   moduleservo.up();
  //   delay(2000);
  //   moduleservo.down();
  //   delay(2000);
  //   moduleservo.detach();
  //   delay(100000);
  // }

  // test vacum
  // delay(1000);
  // moduleservo.down();
  // while (1) {
  //   runVibrationMotor(255);
  //   delay(2000);
  //   runVibrationMotor(0);
  //   runAirMotor(255);
  //   delay(2000);
  //   runAirMotor(0);
  // } 

  // test magnet relay
  // while (1)
  // {
  //   digitalWrite(MAGNET_RELAY ,LOW);
  //   delay(2000);
  // }