import os
import serial
import serial.tools.list_ports
import threading
import tkinter as tk
import time
from threading import Thread
from time import sleep
from itertools import islice
from tkinter import filedialog as fd
from tkinter import ttk
import csv
import keyboard

running = True
f = ""
datafile = ""
writer = ""
labels = ["X", "Y", "Z"]
root = ""
isFirst = True


def timer(sp, interval):
    global f
    writer.writerow({'X': '-', 'Y': '-', 'Z': '-'})
    temp = csv.DictWriter(datafile, fieldnames=labels, lineterminator='\n',
                          delimiter='\n', quotechar='', quoting=csv.QUOTE_NONE, escapechar='\n')
    while running:
        if (keyboard.is_pressed('w')):  # walking
            keyboard.wait('esc')
            print('w')
            for i in range(10):
                temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
            temp.writerow({'X': 'w', 'Y': '', 'Z': '\n'})
        elif (keyboard.is_pressed('s')):  # standing
            keyboard.wait('esc')
            print('s')
            for i in range(10):
                temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
            temp.writerow({'X': 's', 'Y': '', 'Z': '\n'})
        elif (keyboard.is_pressed('t')):  # toe-walking
            keyboard.wait('esc')
            print('t')
            for i in range(10):
                temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
            temp.writerow({'X': 't', 'Y': '', 'Z': '\n'})
        elif (keyboard.is_pressed('r')):  # running
            keyboard.wait('esc')
            print('r')
            for i in range(10):
                temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
            temp.writerow({'X': 'r', 'Y': '', 'Z': '\n'})
        elif (keyboard.is_pressed('a')):  # sitting
            keyboard.wait('esc')
            print('a')
            for i in range(10):
                temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
            temp.writerow({'X': 'a', 'Y': '', 'Z': '\n'})
        elif (keyboard.is_pressed('l')):  # laying down flat
            keyboard.wait('esc')
            print('l')
            for i in range(10):
                temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
            temp.writerow({'X': 'l', 'Y': '', 'Z': '\n'})
        elif (keyboard.is_pressed('j')):  # jumping
            keyboard.wait('esc')
            print('j')
            for i in range(10):
                temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
            temp.writerow({'X': 'j', 'Y': '', 'Z': '\n'})

        # sleep(interval)
        line = sp.readline().decode("utf-8")
        line = line.replace("\r", "").replace("\n", "")
        line = line.split(",")
        imuDict = dict(zip(labels, line))
        print(imuDict)
        # data = [str(val) for val in line.split(' ')]
        # t = time.strftime("%Y-%m-%d %H:%M:%S")
        # newRow = "%s,%s,%s\n" % (t, data[0], data[1])
        # newRow = "%s,%s\n" % (t, line)
        # with open(f.name, "a") as datafile:
        # datafile.write(newRow)
        try:
            writer.writerow(imuDict)
        except Exception:
            break


def collect(strPort, interval):
    global f
    global datafile
    global writer
    global isFirst
    # Ask for a location to save the CSV file
    if isFirst:
        f = fd.asksaveasfile(mode='w', defaultextension=".csv")
    if f is None:  # User canceled save dialog
        return
    # Overwrite existing file
    try:
        os.remove(f)
    except:
        # File does not exist yet
        pass
    datafile = open(f.name, "a")
    temp = csv.DictWriter(datafile, fieldnames=labels, lineterminator='\n',
                          delimiter='\n', quotechar='', quoting=csv.QUOTE_NONE, escapechar='\n')
    if not isFirst:
        for i in range(10):
            temp.writerow({'X': '', 'Y': '', 'Z': '\n'})
    writer = csv.DictWriter(datafile, fieldnames=labels,
                            delimiter=",", lineterminator='\n')

    # writer.writeheader()
    sp = serial.Serial(strPort, 115200)

    time_thread = Thread(target=timer, args=(sp, interval))
    time_thread.start()
    print(threading.activeCount())


def qf(root):
    global isFirst
    # datafile.close()
    isFirst = False
    root.destroy()
    main()


def nf(root):
    global running

    running = False
    root.destroy()
    # datafile.close()


def end():
    global running
    global datafile

    try:
        datafile.close()
        datafile = ""
        tk.Label(root, text="Do you want to start another trial?").pack()
        b = tk.Button(root, text="Yes", command=lambda: qf(root))
        c = tk.Button(root, text="No", command=lambda: nf(root))
        b.pack()
        c.pack()
    except Exception:
        print("You are trying to close an unopened file")


def onIncrement(counter):
    counter.set(counter.get() + 1)


def main():
    global root
    root = tk.Tk()
    root.title("Serial USB IMU/Pressure Sensor Data Collector")
    mainframe = ttk.Frame(root)
    mainframe.grid(column=0, row=0, sticky=(tk.N, tk.W, tk.E, tk.S))
    mainframe.pack()
    serial_label = ttk.Label(mainframe, text="Sensor Serial Port:")
    serial_label.grid(row=0, column=0)
    serial_var = tk.StringVar(root)
    raw_ports = list(serial.tools.list_ports.comports())
    # ports = []
    # for p in raw_ports:
    #     if "USB Serial" in p.description:
    #         ports.append(p.device)'
    # raw_ports = serial.tools.list_ports.comports(include_links=False)
    ports = []
    for port in raw_ports:
        # if "USB Serial" in port.description:
        ports.append(port.device)

    serial_menu = ttk.OptionMenu(mainframe, serial_var, *ports)
    serial_menu.grid(row=0, column=1)
    counter = tk.IntVar()
    counter.set(1)
    # duration_label = ttk.Label(mainframe, textvariable=counter)
    duration_label = ttk.Label(mainframe, textvariable="")
    duration_label.grid(row=1, column=1)
    # duration_increment = ttk.Button(mainframe,
    #                                 text="Increase Collection Interval (sec)",
    #                                 command=lambda: onIncrement(counter))
    # duration_increment.grid(row=1, column=0)
    collect_button = ttk.Button(mainframe, text="Begin Data Collection",
                                command=lambda: collect(serial_var.get(),
                                                        counter.get()))
    collect_button.grid(row=2, column=0, sticky=(tk.E, tk.W))
    end_button = ttk.Button(mainframe, text="End Data Collection",
                            command=lambda: end())
    end_button.grid(row=2, column=1, sticky=(tk.E, tk.W))
    root.mainloop()


if __name__ == '__main__':
    main()
