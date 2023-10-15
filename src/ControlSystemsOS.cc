
#include <ControlSystemsOS.h>

#include <string.h>

#include <Arduino.h>
#include <ArduinoJSON.h>

#define DEBUG 1

#include <debug.h>
#include <interfaces.h>

// #define CSOS_DEBUG_SERIAL Serial
// ALLOCATE NEW INTERFACES (routing table)

// GROUP BY MODULE

ControlSystemsOS::CSOSModule* ControlSystemsOS::csos_modules[I2CIP_NUM_WIRES][I2CIP_MUX_COUNT] = { { nullptr } };

// GLOBAL CONSTANT MAPS

int getMapIndex(const i2cip_id_t& id) {
  for(unsigned char i = 0; i < NUM_DEVICE_TYPES; i++) {
    // Compare strings ignoring case
    if(strcasecmp_P(id, ControlSystemsOS::linker.device_id_progmem[i]) == 0){
      return i;
    }
  }
  return -1;
}

using namespace ControlSystemsOS;
using namespace I2CIP;

CSOSModule::CSOSModule(const uint8_t& wire, const uint8_t& module) : Module(wire, module) {
  // Build Maps

}

DeviceGroup* CSOSModule::deviceGroupFactory(const i2cip_id_t& id) {
  int index = getMapIndex(id);
  if(index < 0) return nullptr;

  return new DeviceGroup(id, linker.device_itype[index], linker.device_factory[index]);
}

bool CSOSModule::parseEEPROMContents(const char* buffer) {
  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print(F("-> Deserializing Module EEPROM to JSON: "));
    DEBUG_DELAY();
  #endif

  // 0. Buffer size
  size_t buflen = strlen(buffer);
  
  // 1. EEPROM -> JSON Deserialization
  // TODO: Buflen + 1 ?
  DeserializationError jsonerr = deserializeJson(this->eeprom_json, buffer, buflen);

  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print("Code 0x");
    CSOS_DEBUG_SERIAL.print(jsonerr.code(), 16);
    CSOS_DEBUG_SERIAL.print("\n");
    DEBUG_DELAY();
  #endif

  if(jsonerr.code() != DeserializationError::Code::Ok) return false;

  // 2. Schema Validation and Loading
  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print(F("-> Verifying JSON:\n"));
  #endif

  // 2a. Base Array of Busses
  if(!this->eeprom_json.is<JsonArray>() || !this->eeprom_json[0].is<JsonObject>()) {
    #ifdef CSOS_DEBUG_SERIAL
      CSOS_DEBUG_SERIAL.print(F("Bad JSON: Invalid Structure!\n"));
      DEBUG_DELAY();
    #endif
  }

  JsonArray busses = this->eeprom_json.as<JsonArray>();

  int busnum = -1;
  for (JsonVariant bus : busses) {
    busnum++;

    #ifdef CSOS_DEBUG_SERIAL
      CSOS_DEBUG_SERIAL.print("[BUS ");
      CSOS_DEBUG_SERIAL.print(busnum+1, HEX);
      CSOS_DEBUG_SERIAL.print("]\n");
    #endif

    // 2b. Bus Root Object
    if(!bus.is<JsonObject>()) {
      #ifdef CSOS_DEBUG_SERIAL
        CSOS_DEBUG_SERIAL.print(F("Bad JSON: Invalid Bus Structure!\n"));
        DEBUG_DELAY();
      #endif
      continue;
    }
    JsonObject root = bus.as<JsonObject>();
  
    for (JsonPair kv : root) {
      // 2c. Array of I2C Addresses
      if(!kv.value().is<JsonArray>() || kv.value().isNull()) {
        #ifdef CSOS_DEBUG_SERIAL
          CSOS_DEBUG_SERIAL.print(F("Bad JSON: Invalid Entry Value!\n"));
          DEBUG_DELAY();
        #endif
        continue;
      }

      const char* key = kv.key().c_str();

      #ifdef CSOS_DEBUG_SERIAL
        DEBUG_DELAY();
        CSOS_DEBUG_SERIAL.print("[ ID '");
        CSOS_DEBUG_SERIAL.print(key);
        CSOS_DEBUG_SERIAL.print("']\n");
      #endif

      // Get DeviceGroup
      DeviceGroup* dg = this->operator[](key);
      if(dg == nullptr) {
        #ifdef CSOS_DEBUG_SERIAL
          DEBUG_DELAY();
          CSOS_DEBUG_SERIAL.print(F("-> Group DNE! Check Libraries.\n"));
          DEBUG_DELAY();
        #endif
        break;
      }

      uint8_t numfqas = kv.value().size();
      if(numfqas == 0) {
        #ifdef CSOS_DEBUG_SERIAL
          DEBUG_DELAY();
          CSOS_DEBUG_SERIAL.print(F("-> Empty! (Skipping)\n"));
          DEBUG_DELAY();
        #endif
        continue;
      }
      i2cip_fqa_t fqas[numfqas];

      #ifdef CSOS_DEBUG_SERIAL
        DEBUG_DELAY();
        CSOS_DEBUG_SERIAL.print("-> [");
        DEBUG_DELAY();
      #endif

      uint8_t i = 0;
      for (JsonVariant addr : kv.value().as<JsonArray>()) {
        if(!addr.is<unsigned int>()) { continue; }
        uint8_t address = addr.as<uint8_t>();
        #ifdef CSOS_DEBUG_SERIAL
          DEBUG_DELAY();
          CSOS_DEBUG_SERIAL.print(" 0x");
          CSOS_DEBUG_SERIAL.print(address, HEX);
          DEBUG_DELAY();
        #endif
        fqas[i] = createFQA(this->getWireNum(), this->getModuleNum(), (uint8_t)busnum, address);
      }

      #ifdef CSOS_DEBUG_SERIAL
        DEBUG_DELAY();
        CSOS_DEBUG_SERIAL.print(" ]\n");
        DEBUG_DELAY();
      #endif

      for (i = 0; i < numfqas; i++) {
        #ifdef CSOS_DEBUG_SERIAL
          DEBUG_DELAY();
          CSOS_DEBUG_SERIAL.print(F("-> Creating Device "));
          CSOS_DEBUG_SERIAL.print(i+1);
          CSOS_DEBUG_SERIAL.print(" / ");
          CSOS_DEBUG_SERIAL.print(numfqas);
          CSOS_DEBUG_SERIAL.print(" :\n");
          DEBUG_DELAY();
        #endif

        // Invoke Factory
        Device* d = (*dg)(fqas[i]); 
        if(d == nullptr) { 
          #ifdef CSOS_DEBUG_SERIAL
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("-> Factory Failed! (Skipping)\n"));
            DEBUG_DELAY();
          #endif
          break;
        }

        #ifdef CSOS_DEBUG_SERIAL
          DEBUG_DELAY();
          CSOS_DEBUG_SERIAL.print(F("-> Adding Device "));
          CSOS_DEBUG_SERIAL.print(i+1);
          CSOS_DEBUG_SERIAL.print(" / ");
          CSOS_DEBUG_SERIAL.print(numfqas);
          CSOS_DEBUG_SERIAL.print(" :\n");
          DEBUG_DELAY();
        #endif
        // Add to Module
        if(!this->add(*d)) {
          delete d;
          #ifdef CSOS_DEBUG_SERIAL
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("_/^^^| ! D E S T R O Y E D ! |^^^\\_\n"));
            DEBUG_DELAY();
          #endif
          continue;
        }

        #ifdef CSOS_DEBUG_SERIAL
          DEBUG_DELAY();
          CSOS_DEBUG_SERIAL.print(F("-> Device Added!\n"));
          DEBUG_DELAY();
        #endif
      }

      #ifdef CSOS_DEBUG_SERIAL
        DEBUG_DELAY();
        CSOS_DEBUG_SERIAL.print(F("-> Group Complete!\n"));
        DEBUG_DELAY();
      #endif
    }
  }
  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print(F("-> EEPROM Parsed Successfully!\n"));
    DEBUG_DELAY();
  #endif
  return true;
}

// Subnetwork state change update + rebuild from discovery
i2cip_errorlevel_t ControlSystemsOS::update(const uint8_t& wire, const uint8_t& mod, bool build) {
  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print(F("= [ NETWORK UPDATE: MODULE "));
    CSOS_DEBUG_SERIAL.print(mod, HEX);
    CSOS_DEBUG_SERIAL.print(" ] =\n");
  #endif

  i2cip_errorlevel_t errlev = I2CIP_ERR_NONE;
  CSOSModule* m = csos_modules[wire][mod];
  
  if(m == nullptr) {
    #ifdef CSOS_DEBUG_SERIAL
      CSOS_DEBUG_SERIAL.print(F("-> Module DNE; Pinging.\n"));
      DEBUG_DELAY();
    #endif

    bool b = MUX::pingMUX(wire, mod);

    #ifdef CSOS_DEBUG_SERIAL
      DEBUG_DELAY();
      if(b) { CSOS_DEBUG_SERIAL.print(F("-> Module Found! Creating.\n")); }
      else { CSOS_DEBUG_SERIAL.print(F("-> Module Not Found.)")); }
    #endif
    
    if(b) {
      // New module found!
      csos_modules[wire][mod] = new CSOSModule(wire, mod);
      m = csos_modules[wire][mod];

      if (build) m->discover();
    } else {
      // No net change
      return I2CIP_ERR_NONE;
    }
  } else {
    #ifdef CSOS_DEBUG_SERIAL
      CSOS_DEBUG_SERIAL.print(F("-> Pass! Updating Module.\n"));
      DEBUG_DELAY();
    #endif
  }

  // csos_modules_lastChecked[wire][module] = millis();

  errlev = m->operator()();
  
  if(errlev > I2CIP_ERR_NONE) {
    #ifdef CSOS_DEBUG_SERIAL
      DEBUG_DELAY();
      CSOS_DEBUG_SERIAL.print(F("-> Fail. Deleting Module.\n"));
      DEBUG_DELAY();
    #endif
    delete csos_modules[wire][mod];
    csos_modules[wire][mod] = nullptr;
  } 
  #ifdef CSOS_DEBUG_SERIAL
    else {
      DEBUG_DELAY();
      CSOS_DEBUG_SERIAL.print(F("-> Pass! Updating Devices.\n"));
      DEBUG_DELAY();
    }
  #endif

  for(uint8_t i = 0; i < NUM_DEVICE_TYPES; i++) {
    if(*(linker.device_id_set[i]) == false) continue;
    DeviceGroup* dg = (*m)[linker.device_id[i]];
    if(dg == nullptr) continue;

    #ifdef CSOS_DEBUG_SERIAL
      DEBUG_DELAY();
      CSOS_DEBUG_SERIAL.print(F("-> [ DEVICE GROUP "));
      CSOS_DEBUG_SERIAL.print(i+1, HEX);
      CSOS_DEBUG_SERIAL.print(" / ");
      CSOS_DEBUG_SERIAL.print(NUM_DEVICE_TYPES, HEX);
      CSOS_DEBUG_SERIAL.print(" ]\n");
      DEBUG_DELAY();
    #endif

    for(uint8_t j = 0; j < dg->numdevices; j++) {
      Device* device = dg->devices[j];
      if(device == nullptr) continue;

      #ifdef CSOS_DEBUG_SERIAL
        DEBUG_DELAY();
        CSOS_DEBUG_SERIAL.print(F("[ DEVICE "));
        CSOS_DEBUG_SERIAL.print(j+1, HEX);
        CSOS_DEBUG_SERIAL.print(" / ");
        CSOS_DEBUG_SERIAL.print(dg->numdevices, HEX);
        CSOS_DEBUG_SERIAL.print(F(" ]\n-> FQA: "));
        CSOS_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(device->getFQA()), HEX);
        CSOS_DEBUG_SERIAL.print(":");
        CSOS_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(device->getFQA()), HEX);
        CSOS_DEBUG_SERIAL.print(":");
        CSOS_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(device->getFQA()), HEX);
        CSOS_DEBUG_SERIAL.print(":");
        CSOS_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(device->getFQA()), HEX);
        CSOS_DEBUG_SERIAL.print("\n");
        DEBUG_DELAY();
      #endif

      errlev = (*m)(*device, false);

      #ifdef CSOS_DEBUG_SERIAL
        switch(errlev) {
          case I2CIP_ERR_NONE:
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("-> 0x0 Pass!\n"));
            DEBUG_DELAY();
            break;
          case I2CIP_ERR_SOFT:
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("-> 0x1 Communication Error!\n"));
            DEBUG_DELAY();
            break;
          case I2CIP_ERR_HARD:
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("-> 0x2 Hardware Lost, ABORT!\n"));
            DEBUG_DELAY();
            break;
        }
      #endif

      if(errlev == I2CIP_ERR_HARD) return errlev;
    }
  }

  return errlev;
}

// Whole-network state change update
i2cip_errorlevel_t ControlSystemsOS::update(bool build) {
  i2cip_errorlevel_t errlev = I2CIP_ERR_NONE;
  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print(F("=== [ NETWORK UPDATE ] ===\n"));
    DEBUG_DELAY();
  #endif
  // 1. Scan for Modules
  for(uint8_t wirenum = 0; wirenum < I2CIP_NUM_WIRES; wirenum++) {
    #ifdef CSOS_DEBUG_SERIAL
      DEBUG_DELAY();
      CSOS_DEBUG_SERIAL.print(F("== [ WIRE "));
      CSOS_DEBUG_SERIAL.print(wirenum+1, HEX);
      CSOS_DEBUG_SERIAL.print(" / ");
      CSOS_DEBUG_SERIAL.print(I2CIP_NUM_WIRES, HEX);
      CSOS_DEBUG_SERIAL.print(F("] ==\n"));
      DEBUG_DELAY();
    #endif
    
    for(uint8_t modnum = 0; modnum < I2CIP_MUX_COUNT; modnum++) {
      errlev = update(wirenum, modnum, csos_modules[wirenum][modnum] == nullptr);
      I2CIP_ERR_BREAK(errlev);
    }
  }

  return errlev;
}

// Control systems update
i2cip_errorlevel_t ControlSystemsOS::fixedUpdate(unsigned long timestamp, CSOSModule& m) {
  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print(F("= [ CONTROLSYSTEMS UPDATE: MODULE "));
    CSOS_DEBUG_SERIAL.print(m.getModuleNum(), HEX);
    CSOS_DEBUG_SERIAL.print(F(" ] =\n"));
  #endif

  i2cip_errorlevel_t errlev = I2CIP_ERR_NONE;

  // 1. FSM Timer Functionality
  Chronos.set(timestamp);

  // 2. Control Systems Fixed Update per-ID
  for(uint8_t i = 0; i < NUM_DEVICE_TYPES; i++) {
    if(*(linker.device_id_set[i]) == false) continue;

    #ifdef CSOS_DEBUG_SERIAL
      DEBUG_DELAY();
      CSOS_DEBUG_SERIAL.print(F("-> DeviceGroup '"));
      CSOS_DEBUG_SERIAL.print(linker.device_id[i]);
      CSOS_DEBUG_SERIAL.print(F("'\n"));
      DEBUG_DELAY();
    #endif

    DeviceGroup* dg = m[linker.device_id[i]];
    if(dg == nullptr) {
      #ifdef CSOS_DEBUG_SERIAL
        DEBUG_DELAY();
        CSOS_DEBUG_SERIAL.print(F("-> Group '"));
        CSOS_DEBUG_SERIAL.print(linker.device_id[i]);
        CSOS_DEBUG_SERIAL.print(F("' DNE! Check Libraries.\n"));
        DEBUG_DELAY();
      #endif
      continue;
    }

    for(uint8_t j = 0; j < dg->numdevices; j++) {
      Device* device = dg->devices[j];
      if(device == nullptr) break;

      #ifdef CSOS_DEBUG_SERIAL
        DEBUG_DELAY();
        CSOS_DEBUG_SERIAL.print(F("-> Updating Device "));
        CSOS_DEBUG_SERIAL.print(j+1);
        CSOS_DEBUG_SERIAL.print(" / ");
        CSOS_DEBUG_SERIAL.print(dg->numdevices);
        CSOS_DEBUG_SERIAL.print("\n");
        DEBUG_DELAY();
      #endif

      errlev = m(*device, true);

      #ifdef CSOS_DEBUG_SERIAL
        switch(errlev) {
          case I2CIP_ERR_NONE:
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("-> 0x0 Pass!\n"));
            DEBUG_DELAY();
            break;
          case I2CIP_ERR_SOFT:
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("-> 0x1 Communication Error!\n"));
            DEBUG_DELAY();
            break;
          case I2CIP_ERR_HARD:
            DEBUG_DELAY();
            CSOS_DEBUG_SERIAL.print(F("-> 0x2 Hardware Lost, ABORT!\n"));
            DEBUG_DELAY();
            break;
        }
      #endif

      if(errlev == I2CIP_ERR_HARD) return errlev;
    }
  }

  return errlev;

  // RPi: "air-temperature" -> Variable-Sensor(s) Linker -> Sensors (i.e. SHT31_Temperature)
  // uC: "sht31" -> ID-Interface Linker -> FQA[], SHT31 -> SHT31_Temperature::read(SHT31(fqa), nullptr, dest);
}

i2cip_errorlevel_t ControlSystemsOS::fixedUpdate(unsigned long timestamp) {
  #ifdef CSOS_DEBUG_SERIAL
    DEBUG_DELAY();
    CSOS_DEBUG_SERIAL.print(F("=== [ CONTROLSYSTEMS UPDATE ] ===\n"));
    DEBUG_DELAY();
  #endif

  i2cip_errorlevel_t errlev = I2CIP_ERR_NONE;

  // 1. Scan for Modules
  for(uint8_t wirenum = 0; wirenum < I2CIP_NUM_WIRES; wirenum++) {
    #ifdef CSOS_DEBUG_SERIAL
      DEBUG_DELAY();
      CSOS_DEBUG_SERIAL.print(F("== [ WIRE "));
      CSOS_DEBUG_SERIAL.print(wirenum+1, HEX);
      CSOS_DEBUG_SERIAL.print(" / ");
      CSOS_DEBUG_SERIAL.print(I2CIP_NUM_WIRES, HEX);
      CSOS_DEBUG_SERIAL.print(F("] ==\n"));
      DEBUG_DELAY();
    #endif
    
    for(uint8_t modnum = 0; modnum < I2CIP_MUX_COUNT; modnum++) {
      CSOSModule* m = csos_modules[wirenum][modnum];
      if(m == nullptr) continue;
      errlev = fixedUpdate(timestamp, *m);
      
      if(errlev == I2CIP_ERR_HARD) return errlev;
    }
  }

  return errlev;

}

// void ControlSystemsOS::deleteDevice(Device* device) {
//   if(device == nullptr) return;

//   // Pick module from FQA
//   uint8_t wirenum = I2CIP_FQA_SEG_I2CBUS(device->getFQA());
//   uint8_t modnum = I2CIP_FQA_SEG_MODULE(device->getFQA());
//   CSOSModule* m = csos_modules[wirenum][modnum];
//   if(m == nullptr) return;

//   m->remove(device);
// }

// template <typename G, typename A> i2cip_errorlevel_t ControlSystemsOS::handleInputDevice(Device* device, const A& args) {
//   InputGetter* input = device->getInput();
//   if(input == nullptr) return I2CIP_ERR_SOFT;

//   InputInterface<G, A>* interface = (InputInterface<G, A>*) input;
//   G value = interface->getCache();
//   i2cip_errorlevel_t errlev = interface->get(value, args);
//   Serial.print("Value: ");
//   Serial.print(value);
//   return errlev;
// }

// template <typename S, typename B>  i2cip_errorlevel_t ControlSystemsOS::handleOutputDevice(Device* device, const S& value, const B& args) {
//   OutputSetter* output = device->getOutput();
//   if(output == nullptr) return I2CIP_ERR_SOFT;

//   OutputInterface<S, B>* interface = (OutputInterface<S, B>*) output;
//   i2cip_errorlevel_t errlev = interface->set(value, args);
//   return errlev;
// }