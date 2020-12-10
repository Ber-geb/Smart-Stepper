import tkinter as tk
from tkinter import filedialog
import os
import csv

# Ask user if they want to format data separately for each movement or combine them

msg = input("Enter 'a' for separate formatting and or press 'b' for combined formatting\n")

if (msg == 'a'):
    # Data file needs to be selected (will see if we can select more than one)
    root = tk.Tk()
    root.withdraw()

    file_path = filedialog.askopenfilename()

    root.destroy()

    # Prompt user for name of participant in data trial
    firstName = input("First name: ")
    firstName = firstName.strip()


    # Read from data and use the key values to determine which file the data needs
    # to be formatted in

    keys = ['w', 't', 's', 'p', 'r', 'y', 'a', 'z', 'l', 'j', 'h', 'i', 'b', 'd']
    currentKey = ""
    changePosition = False
    fileName = ""

    with open(file_path) as f:
        for line in f:
            if ("-,-,-" in line or len(line) <= 1):  # Skip any line with "-,-,-"
                continue
            if changePosition:
                if not os.path.exists(fileName):
                    with open(fileName, "a") as data:
                        data.write("-,-,-\n")
                        data.write(line)
                        changePosition = False
                    continue
                else:
                    with open(fileName, "a") as data:
                        for i in range(0, 50):
                            data.write(' \n')
                        data.write("-,-,-\n")
                        data.write(line)
                        changePosition = False
                    continue
            else:
                for c in line:
                    if c in keys:  # if true, we are looking at a position change
                        if c == 'w':  # walking
                            currentKey = 'w'
                            changePosition = True
                            fileName = firstName+"_NormalWalking.txt"
                            break
                        elif c == 't':  # toe-walking
                            currentKey = 't'
                            changePosition = True
                            fileName = firstName+"_ToeWalking.txt"
                            break
                        elif c == 's':  # Standing
                            currentKey = 's'
                            changePosition = True
                            fileName = firstName+"_Standing.txt"
                            break
                        elif c == 'p':  # ToeStanding
                            currentKey = 'p'
                            changePosition = True
                            fileName = firstName+"_ToeStanding.txt"
                            break
                        elif c == 'r':  # Jogging
                            currentKey = 'r'
                            changePosition = True
                            fileName = firstName+"_Jogging.txt"
                            break
                        elif c == 'y':  # ToeJogging
                            currentKey = 'y'
                            changePosition = True
                            fileName = firstName+"_ToeJogging.txt"
                            break
                        elif c == 'a':  # Sitting
                            currentKey = 'a'
                            changePosition = True
                            fileName = firstName+"_Sitting.txt"
                            break
                        elif c == 'z':  # ToeSitting
                            currentKey = 'z'
                            changePosition = True
                            fileName = firstName+"_ToeSitting.txt"
                            break
                        elif c == 'h':  # Normal Walking Up Stairs
                            currentKey = 'h'
                            changePosition = True
                            fileName = firstName+"_NormalWalkingUpStairs.txt"
                            break
                        elif c == 'i':  # Normal Walking Down Stairs
                            currentKey = 'i'
                            changePosition = True
                            fileName = firstName+"_NormalWalkingDownStairs.txt"
                            break
                        elif c == 'b':  # Toe Walking Up Stairs
                            currentKey = 'b'
                            changePosition = True
                            fileName = firstName+"_ToeWalkingUpStairs.txt"
                            break
                        elif c == 'd':  # Toe Walking Down Stairs
                            currentKey = 'd'
                            changePosition = True
                            fileName = firstName+"_ToeWalkingDownStairs.txt"
                            break
                        elif c == 'l':  # Laying Down
                            currentKey = 'l'
                            changePosition = True
                            fileName = firstName+"_LayingDown.txt"
                            break
                        elif c == 'j':  # Jumping
                            currentKey = 'j'
                            changePosition = True
                            fileName = firstName+"_Jumping.txt"
                            break

                if not changePosition:
                    with open(fileName, "a") as k:
                        k.write(line)

elif msg == 'b':
    # Data file needs to be selected (will see if we can select more than one)
    root = tk.Tk()
    root.withdraw()

    file_path = filedialog.askopenfilenames()

    root.destroy()

    # What movement are we currently formatting
    msg = input("Which movement? \n 1. Normal Walking \n 2. Toe Walking \n 3. Jogging \n 4. Toe Jogging \n 5. Standing \n 6. Toe Standing \n 7. Sitting \n 8. Toe Sitting \n 9. Normal Walking Up Stairs \n 10. Toe Walking Up Stairs \n 11. Normal Walking Down Stairs \n 12. Toe Walking Down Stairs \n")

    if msg == '1': # Normal Walking
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("NormalWalking.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '2': # Toe Walking
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("ToeWalking.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '3': # Jogging
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("Jogging.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '4': # Toe Jogging
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("ToeJogging.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '5': # Standing
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("Standing.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '6': # Toe Standing
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("ToeStanding.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '7': # Sitting
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("Sitting.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '8': # Toe Sitting
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("ToeSitting.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '9': # Normal Walking Up Stairs
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("NormalWalkingUpStairs.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '10': # Toe Walking Up Stairs
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("ToeWalkingUpStairs.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '11': # Normal Walking Down Stairs
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("NormalWalkingDownStairs.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)
    elif msg == '12': # Toe Walking Down Stairs
        for filePath in file_path:
            with open(filePath, "r", newline='') as db:
                csv_reader = csv.reader(db)
                dbEntries = []
                for row in csv_reader:
                    if row[0] == '-':
                        continue
                    dbEntries.append(row)
                with open("ToeWalkingDownStairs.csv", "a", newline='') as f:
                    csv_writer = csv.writer(f)
                    csv_writer.writerows(dbEntries)

    
            
