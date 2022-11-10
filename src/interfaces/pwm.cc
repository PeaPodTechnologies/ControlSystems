#include <interfaces/pwm.h>

#include <Arduino.h>

#include <types.h>
#include <interface.h>

static i2cip_errorlevel_t PWM::set(const i2cip_fqa_t& fqa, const uint16_t& value, const pwm_channel_t& args) override {
  // Encode LED on/off values
  uint16_t on = 0, off = 0;
  if (value == 0x0000) {
    // OFF
    off = 0x1000;
  } else if (value > 0xFFF) {
    // ON
    on = 0x1000;
  } else {
    // Truncate
    off = value & 0xFFF;
  }

  // Write registers
  i2cip_errorlevel_t errlev = Device::writeRegister(fqa, PWM_CHANNEL_TO_LEDREG(args), (uint8_t)(on & 0xFF));
  I2CIP_ERR_BREAK(errlev);
  i2cip_errorlevel_t errlev = Device::writeRegister(fqa, PWM_CHANNEL_TO_LEDREG(args) + 1, (uint8_t)(on >> 8));
  I2CIP_ERR_BREAK(errlev);
  i2cip_errorlevel_t errlev = Device::writeRegister(fqa, PWM_CHANNEL_TO_LEDREG(args) + 2, (uint8_t)(off & 0xFF));
  I2CIP_ERR_BREAK(errlev);
  i2cip_errorlevel_t errlev = Device::writeRegister(fqa, PWM_CHANNEL_TO_LEDREG(args) + 3, (uint8_t)(off >> 8));
  return errlev;
}

static const char* PWM::getID(void) override {
  return PWM_ID;
}