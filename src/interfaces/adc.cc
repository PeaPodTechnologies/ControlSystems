#include <interfaces/adc.h>

#include <Arduino.h>

#include <types.h>
#include <interface.h>

static i2cip_errorlevel_t get(const i2cip_fqa_t& fqa, float& dest, const adc_channel_t& args) override {
  // Set config register values
  uint16_t config =
    ADC_REG_CONFIG_CQUE_1CONV   | // Set CQUE to any value other than none so we can use it in RDY mode
    ADC_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
    ADC_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low (default val)
    ADC_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
    ADC_REG_CONFIG_MODE_SINGLE  | // Single-ended discrete reading
    ADC_GAIN                    | // Gain setting
    ADC_SPS                     | // Sampling rate setting
    ADC_CHANNEL_TO_MUX(args)    | // Set MUX from channel #
    ADC_REG_CONFIG_OS_SINGLE;     // Set 'start single-conversion' bit

  // Overwrite config register
  i2cip_errorlevel_t errlev = Device::writeRegister(fqa, ADC_REG_POINTER_CONFIG, config);
  I2CIP_ERR_BREAK(errlev);

  // Write threshold registers
  errlev = Device::writeRegister(fqa, ADC_REG_POINTER_HITHRESH, (uint16_t)0x8000);
  I2CIP_ERR_BREAK(errlev);
  errlev = Device::writeRegister(fqa, ADC_REG_POINTER_LOWTHRESH, (uint16_t)0x0000);
  I2CIP_ERR_BREAK(errlev);

  // Wait for the conversion to complete
  uint8_t timeout = 0;
  uint16_t ready = 0;
  do {
    if(timeout == ADC_TIMEOUT) {
      errlev = I2CIP_ERR_SOFT;
      break;
    }
    errlev = Device::readRegisterWord(fqa, ADC_REG_POINTER_CONFIG, ready);
    timeout++;
  } while((ready & 0x8000 == 0) && (errlev == I2CIP_ERR_NONE));
  I2CIP_ERR_BREAK(errlev);

  // Read the conversion results
  uint16_t result = NAN;
  errlev = Device::readRegisterWord(fqa, ADC_REG_POINTER_CONVERT, result);
  I2CIP_ERR_BREAK(errlev);
  uint8_t buf [2] = { 0 };
  result &= read(buf, 2);

  // Shift 12-bit results right 4 bits for the ADS1015, making sure we keep the sign bit intact
  uint16_t res = (((uint16_t)buf[0] << 8) | buf[1]) >> ADC_SHIFT;
  if (res > 0x07FF) {
    // negative number - extend the sign to 16th bit
    res |= 0xF000;
  }
  *dest = computeVolts((int16_t)res);
  return result;
}

float ADC::computeVolts(int16_t counts) {
  // see data sheet Table 3
  float fsRange;
  switch (ADC_GAIN) {
  case GAIN_TWOTHIRDS:
    fsRange = 6.144f;
    break;
  case GAIN_ONE:
    fsRange = 4.096f;
    break;
  case GAIN_TWO:
    fsRange = 2.048f;
    break;
  case GAIN_FOUR:
    fsRange = 1.024f;
    break;
  case GAIN_EIGHT:
    fsRange = 0.512f;
    break;
  case GAIN_SIXTEEN:
    fsRange = 0.256f;
    break;
  default:
    fsRange = 0.0f;
  }
  return counts * (fsRange / (32768 >> ADC_SHIFT));
}