#ifndef CSOS_DEBUG_H_
#define CSOS_DEBUG_H_

// CROSS-LIBRARY DEBUG COMPATIBILITY
#ifdef DEBUG_SERIAL
#define CSOS_DEBUG_SERIAL DEBUG_SERIAL
#endif

#ifdef DEBUG
#if DEBUG == true
#define CSOS_DEBUG_SERIAL Serial
#endif
#endif

#endif