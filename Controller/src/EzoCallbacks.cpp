#include "EzoCallbacks.h"
#include <Arduino.h>

void onEzoRead(Ezo_board* device, const char* response, EzoCommandManager& mgr)
{
    // Prints the data in the following format:
    // READING,DEVICE_NAME,ADDRESS,VALUE

    Serial.print("READING,");
    Serial.print(device->get_name());
    Serial.print(",0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(",");
    Serial.println(response);
}

void onEzoGetName(Ezo_board* device, const char* response, EzoCommandManager& mgr)
{
    // Prints the data in the following format:
    // NAME,DEVICE_NAME,ADDRESS,VALUE

    Serial.print("NAME,");
    Serial.print(device->get_name());
    Serial.print(",0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(",");
    Serial.println(response);
}

void onEzoGetInfo(Ezo_board* device, const char* response, EzoCommandManager& mgr)
{
    // Response format is "EZO,DEVICE_TYPE,VERSION". We print it in the following format:
    // INFO,DEVICE_NAME,ADDRESS,DEVICE_TYPE,VERSION
    Serial.println("Received INFO response: " + String(response));
    String responseString = String(response);
    int firstComma = responseString.indexOf(',');
    int secondComma = responseString.indexOf(',', firstComma + 1);
    if (firstComma == -1 || secondComma == -1) {
        Serial.print("Warning: unexpected INFO response format from device 0x");
        Serial.println(device->get_address(), HEX);
        return;
    }

    String deviceType = responseString.substring(0, firstComma);
    String version = responseString.substring(firstComma + 1, secondComma);

    Serial.print("INFO,");
    Serial.print(device->get_name());
    Serial.print(",0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(",");
    Serial.print(deviceType);
    Serial.print(",");
    Serial.println(version);

}

void onEzoCalibrate(Ezo_board* device, const char* response, EzoCommandManager& mgr)
{
    Serial.print("CALIBRATE,");
    Serial.print(device->get_name());
    Serial.print(",0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(",");
    Serial.println(response);
}

void onEzoEmptyResponse(Ezo_board* device, const char* response, EzoCommandManager& mgr)
{
    // This can be used as a generic callback for commands where we don't expect a meaningful response, but want to log completion or trigger follow-up actions.
    Serial.print("COMMAND_COMPLETE,");
    Serial.print(device->get_name());
    Serial.print(",0x");
    Serial.println(device->get_address(), HEX);
}