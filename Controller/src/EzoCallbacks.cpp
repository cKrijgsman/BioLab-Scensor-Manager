#include "EzoCallbacks.h"
#include <Arduino.h>

void onEzoRead(Ezo_board* device, const char* response)
{
    Serial.print(device->get_name());
    Serial.print(" (0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(") = ");
    Serial.println(response);
}

void onEzoGetName(Ezo_board* device, const char* response)
{
    Serial.print("Device 0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(" name: ");
    Serial.println(response);
}

void onEzoSetName(Ezo_board* device, const char* response)
{
    Serial.print("Device 0x");
    Serial.print(device->get_address(), HEX);
    Serial.println(" name updated");
}

void onEzoGetInfo(Ezo_board* device, const char* response)
{
    Serial.print("Device ");
    Serial.print(device->get_name());
    Serial.print(" info: ");
    Serial.println(response);
}

void onEzoScan(Ezo_board* device, const char* response)
{
    Serial.print("Found EZO device at 0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(" → ");
    Serial.println(response);
}