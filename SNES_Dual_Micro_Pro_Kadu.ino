/*
  Dual SNES to USB adapter
  Developed for and tested on Arduino Leonardo Pro Micro (ATMEGA32u4)
  
  Adapted from Dome's Blog - Building a USB SNES Controller 
  https://blog.chybby.com/posts/building-a-usb-snes-controller

  SNES controller pinout:
               ________________
              | 0 0 0 0 | 0 0 0 ) 
               ----------------      
                + C L D   N N G
                V L A A   C C N
                  K T T       D
                    C A
                    H

  Arduino pins:
    VCC ----------------> +V
    GND ----------------> GND
    CLOCK --------------> 2
    LATCH --------------> 3
    DATA CONTROLLER 1 --> A0
    DATA CONTROLLER 2 --> A1

*/

// Install from library manager (HID-Project by NicoHood)
#include <HID-Project.h> 

#define CLOCK_PIN 3
#define LATCH_PIN 2
#define DATA_PIN A0
#define DATA2_PIN A1

#define SNES_BUTTON_B 0
#define SNES_BUTTON_Y 1
#define SNES_BUTTON_SELECT 2
#define SNES_BUTTON_START 3
#define SNES_BUTTON_UP 4
#define SNES_BUTTON_DOWN 5
#define SNES_BUTTON_LEFT 6
#define SNES_BUTTON_RIGHT 7
#define SNES_BUTTON_A 8
#define SNES_BUTTON_X 9
#define SNES_BUTTON_L 10
#define SNES_BUTTON_R 11
#define SNES_BUTTON_UNDEF_1 12
#define SNES_BUTTON_UNDEF_2 13
#define SNES_BUTTON_UNDEF_3 14
#define SNES_BUTTON_UNDEF_4 15

const uint8_t num_buttons = 16;

// Map SNES buttons to HID joypad buttons.
const uint8_t snes_id_to_hid_id[] = { 2, 4, 7, 8, 0, 0, 0, 0, 1, 3, 5, 6, 10, 11, 12, 13 };

void setup() {
  delay(10);
  Gamepad.begin();
  Gamepad1.begin();
  Gamepad2.begin();
  Gamepad.releaseAll();
  Gamepad1.releaseAll();
  Gamepad1.releaseAll();

  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);
  pinMode(DATA2_PIN, INPUT);

  digitalWrite(CLOCK_PIN, HIGH);
}

// Report button states over HID.
void reportButtons(bool button_states1[num_buttons],bool button_states2[num_buttons]) {
  // D-Pad.
  int8_t dpad_status1 = GAMEPAD_DPAD_CENTERED;
  Gamepad1.xAxis(0);
  Gamepad1.yAxis(0);
  int8_t dpad_status2 = GAMEPAD_DPAD_CENTERED;
  Gamepad2.xAxis(0);
  Gamepad2.yAxis(0);

  if (button_states1[SNES_BUTTON_UP]) {
    dpad_status1 = GAMEPAD_DPAD_UP;
    Gamepad1.yAxis(-32768);
    if (button_states1[SNES_BUTTON_LEFT]) {
      dpad_status1 = GAMEPAD_DPAD_UP_LEFT;
      Gamepad1.xAxis(-32768);
    } else if (button_states1[SNES_BUTTON_RIGHT]) {
      dpad_status1 = GAMEPAD_DPAD_UP_RIGHT;
      Gamepad1.xAxis(32767);
    }
  } else if (button_states1[SNES_BUTTON_DOWN]) {
    dpad_status1 = GAMEPAD_DPAD_DOWN;
    Gamepad1.yAxis(32767);
    if (button_states1[SNES_BUTTON_LEFT]) {
      dpad_status1 = GAMEPAD_DPAD_DOWN_LEFT;
      Gamepad1.xAxis(-32768);
    } else if (button_states1[SNES_BUTTON_RIGHT]) {
      dpad_status1 = GAMEPAD_DPAD_DOWN_RIGHT;
      Gamepad1.xAxis(32767);
    }
  } else if (button_states1[SNES_BUTTON_LEFT]) {
    dpad_status1 = GAMEPAD_DPAD_LEFT;
    Gamepad1.xAxis(-32768);
  } else if (button_states1[SNES_BUTTON_RIGHT]) {
    dpad_status1 = GAMEPAD_DPAD_RIGHT;
    Gamepad1.xAxis(32767);
  }

  if (button_states2[SNES_BUTTON_UP]) {
    dpad_status2 = GAMEPAD_DPAD_UP;
    Gamepad2.yAxis(-32768);
    if (button_states2[SNES_BUTTON_LEFT]) {
      dpad_status2 = GAMEPAD_DPAD_UP_LEFT;
      Gamepad2.xAxis(-32768);
    } else if (button_states2[SNES_BUTTON_RIGHT]) {
      dpad_status2 = GAMEPAD_DPAD_UP_RIGHT;
      Gamepad2.xAxis(32767);
    }
  } else if (button_states2[SNES_BUTTON_DOWN]) {
    dpad_status2 = GAMEPAD_DPAD_DOWN;
    Gamepad2.yAxis(32767);
    if (button_states2[SNES_BUTTON_LEFT]) {
      dpad_status2 = GAMEPAD_DPAD_DOWN_LEFT;
      Gamepad2.xAxis(-32768);
    } else if (button_states2[SNES_BUTTON_RIGHT]) {
      dpad_status2 = GAMEPAD_DPAD_DOWN_RIGHT;
      Gamepad2.xAxis(32767);
    }
  } else if (button_states2[SNES_BUTTON_LEFT]) {
    dpad_status2 = GAMEPAD_DPAD_LEFT;
    Gamepad2.xAxis(-32768);
  } else if (button_states2[SNES_BUTTON_RIGHT]) {
    dpad_status2 = GAMEPAD_DPAD_RIGHT;
    Gamepad2.xAxis(32767);
  }

  Gamepad1.dPad1(dpad_status1);
  Gamepad1.dPad2(dpad_status1);

  Gamepad2.dPad1(dpad_status2);
  Gamepad2.dPad2(dpad_status2);

  for (uint8_t snes_id = 0; snes_id < num_buttons; snes_id++) {
    if (snes_id >= 4 && snes_id <= 7) {
      // D-Pad.
      continue;
    }

    if (button_states1[snes_id]) {
      Gamepad1.press(snes_id_to_hid_id[snes_id]);
    } else {
      Gamepad1.release(snes_id_to_hid_id[snes_id]);
    }

    if (button_states2[snes_id]) {
      Gamepad2.press(snes_id_to_hid_id[snes_id]);
    } else {
      Gamepad2.release(snes_id_to_hid_id[snes_id]);
    }
  }
}

void loop() {
  // Collect button state info from controller.
  // Send data latch.
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(12);
  digitalWrite(LATCH_PIN, LOW);

  delayMicroseconds(6);

  bool button_states1[num_buttons];
  bool button_states2[num_buttons];

  for (uint8_t id = 0; id < num_buttons; id++) {
    // Sample the button state.
    int button_pressed1 = digitalRead(DATA_PIN) == LOW;
    int button_pressed2 = digitalRead(DATA2_PIN) == LOW;
    button_states1[id] = button_pressed1;
    button_states2[id] = button_pressed2;

    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(6);
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(6);
  }

  // Update HID button states.
  reportButtons(button_states1,button_states2);
  Gamepad1.write();
  Gamepad2.write();

  delay(16);
}