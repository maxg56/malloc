#!/bin/bash

# Script pour tester l'overhead de pages du malloc custom vs système

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Page Overhead Testing Script ===${NC}"
echo "Page size: $(getconf PAGESIZE) bytes"

# Build si nécessaire
if [ ! -f "libft_malloc.so" ]; then
    echo -e "${YELLOW}Building library...${NC}"
    export HOSTTYPE=Testing
    make re > /dev/null 2>&1
    ln -sf libft_malloc_Testing.so libft_malloc.so
fi

# Résultats
results_file="page_overhead_results.csv"
echo "test_name,system_faults,custom_faults,overhead_pages,score" > "$results_file"

total_tests=0
passed_tests=0
total_overhead=0

# Fonction de test simplifiée
test_overhead() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    
    echo -e "${BLUE}Testing $test_name...${NC}"
    
    # Compile les deux versions
    gcc -o "${test_name}_sys" "$test_file" 2>/dev/null
    gcc -o "${test_name}_custom" "$test_file" -L. -lft_malloc 2>/dev/null
    
    # Run system version
    local sys_output=$(/usr/bin/time -v "./${test_name}_sys" 2>&1)
    local sys_faults=$(echo "$sys_output" | grep "Minor.*page faults:" | sed 's/.*: //')
    
    # Run custom version  
    local custom_output=$(./run_linux.sh /usr/bin/time -v "./${test_name}_custom" 2>&1)
    local custom_faults=$(echo "$custom_output" | grep "Minor.*page faults:" | sed 's/.*: //')
    
    # Clean up binaries
    rm -f "${test_name}_sys" "${test_name}_custom"
    
    if [ -n "$sys_faults" ] && [ -n "$custom_faults" ]; then
        local overhead=$((custom_faults - sys_faults))
        local score=0
        
        if [ $overhead -le 20 ] && [ $overhead -ge 0 ]; then
            score=5
            echo -e "${GREEN}✓ $test_name: $sys_faults → $custom_faults (+$overhead pages) [Score: 5/5]${NC}"
            passed_tests=$((passed_tests + 1))
        else
            echo -e "${RED}✗ $test_name: $sys_faults → $custom_faults (+$overhead pages) [Score: 0/5]${NC}"
        fi
        
        echo "$test_name,$sys_faults,$custom_faults,$overhead,$score" >> "$results_file"
        total_overhead=$((total_overhead + overhead))
        total_tests=$((total_tests + 1))
    else
        echo -e "${YELLOW}Could not extract page faults for $test_name${NC}"
    fi
    echo
}

# Tester les fichiers disponibles
for test_file in test/test0.c test/test1.c test/test2.c test/test3.c  test/test5.c ; do
    [ -f "$test_file" ] && test_overhead "$test_file"
done

# Résumé
echo -e "${BLUE}=== SUMMARY ===${NC}"
echo "Tests run: $total_tests"
echo -e "Tests passed (≤20 pages): ${GREEN}$passed_tests${NC}" 
echo -e "Tests failed (>20 pages): ${RED}$((total_tests - passed_tests))${NC}"

if [ $total_tests -gt 0 ]; then
    avg_overhead=$((total_overhead / total_tests))
    echo -e "Average overhead: ${YELLOW}$avg_overhead pages${NC}"
    
    if [ $total_tests -eq $passed_tests ]; then
        echo -e "${GREEN}Overall Score: 5/5${NC}"
    elif [ $passed_tests -gt 0 ]; then
        echo -e "${YELLOW}Overall Score: Partial ($passed_tests/$total_tests)${NC}"
    else
        echo -e "${RED}Overall Score: 0/5${NC}"
    fi
fi

echo -e "Results: ${YELLOW}$results_file${NC}"