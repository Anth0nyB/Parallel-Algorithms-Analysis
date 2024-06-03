import sys
import pandas as pd
from os.path import join

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} <path to dir of csv> <name of csv>")
    exit(0)

root_dir = sys.argv[1]
job_name = sys.argv[2]

raw_file = join(root_dir, job_name)

raw = pd.read_csv(raw_file, index_col=False, usecols=lambda x: x.rstrip(","))
raw = raw.iloc[:, :-1]
raw.set_index(raw.columns[0], inplace=True)

""" formatted = {event1: {problem1: data[[n1_threads], [n2_threads], ...], problem2: data[]}, event2: ...} """

formatted = {}
problems = []

for index, row in raw.iterrows():
    # print(index)
    # print(row)

    problem = index.split(" ")[:-1]
    print(problem)

    if len(problem) == 3:
        threads, m, n = problem
        size = f"{m}x{n}"

    elif len(problem) == 4:
        threads, m, k, n = problem
        size = f"{m}x{k}x{n}"

    threads = int(threads)

    if size not in problems:
        problems.append(size)

    for event, value in row.items():
        if event not in formatted:
            formatted[event] = {}
        if size not in formatted[event]:
            formatted[event][size] = []

        if event == "Runtime":
            formatted[event][size].append(float(value))
        else:
            formatted[event][size].append(int(value))

# Removes the '_raw' suffix
out_path = join(root_dir, f"{job_name[:-8]}.csv")

with open(out_path, "w") as file:
    for problem in problems:
        file.write(f",{problem}")
    file.write("\n")

    for event in formatted.keys():
        file.write(event)
        for problem, data in formatted[event].items():
            file.write(",")
            file.write('"')
            for count in data[:-1]:
                file.write(f"{count},")
            file.write(str(data[-1]))
            file.write('"')
        file.write("\n")
