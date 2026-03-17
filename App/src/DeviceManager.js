import { Device } from "./Device.js"


export class DeviceManager {

    controller = null
    devices = []

    constructor(controller) {
        this.controller = controller
    }

    getDevicesFromController(){
        this.controller.write("LIST", (err) => {
            if (err) console.error(`Error on write: ${err}`)
        })
    }


    addDevice(data){
        const address = parseInt(data[0], 16)
        const name = data[1]

        const device = new Device(address, name)
        device.startMonitoring(5000, this.controller)

        this.devices.push(device)
        console.log(`Added Device: ${name} at ${address}`)
    }

    parseMessage(message){
        const [type,...commandData] = message.split(",")

        console.log(`${type}: ${commandData}`)
        switch (type){
            case "DEVICE":
                this.addDevice(commandData)
                break;
            case "READING":
                this.parseRead(commandData)
                break
        }
    }

    parseRead(messageData){
        //READING,RTD-Sensor1,0x66,22.726
        const name = messageData[0]
        const address = messageData[1]
        const value = messageData[2]

        console.log(`Data received for ${name} with value: ${value}`)
    }


}