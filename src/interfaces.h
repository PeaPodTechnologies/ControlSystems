#ifndef CSOS_INTERFACES_H_
#define CSOS_INTERFACES_H_

#include <Arduino.h>

#include <I2CIP.h>

#include <interfaces/adc.h>
#include <interfaces/gpio.h>
#include <interfaces/pwm.h>

#define NUM_DEVICE_TYPES 4 // 3 + EEPROM

namespace ControlSystemsOS {

  class Linker {
    public:
      const I2CIP::factory_device_t device_factory[NUM_DEVICE_TYPES] = {
        &I2CIP::eepromFactory,
        &ControlSystemsOS::adcFactory,
        &ControlSystemsOS::gpioFactory,
        &ControlSystemsOS::pwmFactory,
      };
      
      const char* device_id_progmem[NUM_DEVICE_TYPES] = {
        I2CIP::i2cip_eeprom_id,
        ControlSystemsOS::csos_id_adc,
        ControlSystemsOS::csos_id_gpio,
        ControlSystemsOS::csos_id_pwm,
      };

      char* device_id[NUM_DEVICE_TYPES] = { 
        &EEPROM::_id[0],
        &ADC::_id[0],
        &GPIO::_id[0],
        &PWM::_id[0],
      };
      
      bool* device_id_set[NUM_DEVICE_TYPES] = {
        &EEPROM::_id_set,
        &ADC::_id_set,
        &GPIO::_id_set,
        &PWM::_id_set,
      };

      const I2CIP::i2cip_itype_t device_itype[NUM_DEVICE_TYPES] = {
        I2CIP_ITYPE_INPUT,
        I2CIP_ITYPE_INPUT,
        I2CIP_ITYPE_IO,
        I2CIP_ITYPE_OUTPUT,
      };
  };
};

#endif