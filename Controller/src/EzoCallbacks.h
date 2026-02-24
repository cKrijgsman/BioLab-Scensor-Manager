#pragma once

#include "EzoCommandManager.h"
#include <Ezo_i2c.h>

// Called when a read ("R") command finishes
void onEzoRead(Ezo_board* device, const char* response, EzoCommandManager& mgr);

// Called when a name ("Name,?") command finishes
void onEzoGetName(Ezo_board* device, const char* response, EzoCommandManager& mgr);

// Called when a set name ("Name,n") command finishes
void onEzoSetName(Ezo_board* device, const char* response, EzoCommandManager& mgr);

// Called when an info ("I") command finishes
void onEzoGetInfo(Ezo_board* device, const char* response, EzoCommandManager& mgr);

// Called when a scan-identify ("I") command finishes
void onEzoScan(Ezo_board* device, const char* response, EzoCommandManager& mgr);