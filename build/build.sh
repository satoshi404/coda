#!/bin/bash

# ══════════════════════════════════════════════════════════════
#  Build Script - Compila o projeto Coda
# ══════════════════════════════════════════════════════════════

set -e  # Exit on error

# ──────────────────────────────────────
#  Colors
# ──────────────────────────────────────
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
MAGENTA="\033[35m"
CYAN="\033[36m"
BOLD="\033[1m"
RESET="\033[0m"

print() { printf '%b\n' "$1"; }

# ──────────────────────────────────────
#  Configuration from environment
# ──────────────────────────────────────
TOOLCHAIN="${CODA_TOOLCHAIN:-gcc}"
GLX="${CODA_GLX:-Opengl}"
WIN="${CODA_WIN:-Disable}"
TEST="${CODA_TEST:-Disable}"
BUILD_DIR="${CODA_BUILD_DIR:-build}"

# ──────────────────────────────────────
#  Directories
# ──────────────────────────────────────
SRC_DIR="source"
OBJ_DIR="$BUILD_DIR/obj"
DEP_DIR="$BUILD_DIR/deps"
LIB_DIR="$BUILD_DIR/lib"
BIN_DIR="$BUILD_DIR/bin"

# ──────────────────────────────────────
#  Detect OS
# ──────────────────────────────────────
detect_os() {
    case "$(uname -s)" in
        Linux*)     OS="Linux";;
        Darwin*)    OS="Mac";;
        MINGW*|MSYS*|CYGWIN*) OS="Windows";;
        *)          OS="Unknown";;
    esac
}

# ──────────────────────────────────────
#  Setup compiler and flags
# ──────────────────────────────────────
setup_compiler() {
    case "$TOOLCHAIN" in
        gcc)
            CC="gcc"
            CXX="g++"
            CFLAGS="-Wall -Wextra -O2 -fPIC"
            CXXFLAGS="-Wall -Wextra -O2 -fPIC -std=c++17"
            ;;
        llvm|clang)
            CC="clang"
            CXX="clang++"
            CFLAGS="-Wall -Wextra -O2 -fPIC"
            CXXFLAGS="-Wall -Wextra -O2 -fPIC -std=c++17"
            ;;
        msvc)
            if [ "$OS" != "Windows" ]; then
                print "${RED}Error: MSVC only available on Windows${RESET}"
                exit 1
            fi
            CC="cl"
            CXX="cl"
            CFLAGS="/W4 /O2 /MD"
            CXXFLAGS="/W4 /O2 /MD /std:c++17"
            ;;
        *)
            print "${RED}Error: Unknown toolchain: $TOOLCHAIN${RESET}"
            exit 1
            ;;
    esac
    
    # Graphics API flags
    case "$GLX" in
        Opengl)
            CFLAGS="$CFLAGS -DUSE_OPENGL"
            CXXFLAGS="$CXXFLAGS -DUSE_OPENGL"
            LIBS="-lGL -lX11"
            ;;
        Vulkan)
            CFLAGS="$CFLAGS -DUSE_VULKAN"
            CXXFLAGS="$CXXFLAGS -DUSE_VULKAN"
            LIBS="-lvulkan -lX11"
            ;;
    esac
    
    # Window flags ( Run wine for run coda.exe in linux )
    if [ "$WIN" = "Enable" ]; then
        CFLAGS="$CFLAGS -DENABLE_WINDOWING"
        CXXFLAGS="$CXXFLAGS -DENABLE_WINDOWING"
    fi
    
    # Test flags
    if [ "$TEST" = "Enable" ]; then
        CFLAGS="$CFLAGS -DENABLE_TESTS -g"
        CXXFLAGS="$CXXFLAGS -DENABLE_TESTS -g"
    fi
    
    # OS-specific flags
    if [ "$OS" = "Linux" ]; then
        LIBS="$LIBS -ldl -lpthread -lm"
        SHARED_FLAG="-shared"
        LIB_EXT=".so"
        EXE_EXT=""
    elif [ "$OS" = "Windows" ]; then
        LIBS="$LIBS -lopengl32 -lgdi32"
        SHARED_FLAG="-shared"
        LIB_EXT=".dll"
        EXE_EXT=".exe"
    elif [ "$OS" = "Mac" ]; then
        LIBS="$LIBS -framework OpenGL -framework Cocoa"
        SHARED_FLAG="-dynamiclib"
        LIB_EXT=".dylib"
        EXE_EXT=""
    fi
}

# ──────────────────────────────────────
#  Compile source file
# ──────────────────────────────────────
compile_source() {
    local src_file="$1"
    local obj_file="$2"
    local dep_file="$3"
    
    local ext="${src_file##*.}"
    local compiler
    local flags
    
    if [ "$ext" = "c" ]; then
        compiler="$CC"
        flags="$CFLAGS"
    else
        compiler="$CXX"
        flags="$CXXFLAGS"
    fi
    
    # Generate dependency file
    if [ "$TOOLCHAIN" != "msvc" ]; then
        $compiler $flags -MMD -MP -MF "$dep_file" -c "$src_file" -I source -o "$obj_file"
    else
        # MSVC doesn't support -MMD
        $compiler $flags /c "$src_file" -I source /Fo:"$obj_file"
    fi
}

# ──────────────────────────────────────
#  Main build process
# ──────────────────────────────────────
print
print "${CYAN}${BOLD}════════════════════════════════════════${RESET}"
print "${CYAN}${BOLD}     Coda Build System${RESET}"
print "${CYAN}${BOLD}════════════════════════════════════════${RESET}"
print

detect_os
setup_compiler

print "${BLUE}OS:${RESET} ${MAGENTA}$OS${RESET} | ${BLUE}Toolchain:${RESET} ${MAGENTA}$TOOLCHAIN${RESET} | ${BLUE}Renderer:${RESET} ${MAGENTA}$GLX${RESET}"
print

# Create directories
print "${BLUE}Creating build directories...${RESET}"
mkdir -p "$OBJ_DIR" "$DEP_DIR" "$LIB_DIR" "$BIN_DIR"

# Check if source directory exists
if [ ! -d "$SRC_DIR" ]; then
    print "${RED}Error: Source directory not found: $SRC_DIR${RESET}"
    exit 1
fi

# Compile all sources
print "${BLUE}${BOLD}Compiling source files...${RESET}"

count=0
total=$(find "$SRC_DIR" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.cc" \) 2>/dev/null | wc -l)

if [ "$total" -eq 0 ]; then
    print "${YELLOW}Warning: No source files found in $SRC_DIR${RESET}"
    exit 0
fi

while IFS= read -r src_file; do
    count=$((count + 1))
    
    # Get relative path
    rel_path="${src_file#$SRC_DIR/}"
    obj_file="$OBJ_DIR/${rel_path%.*}.o"
    dep_file="$DEP_DIR/${rel_path%.*}.d"
    
    # Create subdirectories
    mkdir -p "$(dirname "$obj_file")"
    mkdir -p "$(dirname "$dep_file")"
    
    # Check if recompilation is needed
    if [ -f "$obj_file" ] && [ "$obj_file" -nt "$src_file" ]; then
        print "${GREEN}[$count/$total]${RESET} ${CYAN}Cached${RESET} $rel_path"
        continue
    fi
    
    print "${GREEN}[$count/$total]${RESET} ${YELLOW}Compile${RESET} $rel_path"
    
    if ! compile_source "$src_file" "$obj_file" "$dep_file"; then
        print "${RED}${BOLD}✗ Compilation failed: $src_file${RESET}"
        exit 1
    fi
done < <(find "$SRC_DIR" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.cc" \))

print "${GREEN}${BOLD}✓ All sources compiled${RESET}"

# Link
print
print "${BLUE}${BOLD}Linking...${RESET}"

obj_files=$(find "$OBJ_DIR" -type f -name "*.o" 2>/dev/null)

if [ -z "$obj_files" ]; then
    print "${YELLOW}Warning: No object files found${RESET}"
    exit 0
fi

# Link shared library (if windowing enabled)
if [ "$WIN" = "Enable" ]; then
    lib_objs=$(echo "$obj_files" | grep -v "main.o" || true)
    
    if [ -n "$lib_objs" ]; then
        output="$LIB_DIR/coda${LIB_EXT}"
        print "${MAGENTA}→${RESET} Library: $output"
        
        if [ "$TOOLCHAIN" = "msvc" ]; then
            link /DLL /OUT:"$output" $lib_objs $LIBS
        else
            $CXX $SHARED_FLAG -o "$output" $lib_objs $LIBS
        fi
        
        print "${GREEN}✓${RESET} Library created"
    fi
fi

# Link executable
output="$BIN_DIR/coda${EXE_EXT}"
print "${MAGENTA}→${RESET} Executable: $output"

if [ "$TOOLCHAIN" = "msvc" ]; then
    link /OUT:"$output" $obj_files $LIBS
else
    $CXX -o "$output" $obj_files $LIBS
fi

print "${GREEN}✓${RESET} Executable created"

print
print "${GREEN}${BOLD}════════════════════════════════════════${RESET}"
print "${GREEN}${BOLD}  Build completed successfully! 🎉${RESET}"
print "${GREEN}${BOLD}════════════════════════════════════════${RESET}"
print

# Show output summary
print "${BLUE}Output:${RESET}"
print "  ${GREEN}Executable:${RESET} $BIN_DIR/coda${EXE_EXT}"
if [ "$WIN" = "Enable" ]; then
    print "  ${GREEN}Library:${RESET}    $LIB_DIR/coda${LIB_EXT}"
fi
print