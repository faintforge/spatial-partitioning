import matplotlib.pyplot as plt
import json

class Data:
    def __init__(self, name, file) -> None:
        self.file = file
        self.name = name
        with open(file) as fp:
            self.data = json.load(fp)

class Graph:
    def __init__(self) -> None:
        self.fig, self.ax = plt.subplots()
        self.ax.set(xlabel="box count", ylabel="time (ms)")
        self.ax.grid()

    def plot_data(self, data: Data) -> None:
        x = []
        y = []
        for benchmark in data.data:
            x.append(benchmark["box_count"])
            y.append(benchmark["average"])
        self.ax.plot(x, y, label=data.name);
        plt.legend()

    def save(self, filepath: str):
        self.fig.savefig(filepath)

data = [
    Data("naive", "naive.json"),
    Data("quadtree", "quadtree.json")
]

graph = Graph()
for d in data:
    graph.plot_data(d)
graph.save("graph.png")
