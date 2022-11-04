# PeaPod Microcontroller Software

A library written in Arduino C++ for modular plug-and-play low-level control systems through a high-level API. Implemented with PlatformIO and tested on AVR.

# Overview

The goals of this library are threefold:
1. Error-free asynchronous communication with a **dynamic** list of multiplexers and I2C devices;
2. **Abstraction** from those multiplexer groups into hot-swappable `Module`s, and I2C devices into `Sensor`s (environment variable *getters*) and `Actuator`s (environment variable *setters*);
3. **Mapping** together `Sensor`s and `Actuator`s into coherent `ControlSystem`s: feedback systems, each with a "parameter" that can be set as the target value of the environment variable of interest;

## Installation
> This will eventually be migrated to an Arduino library. For now, see #Development##Installation

# Development

## Testing

`pio test -e peapod`

## API

### Object Instance Tree
```
PeaPod (God Object)
|-ControlSystem*'s HashMap (for instruction handling & calculation)
| |-Control Systems [key: environment variable being controlled]
|   |-Sensor* (envvar getter)
|   |-Actuator* (envvar setter)
|   |-parameter (string)
|   |-target    (float)
|
|-Module List (hardware state change tracking -> Interface lifecycle management)
| |-Sensor* Array
| |-Actuator* Array
| |-Interface* Array (support)
| |-array lengths (byte)
```

### Inheritance Tree

```
Interface
|-ADC
|-EEPROM
|-GPIO
|-PWM
|-Any I2C Sensors/Actuators (multiple inheritance)

Module (MUX device)

Sensor
|-All Sensors

Actuator
|-All Actuators
```

### Dependency Summary

> Not including external libraries or `utils/`.

```
Interface
|-MUX (also included in Interface)
|-All other interfaces
|-All I2C Sensors/Actuators
```


## Style Guide

- Sensor and actuator instance-related code should not be const
- Static API code (i.e. ids, factories) should be const (and preferrably PROGMEM)
- Avoid passing around pointers to const pointers. It's already const.