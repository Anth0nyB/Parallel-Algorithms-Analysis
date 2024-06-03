import pandas as pd
import sys
import matplotlib.pyplot as plt

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(f"usage: python graphs.py <lib> <left y-axis bound> <right y-axis bound>")
        exit(-1)

    lib = sys.argv[1]
    yl_lim = float(sys.argv[2])
    yr_lim = float(sys.argv[3])
    binds_list = [
        "sockets_close",
        "threads_close",
        "cores_close",
        "_close",
        "sockets_spread",
        "threads_spread",
        "cores_spread",
        "_spread",
    ]
    
    events = ["CYCLE_ACTIVITY:CYCLES_L2_MISS", "CYCLE_ACTIVITY:STALLS_L2_MISS"]
    # events = ["CYCLE_ACTIVITY:CYCLES_L2_MISS", "CYCLE_ACTIVITY:STALLS_L2_MISS", "L2_RQSTS:MISS", "perf::PERF_COUNT_HW_CACHE_L1D:READ:MISS"]
    threads = [1, 4, 12, 20, 36, 48]
    threads = threads[1:]
    problem_sizes = ["50kx1k"]

    # Set up the figure with a small first row and col, for labels in the future
    heights = [1, 20, 20]
    widths = [1, 10, 10, 10, 10]
    # widths = [1, 10, 10]
    fig, axs = plt.subplots(
        len(heights),
        len(widths),
        figsize=(12, 8),
        gridspec_kw={"height_ratios": heights, "width_ratios": widths},
    )
    
    title = "OpenBLAS" if lib == "ob" else "MKL"
    fig.suptitle(f"{title} Hit Rates and Event Counts")
    # fig.suptitle(f"{title} Runtime and Event Counts")
    

    for idx, binds in enumerate(binds_list):
        # Read in the data
        data = pd.read_csv(f"data/dgeqrf/{lib}/{binds}_dgeqrf_{lib}.csv", sep=",")
        data = data.rename(columns={"Unnamed: 0": "Events"})
        data.set_index(data.columns[0], inplace=True)
        
        # Retrieve counts for the two specified events
        # counts = []
        # for event in events:
        #     count = data.loc[event, problem_sizes[0]]
        #     counts.append([int(x) for x in count.split(",")[1:]])
            

        
        # Calculate the hit rates for l1
        l1_read_hit = data.loc["perf::PERF_COUNT_HW_CACHE_L1D:READ:ACCESS", problem_sizes[0]].split(",")[1:]
        l1_read_hit = [int(x) for x in l1_read_hit]
        
        l1_read_miss = data.loc["perf::PERF_COUNT_HW_CACHE_L1D:READ:MISS", problem_sizes[0]].split(",")[1:]
        l1_read_miss = [int(x) for x in l1_read_miss]
        
        l1_read_rate = []
        for i in range(len(l1_read_hit)):
            l1_read_rate.append(l1_read_hit[i] / (l1_read_hit[i] + l1_read_miss[i]))

        # Calculate the hit rates for l2
        l2_miss = data.loc["L2_RQSTS:MISS", problem_sizes[0]].split(",")[1:]
        l2_miss = [int(x) for x in l2_miss]    # all requests that miss l2 cache
        
        l2_ref = data.loc["L2_RQSTS:REFERENCES", problem_sizes[0]].split(",")[1:]
        l2_ref = [int(x) for x in l2_ref]      # all requests to l2 cache
        
        l2_rate = []
        for i in range(len(l2_miss)):
            l2_rate.append((l2_ref[i] - l2_miss[i]) / l2_ref[i])

        counts = [l1_read_rate, l2_rate]
        events = ["l1 read hit rate", "l2 hit rate"]

        # Retrieve runtimes
        times = data.loc["Runtime", problem_sizes[0]]
        times = [float(x) for x in times.split(",")]
        times = times[1:]

        # graph the runtime using the left y-axis
        row = idx // (len(widths) - 1) + 1
        col = idx % (len(widths) - 1) + 1
        print(row, col)
        axs[row, col].plot(threads, times, color="black", label="Runtime")
        axs[row, col].set_xlabel("Threads")
        axs[row, col].set_ylabel("Runtime (s)")
        axs[row, col].set_ylim([0, yl_lim])
        axs[row, col].tick_params(axis="y", labelcolor="black")

        # graph the event counts using the right y-axis
        ax2 = axs[row, col].twinx()
        colors = ["tab:red", "tab:blue", "yellow", "green"]
        ax2.set_ylabel("Counts", color="black")
        ax2.set_ylim([0, yr_lim])
        for j, c in enumerate(counts):
            ax2.plot(threads, c, color=colors[j], label=f"{events[j]}")
        ax2.tick_params(axis="y", labelcolor="black")

        # for the legend
        handles_right, labels_right = ax2.get_legend_handles_labels()

    # Clearing the contents of the first row and column
    for ax in axs[0, :]:
        ax.clear()
        ax.axis("off")  # Hide axes
        for spine in ax.spines.values():  # Hide spines
            spine.set_visible(False)
    for ax in axs[1:]:
        ax[0].clear()
        ax[0].axis("off")  # Hide axes
        for spine in ax[0].spines.values():  # Hide spines
            spine.set_visible(False)

    # Put labels in the first row and col
    # cols = ["Sockets", "Cores", "Unspecified"]
    cols = ["Sockets", "Threads", "Cores", "Unspecified"]
    for i, ax in enumerate(axs[0, 1:]):
        ax.text(
            0.5,
            0.5,
            cols[i],
            horizontalalignment="center",
            verticalalignment="center",
        )
    rows = ["Close", "Spread"]
    for i, ax in enumerate(axs[1:]):
        ax[0].text(
            0.5,
            0.5,
            rows[i],
            horizontalalignment="center",
            verticalalignment="center",
            rotation=90,
        )

    # Adding legend
    handles = []
    labels = []
    handles_left, labels_left = axs[1, 1].get_legend_handles_labels()
    handles.extend(handles_left + handles_right)
    labels.extend(labels_left + labels_right)
    fig.legend(handles, labels, loc="upper left")

    # Adjusting layout
    fig.tight_layout()
    
    # Adjusting padding
    fig.subplots_adjust(hspace=0.4, wspace=0.4)

    # Show plot
    plt.show()
