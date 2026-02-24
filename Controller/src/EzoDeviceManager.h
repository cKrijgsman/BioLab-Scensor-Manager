#pragma once

#include <vector>
#include <Ezo_i2c.h>
#include <Wire.h>
#include "EzoCommandManager.h"
#include <cstring>
#include <utility>

class EzoDeviceManager {
public:
    EzoDeviceManager(TwoWire& wire);

    // Scan I2C bus, returns true if a new device was found
    bool scan(EzoCommandManager& cmdManager);

    // Alows for updating the name of a device in the list.
    void setName(uint8_t address, const char* name, EzoCommandManager& cmdManager);

    void getName(uint8_t address, EzoCommandManager& cmdManager);
    void getName(const char *name, EzoCommandManager& cmdManager);

    void read(uint8_t address, EzoCommandManager& cmdManager);  // Reads from a device based on its address
    void read(const char *name, EzoCommandManager& cmdManager); // Reads from a device based on its name.

    void info(uint8_t address, EzoCommandManager& cmdManager);  // Gets info from a device based on its address
    void info(const char *name, EzoCommandManager& cmdManager); // Gets info from a device based on its name.

    void setI2CAddress(uint8_t old_address, uint8_t new_address, EzoCommandManager& cmdManager);

    // Read-only access to device list
    const std::vector<Ezo_board*>& getDevices() const;

private:
    TwoWire& _wire;
    std::vector<Ezo_board*> _device_list;
    // Owned C-strings for device names allocated by this manager
    std::vector<std::pair<Ezo_board*, char*>> _owned_names;

    // Helper to allocate and assign a stable name pointer for an Ezo_board
    void setDeviceNameOwned(Ezo_board* device, const String& name);
    // Helper to free any owned name for a device
    void freeDeviceName(Ezo_board* device);

    void addDevice(Ezo_board* device);
    void clearDevices();
    void sendRead(Ezo_board* device, EzoCommandManager& cmdManager);
    void sendGetName(Ezo_board* device, EzoCommandManager& cmdManager);
    void sendInfo(Ezo_board* device, EzoCommandManager& cmdManager);
};
