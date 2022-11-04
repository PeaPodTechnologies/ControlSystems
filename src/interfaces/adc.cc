#include <interfaces/adc.h>

#include <Arduino.h>

#include <types.h>
#include <interface.h>

float Interface::ADC::computeVolts(int16_t counts) {
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

errorlevel_t Interface::ADC::readVoltage(uint8_t channel, float* dest) {
  if (channel > 3) {
    return ERR_SOFT;
  }

  // Set config register values
  uint16_t config =
    ADC_REG_CONFIG_CQUE_1CONV |   // Set CQUE to any value other than
                                  // none so we can use it in RDY mode
    ADC_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
    ADC_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low (default val)
    ADC_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
    ADC_REG_CONFIG_MODE_SINGLE |  // Single-ended discrete reading
    ADC_GAIN |                    // Gain setting
    ADC_SPS |                     // Sampling rate setting
    ADC_CHANNEL_TO_MUX(channel) | // Set MUX from channel #
    ADC_REG_CONFIG_OS_SINGLE;     // Set 'start single-conversion' bit

  // Write config register to the ADC
  errorlevel_t result = writeRegister(ADC_REG_POINTER_CONFIG, config);

  // Set ALERT/RDY to RDY mode.
  result &= writeRegister(ADC_REG_POINTER_HITHRESH, (uint16_t)0x8000);
  result &= writeRegister(ADC_REG_POINTER_LOWTHRESH, (uint16_t)0x0000);

  // Wait for the conversion to complete
  // TODO: how long does this take?
  while (!ready());

  // Read the conversion results
  result &= write((uint8_t)(ADC_REG_POINTER_CONVERT & 0x8000));
  uint8_t buf [2] = { 0 };
  result &= read(buf, 2);

  // Shift 12-bit results right 4 bits for the ADS1015, making sure we keep the sign bit intact
  uint16_t res = ((buf[0] << 8) | buf[1]) >> ADC_SHIFT;
  if (res > 0x07FF) {
    // negative number - extend the sign to 16th bit
    res |= 0xF000;
  }
  *dest = computeVolts((int16_t)res);
  return result;
}

bool Interface::ADC::ready(void) {
  uint8_t buf [2] = { 0 };
  write((uint8_t)(ADC_REG_POINTER_CONFIG & 0x8000));
  read(buf, 2);
  return ((buf[0] << 8) | buf[1]) != 0;
}