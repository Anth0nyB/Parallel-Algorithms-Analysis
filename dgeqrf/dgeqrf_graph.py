import matplotlib.pyplot as plt

file_path = "dgeqrf.csv"

def fill_dictionary(my_dict, file):
    for line in file:        
        line_data = line.strip().split(",")
        line_data.pop()

        if len(line_data) != 4:
            return

        line_data[0] = float(line_data[0])
        line_data[1:] = [int(element) for element in line_data[1:]]
        my_dict.setdefault(f"{line_data[1]}x{line_data[2]}", []).append((line_data[3], line_data[0]))


def graph_results(points, title):
    fig, ax = plt.subplots()
    ax.set_xlabel("Threads")
    ax.set_ylabel("Execution Time (s)")

    for key, value in points.items():
        x, y = zip(*value)
        print(key, value)
        plt.plot(x, y, label=key)
        
    plt.title(f"DGEQRF with {title}")
    plt.legend()
    plt.show()

try: 
    ob_data = {}
    mkl_data = {}

    file = open(file_path, 'r')

    line = file.readline()
    line = file.readline()

    fill_dictionary(ob_data, file)
    graph_results(ob_data, "OpenBLAS")
    
    line = file.readline()

    fill_dictionary(mkl_data, file)
    graph_results(mkl_data, "MKL")  

except FileNotFoundError:
    print(f"Could not open {file_path}")


