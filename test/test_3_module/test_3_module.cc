#include <Arduino.h>
#include <unity.h>

#define CSOS_DEBUG_SERIAL Serial
#define I2CIP_DEBUG_SERIAL Serial

#include <ControlSystemsOS.h>

#define TEST_ADDR 0x44

using ControlSystemsOS::CSOSModule;

CSOSModule m = CSOSModule(0, 0);

i2cip_fqa_t test_fqa = I2CIP_FQA_CREATE(0, 0, 0, TEST_ADDR);

void test_module_check(void) {
  // Ping MUX
  TEST_ASSERT_TRUE(MUX::pingMUX(0, 0));

  TEST_ASSERT_TRUE(m.operator()() == I2CIP_ERR_NONE);
}

void test_module_build(void) {
  // Test Module Build
  TEST_ASSERT_TRUE(Module::build(m));
}

void test_module_pass(void) {

  TEST_ASSERT_TRUE(m.operator[](test_fqa) != nullptr);

}

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_module_check);
  RUN_TEST(test_module_build);
  RUN_TEST(test_module_pass);

  UNITY_END();
}

void loop() {

}