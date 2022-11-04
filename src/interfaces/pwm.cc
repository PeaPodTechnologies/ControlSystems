#include <interfaces/pwm.h>

#include <Arduino.h>

#include <types.h>
#include <interface.h>

errorlevel_t Interface::PWM::begin(void) {
  errorlevel_t result = thisinterface.begin();

  if(result != ERR_NONE) {
    return result;
  }

  result &= setPWMFreq(PWM_FREQ);

  // Configure totem-pole output
  uint8_t oldmode;
  result &= write((uint8_t)PCA9685_MODE2);
  result &= read(&oldmode);
  
  uint8_t buf [2] = { PCA9685_MODE2, 0 };
  buf[1] = oldmode | MODE2_OUTDRV;

  result &= write(buf, 2);
  return result;
}

errorlevel_t Interface::PWM::setPWM(uint8_t pin, float pwm) {
  pwm = constrain(pwm, 0.0f, 1.0f);
  uint16_t on = 0, off = 0;
  if (pwm < PWM_EPSILON) {
    // OFF
    off = 4096;
  } else if (1.0f - pwm < PWM_EPSILON) {
    // ON
    on = 4096;
  } else {
    // pwm % of 0-4095 inclusive
    off = (uint16_t)(pwm * 4096.0f);
  }

  uint8_t buf [5] = { 0x06 + (4 * pin), on, on >> 8, off, off >> 8};
  return write(buf, 5);
}

errorlevel_t Interface::PWM::setPWMFreq(float freq) {
  // Constrain frequency, calculate prescale
  freq = constrain(freq, PWM_FREQ_MIN, PWM_FREQ_MAX);
  uint8_t prescale = (uint8_t)constrain(((PWM_OSC_FREQ / (freq * 4096.0)) + 0.5) - 1, PWM_PRESCALE_MIN, PWM_PRESCALE_MAX);

  // Reusable buffer
  uint8_t buf [2] = { PCA9685_MODE1, 0 };
  uint8_t oldmode;

  // Read Mode 1 register
  errorlevel_t result = write((uint8_t)PCA9685_MODE1);
  result &= read(&oldmode);

  // Sleep
  buf[1] = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
  result &= write(buf, 2);

  // Write new prescale
  buf[0] = PCA9685_PRESCALE;
  buf[1] = prescale;
  result &= write(buf, 2);

  // Resume old mode
  buf[0] = PCA9685_MODE1;
  buf[1] = oldmode;
  result &= write(buf, 2);

  delay(5);

  buf[1] |= MODE1_RESTART | MODE1_AI;
  result &= write(buf, 2);

  return result;
}

errorlevel_t Interface::PWM::reset(void) {
  uint8_t buf[2] = { PCA9685_MODE1, MODE1_RESTART };
  write(buf, 2);
  return ERR_NONE;
}