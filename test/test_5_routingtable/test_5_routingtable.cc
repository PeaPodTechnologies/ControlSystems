#include <Arduino.h>
#include <unity.h>

#include <utils/bst.h>
#include <utils/hashtable.h>
#include <i2cip/routingtable.h>
#include <I2CIP.h>
#include "../config.h"

DeviceTable routingtable = DeviceTable();

void test_routingtable_empty(void) {
  i2cip_fqa_t fqa = 0;
  DeviceGroup* group = routingtable["null"];
  const char* id = routingtable[fqa];
  TEST_ASSERT_EQUAL_PTR_MESSAGE(nullptr, group, "Hashtable[\"null\"] -> nullptr");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(nullptr, id, "BST[0] -> nullptr");
}

void test_routingtable_add(void) {
  i2cip_device_t* device = routingtable.add("test", 1);
  TEST_ASSERT_FALSE_MESSAGE(device == nullptr, "New Entry: Added");
  TEST_ASSERT_EQUAL_UINT_MESSAGE(1, device->key, "New Entry: FQA match");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("test", device->value, "New Entry: ID match");
  const char* key = routingtable[1];
  TEST_ASSERT_EQUAL_STRING_MESSAGE(device->value, key, "BST: FQA lookup");
  DeviceGroup* group = routingtable["test"];
  TEST_ASSERT_FALSE_MESSAGE(group == nullptr, "HT: Device Group lookup by ID");
  TEST_ASSERT_EQUAL_UINT_MESSAGE(1, group->numdevices, "Device Group: Device Count");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("test", group->key, "Device Group: ID match");
  TEST_ASSERT_EQUAL_UINT_MESSAGE(1, group->contains(device), "Device Group: devices[0] FQA match");
}

void test_routingtable_overwrite(void) {
  i2cip_device_t* device = routingtable.add("test1", 1, false);
  TEST_ASSERT_FALSE_MESSAGE(device == nullptr, "NO Overwrite: Return Existing Device");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("test", device->value, "NO Overwrite");
  device = routingtable.add("test1", 1);
  TEST_ASSERT_EQUAL_STRING_MESSAGE("test1", device->value, "Overwrite");
}

void test_routingtable_addgroup(void) {
  i2cip_fqa_t fqas[3] = {1, 2, 3};
  // Add without overwriting {1:"test1"} from the previous test
  const DeviceGroup& devicegroup = routingtable.addGroup("test2", fqas, 3, false);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(2, devicegroup.numdevices, "Add Group: Device count match");
  for (int i = 0; i < devicegroup.numdevices; i++) {
    if(devicegroup.devices[i]->key == 1) {
      TEST_ASSERT_EQUAL_STRING_MESSAGE("test1", devicegroup.devices[i]->value, "Add Group: Overwrote");
    } else {
      TEST_ASSERT_EQUAL_STRING_MESSAGE("test2", devicegroup.devices[i]->value, "Added entry ID match");
    }
  }
}

void test_routingtable_remove(void) {
  TEST_ASSERT_FALSE_MESSAGE(routingtable.remove(0), "Remove: Nonexistent entry");
  TEST_ASSERT_TRUE_MESSAGE(routingtable.remove(1), "Found entry and attempted to remove!");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(nullptr, routingtable[1], "Entry removed successfully!");
  uint8_t n = routingtable["test1"]->numdevices;
  TEST_ASSERT_EQUAL_UINT_MESSAGE(0, n, "Entry removed successfully!");
  TEST_ASSERT_TRUE_MESSAGE(routingtable.remove(2), "Removed remaining group entries! (1/2)");
  TEST_ASSERT_TRUE_MESSAGE(routingtable.remove(3), "Removed remaining group entries! (2/2)");
  n = routingtable["test2"]->numdevices;
  TEST_ASSERT_EQUAL_UINT_MESSAGE(0, n, "Device group removed successfully!");
}

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_routingtable_empty);
  RUN_TEST(test_routingtable_add);
  RUN_TEST(test_routingtable_overwrite);
  RUN_TEST(test_routingtable_addgroup);
  RUN_TEST(test_routingtable_remove);

  UNITY_END();
}

void loop() {

}

/**
   * Scans the network for modules, and allocates and builds a route table based on SPRT EEROM.
   */
  // i2cip_errorlevel_t scanModule(RoutingTable& rt, const uint8_t& modulenum, const uint8_t& wirenum) {
  //   // Reusable buffer
  //   char eeprom_raw[I2CIP_EEPROM_SIZE] = { '\0' };

  //   // Scan every module's EEPROM
      
  //   i2cip_fqa_t fqa = createFQA(wirenum, modulenum, I2CIP_MUX_BUS_DEFAULT, I2CIP_EEPROM_ADDR);

  //   uint16_t bytes_read = 0;
  //   i2cip_errorlevel_t errlev = EEPROM::readContents(fqa, (uint8_t*)eeprom_raw, bytes_read);
  //   I2CIP_ERR_BREAK(errlev);

  //   // Convert char[] to json
  //   StaticJsonDocument<I2CIP_EEPROM_SIZE> eeprom_json;
  //   DeserializationError jsonerr = deserializeJson(eeprom_json, eeprom_raw);
  //   if(jsonerr) {
  //     // This module's JSON is a dud
  //     return I2CIP_ERR_SOFT;
  //   }

  //   // Read JSON to allocate table
  //   // TODO: Find a better way to do this
  //   JsonArray arr = eeprom_json.as<JsonArray>();

  //   uint8_t busnum = 0, totaldevices = 0;
  //   for (JsonObject bus : arr) {
  //     // Count reachable devices in each device group
  //     uint8_t devicecount = 0;
  //     for (JsonPair device : bus) {
  //       // Device addresses
  //       JsonArray addresses = device.value().as<JsonArray>();

  //       // See if each device is reachable
  //       uint8_t addressindex = 0;
  //       for (JsonVariant address : addresses) {
  //         fqa = createFQA(wirenum, modulenum, busnum, address.as<uint8_t>());
  //         if(Device::ping(fqa) > I2CIP_ERR_NONE) {
  //           // Device unreachable, remove from JSON; next device
  //           addresses.remove(addressindex);
  //           errlev = I2CIP_ERR_SOFT;
  //           continue;
  //         }
  //         // Device reachable; increment device count and address index
  //         devicecount++;
  //         addressindex++;
  //       }
  //     }
  //     busnum++;

  //     // Add the number of devices on this bus to the tally
  //     totaldevices += devicecount;
  //   }

  //   // Read JSON to fill table
  //   busnum = 0;
  //   for (JsonObject bus : arr) {
  //     for (JsonPair device : bus) {
  //       // Device addresses
  //       JsonArray addresses = device.value().as<JsonArray>();
  //       // Device ID (stack)
  //       const char* id = device.key().c_str();
        
  //       // Add to table
  //       for (JsonVariant address : addresses) {
  //         fqa = createFQA(wirenum, modulenum, busnum, address.as<uint8_t>());
  //         rt.add(id, fqa);
  //       }
  //     }
  //     busnum++;
  //   }
  //   return errlev;
  // }