#pragma once

#include <vector>
#include <Ezo_i2c.h>
#include <Wire.h>

class EzoDeviceManager {
public:
    EzoDeviceManager(TwoWire& wire);

    // Scan I2C bus, returns true if a new device was found
    bool scan(EzoCommandManager cmdManager);

    // Alows for updating the name of a device in the list.
    void setName(uint8_t address, const char* name, EzoCommandManager cmdManager);

    void name(uint8_t address, EzoCommandManager cmdManager);
    void name(const char *name, EzoCommandManager cmdManager);

    void read(uint8_t address, EzoCommandManager cmdManager);  // Reads from a device based on its address
    void read(const char *name, EzoCommandManager cmdManager); // Reads from a device based on its name.

    // Read-only access to device list
    const std::vector<Ezo_board*>& getDevices() const;

private:
    TwoWire& _wire;
    std::vector<Ezo_board*> _device_list;

    void addDevice(Ezo_board* device);
    void clearDevices();
};
