#!/bin/bash

# Set the directory to the first argument or the current directory if none is provided
dir="${1:-.}"

# Navigate to the specified directory
cd "$dir" || exit

# ANSI color codes
GREEN='\033[0;32m'
GREEN_B='\033[0;30m\033[42m'
YELLOW='\033[0;33m'
YELLOW_B='\033[0;30m\033[43m'
RED='\033[0;31m'
RED_B='\033[0;30m\033[41m'
CYAN='\033[0;36m'
CYAN_B='\033[0;30m\033[46m'
GREY='\033[0;90m'
NC='\033[0m' # No Color

# Category configurations
declare -A categories
categories[SYN]="Kernel compilation failed to complete|SYN|Synthesis Failed|${RED}|${RED_B}|${NC}"
categories[SCG]="Unable to create system connectivity graph|SCG|System Connectivity Graph Error|${RED}|${RED_B}|${NC}"
categories[SBD]="An error has occurred during generation of the system block diagram|SBD|System Block Diagram Graph Error|${RED}|${RED_B}|${NC}"
categories[PPT]="_full_place_pre.tcl failed|PPT|Pre-place tcl script error|${RED}|${RED_B}|${NC}"
categories[CL7]="Design is not routable as its global congestion level is 7|CL7|Congestion Level 7"
categories[PCN]="Routing results verification failed due to partially-conflicted nets|PCN|Partially Conflicted Nets"
categories[TIM]="One or more unscalable system clocks did not meet their required target frequency|TIM|Unscalable clock failed to meet timing constraints"
categories[RES]="over-utilized in Pblock|RES|A resource was over-utilized in a pblock region|${RED}|${RED_B}|${NC}"
categories[BLT]="Step xclbinutil: Completed|BLT|Built Successfully|${GREEN}|${GREEN_B}|${GREY}"
categories[CAN]="CANCELLED AT|CAN|Job Cancelled|${YELLOW}|${YELLOW_B}|${GREY}"
categories[MEM]="Some of your processes may have been killed by the cgroup out-of-memory handler|MEM|Out-of-Memory|${YELLOW}|${YELLOW_B}|${GREY}"
categories[CRS]="Sorry, but it appears that a AMD program has terminated unexpectedly.|CRS|Crashed|${YELLOW}|${YELLOW_B}|${GREY}"
categories[UKN]="|UKN|Unknown|${CYAN}|${CYAN_B}|${GREY}"

# Function to get clock scaling info
get_clock_scaling_info() {
    local file=$1
    local scaling_info=""
    while IFS= read -r line; do
        if [[ $line =~ .*The\ (kernel\ clock|system\ clock)\ (.+)\ has\ an\ original\ frequency\ equal\ to\ ([0-9.]+)\ MHz.\ The\ frequency\ has\ been\ automatically\ changed\ to\ ([0-9.]+)\ MHz.* ]]; then
            scaling_info+="${NC} | ${BASH_REMATCH[2]}: ${YELLOW}${BASH_REMATCH[3]} MHz -> ${BASH_REMATCH[4]} MHz${NC}"
        fi
        if [[ $line =~ .*For\ clock\ (.+),\ the\ auto\ scaled\ frequency\ ([0-9.]+)\ MHz\ exceeds\ the\ original\ specified\ frequency.\ The\ compiler\ will\ select\ the\ original\ specified\ frequency\ of\ ([0-9.]+)\ MHz.* ]]; then
            scaling_info+="${NC} | ${BASH_REMATCH[1]}: ${GREEN}${BASH_REMATCH[3]} MHz${NC} < ${BASH_REMATCH[2]} MHz${NC}"
        fi
    done < "$file"
    echo "$scaling_info"
}

# Header for the output table
echo -e "File | Code | Category | Clock Scaling Info"

# Loop through all .out files
for file in *.out; do
    matched=false
    clock_info=""
    for key in "${!categories[@]}"; do
        IFS='|' read -r -a params <<< "${categories[$key]}"
        if [[ -n ${params[0]} ]] && grep -q "${params[0]}" "$file"; then
            if [[ "$key" == "BLT" ]]; then
                clock_info=$(get_clock_scaling_info "$file")
            fi
            echo -e "${params[3]}$file |${params[4]} ${params[1]} ${params[3]}|${params[5]} ${params[2]}${NC}$clock_info"
            matched=true
            break
        fi
    done
    if [ "$matched" = false ]; then
        IFS='|' read -r -a params <<< "${categories[UKN]}"
        echo -e "${params[3]}$file |${params[4]} ${params[1]} ${params[3]}|${params[5]} ${params[2]}${NC}"
    fi
done
