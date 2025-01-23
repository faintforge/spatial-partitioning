import matplotlib.pyplot as plt
import json
import collections
import sys

if len(sys.argv) != 3:
    print("Please pass in a benchmark JSON file and a file prefix.")
    print(f"{sys.argv[0]} [benchmark] [prefix]")
    exit(1)

with open(sys.argv[1], "rb") as f:
    data = json.load(f)

def graph(name, data_point, include_naive=False):
    fig, ax = plt.subplots()
    ax.set(xlabel="box count", ylabel="time (ms)")
    ax.grid()
    for strat in data:
        if not include_naive and strat == "naive":
            continue;

        print(f"{strat}: {sys.argv[2]}-{name}.png")
        run_times = {}
        for box_count in data[strat]["children"]:
            run_times[int(box_count)] = float(data_point(data[strat]["children"][box_count]))

        sorted_run_times = collections.OrderedDict(sorted(run_times.items()))
        ax.plot(list(sorted_run_times.keys()), list(sorted_run_times.values()), label=strat)
        plt.legend()

    fig.savefig(f"graphs/{sys.argv[2]}-{name}.png")

graph("average", lambda box: box["average"])
graph("insert", lambda box: box["children"]["insert"]["average"])
graph("collision", lambda box: box["children"]["collision"]["average"])
def query(box):
    children = box["children"]["collision"]["children"]
    if "query" in children:
        return children["query"]["average"]
    return 0.0
graph("query", query)

# graph("naive", lambda box: box["average"], True)
