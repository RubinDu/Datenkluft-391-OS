#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "lib.h"

extern void initiate_shells();

// extern void context_switch();

extern void context_switch(int next_pid);

#endif