import { SerialPort } from 'serialport'
import { ReadlineParser } from '@serialport/parser-readline'
import { DeviceManager } from "./src/DeviceManager.js";


// Start a serial monitor on port COM6
const controller = new SerialPort({
    path: 'COM6',
    baudRate: 115200,
})
const parser = controller.pipe(new ReadlineParser({ delimiter: '\r\n' }))


// create the device manager
const deviceManager = new DeviceManager(controller)

parser.on('data', (data) => {
    if (data[0] === "#") {
        console.log(data)
    } else {
        deviceManager.parseMessage(data)
    }
})

console.log("App started!")

deviceManager.getDevicesFromController()