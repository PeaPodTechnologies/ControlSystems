#ifndef CSOS_CSOS_H_
#define CSOS_CSOS_H_

#include <Arduino.h>
#include <I2CIP.h>
#include <chrono.h>

#include <ArduinoJson.h>

#include <interfaces.h>

// #include <sensors/sensor.h>
// #include <actuators/actuator.h>

#define CSOS_MODULE_JSON_DOCSIZE (size_t)(I2CIP_EEPROM_SIZE * 3 / 2)

using namespace I2CIP;

typedef i2cip_errorlevel_t (* interfaceHandler_t)(Device*);

namespace ControlSystemsOS {

  typedef enum {
    CSOS_NULL =   0x0,
    CSOS_BOOL =   0x1,
    CSOS_UINT8 =  0x2,
    CSOS_INT8 =   0x3,
    CSOS_UINT16 = 0x4,
    CSOS_INT16 =  0x5,
    CSOS_UINT32 = 0x6,
    CSOS_INT32 =  0x7,
    CSOS_FLOAT =  0x8,
    CSOS_DOUBLE = 0x9,
    CSOS_STRING = 0xA,
  } csos_types_t;

  class CSOSModule : public Module {
    private:
      StaticJsonDocument<CSOS_MODULE_JSON_DOCSIZE> eeprom_json;

    protected:
      DeviceGroup* deviceGroupFactory(const i2cip_id_t& id) override;

      bool parseEEPROMContents(const char* eeprom_contents) override;

    public:
      CSOSModule(const uint8_t& wire, const uint8_t& module);

      ~CSOSModule() { }
  };

  i2cip_errorlevel_t update(bool build = false);
  i2cip_errorlevel_t update(const uint8_t& wire, const uint8_t& mod, bool build = true);
  i2cip_errorlevel_t fixedUpdate(unsigned long timestamp);
  i2cip_errorlevel_t fixedUpdate(unsigned long timestamp, CSOSModule& m);
      
  extern CSOSModule* csos_modules[I2CIP_NUM_WIRES][I2CIP_MUX_COUNT];

  static Linker linker;
  
  // extern fsm_timestamp_t csos_modules_lastChecked[I2CIP_NUM_WIRES][I2CIP_MUX_COUNT];

  // extern const csos_types_t csos_map_interface_state[];
  // extern const csos_types_t csos_map_interface_args[];

  // extern bool stateChange;

  // new SHT31(fqa)

  // void initialize(void);
  

  // i2cip_errorlevel_t handleDevice(Device* device);
  // template <typename G, typename A> i2cip_errorlevel_t handleInputDevice(Device* device, const A& args);
  // template <typename S, typename B> i2cip_errorlevel_t handleOutputDevice(Device* device, const S& value, const B& args);

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