/*
 * Copyright (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "mbed.h"
//#include "mbed-os.h"
#include "PinMap.h"
#include "BufferedSerial.h"
#include "GenericPingPong.h"

void SystemClock_Config(void);

extern BufferedSerial *ser;
extern void dump(const char *title, const void *data, int len, bool dwords = false);

#define dprintf(...) { ser->printf(__VA_ARGS__); ser->printf("\r\n"); }