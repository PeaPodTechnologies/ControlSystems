#include <interface.h>

#include <Arduino.h>

#include <types.h>

Interface::Interface(const i2cip_fqa_t& fqa) : fqa(fqa) { }

template <typename S> i2cip_errorlevel_t OutputInterface<S>::set(const S& value) {
  return set(this->fqa, value);
}

template <typename G> i2cip_errorlevel_t InputInterface<G>::set(G& dest) {
  return get(this->fqa, dest);
}