#ifndef _PIT_H
#define _PIT_H

#include "lib.h"

extern void pit_init(uint16_t frequency);

extern void pit_handler();

#endif