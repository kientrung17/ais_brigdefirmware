#!/usr/bin/env python3
# generate_cmakelists.py — ghi đường dẫn TƯƠNG ĐỐI (không ghi ổ đĩa)

import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(BASE_DIR, "src")
INCLUDE_ROOT = os.path.join(BASE_DIR, "include")
OUTPUT_FILE = os.path.join(BASE_DIR, "CMakeLists.txt")

REQUIRES = [
    # thêm component phụ thuộc nếu cần, ví dụ: "json",
]

SOURCE_EXTS = (".c", ".cc", ".cxx", ".cpp")

def fslash(p: str) -> str:
    return p.replace("\\", "/")

def rel_to_base(p: str) -> str:
    """Đưa path về tương đối so với BASE_DIR và dùng forward slash."""
    return fslash(os.path.relpath(p, BASE_DIR))

def collect_sources(src_dir):
    sources = []
    if not os.path.isdir(src_dir):
        return sources
    for root, _, files in os.walk(src_dir):
        for f in files:
            if f.lower().endswith(SOURCE_EXTS):
                full = os.path.join(root, f)
                sources.append(rel_to_base(full))
    return sorted(sources)

def collect_include_dirs(include_root):
    dirs = set()
    if not os.path.isdir(include_root):
        return []
    for root, subdirs, files in os.walk(include_root):
        if files or subdirs:
            dirs.add(rel_to_base(root))
    return sorted(dirs)

def write_cmakelists(sources, include_dirs, output_path, requires):
    with open(output_path, "w", encoding="utf-8") as fp:
        fp.write("idf_component_register(\n")
        fp.write("    SRCS\n")
        for s in sources:
            fp.write(f'        "{s}"\n')

        fp.write("    INCLUDE_DIRS\n")
        if include_dirs:
            for inc in include_dirs:
                fp.write(f'        "{inc}"\n')
        else:
            # vẫn add include/ nếu không có thư mục con
            if os.path.isdir(INCLUDE_ROOT):
                fp.write('        "include"\n')

        if requires:
            fp.write("    REQUIRES\n")
            for r in requires:
                fp.write(f"        {r}\n")
        fp.write(")\n")

def main():
    sources = collect_sources(SRC_DIR)
    include_dirs = collect_include_dirs(INCLUDE_ROOT)
    if not sources:
        print("[Warn] Không tìm thấy file nguồn trong src/ (.c/.cpp).")
    write_cmakelists(sources, include_dirs, OUTPUT_FILE, REQUIRES)
    print(f"[OK] Tạo '{OUTPUT_FILE}' với {len(sources)} source và {len(include_dirs)} include dirs (đều là đường dẫn tương đối).")

if __name__ == "__main__":
    main()
