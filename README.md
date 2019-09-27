# hardware-memtrack #

This module contains the STMicroelectronics Memtrack HAL source code.
It is part of the STMicroelectronics delivery for Android (see the [delivery][] for more information).

[delivery]: https://wiki.st.com/stm32mpu/wiki/STM32MP15_distribution_for_Android_release_note_-_v1.0.0

## Description ##

This module version includes the first version of the Memtrack Android abstraction layer.
Please see the Android delivery release notes for more details.

## Documentation ##

* The [release notes][] provide information on the release.
* The [distribution package][] provides detailed information on how to use this delivery.

[release notes]: https://wiki.st.com/stm32mpu/wiki/STM32MP15_distribution_for_Android_release_note_-_v1.0.0
[distribution package]: https://wiki.st.com/stm32mpu/wiki/STM32MP1_Distribution_Package_for_Android

## Dependencies ##

This module can't be used alone. It is part of the STMicroelectronics delivery for Android.

```
PRODUCT_PACKAGES += \
    memtrack.stm \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service
```
The file manifest.xml can be updated in consequence

## Containing ##

This directory contains the sources and associated Android makefile to generate the memtrack.stm library.

## License ##

This module is distributed under the Apache License, Version 2.0 found in the [LICENSE](./LICENSE) file.
