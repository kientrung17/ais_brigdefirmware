import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

INCLUDE_DIR = os.path.join(BASE_DIR, "include")
SRC_DIR = os.path.join(BASE_DIR, "src")
CMAKELISTS_PATH = os.path.join(BASE_DIR, "CMakeLists.txt")

REQUIRES = [
    "driver",
    "esp_event",
    "nvs_flash",
    "ais_commonlib",
    "esp_wifi",
    "esp_adc"
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
    """Tìm tất cả file .a trong src/ và thư mục con (nếu có)."""
    libs = []
    for dirpath, dirnames, filenames in os.walk(src_dir):
        for f in filenames:
            if f.endswith(".a"):
                full_path = os.path.join(dirpath, f)
                rel_path = os.path.relpath(full_path, BASE_DIR).replace("\\", "/")
                libs.append(rel_path)
    return sorted(libs)


def find_cpp_files(src_dir):
    """Tìm tất cả file .cpp trong src/ và thư mục con, trả về path tương đối từ BASE_DIR."""
    files = []
    for dirpath, dirnames, filenames in os.walk(src_dir):
        for f in filenames:
            if f.endswith(".cpp"):
                full_path = os.path.join(dirpath, f)
                rel_path = os.path.relpath(full_path, BASE_DIR).replace("\\", "/")
                files.append(rel_path)
    return sorted(files)


def generate_cmakelists(include_dirs, static_libs, cpp_files):
    lines = []

    lines.append("idf_component_register(")
    lines.append("    SRCS")
    if cpp_files:
        for cpp in cpp_files:
            # cpp đã là đường dẫn 'src/xxx/yyy.cpp' tương đối từ BASE_DIR
            lines.append(f'        "{cpp}"')
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
            # lib là đường dẫn 'src/xxx/libsomething.a' tương đối từ BASE_DIR
            lines.append(f'    "${{CMAKE_CURRENT_LIST_DIR}}/{lib}"')
        lines.append(")")

    return "\n".join(lines)


def main():
    include_dirs = find_include_dirs(INCLUDE_DIR) if os.path.isdir(INCLUDE_DIR) else []
    static_libs = find_static_libs(SRC_DIR) if os.path.isdir(SRC_DIR) else []
    cpp_files = find_cpp_files(SRC_DIR) if os.path.isdir(SRC_DIR) else []

    cmake_content = generate_cmakelists(include_dirs, static_libs, cpp_files)

    with open(CMAKELISTS_PATH, "w", encoding="utf-8") as f:
        f.write(cmake_content)

    print("✅ Generated CMakeLists.txt")


if __name__ == "__main__":
    main()
