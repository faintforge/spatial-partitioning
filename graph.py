import matplotlib.pyplot as plt
import json

with open("quadtree.json", "rb") as f:
    data = json.loads(f.read())

print(json.dumps(data, indent=4))

x = []
average_y = []
min_y = []
max_y = []
for benchmark in data:
    x.append(benchmark["box_count"])
    average_y.append(benchmark["average"])
    min_y.append(benchmark["min"])
    max_y.append(benchmark["max"])

fig, ax = plt.subplots()
ax.plot(x, average_y, label="average")
ax.plot(x, min_y, label="min")
ax.plot(x, max_y, label="max")
plt.legend()

ax.set(xlabel = "Box count", ylabel="time (ms)")
ax.grid()

fig.savefig("test.png")
plt.show()
