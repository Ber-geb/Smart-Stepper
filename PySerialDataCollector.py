import os
import serial
import serial.tools.list_ports
import threading
import tkinter as tk
import time
from threading import Thread
from time import sleep
from tkinter import filedialog as fd
from tkinter import ttk
import csv

running = True
f = ""
datafile = ""
writer = ""
labels = ["X", "Y", "Z"]
root = ""


def timer(sp, interval):
    global f
    while running:
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
        writer.writerow(imuDict)


def collect(strPort, interval):
    global f
    global datafile
    global writer
    # Ask for a location to save the CSV file
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
    writer = csv.DictWriter(datafile, fieldnames=labels,
                            delimiter=",", lineterminator='\n')
    writer.writeheader()
    sp = serial.Serial(strPort, 115200)
    time_thread = Thread(target=timer, args=(sp, interval))
    time_thread.start()

def qf(root):
    # datafile.close()
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

    datafile.close()
    datafile = ""
    tk.Label(root, text = "Do you want to start another trial?").pack()
    b = tk.Button(root, text="Yes", command=lambda: qf(root))
    c = tk.Button(root, text="No", command=lambda: nf(root))
    b.pack()
    c.pack()
    


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
    duration_label = ttk.Label(mainframe, textvariable=counter)
    duration_label.grid(row=1, column=1)
    duration_increment = ttk.Button(mainframe,
                                    text="Increase Collection Interval (sec)",
                                    command=lambda: onIncrement(counter))
    duration_increment.grid(row=1, column=0)
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
