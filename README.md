# gpio-scripts
coreboot GPIO scripts. Based on [Th3Fanbus' repo](https://github.com/Th3Fanbus/gpio-scripts/).

## gengpio.c
This [was uploaded by Stefan Reinauer to coreboot Gerrit](https://review.coreboot.org/14407).
[Riku_V](https://codeberg.org/Riku_V/) modified this to automatically parse inteltool logs.

### Usage
    make
    ./gengpio < inteltool.log > gpio.c

## hswgpio.c
This is [Th3Fanbus'](https://github.com/Th3Fanbus/gpio-scripts/) creation. It's for Haswell/Broadwell ULT. Needs some cleaning.

