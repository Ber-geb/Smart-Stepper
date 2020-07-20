from bluepy.btle import Peripheral, UUID, BTLEDisconnectError, DefaultDelegate
import time
import csv

f = ""
filename = ""
writer = ""
labels = ["X", "Y", "Z"]

class NotificationDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        global filename
        global csv_f
        # f = fd.asksaveasfile(mode='w', defaultextension=".csv")
        
        # xyz = data.decode().split('|')
        # print("X = {xyz[0]}, Y = {xyz[1]}, Z = {xyz[2]}")
        # xyz = data.decode("utf-8")
        # xyz = data.decode().split(',')
        xyz = data.decode("utf-8").split(',')
        imuDict = dict(zip(labels,xyz))
        print(imuDict)
        writer.writerow(imuDict)
        # csv_f.writerow(xyz)
        # with open(filename, "a") as datafile:
        #     datafile.write(xyz)
        # f.write(str(xyz) + "\n")
        

try:
    arduino = Peripheral(deviceAddr="c7:61:7d:c3:da:d1")
    arduino.setDelegate(NotificationDelegate())
except BTLEDisconnectError as error:
    print(error)
    print("Peripheral not found. Is it ON?")
else:
    filename = time.strftime("%Y%m%d_%H%M%S.csv")
    f = open(filename, "a")
    writer = csv.DictWriter(f, fieldnames=labels, delimiter="\t")
    writer.writeheader()
    # writer = csv.writer(f, delimiter='\t')
    # fieldnames = ['X','Y','Z']
    # csv_f = csv.DictWriter(f, fieldnames=fieldnames)
    #services = arduino.getServices()
    characteristics = arduino.getCharacteristics()
    for characteristic in characteristics:
        try:
            print("Value: {characteristic.read().decode()}, UUID: {characteristic.uuid}, Handle: {characteristic.handle}, ValueHandle: {characteristic.valHandle}")
            # f = open(filename, "a")
        except:
            pass
    #start_notification_data = b"\x01\x00"
    arduino.writeCharacteristic(13, bytes([1])) # arduino.writeCharacteristic(13, start_notification_data) will also work, 13 comes from "characteristic handle + 1"

    while True:
        try:
            if arduino.waitForNotifications(0.1):
                # handleNotification() gets called
                continue
            print("Waiting...")
        except KeyboardInterrupt:
            f.close()
            break

    arduino.disconnect()