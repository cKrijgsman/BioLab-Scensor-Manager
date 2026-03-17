export class Device {

    address = 0;
    name = "";
    type = "";
    status = "";
    lastSeen = "";
    lastUpdated = "";

    monit = null

    constructor(address, deviceName) {
        this.address = address;

        const [type, name] = deviceName.split("-");

        this.name = name;
        this.type = type;
        this.status = 1;
        this.lastSeen = Date.now();
        this.lastUpdated = Date.now();
    }

    startMonitoring(interval = 1000, serial) {
        if (this.monit) clearInterval(this.monit)

        this.monit = setInterval(() => {
            this.read(serial)
        }, interval)
    }


    read(serial){
        serial.write(`READ ${this.address}`, (err) => {
            if (err) console.error(`Writing error: ${err}`)
        })
    }
}