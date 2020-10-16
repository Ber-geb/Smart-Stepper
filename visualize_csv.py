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

x = []
y = []
z = []
size = 0

root.destroy()
with open(file_path, 'r') as filestream:
    for line in filestream:
        currentline = line.replace("\n", "").split(",")
        if len(currentline) <= 2 or currentline[0] == '-':
            continue
        else:
            x.append(float(currentline[0]))
            y.append(float(currentline[1]))
            z.append(float(currentline[2]))
            size += 1

# style
plt.style.use('seaborn-darkgrid')

# create a color palette
palette = plt.get_cmap('Set1')

plt.title("Visual Representation of Walking Pattern")
plt.xlabel("Time")
plt.ylabel("Heading, Pitch, Roll")

plt.plot(x, color="red", label="Heading(X)")
plt.plot(y, color="blue", label="Pitch(Y)")
plt.plot(z, color="green", label="Roll(Z)")

plt.legend(loc=1, ncol=1)
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
