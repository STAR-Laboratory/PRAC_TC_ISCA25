import sys

input_file = sys.argv[1]
output_file = sys.argv[2]

def get_addr_from_row(row, bank=17):
    addr = row
    addr <<= 12
    addr |= bank
    addr <<= 6
    return addr

with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
    for line in infile:
        parts = line.strip().split()
        first_number = parts[0]
        second_number = int(parts[1])
        second_number += 32
        outfile.write(f"{first_number} {second_number}\n")
    for row in range(1 << 16):
        addr = get_addr_from_row(row)
        outfile.write(f"127 {addr}\n")
