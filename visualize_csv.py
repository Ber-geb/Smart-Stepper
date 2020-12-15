# This code is used to visualize the csv files

# UPDATE: Code is not working as intended...hopefully I can fix it later on
# UPDATE 2: Code is now working... Yay!

import matplotlib.pyplot as plt
import csv
import numpy as np
import tkinter as tk
from tkinter import filedialog

root = tk.Tk()
root.withdraw()

file_path = filedialog.askopenfilename()

ax = []
ay = []
az = []
gx = []
gy = []
gz = []
pP = []
pX = []
pY = []
pZ = []

size = 0

root.destroy()
with open(file_path, 'r') as filestream:
    for line in filestream:
        currentline = line.replace("\n", "").split(",")
        if len(currentline) <= 2 or currentline[0] == '-':
            continue
        else:
            # ax.append(float(currentline[0]))
            # ay.append(float(currentline[1]))
            # az.append(float(currentline[2]))
            # gx.append(float(currentline[3]))
            # gy.append(float(currentline[4]))
            # gz.append(float(currentline[5]))
            pP.append(float(currentline[8]))
            pX.append(float(currentline[9]))
            # pY.append(float(currentline[10]))
            # pZ.append(float(currentline[11]))
            size += 1

# style
plt.style.use('seaborn-darkgrid')

# create a color palette
palette = plt.get_cmap('Set1')

plt.title("Visual Representation of Walking Pattern")
plt.xlabel("Time")
plt.ylabel("Pressure and Position")

# plt.plot(ax, color="red", label="Heading(X)")
# plt.plot(ay, color="blue", label="Pitch(Y)")
# plt.plot(az, color="green", label="Roll(Z)")
# plt.plot(gx, color="red", label="Heading(X)")
# plt.plot(gy, color="blue", label="Pitch(Y)")
# plt.plot(gz, color="green", label="Roll(Z)")
plt.plot(pP, color="red", label="FSLP Pressure")
plt.plot(pX, color="blue", label="FSLP Position")
# plt.plot(pY, color="blue", label="FSR 1 Pressure")
# plt.plot(pZ, color="green", label="FSR 2 Pressure")

plt.legend(loc=1, bbox_to_anchor=(1.11, 1.15), ncol=1, fontsize="small")
plt.show()


# plt.plot(z, x, label='Loaded from file!', scaley=True)
# plt.plot(z, y, label='Loaded from file!', scaley=True)
# plt.yticks(np.arange(0, 110, step=10))
# plt.xticks([])
# plt.yticks([])

# plt.xlabel('Time')
# plt.ylabel('Heading, Pitch, Roll')
# plt.title('Interesting Graph\nCheck it out')
# plt.legend()
# plt.show()
