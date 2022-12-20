/*=============================================
    Program : Traffic Lights
    Author : LAPTOP-MHHCOEAP\Charitha Dasanayake
    Time : 8:57 PM
    Date : 10/4/2021
    Created with Codino Studio
  =============================================*/

#define ARRAY_SIZE(array) ((sizeof(array))/(sizeof(array[0])))
#define modeSwitch1 27
#define modeSwitch2 38
#include <LiquidCrystal.h>

LiquidCrystal lcd(28, 29, 30, 31, 32, 33);

//don't use equal traffic densities | maximum is 7, minimum is 1
//4 pins to Read green lights status
//add buttons to seprate functions

// Red, Yellow, Green
// 2, 3, 4 Lane 1
// 5, 6, 7 Lane 2
// 8, 9, 10 Lane 3
// 11, 12, 13 Lane 4
const byte light[4][3] = {
  {2, 3, 4},
  {5, 6, 7},
  {8, 9, 10},
  {11, 12, 13}
};

//to check green lights status
const byte greenLight[4] = {light[0][2], light[1][2], light[2][2], light[3][2]};

// 22 common A IR sensors
// 23, 24, 25, 26 othet B IR sensors
const byte irSensors[5] = {22, 23, 24, 25, 26};
const byte rfReceiver[4] = {34, 35, 36, 37};
int laneSeq[4];
int laneSeq2[4];
byte vehicles[5];
byte stayEmg = 0;
byte workMode = 1;
const byte commonDelay = 100;
boolean enterVehicle[4] = {false, false, false, false};
boolean greenLightPass = false;
boolean pressSwitch1 = false;
boolean fullDensity = true;
boolean modeOne = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  //set LED lights
  for (byte i = 0; i < ARRAY_SIZE(light); i++) {
    for (byte j = 0; j < ARRAY_SIZE(light[0]); j++) {
      pinMode(light[i][j], OUTPUT);
    }
  }
  pinMode(modeSwitch1, INPUT);
  pinMode(modeSwitch2, INPUT);
  //set IR sensors
  for (byte i = 0; i < ARRAY_SIZE(irSensors); i++) {
    pinMode(irSensors[i], INPUT);
  }

  for (byte i = 0; i < ARRAY_SIZE(rfReceiver); i++) {
    pinMode(rfReceiver[i], OUTPUT);
  }

  lcd.setCursor(1, 0);
  lcd.print("Traffic  Light");
  lcd.setCursor(5, 1);
  lcd.print("System");
  delay(3000);
}

void loop() {

  String nom;

  if (workMode == 1) {

    if (!modeOne) {
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Mode 1");

      Serial.print("Mode: ");
      Serial.println(workMode);

      delay(2000);
      lcd.clear();

      initCursor();
      nom = "NL";
      modeOneCursor(nom);

      fixedTime();
      lcd.clear();
      lcd.print("   Select Mode   ");
      modeOne = true;
    }

    fullDensity = true;

    if (digitalRead(modeSwitch1) == HIGH) {
      workMode = 2;
      modeOne = false;
    }
    if (digitalRead(modeSwitch2) == HIGH) {
      workMode = 4;
      modeOne = false;
    }

  }

  if (workMode == 2) {
    Serial.println("Mode: 2-1");

    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Mode 2-1");
    delay(2000);
    lcd.clear();
    initCursor();

    on_off_RG_Lights(1, 1); //keep first lane greenLight
    while (fullDensity) {
      count_vehicle();
      for (byte i = 1; i < ARRAY_SIZE(vehicles); i++) {
        if (vehicles[i] >= 7) { //set maximum density for a lane
          fullDensity = false;
          Serial.print("Reached to maximum density at lane ");
          Serial.println(i);
          workMode = 3;
          Serial.println("Mode: 2-2");
        }
      }
    }
    if (workMode == 3) {
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Mode 2-2");
      delay(2000);
      lcd.clear();
      initCursor();
      for (byte i = 1; i < ARRAY_SIZE(vehicles); i++) {
        displayVehNo(i);
      }

      on_off_RG_Lights(1, 0); //turn into workMode 2
      maxtomin_density();
      for (byte i = 0; i < ARRAY_SIZE(laneSeq); i++) {
        on_off_RG_Lights(laneSeq2[i], 1);
        while (vehicles[laneSeq2[i]] > 0) {
          count_vehicle(); //count traffic light leaving vehicles
          Serial.print("vehicles are leaving from lane ");
          Serial.println(laneSeq2[i]);
        }
        on_off_RG_Lights(laneSeq2[i], 0);
        if (i < 3) {
          on_off_Y_Lights((byte)laneSeq2[i], (byte)laneSeq2[i + 1], 1);
          delay(1000);
          on_off_Y_Lights((byte)laneSeq2[i], (byte)laneSeq2[i + 1], 0);
        } else {
          digitalWrite(light[laneSeq2[i]][1], 1);
          delay(1000);
          digitalWrite(light[laneSeq2[i]][1], 0);
        }
      }
      Serial.println("-------------return to Mode 1--------------");
      workMode = 1;
      for (byte i = 0; i < ARRAY_SIZE(vehicles); i++) {
        vehicles[i] = 0;
      }
    }

  }
  else if (workMode == 4) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Mode 3");
    delay(2000);
    lcd.clear();
    
    initCursor();
    nom = "NL";
    modeOneCursor(nom);

    on_off_RG_Lights(1, 1);
    //emg code
    for (byte i = 1; i < ARRAY_SIZE(rfReceiver); i++) {
      if(rfReceiver[i]==HIGH){
          stayEmg = i+1;
          
          
      }
    }
    on_off_RG_Lights(1, 0);
    //work as fixted time + particular road
    


  }

}

//Turn on Off Red & Green lights
void on_off_RG_Lights(byte onLight_Lane, byte mode) {
  for (byte i = 0; i < ARRAY_SIZE(light); i++) {
    if (i == (onLight_Lane - 1)) {
      digitalWrite(light[i][2], mode);
    } else {
      digitalWrite(light[i][0], mode);
    }
  }
}

//Turn On Off Yellow lights
void on_off_Y_Lights(byte lane1, byte lane2, byte mode) {
  digitalWrite(light[lane1 - 1][1], mode);
  digitalWrite(light[lane2 - 1][1], mode);
  Serial.println("------------------Yellow------------------");
}

//Turn On Off Red Yellow lights
void on_off_YR_Lights(byte YellowLane, byte mode) {
  boolean count2 = false;
  byte lane = YellowLane;
  for (byte i = 0; i < ARRAY_SIZE(light); i++) {
    if (i == (lane - 1)) {
      digitalWrite(light[i][1], mode);
      if (YellowLane == 4) {
        digitalWrite(light[0][1], mode);
        digitalWrite(light[i][0], !mode);
      } else {
        if (!count2) {
          lane++;
          count2 = true;
        }
      }
    } else {
      digitalWrite(light[i][0], mode);
    }

  }
}

//Fixed time lights (time period 1 s)
void fixedTime() {
  for (byte i = 1; i <= ARRAY_SIZE(light); i++) {
    on_off_RG_Lights(i, 1);
    delay(1000);
    on_off_RG_Lights(i, 0);
    on_off_YR_Lights(i, 1);
    delay(1000);
    on_off_YR_Lights(i, 0);
  }
}

//count vehicle density on all roads
void count_vehicle() {
  for (byte i = 1; i < ARRAY_SIZE(irSensors); i++) {

    // !!!!! take the first condition IR sensor sends value HIGH !!!!!

    if ((digitalRead(irSensors[i])) == HIGH) {
      if (enterVehicle[i-1] == false) {
        enterVehicle[i-1] = true;
        vehicles[i]++;  // works with B IR sensors
        Serial.print("lane ");
        Serial.print(i);
        Serial.print(" no. vehicles enter ");
        Serial.println(vehicles[i]);

        displayVehNo(i);
        delay(100);
      }
    }
    if ((digitalRead(irSensors[i])) == LOW) {
      enterVehicle[i-1] = false;
    }
    if (digitalRead(greenLight[i - 1]) == HIGH && (digitalRead(irSensors[0])) == HIGH) {
      if (greenLightPass == false) {
        greenLightPass = true;
        vehicles[i]--;  // works with A IR sensors
        Serial.print("lane ");
        Serial.print(i);
        Serial.print(" no. vehicles exit ");
        Serial.println(vehicles[i]);

        displayVehNo(i);
        delay(100);
      }
    }
    if (digitalRead(greenLight[i - 1]) == HIGH && (digitalRead(irSensors[0])) == LOW) {
      greenLightPass = false;
    }
  }
}

void maxtomin_density() {
  for (int i = 1; i < ARRAY_SIZE(vehicles); i++) {
    laneSeq[i - 1] = vehicles[i];
  }

  qsort (laneSeq, ARRAY_SIZE(laneSeq), sizeof(int), myCompareFunction); //sort array max to min

  delay(commonDelay);
  //sort arrray index
  for (int i = 1; i < ARRAY_SIZE(vehicles); i++) {
    for (int j = 1; j < ARRAY_SIZE(vehicles); j++) {
      if (laneSeq[i - 1] == vehicles[j]) {
        laneSeq2[i - 1] = j;
      }
    }
  }
  for (int i = 0; i < ARRAY_SIZE(laneSeq2); i++) {
    Serial.print(laneSeq2[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

int myCompareFunction (const void * arg1, const void * arg2) {
  int * a = (int *) arg1;
  int * b = (int *) arg2;

  if (*a < *b)
    return 1;

  if (*a > *b)
    return -1;

  return 0;
}

void initCursor() {

  lcd.setCursor(1, 0);
  lcd.print("L1:");
  lcd.setCursor(9, 0);
  lcd.print("L2:");
  lcd.setCursor(1, 1);
  lcd.print("L3:");
  lcd.setCursor(9, 1);
  lcd.print("L4:");
}

void modeOneCursor(String nom) {

  lcd.setCursor(5, 0);
  lcd.print(nom);
  lcd.setCursor(13, 0);
  lcd.print(nom);
  lcd.setCursor(5, 1);
  lcd.print(nom);
  lcd.setCursor(13, 1);
  lcd.print(nom);
}

void displayVehNo(byte i) {
  if (i == 1) {
    lcd.setCursor(5, 0);
  } else if (i == 2) {
    lcd.setCursor(13, 0);
  } else if (i == 3) {
    lcd.setCursor(5, 1);
  } else if (i == 4) {
    lcd.setCursor(13, 1);
  }
  lcd.print(vehicles[i]);
}
