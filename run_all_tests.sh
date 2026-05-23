#!/bin/bash

# Cores para o output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

mkdir -p results

echo -e "${BLUE}Running Tomasulo Simulator Tests...${NC}"
echo "--------------------------------------------------"

# Run default instructions
echo -e "${GREEN}Testing instructions.txt (default)...${NC}"
yes "" | timeout 30s ./tomasulo instructions.txt > "results/result_instructions.log" 2>&1
echo "Result saved to results/result_instructions.log"
echo "--------------------------------------------------"

# Run all files in tests directory
for test_file in tests/*.txt; do
    base_name=$(basename "$test_file" .txt)
    echo -e "${GREEN}Testing $test_file...${NC}"
    yes "" | timeout 30s ./tomasulo "$test_file" > "results/result_${base_name}.log" 2>&1
    echo "Result saved to results/result_${base_name}.log"
    echo "--------------------------------------------------"
done

echo -e "${BLUE}All tests completed. Results are in the 'results' folder.${NC}"
