#define MAIN true

#include <Arduino.h>

#include <debug.h>

#include <ControlSystemsOS.h>
#include <chrono.h>

#define FIXED_UPDATE_DELTA 1000

fsm_timestamp_t start = 0;
fsm_timestamp_t lastFixedUpdate = 0;

bool rebuild = false;

void setup(void) {
  // Serial
  Serial.begin(115200);
  while(!Serial);

  start = millis();

  Serial.println("==== [ CYCLE 0 (BUILD) ] ====");

  delay(1000);

  i2cip_errorlevel_t errlev = ControlSystemsOS::update(true);
  if(errlev > I2CIP_ERR_NONE) {
    Serial.println("==== [ BUILD FAILED, FREEZING ] ====");
    while(true) { // Blink
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
}

uint8_t cycle = 0;

void loop(void) {
  cycle++;
  Serial.print("\n\n==== [ CYCLE ");
  Serial.print(cycle);
  Serial.println(" ] ====");

  delay(1000);

  fsm_timestamp_t cyclestart = millis();
  
  // 1. Refresh Update - Network Status Handling & Rebuild

  // 1a. Module "Roll-Call" Check
  i2cip_errorlevel_t errlev = ControlSystemsOS::update(rebuild);
  // If we lose hardware, return and do rebuild ASAP
  rebuild = (errlev == I2CIP_ERR_HARD);
  if(errlev > I2CIP_ERR_NONE) return;

  // 1b. Module Control Systems Refresh
  fsm_timestamp_t now = millis();
  if((now - lastFixedUpdate) > FIXED_UPDATE_DELTA) {
    errlev = ControlSystemsOS::fixedUpdate(now);
    lastFixedUpdate = millis();
  }
}
