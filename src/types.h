#ifndef CSOS_TYPES_H_
#define CSOS_TYPES_H_

// ERROR LEVEL

/**
 * Degree of error occurred performing an I2C operation
 */
typedef enum {
  ERR_NONE, 
  ERR_SOFT, // The device is alive and reachable, but did not behave as expected (software error, etc.)
  ERR_HARD  // The device is not reachable
} errorlevel_t;

/**
 * = A & B operator for error levels. Returns whichever is greater.
 */
const errorlevel_t& operator&(const errorlevel_t& prev, const errorlevel_t& current) {
  return current > prev ? current : prev;
}

/**
 * A &= B operator for error levels. Returns whichever is greater.
 */
errorlevel_t operator&=(const errorlevel_t& prev, const errorlevel_t& current) {
  return (prev & current);
}

// INTERFACE STATE

/**
 * General state information. Passed around as `interfacestate_t`, binary OR-ed, and stored as a `uint8_t`
 */
typedef enum {
  ISTATE_OFF      = 0x0, // Disconnected, off, or not found. Will be deallocated on next module ping.
  ISTATE_ON       = 0x1, // Found, on, and initialized. Will be allocated on next module ping.
  ISTATE_SUCCESS  = 0x2, // 
} interfacestate_t;

/**
 * A |= B conversion operator from `interfacestate_t` "bits" to uint8_t "byte".
 */
uint8_t operator|=(const uint8_t lhs, const interfacestate_t& rhs) {
  return lhs | (uint8_t)rhs;
}

// Addressing

// ADDR
// SUBNET[]
  // MUX
  // BUS

#endif