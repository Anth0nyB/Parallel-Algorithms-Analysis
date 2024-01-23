in_file_path = "../native_avail_output.txt"
out_file_path = "../all_events.txt"

try:
    in_file = open(in_file_path, 'r')
except FileNotFoundError:
    print(f"Could not open {in_file_path}")

try:
    out_file = open(out_file_path, 'w')
except FileNotFoundError:
    print(f"Could not open {out_file_path}")



for i in range(24):
    in_file.readline()


for line in in_file:
    if line[0] == '-' or line[0]== '=':
        line = in_file.readline().strip().strip("|").strip()
        out_file.write(line)
        out_file.write("\n")
