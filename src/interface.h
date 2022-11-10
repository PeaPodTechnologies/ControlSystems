#ifndef CSOS_INTERFACES_INTERFACE_H_
#define CSOS_INTERFACES_INTERFACE_H_

// HEADERS

#include <Arduino.h>
#include <I2CIP.h>

#include <types.h>

/**
 * Shifts and masks a number's bits.
 * @param data  Data to shift/mask
 * @param lsb   LSB position
 * @param bits  Number of bits to keep
 **/
#define CSOS_READ_BITS(data, lsb, bits) (((data) >> (lsb)) & ((1 << (bits)) - 1))

/**
 * Overwrites some bits in existing data.
 * @param existing  Existing data
 * @param data      New data
 * @param lsb       Position to insert (LSB)
 * @param bits      Number of bits to overwrite
 **/
#define CSOS_OVERWRITE_BITS(existing, data, lsb, bits) ((existing) & ~(((1 << (bits)) - 1) << (lsb)) | (((data) & ((1 << (bits)) - 1)) << (lsb)))

using namespace I2CIP;

// CLASS
// An Interface is a generic I2CIP peripheral

class Interface {
  public:
    static virtual const char* getID(void) = 0;
}

template <typename G, typename A> class InputInterface : public Interface {
  public:
    static virtual i2cip_errorlevel_t get(const i2cip_fqa_t& fqa, G& dest, const A& args) = 0;
    i2cip_errorlevel_t get(G& dest, const A& args);
};

template <typename S, typename B> class OutputInterface : public Interface {
  public:
    static virtual i2cip_errorlevel_t set(const i2cip_fqa_t& fqa, const S& value, const B& args) = 0;
    i2cip_errorlevel_t set(const S& value, const B& args);
};

template <typename G, typename A, typename S, typename B> class IOInterface : public InputInterface<G, A>, public OutputInterface<S, B> { };

// template <class S> static S* I2CSensorFactory(Interface* i, JsonObject* args);

#endif