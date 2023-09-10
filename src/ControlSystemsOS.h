#ifndef CSOS_CSOS_H_
#define CSOS_CSOS_H_

#include <I2CIP.h>
#include <timer.h>

#include <ArduinoJson.h>

#include <utils/bst.h>
#include <utils/hashtable.h>

// #include <sensors/sensor.h>
// #include <actuators/actuator.h>

#define NUM_DEVICE_TYPES 3

#define CSOS_MODULE_JSON_DOCSIZE (size_t)(I2CIP_EEPROM_SIZE * 3 / 2)

using namespace I2CIP;

// typedef Sensor* (* const factory_sensor_t)(const Device* fqa);

// static i2cip_id_t map_sensor_id[NUM_SENSOR_TYPES] = { nullptr };
// static factory_sensor_t map_sensor_factory[NUM_SENSOR_TYPES] = { nullptr };

// typedef Actuator* (* const factory_actuator_t)(const Device* fqa);

// static i2cip_id_t map_actuator_id[NUM_ACTUATOR_TYPES] = { nullptr };
// static factory_actuator_t map_actuator_factory[NUM_ACTUATOR_TYPES] = { nullptr };

// array of print functions

// array of  

typedef HashTable<DeviceGroup&> i2cip_devicetable_t;

// HashTable
  // key/ID (1) - heap (const char*)
  // HashTableEntry - heap (arr of ptrs)

// BST of device IDs by FQA
typedef BST<i2cip_fqa_t, i2cip_id_t> i2cip_devicetree_t;

// BST node; key: FQA, value: ID
typedef BSTNode<i2cip_fqa_t, i2cip_id_t> i2cip_device_t;

namespace ControlSystemsOS {

  class CSOSModule : public Module {
    public:
      CSOSModule(const uint8_t& wire, const uint8_t& module);

      StaticJsonDocument<CSOS_MODULE_JSON_DOCSIZE> eeprom_json;

      bool parseEEPROMContents(const uint8_t* buffer, size_t buflen) override;
  };
      
  extern BST<i2cip_fqa_t, i2cip_id_t> interfaces;
  extern HashTable<DeviceGroup&> groups;
  extern CSOSModule* modules[I2CIP_NUM_WIRES][I2CIP_MUX_COUNT];

  extern const char* csos_map_device_id[NUM_DEVICE_TYPES];
  extern const i2cip_itype_t csos_map_device_itype[NUM_DEVICE_TYPES];
  extern const factory_device_t csos_map_device_factory[NUM_DEVICE_TYPES];

  HashTableEntry<DeviceGroup&>* addNewGroup(const i2cip_id_t& id, const i2cip_itype_t& itype, const factory_device_t& factory);

  // new SHT31(fqa)

  void initialize(void);
  void update(void);
  void fixedUpdate(unsigned long timestamp);

  /**
   * Find a device group by ID.
   * @param id
   * @returns Pointer to the device group (`nullptr` if none)
   */
  // DeviceGroup* operator[](const char* id);

  /**
   * Find a device ID by FQA.
   * @param fqa
   * @returns Pointer to the device ID (`nullptr` if none)
   */
  // const char* operator[](const i2cip_fqa_t& fqa);

  // const i2cip_devicetree_t& getDevices(void);

  // const i2cip_devicetable_t& getDeviceGroups(void);
};

#endif