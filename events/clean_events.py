all_events_file_path = "all_events.txt"
invalid_events_file_path = "failed_events.txt"
out_file_path = "native_events.txt"

try:
    all_file = open(all_events_file_path, 'r')
except FileNotFoundError:
    print(f"Could not open {all_events_file_path}")

try:
    invalid_file = open(invalid_events_file_path, 'r')
except FileNotFoundError:
    print(f"Could not open {invalid_events_file_path}")

try:
    out_file = open(out_file_path, 'w')
except FileNotFoundError:
    print(f"Could not open {out_file_path}")


invalid_events = []

for line in invalid_file:
    invalid_events.append(line)

for line in all_file:
    if line not in invalid_events:
        out_file.write(line)

