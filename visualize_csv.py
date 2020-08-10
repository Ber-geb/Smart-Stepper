# This code is used to visualize the csv files 
# NOTE: REMEMBER TO REMOVE THE FIRST ROW OF THE CSV FILE YOU ARE 
#       TRYING TO TEST (the column with X, Y, Z)

import matplotlib.pyplot as plt
import csv
import numpy as np

x = []
y = []
z = []
increment = 0

with open('1.csv', 'r') as csvfile:
    plots = csv.reader(csvfile)
    for row in plots:
        x.append(row[0])
        y.append(row[1])
        z.append(increment)
        increment += 1


plt.plot(z, x, label='Loaded from file!', scaley=True)
plt.plot(z, y, label='Loaded from file!', scaley=True)
plt.yticks(np.arange(0,110,step=10))
plt.xticks([])
plt.yticks([])

plt.xlabel('Time')
plt.ylabel('Heading, Pitch, Roll')
plt.title('Interesting Graph\nCheck it out')
plt.legend()
plt.show()
