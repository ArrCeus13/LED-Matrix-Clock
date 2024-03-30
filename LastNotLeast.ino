#include "DS3231.h" // Access RTC
#include "Adafruit_GFX.h" // Dot Matrix Display
#include "PS2Keyboard.h" // Keyboard Input
#include "Max72xxPanel.h" // Font Setting on Adafruit
#include <MD_MAX72xx.h>


DS3231 rtc(SDA, SCL); // RTC Init


const byte LDR_PIN = A7; // LDR Sensor pin
const byte CS_PIN = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const byte LM_PIN = A6;
const byte H_DISPLAYS = 4; // Horizontal displays number
const byte V_DISPLAYS = 1; // Vertical displays number
const byte DataPin = 2; // D-
const byte IRQpin = 3; // D+
const byte BUZ = 4; //buzzer 
bool isFirstTime = true;

Max72xxPanel matrix = Max72xxPanel(CS_PIN, H_DISPLAYS, V_DISPLAYS);
PS2Keyboard keyboard; // Keyboard Object

const byte WAIT = 45; // Delay
const byte SPACER = 1; // Space Between Font
const byte FONT_WIDTH = 3 + SPACER; // The font width is 5 pixels


int waktu = 0; 
float suhu;

struct Alarm {
  bool active;
  byte hours;
  byte minutes;
  byte duration; // duration in seconds
};

Alarm alarms[] = {
  { false },
  { false },
  { false }
}; // Save Alarm in Array

enum class STATE{WAKTU, TANGGAL, SUHU, MENU, SET_WAKTU, SELECT_ALARM, SET_ALARM, SET_DUR, ALARM_ACTIVE, SET_ALARM3, SET_TANGGAL,SET_YEAR};
STATE program_state; // State Setting

enum class M_STATE{JAM, ALARM, TANGGAL};
M_STATE menu_state; // Menu State Setting

enum class A_STATE{A1, A2, A3,};
A_STATE alarm_state; // Alarm State

String inputAlarmHours = "--";
String inputAlarmMinutes = "--";
String alarm3Input = ""; 
byte inputAlarmDuration = 0;
byte inputtedAlarm = 0; // Calculate the Input



byte activeAlarm = 0; // Which Alarm is Active
unsigned long alarmStartTime = 0;

String inputClockHours = "--";
String inputClockMinutes = "--";
String inputClockSeconds = "--";
byte inputtedClock = 0;

String inputTanggal = "--";
String inputBulan = "--";
String inputTahun = "--";
byte inputtedtangbul = 0;

String NRP = "5024201013";
String NAMA = "Arriza Fajar Zhafar Yasar";


String outputStrClock() {
  String _output;
  _output.concat(rtc.getTimeStr());
  return _output;
}


void displayRunningText(String text, int speed) {
  int textLength = text.length();
  int displayWidth = H_DISPLAYS * FONT_WIDTH;
  int startPosition = displayWidth;

  while (startPosition >= -textLength * FONT_WIDTH) {
    matrix.fillScreen(LOW);
    matrix.setCursor(startPosition, 0);
    matrix.print(text);
    matrix.write();
    delay(speed);
    startPosition--;
  }
}

String outputStrTemp() {
  String _outputtemp;
 //float suhurtc = rtc.getTemp();
  unsigned long temptot = 0;
  int waktu2 = millis();
  for (int x = 0; x<100; x++) {
      temptot += analogRead(LM_PIN);
    }
  if (waktu == 0 || (waktu2 - waktu) >= 1000 ) {
    
    float value = temptot/200; 
    float tegangan = value*2; 
    suhu = (tegangan/100)+26;
  }

  _outputtemp.concat("  ");
  _outputtemp.concat(suhu);    
  _outputtemp.concat((char)247);
  _outputtemp.concat("C");
  return _outputtemp;
}

String outputStrDate() {
  String _outputdate;
  _outputdate.concat(rtc.getDateStr(FORMAT_LITTLEENDIAN));
  return _outputdate;
}

void resetAlarmInput() {
  inputtedAlarm = 0;
  inputAlarmHours = "--";
  inputAlarmMinutes = "--";
  inputAlarmDuration = 0;
} // Reset Alarm Input, Press ESC to Chnage the Alarm to _ : _

void printMatrix(String output, int i, int start) {
  if (i == -1) { // i = -1, if want to print center
    i = (FONT_WIDTH * output.length() + matrix.width() + start) / 2;
  } else if (i == -2) { // i = -2, if want to print right alligned
    i = FONT_WIDTH * alarm3Input.length() - SPACER;
  }
  int letter = i / FONT_WIDTH;
  int x = (matrix.width() - 1) - i % FONT_WIDTH;
  int y = (matrix.height() - 8) / 2; // center the text vertically

  while ( x + FONT_WIDTH - SPACER >= 0 && letter >= 0 ) {
    if ( letter < output.length() ) {
      matrix.drawChar(x, y, output[letter], HIGH, LOW, 1);
    }
    --letter;
    x -= FONT_WIDTH;
  }
  matrix.write();
} // Print Matrix Function

void runningMatrix(String output)
{
  for ( int i = 0 ; i < FONT_WIDTH * output.length() + matrix.width() - 1 - SPACER; i++ ) 
  {
    matrix.fillScreen(LOW);
    int letter = i / FONT_WIDTH;
    int x = (matrix.width() - 1) - i % FONT_WIDTH;
    int y = (matrix.height() - 8) / 2; 
    while ( x + FONT_WIDTH - SPACER >= 0 && letter >= 0 ) 
    {
      if ( letter < output.length() ) 
      {
        matrix.drawChar(x, y, output[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= FONT_WIDTH;
    }
    matrix.write();
    delay(WAIT);
  }
}// running text function

void buzzer() {
  digitalWrite(BUZ, HIGH);
  delay(1000);        // ...for 1 sec
  digitalWrite(BUZ, LOW);
  // noTone(BUZ);     // Stop sound...
  delay(1000);        // ...for 1sec
}

void defaultMatrix() {
  STATE prevState = program_state; // Save State Before
  byte detik = rtc.getTime().sec;
  int s;
  String output;
  switch (program_state) { // Chechk State and Determine the Output
    case STATE::WAKTU:
      output = outputStrClock();
      s=1;
      break;
    case STATE::SUHU:
      output = outputStrTemp();
      s=8;
      break;
    case STATE::TANGGAL: 
      output = outputStrDate(); 
      s=1;
      break;
    case STATE::ALARM_ACTIVE:
      if (millis() - alarmStartTime > alarms[activeAlarm].duration * 1000) { // Check whether the alarm has exceeded its duration or not
        alarms[activeAlarm].active = false;
        program_state = STATE::WAKTU;
        return;
      } else { // Check which alarms are active
        switch (activeAlarm) {
          case 0:
            buzzer();
            output = NRP;
            runningMatrix(output);
            break;
          case 1:
            output = NRP + " ";
            output += NAMA;
            runningMatrix(output);
            break;
          case 2:
            output = alarm3Input;
            runningMatrix(output);
            break;
        }
        buzzer();
      }
      break;
    default: // when it's not on the switch, enter return
      return;
  }
  
    matrix.fillScreen(LOW);
    printMatrix(output, -1, s);
    detik = rtc.getTime().sec;
    byte menit = rtc.getTime().min;
    byte jam = rtc.getTime().hour;
    

    if (prevState != program_state) { // if the state changes due to interrupt do
      matrix.fillScreen(LOW);
      return;
    }

    if (program_state != STATE::ALARM_ACTIVE) {
      if ((detik >= 13 && detik < 16) || (detik >= 43 && detik < 46)) {
        program_state = STATE::SUHU;
      } else if ((detik >= 10 && detik < 13) || (detik >=40 && detik < 43)) {
        program_state = STATE::TANGGAL;
      } else {
        program_state = STATE::WAKTU;
      }
      for (byte j = 0; j < 3; ++j) { // check whether there is an active alarm on it
        if (alarms[j].hours == jam && alarms[j].minutes == menit && alarms[j].active) {
          program_state = STATE::ALARM_ACTIVE;
          activeAlarm = j;
          alarmStartTime = millis();
        }
      }
    } else {
      if (millis() - alarmStartTime > alarms[activeAlarm].duration * 1000) { // nCheck whether the alarm has exceeded its duration or not
        alarms[activeAlarm].active = false;
        program_state = STATE::WAKTU;
        return;
      }
    }

    if (prevState != program_state) { // if the state changes due to interrupt do
      matrix.fillScreen(LOW);
      return;
    }
    if (program_state == STATE::WAKTU) {
      output = outputStrClock();
    }
//    if (program_state == STATE::ALARM_ACTIVE && activeAlarm == 2) { // supaya detiknya keupdate
//      output = String(alarms[activeAlarm].duration - ((millis() - alarmStartTime) / 1000)) + " s";
//    }
    delay(WAIT);
  }

int nilai1;
void keyboardHandler() { // for interrupt keyboard
  if (!keyboard.available()) {
    return;    
  }
  char key = keyboard.read(); 
  switch(program_state) {
    case STATE::WAKTU:
    case STATE::SUHU:    
      if (key == PS2_ENTER) {
        program_state = STATE::MENU;
      }
      break;
    case STATE::MENU:
      if (key == PS2_LEFTARROW) {
        switch (menu_state) {
          case M_STATE::JAM:
            menu_state = M_STATE::ALARM;
            break;
          case M_STATE::ALARM:
            menu_state = M_STATE::TANGGAL;
            break;
            case M_STATE::TANGGAL:
            menu_state = M_STATE::JAM;
            break;
        }
      } else if (key == PS2_RIGHTARROW) {
        switch (menu_state) {
          case M_STATE::JAM:
            menu_state = M_STATE::ALARM;
            break;
          case M_STATE::ALARM:
            menu_state = M_STATE::TANGGAL;
            break;
            case M_STATE::TANGGAL:
            menu_state = M_STATE::JAM;
            break;
        }
      } else if (key == PS2_ESC) {
        menu_state = M_STATE::JAM;
        program_state = STATE::WAKTU;
      } else if (key == PS2_ENTER) {
        if (menu_state == M_STATE::JAM) {
          program_state = STATE::SET_WAKTU;
        } else if (menu_state == M_STATE::ALARM) {
          program_state = STATE::SELECT_ALARM;
        } 
        else if (menu_state == M_STATE::TANGGAL) {
          program_state = STATE::SET_TANGGAL;
        } 
      }
      break;
    case STATE::SET_WAKTU:
      if (key == PS2_ESC) {
        program_state = STATE::MENU;
      } else if (key >= '0' && key <= '9') {
        switch (inputtedClock) {
          case 0:
            if (key > '2') { // so that the starting time is no later than 2
              break;
            }// _ _ -> 1 _
            inputClockHours = String(key) + String(inputClockHours[1]);
            ++inputtedClock;
            break;
          case 1:
            if (inputClockHours[0] > '1' && key > '3') {
              break;
            }
            // 1 _ -> 1 9
            inputClockHours = String(inputClockHours[0]) + String(key);
            ++inputtedClock;
            break;
          case 2:
            if (key > '5') {
              break;
            }
            inputClockMinutes = String(key) + String(inputClockMinutes[1]);
            ++inputtedClock;
            break;
          case 3:
            inputClockMinutes = String(inputClockMinutes[0]) + String(key);
            ++inputtedClock;
            break;
            case 4:
            if (key > '5') 
            {
              break;
            }
            inputClockSeconds = String(key) + String(inputClockSeconds[1]);
            ++inputtedClock;
            break;
          case 5:
            inputClockSeconds = String(inputClockSeconds[0]) + String(key);
            ++inputtedClock;
            break;
          default:
            break;
        }
      } else if (key == PS2_BACKSPACE) {
        // 19 23 -> inputtedClock = 4        
        switch (inputtedClock) {
          case 1:
            inputClockHours = "--";
            --inputtedClock;
            break;
          case 2:
            inputClockHours = String(inputClockHours[0]) + "-";
            --inputtedClock;
            break;
          case 3:
            inputClockMinutes = "--";
            --inputtedClock;
            break;
          case 4:
            inputClockMinutes = String(inputClockMinutes[0]) + "-";
            --inputtedClock;
            break;
          case 5:
            inputClockSeconds = "--";
            --inputtedClock;
            break;
          case 6:
            inputClockSeconds= String(inputClockSeconds[0]) + "-";
            --inputtedClock;
            break;
          default:
            break;
        }
      } else if (key == PS2_ENTER) {
        if (inputtedClock >= 4) {
          byte hour = inputClockHours.toInt();
          byte min = inputClockMinutes.toInt();
          byte sec = inputClockSeconds.toInt();
          rtc.setTime(hour, min, sec);
          program_state = STATE::WAKTU;
        }
      }
      break;

      case STATE::SET_TANGGAL:
      if (key == PS2_ESC) {
        program_state = STATE::MENU;
      } else if (key >= '0' && key <= '9') {
        switch (inputtedtangbul) {
          case 0:
            if (key > '3') { // so that the date is no later than 3
              break;
            }// _ _ -> 1 _
            inputTanggal = String(key) + String(inputTanggal[1]);
            ++inputtedtangbul;
            break;
          case 1:
            if (inputTanggal[0] >= '3' && key > '1') {
              break;
            }
            // 1 _ -> 1 9
            inputTanggal = String(inputTanggal[0]) + String(key);
            ++inputtedtangbul;
            break;
          case 2:
            if (key > '1') {
              break;
            }
            inputBulan = String(key) + String(inputBulan[1]);
            ++inputtedtangbul;
            break;
          case 3:
          if(inputBulan[0] >= '1' && key > '2' )
          {
            break;
          }
            inputBulan = String(inputBulan[0]) + String(key);
            ++inputtedtangbul;
            break;
          case 4:
            if (key > '10') 
            {
              break;
            }
            inputTahun = String(key) + String(inputTahun[1]) + String(inputTahun[2]) + String(inputTahun[3]);
            ++inputtedtangbul;
            break;
          case 5:
            if (inputTahun[0] > '1' && key > '9') 
            {
              break;
            }
            inputTahun = String(inputTahun[0]) + String(key) + String(inputTahun[2]) + String(inputTahun[3]);
            ++inputtedtangbul;
            break;
          case 6:
            if (inputTahun[0] > '1' && key > '9') 
            {
              break;
            }
            inputTahun = String(inputTahun[0]) + String(inputTahun[1]) + String(key) + String(inputTahun[3]);
            ++inputtedtangbul;
            break;
          case 7:
            if (inputTahun[0] > '1' && key > '9') 
            {
              break;
            }
            inputTahun = String(inputTahun[0]) + String(inputTahun[1]) + String(inputTahun[2]) + String(key);
            ++inputtedtangbul;
            break;
          default:
            break;
        }
      } else if (key == PS2_BACKSPACE) {
        // 19 23 -> inputtedClock = 4        
        switch (inputtedtangbul) {
          case 1:
            inputTanggal = "--";
            --inputtedtangbul;
            break;
          case 2:
            inputTanggal = String(inputTanggal[0]) + "-";
            --inputtedtangbul;
            break;
          case 3:
            inputBulan = "--";
            --inputtedtangbul;
            break;
          case 4:
            inputBulan = String(inputBulan[0]) + "-";
            --inputtedtangbul;
            break;
          case 5:
        inputTahun = "--";
        --inputtedtangbul;
        break;
      case 6:
        inputTahun = String(inputTahun[0]) + String(inputTahun[1]) + String(inputTahun[2]) + "-";
        --inputtedtangbul;
        break;
      case 7:
        inputTahun = String(inputTahun[0]) + String(inputTahun[1]) + "" + "";
        --inputtedtangbul;
          break;
      case 8:
        inputTahun = String(inputTahun[0]) + "" + "" + "-";
        --inputtedtangbul;
          break;
        }
      } else if (key == PS2_ENTER) {
        if (inputtedtangbul >= 4) {
          uint16_t tahun = inputTahun.toInt() + 2000;
          byte tanggal = inputTanggal.toInt();
          byte bulan = inputBulan.toInt();
          rtc.setDate(tanggal, bulan, tahun);
          program_state = STATE::WAKTU;
        }
      }
      break;

    case STATE::SELECT_ALARM:
      if (key == PS2_ESC) {
        program_state = STATE::MENU;
      } else if (key == PS2_LEFTARROW) {
        switch (alarm_state) {
          case A_STATE::A1:
            alarm_state = A_STATE::A3;
            break;
          case A_STATE::A2:
            alarm_state = A_STATE::A1;
            break;
          case A_STATE::A3:
            alarm_state = A_STATE::A2;
            break;          
        }
      } else if (key == PS2_RIGHTARROW) {
        switch (alarm_state) {
          case A_STATE::A1:
            alarm_state = A_STATE::A2;
            break;
          case A_STATE::A2:
            alarm_state = A_STATE::A3;
            break;
          case A_STATE::A3:
            alarm_state = A_STATE::A1;
            break;           
        }
      } else if (key == PS2_ENTER) {
        if (alarm_state == A_STATE::A3) {
          program_state = STATE::SET_ALARM3;
        } else {
          program_state = STATE::SET_ALARM;          
        }
      }
      break;
    case STATE::SET_ALARM3:
      if (key == PS2_ENTER && alarm3Input.length() > 0) {
        program_state = STATE::SET_ALARM;        
      } else if (key == PS2_ESC) {
        program_state = STATE::SELECT_ALARM;
      } else if (key == PS2_BACKSPACE && alarm3Input.length() > 0) {
        alarm3Input.remove(alarm3Input.length() - 1, 1);
      } else {
        alarm3Input += String(key);
      }
      break;
    case STATE::SET_ALARM:
      if (key == PS2_ESC) {
        program_state = STATE::SELECT_ALARM;
        resetAlarmInput();
      } else if (key >= '0' && key <= '9') {
        switch (inputtedAlarm) {
          case 0:
            if (key > '2') {
              break;
            }
            inputAlarmHours = String(key) + String(inputAlarmHours[1]);
            ++inputtedAlarm;
            break;
          case 1:
            if (inputAlarmHours[0] > '1' && key > '3') {
              break;
            }
            inputAlarmHours = String(inputAlarmHours[0]) + String(key);
            ++inputtedAlarm;
            break;
          case 2:
            if (key > '5') {
              break;
            }
            inputAlarmMinutes = String(key) + String(inputAlarmMinutes[1]);
            ++inputtedAlarm;
            break;
          case 3:
            inputAlarmMinutes = String(inputAlarmMinutes[0]) + String(key);
            ++inputtedAlarm;
            break;
          default:
            break;
        }
      } else if (key == PS2_BACKSPACE) {
        switch (inputtedAlarm) {
          case 1:
            inputAlarmHours = "--";
            --inputtedAlarm;
            break;
          case 2:
            inputAlarmHours = String(inputAlarmHours[0]) + "-";
            --inputtedAlarm;
            break;
          case 3:
            inputAlarmMinutes = "--";
            --inputtedAlarm;
            break;
          case 4:
            inputAlarmMinutes = String(inputAlarmMinutes[0]) + "-";
            --inputtedAlarm;
            break;
          default:
            break;
        }
      } else if (key == PS2_ENTER) {
        if (inputtedAlarm >= 4) {
          program_state = STATE::SET_DUR;
        }
      } else if (key == PS2_BACKSPACE) {
        byte index;
        switch (alarm_state) {
          case A_STATE::A1: {
            index = 0;
            break;
          }
          case A_STATE::A2: {
            index = 1;
            break;
          }
          case A_STATE::A3: {
            index = 2;
            break;
          }
        }
        alarms[index].active = false;
      }
      break;
    case STATE::SET_DUR:
      if (key == PS2_ESC) {
        program_state = STATE::SET_ALARM;
      } else if (key == PS2_UPARROW) {
        ++inputAlarmDuration;
      } else if (key == PS2_DOWNARROW) {
        --inputAlarmDuration;
      } else if (key == PS2_ENTER && inputAlarmDuration) {
        if (inputtedAlarm >= 4) {
          program_state = STATE::SET_DUR;
          byte index = 0;
          switch (alarm_state) { // to find out which index the alarm is active
            case A_STATE::A1:
              index = 0;
              break;
            case A_STATE::A2:
              index = 1;
              break;
            case A_STATE::A3:
              index = 2;
              break;
          break;
          }
          alarms[index].active = true;
          alarms[index].hours = inputAlarmHours.toInt();
          alarms[index].minutes = inputAlarmMinutes.toInt();
          alarms[index].duration = inputAlarmDuration;
          resetAlarmInput();
          program_state = STATE::WAKTU;
        }
      }
      break;
    case STATE::ALARM_ACTIVE:
      if (key == PS2_ESC) {
        program_state = STATE::WAKTU;
        alarms[activeAlarm].active = false;
      }
      break;
  }
}

void setup() {
  pinMode(LDR_PIN, INPUT_PULLUP);
  pinMode(BUZ, OUTPUT);
  keyboard.begin(DataPin, IRQpin); // start keyboardnya
  Serial.begin(9600);
  rtc.begin();
  matrix.setPosition(0, 3, 0);  // Set position of panel 0 (top-right)
  matrix.setPosition(1, 2, 0);  // Set position of panel 1 (top-left)
  matrix.setPosition(2, 1, 0);  // Set position of panel 2 (bottom-right)
  matrix.setPosition(3, 0, 0);  // Set position of panel 3 (bottom-left)

  matrix.setRotation(0, 3);  // Set rotation of panel 0 to 2 (180 degrees clockwise)
  matrix.setRotation(1, 3);  // Set rotation of panel 1 to 2 (180 degrees clockwise)
  matrix.setRotation(2, 3);  // Set rotation of panel 2 to 2 (180 degrees clockwise)
  matrix.setRotation(3, 3);  // Set rotation of panel 3 to 2 (180 degrees clockwise)

  attachInterrupt(digitalPinToInterrupt(DataPin), keyboardHandler, FALLING); // install interrupt
}

void loop() {
  matrix.fillScreen(LOW); // clear matrix
  const int AnalogValue = analogRead(A0);
  const int intensity = map(AnalogValue, 0,1023,15,0);
  matrix.setIntensity(intensity);
  nilai1 = analogRead(LM_PIN);
    if (isFirstTime) {
    runningMatrix("5024201013 Arriza Fajar Zhafar Yasar"); // Display initial text as running text
    isFirstTime = false; // Update the flag to indicate it's not the first time anymore
  }
  else{
  if (program_state == STATE::ALARM_ACTIVE && millis() - alarmStartTime > alarms[activeAlarm].duration * 1000) { // Check whether the alarm has exceeded its duration or not
    alarms[activeAlarm].active = false;
    program_state = STATE::WAKTU;
  } 
  switch (program_state) { // if it's in a loop, check the state
    case STATE::WAKTU:
    case STATE::SUHU:
    case STATE::TANGGAL:
      defaultMatrix();
      break;
    case STATE::ALARM_ACTIVE: {
      buzzer();
      defaultMatrix();
      break;
    }
    case STATE::MENU: {
      switch (menu_state) {
        case M_STATE::JAM:
          printMatrix("TIME", -1, 1);
          break;
        case M_STATE::ALARM:
          printMatrix("ALARM", -1, 1);
          break;
          case M_STATE::TANGGAL:
          printMatrix("DATE", -1, 1);
          break;
      }
      break;
    }
    case STATE::SET_WAKTU: {
      printMatrix(inputClockHours + ":" + inputClockMinutes + ":" + inputClockSeconds, -1, 1);
      break;   
    }
    case STATE::SET_TANGGAL: {
      printMatrix(inputTanggal + "." + inputBulan + "." + inputTahun, -1, 1);
      break;   
    }
    case STATE::SELECT_ALARM: {
      switch (alarm_state) {
        case A_STATE::A1:
          printMatrix("1", -1, 1);
          break;
        case A_STATE::A2:
          printMatrix("2", -1, 1);
          break;
        case A_STATE::A3:
          printMatrix("3", -1, 1);
          break;
      }
      break;
    }
    case STATE::SET_ALARM3: {
      printMatrix(alarm3Input, -2, 1);
      break;
    }
    case STATE::SET_ALARM: {
      printMatrix(inputAlarmHours + ":" + inputAlarmMinutes, -1, 1);
      break;
    }
    case STATE::SET_DUR: {
      printMatrix(String(inputAlarmDuration) + " s", -1, 1);
      break;
    }

  delay(60);
   }
  }
}
