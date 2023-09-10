#include <ControlSystemsOS.h>

#include <string.h>

#include <ArduinoJSON.h>

// ALLOCATE NEW INTERFACES (routing table)

// GROUP BY MODULE

using namespace ControlSystemsOS;

BST<i2cip_fqa_t, i2cip_id_t> ControlSystemsOS::interfaces;
HashTable<DeviceGroup&> ControlSystemsOS::groups;

CSOSModule* ControlSystemsOS::modules[I2CIP_NUM_WIRES][I2CIP_MUX_COUNT] = { { nullptr } };

// GLOBAL CONSTANT MAPS

const char* ControlSystemsOS::csos_map_device_id[NUM_DEVICE_TYPES] = { 
  "sht31",
};

const i2cip_itype_t ControlSystemsOS::csos_map_device_itype[NUM_DEVICE_TYPES] = {
  I2CIP_ITYPE_INPUT
}; 

const factory_device_t csos_map_device_factory[NUM_DEVICE_TYPES] = { nullptr };

int getMapIndex(const i2cip_id_t& id) {
  for(unsigned char i = 0; i < NUM_DEVICE_TYPES; i++) {
    // Compare strings ignoring case
    if(strcasecmp(csos_map_device_id[i], id) == 0){
      return i;
    }
  }
  return -1;
}

CSOSModule::CSOSModule(const uint8_t& wire, const uint8_t& module) : Module(wire, module) {
  // Build Maps
}

bool CSOSModule::parseEEPROMContents(const uint8_t* buffer, size_t buflen) {
  // 1. EEPROM -> JSON Deserialization
  DeserializationError jsonerr = deserializeJson(this->eeprom_json, buffer, buflen);

  if(jsonerr.code() != DeserializationError::Code::Ok) return false;

  // 2. Schema Validation and Loading
  // 2a. Base Array of Busses
  if(!this->eeprom_json.is<JsonArray>()) return false;
  if(!this->eeprom_json[0].is<JsonObject>()) return false;

  JsonArray busses = this->eeprom_json.as<JsonArray>();

  int busnum = -1;
  for (JsonVariant bus : busses) {
    busnum++;

    // 2b. Bus Root Object
    if(!bus.is<JsonObject>()) continue;
    JsonObject root = bus.as<JsonObject>();
  
    for (JsonPair kv : root) {
      const char* key = kv.key().c_str();
      HashTableEntry<DeviceGroup &>* hte = groups[key];
      
      if(hte == nullptr) { 
        int index = getMapIndex(key);
        if(index < 0) continue; // Device ID not found

        const factory_device_t factory = csos_map_device_factory[(unsigned char)index];
        const i2cip_itype_t itype = csos_map_device_itype[(unsigned char)index];

        if(itype == I2CIP_ITYPE_NULL || factory == nullptr) break; //  Missing Map Entry
        hte = addNewGroup(key, itype, factory);
      }

      if(hte == nullptr) continue; // Still Unresolved? Bad Key - No Mappings Found

      // 2c. Array of I2C Addresses
      if(!kv.value().is<JsonArray>() || kv.value().isNull()) continue;

      i2cip_fqa_t fqas[64];
      unsigned char fqa_count = 0;

      for (JsonVariant addr : kv.value().as<JsonArray>()) {
        if(!addr.is<unsigned int>()) { fqa_count = 0; break; }
        fqas[fqa_count] = createFQA(this->getWireNum(), this->getModuleNum(), (uint8_t)busnum, addr.as<uint8_t>());
        fqa_count++;
      }

      if(fqa_count == 0) continue;

      for(unsigned char i = 0; i < fqa_count; i++) {
        Device& d = hte->value(fqas[i]);
        this->add(d);
      }
    }
  }
}

HashTableEntry<DeviceGroup&>* ControlSystemsOS::addNewGroup(const i2cip_id_t& id, const i2cip_itype_t& itype, const factory_device_t& factory) {
  // Temp placeholder - wrong key pointer
  DeviceGroup* dg_temp = new DeviceGroup(id, itype);
  HashTableEntry<DeviceGroup&>* group_temp = groups.set(id, *dg_temp);
  // Ref copied key from hashtable
  DeviceGroup* dg = new DeviceGroup(group_temp->key, itype, factory);
  HashTableEntry<DeviceGroup&>* group = groups.set(id, *dg, true);
  // Cleanup
  if(group != group_temp) group = nullptr;
  delete dg_temp;
  return group;
}

void ControlSystemsOS::initialize(void) {
  update();

  for(uint8_t wirenum = 0; wirenum < I2CIP_NUM_WIRES; wirenum++) {
    for(uint8_t modnum = 0; modnum < I2CIP_MUX_COUNT; modnum++) {
      CSOSModule* m = modules[wirenum][modnum];

      if(m == nullptr) continue;

      Module::build(*m);
    }
  }
}

void ControlSystemsOS::update(void) {
  // 1. Scan for Modules
  for(uint8_t wirenum = 0; wirenum < I2CIP_NUM_WIRES; wirenum++) {
    for(uint8_t modnum = 0; modnum < I2CIP_MUX_COUNT; modnum++) {

      CSOSModule* m = modules[wirenum][modnum];
      
      if(m == nullptr) {
        bool b = MUX::pingMUX(wirenum, modnum);
        
        if(b) {
          // New module found!
          modules[wirenum][modnum] = new CSOSModule(wirenum, modnum);
          m = modules[wirenum][modnum];
        } else {
          continue;
        }
      }
      
      if(m->operator()() > I2CIP_ERR_NONE) {
        delete m;
        modules[wirenum][modnum] = nullptr;
        continue;
      }
    }
  }
}

void ControlSystemsOS::fixedUpdate(unsigned long timestamp) {
  // 1. FSM Timer Functionality
  Timer.set(timestamp);

  // 2. Control Systems Fixed Update

  // RPi: "air-temperature" -> Variable-Sensor(s) Linker -> Sensors (i.e. SHT31_Temperature)
  // uC: "sht31" -> ID-Interface Linker -> FQA[], SHT31 -> SHT31_Temperature::read(SHT31(fqa), nullptr, dest);
}