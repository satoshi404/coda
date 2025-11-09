#!/bin/bash

################################################################################################################

RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
MAGENTA="\033[35m"
CYAN="\033[36m"
BOLD="\033[1m"
RESET="\033[0m"

print() { printf '%b\n' "$1"; }

################################################################################################################

TOOLCHAIN="${CODA_TOOLCHAIN:-gcc}"
GLX="${CODA_GLX:-Opengl}"
WINE="${CODA_WINE:-Disable}"
TEST="${CODA_TEST:-Disable}"
BUILD_DIR="${CODA_BUILD_DIR:-build}"

BIN_DIR="$BUILD_DIR/bin"
LIB_DIR="$BUILD_DIR/lib"

################################################################################################################

detect_os() {
    case "$(uname -s)" in
        Linux*)     
            OS="Linux"
            EXE_EXT=""
            LIB_EXT=".so"
            ;;
        Darwin*)    
            OS="Mac"
            EXE_EXT=""
            LIB_EXT=".dylib"
            ;;
        MINGW*|MSYS*|CYGWIN*) 
            OS="Windows"
            EXE_EXT=".exe"
            LIB_EXT=".dll"
            ;;
        *)          
            OS="Unknown"
            EXE_EXT=""
            LIB_EXT=".so"
            ;;
    esac
}

################################################################################################################

setup_library_path() {
    if [ ! -d "$LIB_DIR" ]; then
        return
    fi
    
    local abs_lib_dir="$(cd "$LIB_DIR" 2>/dev/null && pwd)"
    
    if [ -z "$abs_lib_dir" ]; then
        return
    fi
    
    case "$OS" in
        Linux)
            export LD_LIBRARY_PATH="${abs_lib_dir}:${LD_LIBRARY_PATH}"
            print "${BLUE}Library path (LD_LIBRARY_PATH):${RESET}"
            print "  ${MAGENTA}$abs_lib_dir${RESET}"
            ;;
        Mac)
            export DYLD_LIBRARY_PATH="${abs_lib_dir}:${DYLD_LIBRARY_PATH}"
            print "${BLUE}Library path (DYLD_LIBRARY_PATH):${RESET}"
            print "  ${MAGENTA}$abs_lib_dir${RESET}"
            ;;
        Windows)
            export PATH="${abs_lib_dir}:${PATH}"
            print "${BLUE}Library path (PATH):${RESET}"
            print "  ${MAGENTA}$abs_lib_dir${RESET}"
            ;;
    esac
}

check_dependencies() {
    local executable="$1"
    
    case "$OS" in
        Linux)
            if command -v ldd &> /dev/null; then
                print
                print "${BLUE}Checking dependencies...${RESET}"
                
                if ldd "$executable" 2>&1 | grep -q "not found"; then
                    print "${YELLOW}Warning: Missing dependencies:${RESET}"
                    ldd "$executable" 2>&1 | grep "not found" | while read -r line; do
                        print "  ${RED}✗${RESET} $line"
                    done
                    return 1
                else
                    print "${GREEN}✓${RESET} All dependencies satisfied"
                fi
            fi
            ;;
        Mac)
            if command -v otool &> /dev/null; then
                print
                print "${BLUE}Dependencies:${RESET}"
                otool -L "$executable" | tail -n +2 | head -n 5 | while read -r line; do
                    local lib=$(echo "$line" | awk '{print $1}')
                    print "  ${GREEN}•${RESET} $(basename "$lib")"
                done
            fi
            ;;
    esac
    
    return 0
}

# ──────────────────────────────────────
#  List available libraries
# ──────────────────────────────────────
list_libraries() {
    if [ ! -d "$LIB_DIR" ]; then
        return
    fi
    
    local libs=$(find "$LIB_DIR" -type f \( -name "*${LIB_EXT}" \) 2>/dev/null)
    
    if [ -n "$libs" ]; then
        print
        print "${BLUE}Available libraries:${RESET}"
        echo "$libs" | while read -r lib; do
            local lib_name=$(basename "$lib")
            local lib_size=$(du -h "$lib" 2>/dev/null | cut -f1)
            print "  ${GREEN}•${RESET} $lib_name ${MAGENTA}($lib_size)${RESET}"
        done
    fi
}

# ──────────────────────────────────────
#  Run tests (if enabled)
# ──────────────────────────────────────
run_tests() {
    local test_exe="$BIN_DIR/coda_tests${EXE_EXT}"
    
    if [ ! -f "$test_exe" ]; then
        return
    fi
    
    print
    print "${CYAN}${BOLD}════════════════════════════════════════${RESET}"
    print "${CYAN}${BOLD}     Running Tests${RESET}"
    print "${CYAN}${BOLD}════════════════════════════════════════${RESET}"
    print
    
    chmod +x "$test_exe" 2>/dev/null || true
    
    if "$test_exe"; then
        print
        print "${GREEN}${BOLD}✓ All tests passed${RESET}"
    else
        local exit_code=$?
        print
        print "${RED}${BOLD}✗ Tests failed (exit code: $exit_code)${RESET}"
        exit $exit_code
    fi
}

# ──────────────────────────────────────
#  Main execution
# ──────────────────────────────────────
print
print "${CYAN}${BOLD}════════════════════════════════════════${RESET}"
print "${CYAN}${BOLD}     Running Coda Application${RESET}"
print "${CYAN}${BOLD}════════════════════════════════════════${RESET}"
print

detect_os

exe_name="coda${EXE_EXT}"
executable="$BIN_DIR/$exe_name"

# Check if executable exists
if [ ! -f "$executable" ]; then
    print "${RED}${BOLD}Error: Executable not found${RESET}"
    print "${YELLOW}Expected location:${RESET} $executable"
    print
    print "${YELLOW}Tip:${RESET} Run ${BOLD}./boot.sh --build${RESET} first to compile the project"
    exit 1
fi

# Make executable
chmod +x "$executable" 2>/dev/null || true

# Show configuration
print "${BLUE}Configuration:${RESET}"
print "  ${GREEN}OS:${RESET}         ${MAGENTA}$OS${RESET}"
print "  ${GREEN}Toolchain:${RESET}  ${MAGENTA}$TOOLCHAIN${RESET}"
print "  ${GREEN}Renderer:${RESET}   ${MAGENTA}$GLX${RESET}"
print "  ${GREEN}Wine:${RESET}  ${MAGENTA}$WINE${RESET}"
print "  ${GREEN}Tests:${RESET}      ${MAGENTA}$TEST${RESET}"

# Setup library paths if WINEdoWINEg is enabled
if [ "$WINE" = "Enable" ]; then
    print
    setup_library_path
    list_libraries
fi

# Check dependencies
check_dependencies "$executable"

# Run tests if enabled
if [ "$TEST" = "Enable" ]; then
    run_tests
fi

# Run main application
print
print "${GREEN}${BOLD}════════════════════════════════════════${RESET}"
print "${GREEN}${BOLD}  Starting application...${RESET}"
print "${GREEN}${BOLD}════════════════════════════════════════${RESET}"
print

"$executable" "$@"
exit_code=$?

print
if [ $exit_code -eq 0 ]; then
    print "${GREEN}${BOLD}✓ Application exited successfully${RESET}"
else
    print "${RED}${BOLD}✗ Application exited with code: $exit_code${RESET}"
fi
print

exit $exit_code

################################################################################################################
