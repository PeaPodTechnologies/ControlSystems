#ifndef UNIT_TEST

#include <Arduino.h>

#define DEBUG_SERIAL Serial

#include <debug.h>

#include <ControlSystemsOS.h>

#define FIXED_UPDATE_DELTA 1000

#ifdef FSM_TIMER_H_
  fsm_timestamp_t start = 0;
  fsm_timestamp_t lastFixedUpdate = 0;
#endif

bool rebuild = false;

void setup(void) {
  // Serial
  Serial.begin(115200);
  while(!Serial);

  #ifdef FSM_TIMER_H_
    start = millis();
  #endif

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

  #ifdef FSM_TIMER_H_
    fsm_timestamp_t cyclestart = millis();
  #endif

  // 0. Module State-Change Forward-Propagation
  // TODO: Move to `Module`?

  // 0a. Delete Devices Associated with Lost Modules
  // if(ControlSystemsOS::stateChange) {

  //   for(uint8_t i = 0; i < I2CIP_NUM_WIRES; i++) {
  //     for(uint8_t x = 0; x < I2CIP_MUX_COUNT; x++) {

  //       // Check if module exists
  //       Module* m = ControlSystemsOS::csos_modules[i][x];
  //       if(m == nullptr) {
  //         for(uint8_t j = 0; j < NUM_DEVICE_TYPES; j++) {
  //           const char* key = ControlSystemsOS::csos_map_device_id[j];
  //           I2CIP::DeviceGroup * group = (*m)[key];
  //           if(group == nullptr) continue;

  //           for(uint8_t k = 0; k < group->numdevices; k++) {
  //             Device* device = group->devices[k];
  //             if(device != nullptr) {
  //               // Delete Device
  //               m->remove(device);
  //             }
  //           }
  //         }
  //       }


  //     }
  //   }

  //   ControlSystemsOS::stateChange = false;
  // }
  
  // 1. Refresh Update - Network Status Handling & Rebuild

  // 1a. Module "Roll-Call" Check
  i2cip_errorlevel_t errlev = ControlSystemsOS::update(rebuild);
  // If we lose hardware, return and do FixedUpdate ASAP
  rebuild = (errlev == I2CIP_ERR_HARD);
  if(errlev > I2CIP_ERR_NONE) return;

  // 1b. Device Checking Per-Group
  // fsm_timestamp_t now = millis();
  // if((now - lastFixedUpdate) > FIXED_UPDATE_DELTA) {
  //   errlev = ControlSystemsOS::fixedUpdate(now);
  //   lastFixedUpdate = millis();
  // }

  // 2. Fixed Update - Instruction and Control Handling

}

// Overview:
//
// 1. Module State-Change Forward-Propagation
// 2. Refresh Update - Network Status Handling & Rebuild
// 3. Fixed Update - Instruction and Control Handling
//
// 1. Module State-Change Forward-Propagation
//
// 1a. Delete Devices Associated with Lost Modules
// 1b. Delete Modules
//
// 2. Refresh Update - Network Status Handling & Rebuild
//
// 2a. Module "Roll-Call" Check
// 2b. Device Checking Per-Group
//
// 3. Fixed Update - Instruction and Control Handling

#endif