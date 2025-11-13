import os
import sys

def count_lines(file_path):
    """Count lines in a file."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            return sum(1 for _ in f)
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return 0

def count_lines_in_directory(directory):
    """Recursively count lines in all files in directory."""
    total_lines = 0
    file_count = 0
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            lines = count_lines(file_path)
            if lines > 0:
                print(f"{file_path}: {lines} lines")
                total_lines += lines
                file_count += 1
    
    return total_lines, file_count

if __name__ == "__main__":
    # Use current directory if no argument provided
    directory = sys.argv[1] if len(sys.argv) > 1 else "."
    
    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a valid directory")
        sys.exit(1)
    
    print(f"Counting lines in: {os.path.abspath(directory)}\n")
    total, count = count_lines_in_directory(directory)
    print(f"\nTotal: {total} lines in {count} files")