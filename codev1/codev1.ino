#include <Wire.h>
#include <Sodaq_SHT2x.h>
#include "Sodaq_DS3231.h"
#include <Ultrasonic.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
DateTime dt(2016, 8, 31, 15, 18, 0, 5);

int trigpin = 3;//appoint trigger pin
int echopin = 2;//appoint echo pin
Ultrasonic ultrasonic(trigpin, echopin);
int in1B = 8 ;
int in1A = 7;
int pwm1 = 6;
int en = 9;
int cs = 10;

int LED_out = 4;
int pume_out = 11;
int temp_s = 0;
float cmdistance, indistance;
int mod = 0 ;
int mod_mod = 0 ;
int start_new = 0, mod1_t_on = 4, mod1_t_off = 4, mod2_t_f = 20, mod2_di = 10, mod3_t_LED = 4, t_ref = 26;
int mod1_t_on_set = 0, mod1_t_off_set = 0, di_ref, mod3_t_LED_set = 0;
int year_new, month_new, day_new, hours_new, minute_new;
float h, t;
unsigned long previousMillis = 0;
int conter = 0; int conter_led13 = 0;
int x = 0;
void time_sed ();
void C_sed ();
void mod_s () ;
void js_sed();
float limit(float input, int min_limit, int max_limit)
{
  if (input > max_limit)input = max_limit;
  if (input < min_limit)input = min_limit;
  return input;
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(100);
  pinMode(in1B, OUTPUT);
  pinMode(in1A, OUTPUT);
  pinMode(en, OUTPUT);
  pinMode(cs, OUTPUT);
  pinMode(pwm1, OUTPUT);
  pinMode(LED_out, OUTPUT);
  pinMode(pume_out, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(en, HIGH);
  digitalWrite(cs, HIGH);
  analogWrite(pwm1, 150);
  //////////////////////////
  Wire.begin();
  rtc.begin();
  //rtc.setDateTime(dt);
  ///////////////////////
  mod1_t_on_set = EEPROM.read(9);
  mod1_t_off_set = EEPROM.read(10);
  mod3_t_LED_set = EEPROM.read(11);
  di_ref = EEPROM.read(12);
  t_ref = EEPROM.read(6);

 if(EEPROM.read(6)< 23 && EEPROM.read(6)>30){
  EEPROM.write(1, 4);
  EEPROM.write(2, 4);
  EEPROM.write(3, 60);
  EEPROM.write(4, 9);
  EEPROM.write(5, 4);
  EEPROM.write(6, 25);
 }
}

void loop() {
  /////////
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    long microsec = ultrasonic.timing();
    cmdistance = ultrasonic.CalcDistance(microsec, Ultrasonic::CM); //this result unit is centimeter
    //Serial.println(cmdistance);
    ///////////////////
    time_sed();
    ////////////////
    C_sed();
    ///////////////
    js_sed();
    //////////
    conter++;
    conter_led13++;
    ////////////
    mod_s();
    //////////
  } else {
    StaticJsonBuffer<200> jsonBuffer2;
    String json;
    if (Serial.available()) {
      json = Serial.readString();
      Serial.println(json);
      x = 1;
    }
    if (x == 1) {
      // Serial.println(json);
      JsonObject& root = jsonBuffer2.parseObject(json);
      //            if (!root.success()) {
      //              Serial.println("parseObject() failed");
      //            }

      mod1_t_on = root["data"][1];
      mod1_t_off = root["data"][2];
      mod2_t_f =  root["data"][3];
      mod2_di =    root["data"][4];
      mod3_t_LED = root["data"][5];
      t_ref =     root["data"][6];

      mod1_t_on = limit(mod1_t_on, 1, 10);
      mod1_t_off = limit(mod1_t_off, 1, 10);
      mod2_t_f = limit(mod2_t_f, 1, 200);
      mod2_di = limit(mod2_di, 5, 15);
      mod3_t_LED = limit(mod3_t_LED, 0, 1);
      t_ref = limit(t_ref, 23, 30);
      if (t_ref != 0) {
        //EEPROM.write(0, start_new);
        for (byte z = 0; z < 3; z++) {
          digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
          delay(1000);              // wait for a second
          digitalWrite(13, LOW);
          delay(1000);
        }
        start_new = root["data"][0];
        start_new = limit(start_new, 0, 1);
        //        if (start_new == 1) {
        //          EEPROM.write(7, 0);
        //          EEPROM.write(8, 0);
        //        }
        EEPROM.write(1, mod1_t_on);
        EEPROM.write(2, mod1_t_off);
        EEPROM.write(3, mod2_t_f);
        EEPROM.write(4, mod2_di);
        EEPROM.write(5, mod3_t_LED);
        EEPROM.write(6, t_ref);
      }
      x = 0;

    }
  }
  /////////

}
void cool(byte p) {

  if (p == 1) {
    digitalWrite(en, HIGH);
    digitalWrite(cs, HIGH);
    digitalWrite(in1B, HIGH);
    digitalWrite(in1A, LOW);
    analogWrite(pwm1, 255);
  }

  if (p == 0) {
    digitalWrite(en, HIGH);
    digitalWrite(cs, HIGH);
    digitalWrite(in1B, LOW);
    digitalWrite(in1A, LOW);
    analogWrite(pwm1, 0);
  }


}
void hot(byte p) {
  digitalWrite(en, HIGH);
  digitalWrite(cs, HIGH);
  digitalWrite(in1B, LOW);
  digitalWrite(in1A, HIGH);
  if (p > 1)
    p = 255;
  analogWrite(pwm1, p);
}
void time_sed ()
{
  DateTime now = rtc.now(); //get the current date-time
  year_new = now.year();
  month_new = now.month();
  day_new = now.date();
  hours_new = now.hour();
  //hours_new = now.minute();
  minute_new = now.minute();


}
void C_sed ()
{
  h = SHT2x.GetHumidity();
  t = SHT2x.GetTemperature();

  if (t > EEPROM.read(6)) {
    cool(1);
  } else {
    cool(0);
  }
}
void js_sed () {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["cmdistance"] = di_ref - cmdistance;
  root["Humidity"] = h;
  root["Temperature"] = t;
  root["LED"] = digitalRead(LED_out);
  root["mod"] = EEPROM.read(7);
  root["year"] = year_new;
  root["month"] = month_new;
  root["day"] = day_new;
  root["hour"] = hours_new;
  root["minute"] = minute_new;
  root.printTo(Serial);
  // This prints:
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}
  Serial.println();
}
void mod_s () {
  if (EEPROM.read(7) == 1) {
    if (start_new == 1) {
      start_new = 0;
      digitalWrite(LED_out, LOW);
      digitalWrite(pume_out, LOW);
      EEPROM.write(7, 0);
      EEPROM.write(8, 0);
    }
    if (EEPROM.read(8) == 1) {
      if (conter_led13 > 1) {
        digitalWrite(13,  !digitalRead(13));
        conter_led13 = 0;
      }
      if (conter >= 5) {
        digitalWrite(pume_out, !digitalRead(pume_out));
        conter = 0;
      }
      if (hours_new == mod1_t_on_set) {
        EEPROM.write(8, 2);
        mod1_t_off_set = hours_new + mod1_t_off;
        if (mod1_t_off_set > 23)
          mod1_t_off_set = mod1_t_off_set - 24;
        EEPROM.write(10, mod1_t_off_set);
      }

    } else if (EEPROM.read(8) == 2) {
      digitalWrite(pume_out, LOW);
      if (hours_new == mod1_t_off_set) {
        EEPROM.write(8, 0);
        EEPROM.write(7, 2);
        conter = 0;
      }
    }
  } else if (EEPROM.read(7) == 2) {
    if (conter_led13 > 2) {
      digitalWrite(13,  !digitalRead(13));
      conter_led13 = 0;
    }
    if (conter < 2) {
      if (start_new == 1) {
        start_new = 0;
        digitalWrite(LED_out, LOW);
        digitalWrite(pume_out, LOW);
        EEPROM.write(7, 0);
        EEPROM.write(8, 0);
      }
      digitalWrite(pume_out, HIGH);
    } else {
      digitalWrite(pume_out, LOW);
      if (conter >= mod2_t_f) {
        conter = 0;
      }
    }
    if (di_ref - cmdistance > mod2_di) {
      EEPROM.write(7, 3);
      mod3_t_LED_set = hours_new + mod3_t_LED;
      if (mod3_t_LED_set > 23) {
        mod3_t_LED_set = mod3_t_LED_set - 24;
        EEPROM.write(11, mod3_t_LED_set);
      }
    }
  } else if (EEPROM.read(7) == 3) {
    if (start_new == 1) {
      start_new = 0;
      digitalWrite(LED_out, LOW);
      digitalWrite(pume_out, LOW);
      EEPROM.write(7, 0);
      EEPROM.write(8, 0);
    }
    if (conter_led13 > 3) {
      digitalWrite(13,  !digitalRead(13));
      conter_led13 = 0;
    }
    digitalWrite(LED_out, HIGH);
    if (conter < 2) {
      digitalWrite(pume_out, HIGH);
    } else {
      digitalWrite(pume_out, LOW);
      if (conter >= mod2_t_f) {
        conter = 0;
      }
    }
    if (hours_new == mod3_t_LED_set) {
      digitalWrite(LED_out, LOW);
      digitalWrite(pume_out, LOW);
      conter = 0;
      EEPROM.write(7, 4);
    }

  } else if (EEPROM.read(7) == 4) {
    if (conter_led13 > 4) {
      digitalWrite(13,  !digitalRead(13));
      conter_led13 = 0;
    }
    if (start_new == 1) {
      start_new = 0;
      digitalWrite(LED_out, LOW);
      digitalWrite(pume_out, LOW);
      EEPROM.write(7, 0);
      EEPROM.write(8, 0);
    }
    //เสร็จกินได้

  } else {
    digitalWrite(13, LOW);
    if (start_new == 1) {
      EEPROM.write(7, 1);
      EEPROM.write(8, 1);
      mod1_t_on_set = hours_new + mod1_t_on;
      if (mod1_t_on_set > 23) {
        mod1_t_on_set = mod1_t_on_set - 24;
        EEPROM.write(9, mod1_t_on_set);
      }
      di_ref = cmdistance;
      EEPROM.write(12, di_ref);
      mod1_t_on = EEPROM.read(1);
      mod1_t_off = EEPROM.read(2);
      mod2_t_f = EEPROM.read(3);
      mod2_di = EEPROM.read(4);
      mod3_t_LED = EEPROM.read(5);
      t_ref = EEPROM.read(6);
      start_new = 0;
    }
  }
}

