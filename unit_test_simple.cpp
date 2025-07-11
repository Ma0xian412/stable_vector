#include "stable_vector.h"
#include <iostream>
#include <list>
#include <vector>
#include <chrono>
#include <numeric>

// Simple test framework
#define ASSERT_TRUE(x) if(!(x)) { std::cout << "FAILED: " << #x << std::endl; return false; } else { std::cout << "PASSED: " << #x << std::endl; }
#define ASSERT_EQ(a, b) if((a) != (b)) { std::cout << "FAILED: " << #a << " != " << #b << " (" << (a) << " != " << (b) << ")" << std::endl; return false; } else { std::cout << "PASSED: " << #a << " == " << #b << std::endl; }
#define ASSERT_FALSE(x) if(x) { std::cout << "FAILED: " << #x << " should be false" << std::endl; return false; } else { std::cout << "PASSED: " << #x << " is false" << std::endl; }

bool test_init() {
    std::cout << "\nTest: init\n";
    stable_vector<int> v;
    ASSERT_TRUE(v.empty());
    ASSERT_EQ(v.size(), 0);
    return true;
}

bool test_ctor_initializer_list() {
    std::cout << "\nTest: ctor_initializer_list\n";
    stable_vector<int> v = {0, 1, 2, 3, 4};
    ASSERT_EQ(v.size(), 5);
    ASSERT_EQ(std::accumulate(v.cbegin(), v.cend(), 0), 0 + 1 + 2 + 3 + 4);
    return true;
}

bool test_ctor_element_copies() {
    std::cout << "\nTest: ctor_element_copies\n";
    stable_vector<int> v(5, 1);
    ASSERT_EQ(v.size(), 5);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(std::accumulate(v.cbegin(), v.cend(), 0), 5);
    return true;
}

bool test_ctor_count() {
    std::cout << "\nTest: ctor_count\n";
    stable_vector<int> v(5);
    ASSERT_EQ(v.size(), 5);
    ASSERT_EQ(v[0], 0);
    ASSERT_EQ(std::accumulate(v.cbegin(), v.cend(), 0), 0);
    return true;
}

bool test_ctor_input_iterator() {
    std::cout << "\nTest: ctor_input_iterator\n";
    std::list<int> l = {1, 2, 3, 4, 5};
    stable_vector<int> v(l.begin(), l.end());
    ASSERT_EQ(v.size(), l.size());
    ASSERT_EQ(std::accumulate(v.cbegin(), v.cend(), 0), std::accumulate(l.cbegin(), l.cend(), 0));
    return true;
}

bool test_copy_ctor() {
    std::cout << "\nTest: copy_ctor\n";
    stable_vector<int> v1 = {1, 2, 3, 4, 5};
    stable_vector<int> v2(v1);
    ASSERT_TRUE(v1 == v2);
    v2.push_back(6);
    ASSERT_EQ(v1.size(), 5);
    ASSERT_EQ(v2.size(), 6);
    return true;
}

bool test_multiple_chunks() {
    std::cout << "\nTest: multiple_chunks\n";
    stable_vector<int, 4> v = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_EQ(v.size(), 9);
    
    // Test reference stability
    stable_vector<int, 2> v2 = {1, 2};
    auto* ref = &v2[1];
    for (int i = 3; i < 10; ++i)
        v2.push_back(i);
    ASSERT_TRUE(ref == &v2[1]);
    return true;
}

bool test_iterator_operations() {
    std::cout << "\nTest: iterator_operations\n";
    stable_vector<int> v = {0, 1, 2, 3, 4};
    auto it = v.cbegin() + 3;
    ASSERT_EQ(*it, 3);
    
    it = it - 1;
    ASSERT_EQ(*it, 2);
    
    --it;
    ASSERT_EQ(*it, 1);
    
    it += 4;
    ASSERT_TRUE(it == v.cend());
    ASSERT_TRUE(it == v.end());
    
    it -= 5;
    ASSERT_TRUE(it == v.cbegin());
    ASSERT_TRUE(it == v.begin());
    return true;
}

int main() {
    std::cout << "Running unit tests for stable_vector without boost...\n";
    
    bool all_passed = true;
    all_passed &= test_init();
    all_passed &= test_ctor_initializer_list();
    all_passed &= test_ctor_element_copies();
    all_passed &= test_ctor_count();
    all_passed &= test_ctor_input_iterator();
    all_passed &= test_copy_ctor();
    all_passed &= test_multiple_chunks();
    all_passed &= test_iterator_operations();
    
    if (all_passed) {
        std::cout << "\n✓ All tests passed! Boost dependencies successfully removed.\n";
        return 0;
    } else {
        std::cout << "\n✗ Some tests failed.\n";
        return 1;
    }
}