#!/bin/bash

# Integration test script for price_map

echo "=== price_map Integration Test ==="
echo ""

# Compile and run basic functionality test
echo "1. Testing basic functionality..."
g++ -std=c++14 -I. -Wall -Wextra price_map_test.cpp -o price_map_test
if [ $? -eq 0 ]; then
    echo "   ✓ Compilation successful"
    ./price_map_test > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "   ✓ All tests passed"
    else
        echo "   ✗ Tests failed"
        exit 1
    fi
else
    echo "   ✗ Compilation failed"
    exit 1
fi

# Compile and run usage examples
echo "2. Testing usage examples..."
g++ -std=c++14 -I. price_map_example.cpp -o price_map_example
if [ $? -eq 0 ]; then
    echo "   ✓ Example compilation successful"
    ./price_map_example > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "   ✓ Examples ran successfully"
    else
        echo "   ✗ Examples failed"
        exit 1
    fi
else
    echo "   ✗ Example compilation failed"
    exit 1
fi

# Compile and run performance comparison
echo "3. Testing performance comparison..."
g++ -std=c++14 -I. -O2 comparison_test.cpp -o comparison_test
if [ $? -eq 0 ]; then
    echo "   ✓ Comparison compilation successful"
    ./comparison_test > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "   ✓ Comparison test successful"
    else
        echo "   ✗ Comparison test failed"
        exit 1
    fi
else
    echo "   ✗ Comparison compilation failed"
    exit 1
fi

# Test with the original stable_vector
echo "4. Testing stable_vector compatibility..."
g++ -std=c++14 -I. simple_test.cpp -o simple_test
if [ $? -eq 0 ]; then
    echo "   ✓ stable_vector compilation successful"
    ./simple_test > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "   ✓ stable_vector still works correctly"
    else
        echo "   ✗ stable_vector test failed"
        exit 1
    fi
else
    echo "   ✗ stable_vector compilation failed"
    exit 1
fi

echo ""
echo "=== All Integration Tests Passed! ==="
echo ""
echo "price_map is ready for use as a drop-in replacement for std::map<double, T>"
echo "with improved O(1) performance and reference stability."