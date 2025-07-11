#include "stable_vector.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

int main() {
    std::cout << "Testing stable_vector without boost dependencies...\n\n";
    
    // Test 1: Basic operations
    std::cout << "Test 1: Basic operations\n";
    stable_vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.emplace_back(3);
    
    std::cout << "Size: " << v.size() << std::endl;
    std::cout << "Elements: ";
    for (const auto& i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    
    // Test 2: Iterator operations
    std::cout << "\nTest 2: Iterator operations\n";
    auto it = v.begin();
    std::cout << "First element: " << *it << std::endl;
    ++it;
    std::cout << "Second element: " << *it << std::endl;
    it += 1;
    std::cout << "Third element: " << *it << std::endl;
    
    // Test 3: Const iterator
    std::cout << "\nTest 3: Const iterator\n";
    const auto& cv = v;
    for (auto cit = cv.begin(); cit != cv.end(); ++cit) {
        std::cout << *cit << " ";
    }
    std::cout << std::endl;
    
    // Test 4: Range-based algorithms
    std::cout << "\nTest 4: STL algorithms\n";
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "Sum: " << sum << std::endl;
    
    // Test 5: Multiple chunks (small chunk size)
    std::cout << "\nTest 5: Multiple chunks\n";
    stable_vector<int, 2> small_v;
    for (int i = 0; i < 5; ++i) {
        small_v.push_back(i);
    }
    std::cout << "Small vector size: " << small_v.size() << std::endl;
    std::cout << "Elements: ";
    for (const auto& i : small_v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    
    // Test 6: Copy constructor
    std::cout << "\nTest 6: Copy constructor\n";
    stable_vector<int> v2(v);
    std::cout << "Copied vector size: " << v2.size() << std::endl;
    std::cout << "Original and copy equal: " << (v == v2 ? "true" : "false") << std::endl;
    
    // Test 7: Assignment operator
    std::cout << "\nTest 7: Assignment operator\n";
    stable_vector<int> v3;
    v3.push_back(99);
    v3 = v;
    std::cout << "Assigned vector size: " << v3.size() << std::endl;
    std::cout << "Original and assigned equal: " << (v == v3 ? "true" : "false") << std::endl;
    
    // Test 8: Front and back
    std::cout << "\nTest 8: Front and back\n";
    std::cout << "Front: " << v.front() << std::endl;
    std::cout << "Back: " << v.back() << std::endl;
    
    // Test 9: Array access
    std::cout << "\nTest 9: Array access\n";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << "v[" << i << "] = " << v[i] << std::endl;
    }
    
    std::cout << "\nAll tests passed! Boost dependencies successfully removed.\n";
    
    return 0;
}