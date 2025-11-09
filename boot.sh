#!/bin/bash

################################################################################################################

CODA_VERSION="0.1 (BETA)"
CODA_DATE="09/11/2025"
CODA_AUTHOR="Satoshi"

################################################################################################################

RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
MAGENTA="\033[35m"
CYAN="\033[36m"
BOLD="\033[1m"
RESET="\033[0m"

################################################################################################################

print() {
    printf '%b\n' "$1"
}

################################################################################################################

help() {
    print
    print "${CYAN}${BOLD}=========================================================${RESET}"
    print "${CYAN}${BOLD}                    Linux boot:${RESET}"
    print "${CYAN}${BOLD}=========================================================${RESET}"
    print
    print "${YELLOW} ---- CODA ENGINE ---- ${RESET}"
    print "${RED}Vesion: ${RESET} ${CODA_VERSION}"
    print "${RED}Date:   ${RESET} ${CODA_DATE}"
    print "${RED}Author: ${RESET} ${CODA_AUTHOR}"
    print
    print "${YELLOW}Usage:${RESET}"
    print "  ${BOLD}./boot.sh [OPTIONS] COMMAND${RESET}"
    print
    print "${YELLOW}Options:${RESET}"
    print
    print "  ${GREEN}toolchain=VALUE${RESET}: Set toolchain (default: ${MAGENTA}gcc${RESET}) [ ${MAGENTA}gcc${RESET}, ${MAGENTA}msvc${RESET}, ${MAGENTA}llvm${RESET} ]"
    print "  ${GREEN}glx=VALUE${RESET}:       Set renderer (default: ${MAGENTA}Opengl${RESET}) [ ${MAGENTA}Opengl${RESET}, ${MAGENTA}Vulkan${RESET} ]"
    print "  ${GREEN}wine=VALUE${RESET}:      Run Coda in wine (default: ${MAGENTA}Disable${RESET}) [ ${MAGENTA}Enable${RESET}, ${MAGENTA}Disable${RESET} ]"
    print "  ${GREEN}test=VALUE${RESET}:      Code tests (default: ${MAGENTA}Disable${RESET}) [ ${MAGENTA}Enable${RESET}, ${MAGENTA}Disable${RESET} ]"
    print
    print "${YELLOW}Commands:${RESET}"
    print
    print "  ${GREEN}--run${RESET}    Run coda without compiler"
    print "  ${GREEN}--build${RESET}  Compile coda and run"
    print "  ${GREEN}--info${RESET}   Get config information"
    print
    print "${YELLOW}Examples:${RESET}"
    print "  ${BOLD}./boot.sh --build${RESET}"
    print "  ${BOLD}./boot.sh toolchain=gcc glx=Vulkan --build${RESET}"
    print "  ${BOLD}./boot.sh test=Enable --run${RESET}"
    print
}

################################################################################################################

TOOLCHAIN="gcc"
GLX="Opengl"
WINE="Disable"
TEST="Disable"
COMMAND=""
BUILD_DIR="build"

################################################################################################################

info() {
    print 
    print "${BLUE}${BOLD}Configuration:${RESET}"
    print "  ${GREEN}Toolchain (toolchain):${RESET}  ${MAGENTA}${TOOLCHAIN}${RESET}"
    print "  ${GREEN}Renderer (glx):${RESET}         ${MAGENTA}${GLX}${RESET}"
    print "  ${GREEN}Wine (wine):${RESET}        ${MAGENTA}${WINE}${RESET}"
    print "  ${GREEN}Tests (test):${RESET}           ${MAGENTA}${TEST}${RESET}"
    print "  ${GREEN}Build directory:${RESET}        ${MAGENTA}${BUILD_DIR}${RESET}"
    print
}

################################################################################################################

generate_info_json() {
    mkdir -p "$BUILD_DIR"
    
    cat > "$BUILD_DIR/info.json" <<EOF
{
  "toolchain": "$TOOLCHAIN",
  "renderer": "$GLX",
  "wine": "$WINE",
  "tests": "$TEST",
  "timestamp": "$(date -u +"%Y-%m-%dT%H:%M:%SZ")",
  "build_dir": "$BUILD_DIR"
}
EOF
    
    print "${GREEN}✓${RESET} Generated ${BOLD}$BUILD_DIR/info.json${RESET}"
}

################################################################################################################

run_coda() {
    print
    print "${BLUE}${BOLD}Running Coda...${RESET}"
    print
    
    if [ ! -f "build/run.sh" ]; then
        print "${RED}Error: run.sh not found!${RESET}"
        exit 1
    fi
    
    if [ ! -x "build/run.sh" ]; then
        chmod +x run.sh
    fi
    
    # Pass configuration as environment variables
    export CODA_TOOLCHAIN="$TOOLCHAIN"
    export CODA_GLX="$GLX"
    export CODA_WINE="$WINE"
    export CODA_TEST="$TEST"
    
    ./build/run.sh
}

################################################################################################################

clean_coda() 
{
    print 
    print "${BLUE}${BOLD}Cleaning coda...${RESET}"
    print

    # Test clean
    rm build/obj/* -rf
    rm build/deps/* -rf
    rm build/bin/* -rf
    rm build/*.json

    print "${GREEN}${BOLD}✓ Clean completed!${RESET}"
}

################################################################################################################

build_coda() {
    print
    print "${BLUE}${BOLD}Building Coda...${RESET}"
    print
    
    if [ ! -f "build/build.sh" ]; then
        print "${RED}Error: build.sh not found!${RESET}"
        exit 1
    fi
    
    if [ ! -x "build/build.sh" ]; then
        chmod +x build/build.sh
    fi
    
    # Generate info.json before building
    generate_info_json
    
    # Pass configuration as environment variables
    export CODA_TOOLCHAIN="$TOOLCHAIN"
    export CODA_GLX="$GLX"
    export CODA_WINE="$WINE"
    export CODA_TEST="$TEST"
    export CODA_BUILD_DIR="$BUILD_DIR"
    
    # Execute build script
    if ./build/build.sh; then
        print
        print "${GREEN}${BOLD}✓ Build completed successfully!${RESET}"
        print
        
        # Ask if user wants to run after build
        read -p "$(printf "${YELLOW}Run Coda now? [Y/n]:${RESET} ")" -n 1 -r
        print
        if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
            run_coda
        fi
    else
        print
        print "${RED}${BOLD}✗ Build failed!${RESET}"
        exit 1
    fi
}

################################################################################################################

for arg in "$@"; do
    case "$arg" in
        toolchain=*)
            VALUE="${arg#*=}"
            case "$VALUE" in
                gcc|msvc|llvm) TOOLCHAIN="$VALUE" ;;
                *) print "${RED}Error: Invalid toolchain value → ${VALUE}${RESET}"; help; exit 1 ;;
            esac
            ;;
        glx=*)
            VALUE="${arg#*=}"
            case "$VALUE" in
                Opengl|Vulkan) GLX="$VALUE" ;;
                *) print "${RED}Error: Invalid glx value → ${VALUE}${RESET}"; help; exit 1 ;;
            esac
            ;;
        wine=*)
            VALUE="${arg#*=}"
            case "$VALUE" in
                Enable|Disable) WINE="$VALUE" ;;
                *) print "${RED}Error: Invalid WINE value → ${VALUE}${RESET}"; help; exit 1 ;;
            esac
            ;;
        test=*)
            VALUE="${arg#*=}"
            case "$VALUE" in
                Enable|Disable) TEST="$VALUE" ;;
                *) print "${RED}Error: Invalid test value → ${VALUE}${RESET}"; help; exit 1 ;;
            esac
            ;;
        -i|--info)
            COMMAND="info"
            ;;
        -r|--run)
            COMMAND="run"
            ;;
        -b|--build)
            COMMAND="build"
            ;;
        -c|--clean)
            COMMAND="clean"
            ;;
      
        -h|--help)
            help
            exit 0
            ;;
        *)
            print "${RED}Error: Unknown argument → ${arg}${RESET}"
            help
            exit 1
            ;;
    esac
done

################################################################################################################

if [ $# -eq 0 ]; then
    help
    exit 0
fi

case "$COMMAND" in
    info)
        info
        ;;
    run)
        generate_info_json
        run_coda
        ;;
    build)
        info
        build_coda
        ;;
    clean)
        clean_coda
        ;;
  
    "")
        print "${RED}Error: No command specified${RESET}"
        print "${YELLOW}Use --run, --build, or --info${RESET}"
        print
        help
        exit 1
        ;;
esac

################################################################################################################
