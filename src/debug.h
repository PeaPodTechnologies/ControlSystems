#ifndef CSOS_DEBUG_SERIAL

#include <Arduino.h>

// BASIC DEBUG MACRO
#ifdef DEBUG
#if DEBUG == true
#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif
#endif
#endif

// CROSS-LIBRARY DEBUG COMPATIBILITY
#ifdef DEBUG_SERIAL
#define CSOS_DEBUG_SERIAL DEBUG_SERIAL
#endif

// DEBUG DELAY MACRO FOR SERIAL OUTPUT STABILITY (OPTIONAL)
#ifdef CSOS_DEBUG_SERIAL
#ifndef DEBUG_DELAY
#define DEBUG_DELAY() {delay(30);}
#endif
#endif

#endif

#include <../I2CIP/debug.h>
#include <../FiniteStateMachine/debug.h>