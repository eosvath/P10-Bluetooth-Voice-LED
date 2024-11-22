/**
*   ESP32 controlled P10 LED MATRIX displaying Bluetooth input
*   Code Based on SerialBT and DMD32 library examples
*   @Author: Egon Csaba Osvath
*/

#include "BluetoothSerial.h"
#include <DMD32.h>
#include "fonts/Arial_black_16.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define DISPLAYS_ACROSS 3
#define DISPLAYS_DOWN 1
#define THIRTY_SECONDS_IN_MILLIS 30000U

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// Timer setup.
// create a hardware timer  of ESP32
hw_timer_t * timer = NULL;

// Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, 
// this gets called at the period set in Timer1.initialize();
void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  if (!SerialBT.begin("Voice LED", false)) {
    Serial.println("========== serialBT failed!");
    abort();
  }
  
  Serial.println();
  Serial.println("return the clock speed of the CPU.");
  // return the clock speed of the CPU.
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  delay(500);
  // Use 1st timer of 4.
  // devide cpu clock speed on its speed value by MHz to get 1us for each signal  of the timer.
  timer = timerBegin(0, cpuClock, true);
  delay(500);

  Serial.println();
  Serial.println("Attach triggerScan function to our timer.");
  // Attach triggerScan function to our timer.
  timerAttachInterrupt(timer, &triggerScan, true);
 
  // Set alarm to call triggerScan function.
  // Repeat the alarm (third parameter).
  timerAlarmWrite(timer, 300, true);
  delay(500);

  Serial.println();
  Serial.println("Start an alarm.");
  // Start an alarm.
  timerAlarmEnable(timer);

}

// Subroutine for displaying "running text" on P10 in Single Row mode.
void Single_Row_Display_Mode(String single_Row_Txt) {
  char CA_single_Row_Txt[single_Row_Txt.length() + 1];
  single_Row_Txt.toCharArray(CA_single_Row_Txt, single_Row_Txt.length() + 1);

  Serial.println(CA_single_Row_Txt);

  dmd.clearScreen(true);
  dmd.selectFont(Arial_Black_16);
  dmd.drawMarquee(CA_single_Row_Txt, single_Row_Txt.length(), (32*DISPLAYS_ACROSS)-1, 0);
  long timer = millis();
  boolean ret=false;
  // Change the following value for text speen, lower is faster 
  uint8_t milliseconds_per_caracter = 30;
  while(!ret){
   if ((timer + milliseconds_per_caracter) < millis()) {
     ret=dmd.stepMarquee(-1,0);
     timer=millis();
   }
  }
  delay(500);
}

void loop() {
  // Check for bluetooth input
  if (SerialBT.available()) {
      Serial.print("rx: ");
      bool start = true;
      String receivedText = "";
      // Read BT input and create string
      while (SerialBT.available()) {
        int c = SerialBT.read();
        if (c >= 0) {
          // Starting character is '*'
          if(c=='*' && start)
          {
            start = false;
          }
          // Terminating character is '#'
          else if(c!='#')
          {
            receivedText+=(char)c;
          }
        }
      }
      // Display receiverd text on LED display for 30 seconds
      uint32_t startTime = millis();
      while((millis()-startTime) < THIRTY_SECONDS_IN_MILLIS)
      {
        Serial.println(receivedText.c_str());
        Single_Row_Display_Mode(receivedText);
        delay(500);
      }
  }
}
