#include "EzoDeviceManager.h"
#include <Ezo_i2c_util.h>

EzoDeviceManager::EzoDeviceManager(TwoWire& wire)
    : _wire(wire) {}

bool EzoDeviceManager::scan() {
    clearDevices();

    bool deviceFound = false;

    for (uint8_t addr = 20; addr < 128; ++addr) {
        _wire.beginTransmission(addr);
        uint8_t error = _wire.endTransmission();

        if (error == 0) {
            deviceFound = true;
            // Create new device
            Ezo_board* device = new Ezo_board(addr, "found device", &_wire);

            Serial.println();
            device->send_cmd("I");
            delay(300);
            // TODO forwards this to the Computer Unit
            receive_and_print_response(*device);

            addDevice(device);
        }
    }

    return deviceFound;
}

/**
 * Updates the name of the divice in the list at the given address.
 * This will not be rememberd between scans. 
 */
void EzoDeviceManager::setName(uint8_t address, const char* name) {
    for (Ezo_board* device : _device_list) {
        if (device->get_address() == address) {
            device->set_name(name);

            Serial.print("Device at address ");
            Serial.print(device->get_address());
            Serial.print(" renamed to ");
            Serial.println(device->get_name());

            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}


/**
 * Sends the read command to the device in the list on the given address.
 */
void EzoDeviceManager::read(uint8_t address) {
    for (Ezo_board* device : _device_list) {
        if (device->get_address() == address) {
            device->send_cmd("R");
            // TODO Remove this once we know how to format the read data
            receive_and_print_reading(*device);
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}


/**
 * Sends the read command to the device in the list with the given name.
 */
void EzoDeviceManager::read(const char* name) {
    for (Ezo_board* device : _device_list) {
        if (strcmp(device->get_name(), name) == 0) {
            device->send_cmd("R");
            // TODO Remove this once we know how to format the read data
            receive_and_print_reading(*device);
            return;
        }
    }

    Serial.print("No device found with name ");
    Serial.println(name);
}


/**
 * Adds a device to the list.
 */
void EzoDeviceManager::addDevice(Ezo_board* device) {
    _device_list.push_back(device);
}

const std::vector<Ezo_board*>& EzoDeviceManager::getDevices() const {
    return _device_list;
}

void EzoDeviceManager::clearDevices() {
    for (Ezo_board* dev : _device_list) {
        delete dev;
    }
    _device_list.clear();
}