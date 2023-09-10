#include <interfaces/gpio.h>

#include <Arduino.h>
#include <I2CIP.h>

using namespace I2CIP;

const char* id_gpio = "ADS1015";

GPIO::GPIO(const i2cip_fqa_t& fqa) : Device(fqa, id_gpio) { }

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