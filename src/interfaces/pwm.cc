#include <interfaces/pwm.h>

#include <Arduino.h>

const char ControlSystemsOS::csos_id_pwm[] PROGMEM = {"PCA9685"};

bool ControlSystemsOS::PWM::_id_set;
char ControlSystemsOS::PWM::_id[I2CIP_ID_SIZE];

I2CIP::Device* ControlSystemsOS::pwmFactory(const i2cip_fqa_t& fqa) {
  if(!ControlSystemsOS::PWM::_id_set) {
    uint8_t idlen = strlen_P(csos_id_pwm);

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_DELAY();
      I2CIP_DEBUG_SERIAL.print(F("Loading GPIO ID PROGMEM to Static Array @0x"));
      I2CIP_DEBUG_SERIAL.print((uint16_t)(&(ControlSystemsOS::PWM::_id[0])), HEX);
      I2CIP_DEBUG_SERIAL.print(F(" ("));
      I2CIP_DEBUG_SERIAL.print(idlen+1);
      I2CIP_DEBUG_SERIAL.print(F(" bytes) '"));
    #endif

    // Read in PROGMEM
    for (uint8_t k = 0; k < idlen; k++) {
      char c = pgm_read_byte_near(csos_id_pwm + k);
      #ifdef I2CIP_DEBUG_SERIAL
        DEBUG_SERIAL.print(c);
      #endif
      ControlSystemsOS::PWM::_id[k] = c;
    }

    ControlSystemsOS::PWM::_id[idlen] = '\0';
    ControlSystemsOS::PWM::_id_set = true;

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_SERIAL.print("'\n");
      DEBUG_DELAY();
    #endif
  }

  return (I2CIP::Device*)(new ControlSystemsOS::PWM(fqa));
}

using namespace ControlSystemsOS;

PWM::PWM(const i2cip_fqa_t& fqa) : Device(fqa, (const char*)_id), OutputInterface<uint16_t, args_pwm_t>((Device*)this) {
  if(!PWM::_id_set) {
    uint8_t idlen = strlen_P(csos_id_pwm);

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_DELAY();
      I2CIP_DEBUG_SERIAL.print(F("Loading PWM ID PROGMEM to Static Array @0x"));
      I2CIP_DEBUG_SERIAL.print((uint16_t)(&(PWM::_id[0])), HEX);
      I2CIP_DEBUG_SERIAL.print(F(" ("));
      I2CIP_DEBUG_SERIAL.print(idlen+1);
      I2CIP_DEBUG_SERIAL.print(F(" bytes) '"));
    #endif

    // Read in PROGMEM
    for (uint8_t k = 0; k < idlen; k++) {
      char c = pgm_read_byte_near(csos_id_pwm + k);
      #ifdef I2CIP_DEBUG_SERIAL
        DEBUG_SERIAL.print(c);
      #endif
      PWM::_id[k] = c;
    }

    PWM::_id[idlen] = '\0';
    PWM::_id_set = true;

    #ifdef I2CIP_DEBUG_SERIAL
      DEBUG_SERIAL.print("'\n");
      DEBUG_DELAY();
    #endif
  }
}

i2cip_errorlevel_t PWM::set(const uint16_t& value, const args_pwm_t& args) {
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
  i2cip_errorlevel_t errlev = this->writeRegister(PWM_CHANNEL_TO_LEDREG(args), (uint8_t)(on & 0xFF));
  I2CIP_ERR_BREAK(errlev);
  errlev = this->writeRegister((uint8_t)(PWM_CHANNEL_TO_LEDREG(args) + 0x1), (uint8_t)(on >> 8));
  I2CIP_ERR_BREAK(errlev);
  errlev = this->writeRegister((uint8_t)(PWM_CHANNEL_TO_LEDREG(args) + 0x2), (uint8_t)(off & 0xFF));
  I2CIP_ERR_BREAK(errlev);
  errlev = this->writeRegister((uint8_t)(PWM_CHANNEL_TO_LEDREG(args) + 0x3), (uint8_t)(off >> 8));
  return errlev;
}

const args_pwm_t& PWM::getDefaultB(void) const {
  return default_b;
}

void PWM::resetFailsafe(void) {
  this->setValue(default_failsafe);
}