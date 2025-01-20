#!/bin/bash

# Compile and run iverilog tests
echo "Running iverilog tests..."
iverilog -g2012 -o shift_tb rtl/shift.sv rtl/shift_tb.sv
./shift_tb | grep "^disks = " > iverilog_results.log

echo -e "\n=== Test Cases ===\n"
echo "Format: disks = BOARD_STATE, dir = DIRECTION, result = SHIFTED_BOARD"
echo "----------------------------------------"

# Clear C++ results file
> test_results.log

# Run C++ tests with random inputs from iverilog results
test_count=1
matches=0
while IFS= read -r line; do
    if [[ $line =~ ^disks[[:space:]]*=[[:space:]]*([0-9a-fA-F]+),[[:space:]]*dir[[:space:]]*=[[:space:]]*([0-9]+) ]]; then
        disks="${BASH_REMATCH[1]}"
        dir="${BASH_REMATCH[2]}"
        echo "Test #$test_count:"
        echo "Verilog: $line"
        
        # Run C++ test and capture its output
        ./othello --test shift "0x$disks" "$dir" > /dev/null
        cpp_result=$(cat test_results.log)  # Get the entire file content
        echo "C++:     $cpp_result"
        
        # Compare the results for this test
        if [ "$line" == "$cpp_result" ]; then
            echo "Result:  ✅ Match"
            ((matches++))
        else
            echo "Result:  ❌ Differ"
            echo "Details:"
            echo "  Input board: 0x$disks"
            echo "  Direction:   $dir"
            if [[ $line =~ result[[:space:]]*=[[:space:]]*([0-9a-fA-F]+) ]]; then
                v_result="${BASH_REMATCH[1]}"
                echo "  Verilog output: $v_result"
            fi
            if [[ $cpp_result =~ result[[:space:]]*=[[:space:]]*([0-9a-fA-F]+) ]]; then
                c_result="${BASH_REMATCH[1]}"
                echo "  C++ output:     $c_result"
            fi
        fi
        echo "----------------------------------------"
        ((test_count++))
        
        # Clear the results file for next test
        > test_results.log
    fi
done < iverilog_results.log

# Final summary
echo -e "\nFinal Results:"
total_tests=$((test_count-1))
if [ $total_tests -eq 0 ]; then
    echo "❌ No tests were run!"
    exit 1
fi

if [ $matches -eq $total_tests ]; then
    echo "✅ All $total_tests tests match!"
else
    echo "❌ Tests differ"
    echo "   $matches out of $total_tests tests passed"
fi

# Cleanup
rm -f shift_tb iverilog_results.log 