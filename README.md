# LHe-Level-Meter

Firmware of liquid He monitors, that monitor the level of He dewers. They are meant to be used in conjunction with [Helium Management](https://github.com/SampleEnvironment/Helium-Management) application.

## Getting started
### Requisities
In order to start developing/testing/flashing Firmware you will need: 
- [Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio)
- A debugger for example *Atmel Power debugger*
- Levelmeter



### Preparations
Before first compiling the project in Microchip Studio the submodule of the avr-library needs to be initialized:
```
git submodule update --init
```

### Choosing a Release
In order to generate a executable Firmware of a specific release, `git tag` will list all available releases and 
```git checkout --recurse-submodules vX.XXX``` 
will load the release specified by `vX.XXX` (for example `v1.221`).

### Build configurations (from v1.219):
Build configurations are used to generate firmware binarys that are meant for the same device Type, but with varying Hardware, for example the display that is used. Build configuration names for Levelmeter have the following Format `'DisplayType'-'DisplayConfiguration'` and can be set via the dropdown menu depicted in the screenshot below.

![build_configurationslvlmeter](https://user-images.githubusercontent.com/85115389/203858797-030be3e1-eace-49d6-90cc-699f5324b870.png)



Buildconfigurations have the following Format ```'DisplayType'-'DisplayConfiguration'```. All different configurations are listed and described in the tables below:


| Display Type | Description                                            |
|--------------|--------------------------------------------------------|
| DISP_3000    | original rgb LCD Display 132x176px Resolution          |
| ili9341      | Newer 2.2" rgb Display with a higher resolution of 240x320px  |

| Display Configuration | Description                                                                                         |
|-------------------|---------------------------------------------------------------------------------------------------------|
| Default           | Default Display Configuration                                                                           |
| ILL               | Disply is rotated 180° in ILL Levelmeters, because a different housing is used                             |



### Manual

For further Information please refer to the [wiki](https://github.com/SampleEnvironment/LHe-Level-Meter/wiki/) pages 

