import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

INCLUDE_DIR = os.path.join(BASE_DIR, "include")
SRC_DIR = os.path.join(BASE_DIR, "src")
CMAKELISTS_PATH = os.path.join(BASE_DIR, "CMakeLists.txt")

REQUIRES = [
    "ais_framework"
]


def find_include_dirs(root):
    include_dirs = set()
    # Luôn thêm thư mục include gốc
    include_dirs.add(os.path.relpath(root, BASE_DIR).replace("\\", "/"))

    for dirpath, dirnames, filenames in os.walk(root):
        if any(fname.endswith((".h", ".hpp")) for fname in filenames):
            rel_path = os.path.relpath(dirpath, BASE_DIR).replace("\\", "/")
            include_dirs.add(rel_path)
    return sorted(include_dirs)


def find_static_libs(src_dir):
    return [
        f for f in os.listdir(src_dir)
        if f.endswith(".a") and os.path.isfile(os.path.join(src_dir, f))
    ]


def find_cpp_files(src_dir):
    return [
        f for f in os.listdir(src_dir)
        if f.endswith(".cpp") and os.path.isfile(os.path.join(src_dir, f))
    ]


def generate_cmakelists(include_dirs, static_libs, cpp_files):
    lines = []

    lines.append("idf_component_register(")
    lines.append("    SRCS")

    if cpp_files:
        for cpp in cpp_files:
            lines.append(f'        "src/{cpp}"')
    else:
        lines.append('        ""')  # Nếu không có source .cpp, tránh lỗi

    lines.append("    INCLUDE_DIRS")
    for inc in include_dirs:
        lines.append(f'        "{inc}"')

    lines.append("    REQUIRES")
    for r in REQUIRES:
        lines.append(f"        {r}")
    lines.append(")")

    if static_libs:
        lines.append("")
        lines.append("target_link_libraries(${COMPONENT_LIB} INTERFACE")
        for lib in static_libs:
            lines.append(f'    "${{CMAKE_CURRENT_LIST_DIR}}/src/{lib}"')
        lines.append(")")

    return "\n".join(lines)


def main():
    include_dirs = find_include_dirs(INCLUDE_DIR)
    static_libs = find_static_libs(SRC_DIR)
    cpp_files = find_cpp_files(SRC_DIR)

    cmake_content = generate_cmakelists(include_dirs, static_libs, cpp_files)

    with open(CMAKELISTS_PATH, "w") as f:
        f.write(cmake_content)

    print("✅ Generated CMakeLists.txt")


if __name__ == "__main__":
    main()
