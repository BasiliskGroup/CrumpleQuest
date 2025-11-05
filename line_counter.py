import os
import sys

def count_lines_in_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            return sum(1 for _ in f)
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0

def count_lines_in_directory(root_dir):
    total_lines = 0
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            line_count = count_lines_in_file(filepath)
            total_lines += line_count
            print(f"{filepath}: {line_count} lines")
            
    print("\n============================")
    print(f"Total lines in '{root_dir}': {total_lines}")
    print("============================")

if __name__ == "__main__":
    count_lines_in_directory(sys.argv[1])
