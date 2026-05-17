import random
import sys

# Usage: python3 generator.py input.pbm <width> <height>

# 30% of density of alive cells.
def generate_pbm(file, width, height, density=0.3):
  with open(file, 'w') as f:
    f.write("P1\n")
    f.write(f"{width} {height}\n")
    for _ in range(height):
      row = ["1" if random.random() < density else "0" for _ in range(width)]
      f.write(" ".join(row) + "\n")

if __name__ == "__main__":
  generate_pbm(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))