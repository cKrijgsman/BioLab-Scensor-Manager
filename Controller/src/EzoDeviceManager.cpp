#include "EzoDeviceManager.h"
#include <Ezo_i2c_util.h>
#include "EzoCommandManager.h"
#include "EzoCallbacks.h"

EzoDeviceManager::EzoDeviceManager(TwoWire& wire)
    : _wire(wire) {}

bool EzoDeviceManager::scan(EzoCommandManager cmdManager) {
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
            cmdManager.queueCommand(device, "I", 300, [this](Ezo_board* device, const char* response, EzoCommandManager& mgr){
                //?I,RTD,2.15
                String responseString = String(response);
                int typeStartIndex = responseString.indexOf(',');
                int typeEndIndex = responseString.indexOf(',', typeStartIndex + 1);

                String deviceType = responseString.substring(typeStartIndex + 1, typeEndIndex);
                
                mgr.queueCommand(device, "Name?", 300, [this, deviceType](Ezo_board* device, const char* nameResponse, EzoCommandManager& mgr){
                    String nameResponseString = String(nameResponse);
                    int nameStart = nameResponseString.indexOf(',');
                    String deviceName = deviceType + "-" + nameResponseString.substring(nameStart + 1);

                    device->set_name(deviceName.c_str());

                    Serial.print("Device at 0x");
                    Serial.print(device->get_address(), HEX);
                    Serial.print("(");
                    Serial.print(device->get_address());
                    Serial.print(") with name: ");
                    Serial.println(device->get_name());

                    addDevice(device);
                });
            });
        }
    }

    return deviceFound;
}

/**
 * Updates the name of the divice in the list at the given address.
 * This will not be rememberd between scans. 
 */
void EzoDeviceManager::setName(uint8_t address, const char* name, EzoCommandManager cmdManager) {
    for (Ezo_board* device : _device_list) {
        if (device->get_address() == address) {
            device->set_name(name);

            cmdManager.queueCommand(device, strcat("Name,",name), 300, onEzoSetName);
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}


/**
 * Sends the read command to the device in the list on the given address.
 */
void EzoDeviceManager::read(uint8_t address, EzoCommandManager cmdManager) {
    for (Ezo_board* device : _device_list) {
        if (device->get_address() == address) {
            // Queue a Read command in the manager
            cmdManager.queueCommand(device, "R", 600, onEzoRead);
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}


/**
 * Sends the read command to the device in the list with the given name.
 */
void EzoDeviceManager::read(const char* name, EzoCommandManager cmdManager) {
    for (Ezo_board* device : _device_list) {
        if (strcmp(device->get_name(), name) == 0) {
            cmdManager.queueCommand(device, "R", 600, onEzoRead);
            return;
        }
    }

    Serial.print("No device found with name ");
    Serial.println(name);
}

void EzoDeviceManager::name(uint8_t address, EzoCommandManager cmdManager) {
    for (Ezo_board* device : _device_list) {
        if (device->get_address() == address) {
            cmdManager.queueCommand(device,"Name,?",300,onEzoGetName);
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}


void EzoDeviceManager::name(const char *name, EzoCommandManager cmdManager) {
    for (Ezo_board* device : _device_list) {
        if (strcmp(device->get_name(), name) == 0) {
            cmdManager.queueCommand(device,"Name,?",300,onEzoGetName);
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