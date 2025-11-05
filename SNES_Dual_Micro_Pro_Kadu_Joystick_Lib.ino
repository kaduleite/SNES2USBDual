/*
  Dual SNES to USB adapter
  Developed for and tested on Arduino Leonardo Pro Micro (ATMEGA32u4)
  
  Adapted from Dome's Blog - Building a USB SNES Controller 
  https://blog.chybby.com/posts/building-a-usb-snes-controller

  SNES controller pinout:
                1 2 3 4   5 6 7
              ┌─────────┬───────┐
              │ 0 0 0 0 | 0 0 0 ) 
              └─────────┴───────┘
                + C L D   N N G
                V L A A   C C N
                  K T T       D
                    C A
                    H

  Arduino pins:
    VCC ----------------> +V   (common VCC    - SNES PIN 1)
    GND ----------------> GND  (common ground - SNES PIN 7)
    LATCH --------------> 2    (common latch  - SNES PIN 3)
    CLOCK --------------> 3    (common clock  - SNES PIN 2)
    DATA CONTROLLER 1 --> A0   (controller 1  - SNES PIN 4)
    DATA CONTROLLER 2 --> A1   (controller 2  - SNES PIN 4)

*/

// Install Joystick Librabry 
// MHeironimus/ArduinoJoystickLibrary 
// https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Joystick.h>

// ---------- Hardware ----------
#define LATCH_PIN  2
#define CLOCK_PIN  3
#define DATA1_PIN  A0
#define DATA2_PIN  A1

// ---------- Mapeamento SNES ----------
enum {
  SNES_B = 0, SNES_Y, SNES_SELECT, SNES_START,
  SNES_UP, SNES_DOWN, SNES_LEFT, SNES_RIGHT,
  SNES_A, SNES_X, SNES_L, SNES_R,
  SNES_U1, SNES_U2, SNES_U3, SNES_U4,
  SNES_BUTTONS
};

// Conversão botão SNES → índice de botão HID (mesma ordem do sketch original)
const uint8_t snes2hid[SNES_BUTTONS] = {
  2, 4, 7, 8,        // B  Y  Select Start
  0, 0, 0, 0,        // (D-Pad – tratados como hat)
  1, 3, 5, 6,        // A X L R
  10,11,12,13        // Reservados
};

// ---------- Objetos Joystick -----------
/*  Parâmetros:
    reportID, tipo, n° botões, n° hats,
    X, Y,
    Z, Rx, Ry, Rz, Rudder, Throttle, Accelerator, Brake, Steering
*/
Joystick_ Joystick1(0x01, JOYSTICK_TYPE_GAMEPAD,
                    16, 1,          // 16 botões, 1 hat
                    true, true,     // X, Y
                    false,false,false,false,false,false,false,false,false); // não inclui mais nada

Joystick_ Joystick2(0x02, JOYSTICK_TYPE_GAMEPAD,
                    16, 1,          // 16 botões, 1 hat
                    true, true,     // X, Y
                    false,false,false,false,false,false,false,false,false); // não inclui mais nada

// Valores de eixo para esta biblioteca (0-1023 por padrão)
const int AXIS_MIN    =    0;
const int AXIS_CENTER =  512;
const int AXIS_MAX    = 1023;

// Hat values (D-Pad)
const int HAT_UP        = 0;
const int HAT_UP_RIGHT  = 45;
const int HAT_RIGHT     = 90;
const int HAT_DOWN_RIGHT = 135;
const int HAT_DOWN      = 180;
const int HAT_DOWN_LEFT = 225;
const int HAT_LEFT      = 270;
const int HAT_UP_LEFT   = 315;
const int HAT_OFF       = -1;

// ----------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA1_PIN, INPUT_PULLUP);
  pinMode(DATA2_PIN, INPUT_PULLUP);

  digitalWrite(CLOCK_PIN, HIGH);

  // Inicia joysticks sem envio automático
  Joystick1.begin(false);
  Joystick2.begin(false);

  // Centra e libera tudo
  resetGamepad(Joystick1);
  resetGamepad(Joystick2);
}

// Centraliza e solta todos os botões de um gamepad
void resetGamepad(Joystick_ &js){
  js.setXAxis(AXIS_CENTER);
  js.setYAxis(AXIS_CENTER);
  js.setHatSwitch(0, HAT_OFF);
  for (uint8_t b=0;b<16;b++) js.setButton(b, 0);
  js.sendState();
}

// ----------------------------------------------------
void loop() {
  // Pulso de LATCH – copia o estado atual do SNES para o shift-register
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(12);
  digitalWrite(LATCH_PIN, LOW);
  delayMicroseconds(6);

  bool state1[SNES_BUTTONS];
  bool state2[SNES_BUTTONS];

  // Lê 16 bits (B, Y, Select, Start, ↑, ↓, ←, →, A, X, L, R, ...)
  for (uint8_t i = 0; i < SNES_BUTTONS; i++) {
    state1[i] = (digitalRead(DATA1_PIN) == LOW);    // ativo em LOW
    state2[i] = (digitalRead(DATA2_PIN) == LOW);

    // CLOCK ↓↑ para avançar no shift-register
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(6);
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(6);
  }

  // ---------- Atualiza Hat (D-Pad) ----------
  updateHat(state1, Joystick1);
  updateHat(state2, Joystick2);

  // ---------- Atualiza eixos X e Y analógicos (stick esquerdo) ----------
  updateXY(state1, Joystick1);
  updateXY(state2, Joystick2);

  // ---------- Atualiza botões ----------
  for (uint8_t i = 0; i < SNES_BUTTONS; i++) {
    // ignora D-Pad (já tratado como hat)

    if (i >= SNES_UP && i <= SNES_RIGHT) continue;

    Joystick1.setButton(snes2hid[i]-1, state1[i]); // -1 para adequar o início em 0
    Joystick2.setButton(snes2hid[i]-1, state2[i]);
  }

  // Envia relatório HID
  Joystick1.sendState();
  Joystick2.sendState();

  delay(16);   // ~60 Hz
}

// Atualiza hat switch (D-Pad)
void updateHat(const bool s[SNES_BUTTONS], Joystick_ &js){
  int hat = HAT_OFF;

  if (s[SNES_UP]) {
    hat = HAT_UP;
    if (s[SNES_LEFT])  hat = HAT_UP_LEFT;
    if (s[SNES_RIGHT]) hat = HAT_UP_RIGHT;
  } 
  else if (s[SNES_DOWN]) {
    hat = HAT_DOWN;
    if (s[SNES_LEFT])  hat = HAT_DOWN_LEFT;
    if (s[SNES_RIGHT]) hat = HAT_DOWN_RIGHT;
  } 
  else if (s[SNES_LEFT]) {
    hat = HAT_LEFT;
  } 
  else if (s[SNES_RIGHT]) {
    hat = HAT_RIGHT;
  }

  js.setHatSwitch(0, hat);
}

// Atualiza eixos X e Y analógicos (stick esquerdo)
// O D-Pad continua lendo como eixos de movimento
void updateXY(const bool s[SNES_BUTTONS], Joystick_ &js){
  int x = AXIS_CENTER;
  int y = AXIS_CENTER;

  if (s[SNES_LEFT])  x = AXIS_MIN;
  if (s[SNES_RIGHT]) x = AXIS_MAX;
  if (s[SNES_UP])    y = AXIS_MIN;
  if (s[SNES_DOWN])  y = AXIS_MAX;

  js.setXAxis(x);
  js.setYAxis(y);
}
