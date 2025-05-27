#!/usr/bin/env python
import matplotlib.pyplot as plt
import sys

title = f"{sys.argv[1]} csúcs, {sys.argv[2]} él, {sys.argv[3]} terminál"
filename = f"{sys.argv[1]}_{sys.argv[2]}_{sys.argv[3]}"
columns = 9
x = []
y = [[] for i in range(columns + 1)]
x, y1, y2, y3, y4, y5 = [], [], [], [], [], []
with open("inputs/" + filename + ".txt", "r") as file:
    for line in file:
        if line[0] == "#":
            continue
        values = line.strip().split()
        x.append(float(values[0]))
        for i in range(1, columns + 1):
            y[i].append(float(values[i]))

# results
# plt.figure()
plt.plot(x, y[1], label="1. alg")
plt.plot(x, y[4], label="2. alg, 1000 futás átlaga")
plt.plot(x, y[5], label="2. alg, 1000 futás, legjobb 10")
plt.plot(x, y[6], label="Alsó korlát")
plt.xlabel("R/S")
plt.ylabel("Eredmény")
plt.title(title)
plt.legend()
plt.grid(True)
plt.savefig("plots/" + filename + ".png")

# algorithm 2 phase 2 and 3
# plt.figure()
plt.plot(x, y[2], label="2. alg 2. fázisa, 1000 futás átlaga")
plt.plot(x, y[3], label="2. alg 3. fázisa, 1000 futás átlaga")
plt.xlabel("R/S")
plt.ylabel("Eredmény")
plt.title(title)
plt.legend()
plt.grid(True)
plt.savefig("plots/" + filename + ".png")

# approximation ratios
y16 = [i / j for i, j in zip(y[1], y[6])]
y46 = [i / j for i, j in zip(y[4], y[6])]
y56 = [i / j for i, j in zip(y[5], y[6])]
plt.figure()
plt.plot(x, y16, label="1. alg")
plt.plot(x, y46, label="2. alg, 1000 futás átlaga")
plt.plot(x, y56, label="2. alg, 1000 futás, legjobb 10")
plt.xlabel("R/S")
plt.ylabel("Közelítési arány")
plt.title(title)
plt.legend()
plt.grid(True)
plt.savefig("plots/" + filename + ".png")
