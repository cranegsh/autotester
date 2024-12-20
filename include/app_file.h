#include <stdio.h>
#include <stdint.h>

#include "sysconfig.h"

BOOL_INT32 fileWrite_Word(FILE *dev, uint16_t addr, uint16_t value);
BOOL_INT32 fileRead_Word(FILE *dev, uint16_t addr, uint16_t *value);
BOOL_INT32 fileWrite_Byte(FILE *dev, uint16_t addr, uint8_t value);
BOOL_INT32 fileRead_Byte(FILE *dev, uint16_t addr, uint8_t *value);
