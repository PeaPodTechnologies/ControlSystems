#include <interfaces/gpio.h>

#include <Arduino.h>
#include <I2CIP.h>

const char ControlSystemsOS::csos_id_gpio[] PROGMEM = {"MCP23017"};

bool ControlSystemsOS::GPIO::_id_set;
char ControlSystemsOS::GPIO::_id[I2CIP_ID_SIZE];

// Handles ID pointer assignment too
I2CIP::Device* ControlSystemsOS::gpioFactory(const i2cip_fqa_t& fqa) {
  if(!ControlSystemsOS::GPIO::_id_set) {
    uint8_t idlen = strlen_P(csos_id_gpio);

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_DELAY();
      I2CIP_DEBUG_SERIAL.print(F("Loading GPIO ID PROGMEM to Static Array @0x"));
      I2CIP_DEBUG_SERIAL.print((uint16_t)(&(ControlSystemsOS::GPIO::_id[0])), HEX);
      I2CIP_DEBUG_SERIAL.print(F(" ("));
      I2CIP_DEBUG_SERIAL.print(idlen+1);
      I2CIP_DEBUG_SERIAL.print(F(" bytes) '"));
    #endif

    // Read in PROGMEM
    for (uint8_t k = 0; k < idlen; k++) {
      char c = pgm_read_byte_near(csos_id_gpio + k);
      #ifdef I2CIP_DEBUG_SERIAL
        DEBUG_SERIAL.print(c);
      #endif
      ControlSystemsOS::GPIO::_id[k] = c;
    }

    ControlSystemsOS::GPIO::_id[idlen] = '\0';
    ControlSystemsOS::GPIO::_id_set = true;

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_SERIAL.print("'\n");
      DEBUG_DELAY();
    #endif
  }

  return (I2CIP::Device*)(new ControlSystemsOS::GPIO(fqa));
}

using namespace ControlSystemsOS;

GPIO::GPIO(const i2cip_fqa_t& fqa) : Device(fqa, (const char*)GPIO::_id), IOInterface<state_gpio_t, args_gpio_t, state_gpio_t, args_gpio_t>((Device*)this) {
  if(!GPIO::_id_set) {
    uint8_t idlen = strlen_P(csos_id_gpio);

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_DELAY();
      I2CIP_DEBUG_SERIAL.print(F("Loading GPIO ID PROGMEM to Static Array @0x"));
      I2CIP_DEBUG_SERIAL.print((uint16_t)(&(GPIO::_id[0])), HEX);
      I2CIP_DEBUG_SERIAL.print(F(" ("));
      I2CIP_DEBUG_SERIAL.print(idlen+1);
      I2CIP_DEBUG_SERIAL.print(F(" bytes) '"));
    #endif

    // Read in PROGMEM
    for (uint8_t k = 0; k < idlen; k++) {
      char c = pgm_read_byte_near(csos_id_gpio + k);
      #ifdef I2CIP_DEBUG_SERIAL
        DEBUG_SERIAL.print(c);
      #endif
      GPIO::_id[k] = c;
    }

    GPIO::_id[idlen] = '\0';
    GPIO::_id_set = true;

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_SERIAL.print("'\n");
      DEBUG_DELAY();
    #endif
  }
}

i2cip_errorlevel_t GPIO::get(state_gpio_t& dest, const args_gpio_t& args) {
  // Set pin mode
  i2cip_errorlevel_t errlev = this->pinMode(args, GPIO_PINMODE_INPUT);
  I2CIP_ERR_BREAK(errlev);

  // Read in register
  uint8_t value;
  errlev = this->readRegisterByte(GPIO_PIN_REG(GPIO_GPIO, args), value);
  dest = (errlev > I2CIP_ERR_NONE ? GPIO_PIN_UNDEF : (state_gpio_t)READ_BITS(value, GPIO_PIN_SHIFT(args), 1));
  return errlev;
}

i2cip_errorlevel_t GPIO::set(const state_gpio_t& value, const args_gpio_t& args) {
  if(value == GPIO_PIN_UNDEF) {
    return I2CIP_ERR_SOFT;
  }

  // Set pin mode
  i2cip_errorlevel_t errlev = this->pinMode(args, GPIO_PINMODE_OUTPUT);
  I2CIP_ERR_BREAK(errlev);
  
  // Read register
  uint8_t existing;
  errlev = this->readRegisterByte(GPIO_PIN_REG(GPIO_GPIO, args), existing);
  I2CIP_ERR_BREAK(errlev);

  // Write register
  return this->writeRegister(GPIO_PIN_REG(GPIO_GPIO, args), OVERWRITE_BITS(existing, value, (args % 8), 1));
}

i2cip_errorlevel_t GPIO::pinMode(const args_gpio_t& pin, const gpio_pinmode_t& mode) {
  // Read in register
  uint8_t existing;
  i2cip_errorlevel_t errlev = this->readRegisterByte(GPIO_PIN_REG(GPIO_IODIR, pin), existing);
  I2CIP_ERR_BREAK(errlev);

  // Write to register
  return this->writeRegister(GPIO_PIN_REG(GPIO_IODIR, pin), OVERWRITE_BITS(existing, mode, (pin % 8), 1));
}

// args_gpio_t getDefaultA(void) const override;
// state_gpio_t getDefaultCache(void) const override;
// args_gpio_t getDefaultB(void) const override;
// state_gpio_t getFailsafe(void) const override;

const args_gpio_t& GPIO::getDefaultA(void) const {
  return default_a;
}

void GPIO::clearCache(void) {
  this->setCache(default_cache);
}

const args_gpio_t& GPIO::getDefaultB(void) const {
  return default_b;
}

void GPIO::resetFailsafe(void) {
  this->setValue(default_failsafe);
}