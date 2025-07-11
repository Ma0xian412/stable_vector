#include "price_map.h"
#include <iostream>
#include <string>
#include <memory>
#include <cassert>
#include <chrono>

// Test struct to use as value type
struct Order 
{
    int id;
    int quantity;
    std::string symbol;
    
    Order() : id(0), quantity(0), symbol("") {}
    Order(int id, int qty, const std::string& sym) 
        : id(id), quantity(qty), symbol(sym) {}
    
    bool operator==(const Order& other) const 
    {
        return id == other.id && quantity == other.quantity && symbol == other.symbol;
    }
    
    bool operator!=(const Order& other) const 
    {
        return !(*this == other);
    }
};

// Helper function to print test results
void test_assert(bool condition, const std::string& test_name)
{
    if (condition) {
        std::cout << "[PASS] " << test_name << std::endl;
    } else {
        std::cout << "[FAIL] " << test_name << std::endl;
        assert(condition);
    }
}

// Basic functionality tests
void test_basic_operations()
{
    std::cout << "\n=== Testing Basic Operations ===" << std::endl;
    
    // Create price_map with reasonable price book parameters
    // Opening price: 100.0, up/down limits: 10%, tick size: 0.01
    price_map<Order> book(100.0, 10.0, 10.0, 0.01);
    
    // Test empty container
    test_assert(book.empty(), "Empty container");
    test_assert(book.size() == 0, "Size is zero");
    test_assert(book.begin() == book.end(), "Begin equals end for empty container");
    
    // Test insertion
    Order order1(1, 100, "AAPL");
    auto result1 = book.insert(100.50, order1);
    test_assert(result1.second == true, "Insert returns true for new element");
    test_assert(book.size() == 1, "Size is 1 after insertion");
    test_assert(!book.empty(), "Container not empty after insertion");
    
    // Test duplicate insertion
    Order order2(2, 200, "AAPL");
    auto result2 = book.insert(100.50, order2);
    test_assert(result2.second == false, "Insert returns false for duplicate key");
    test_assert(book.size() == 1, "Size remains 1 after duplicate insertion");
    
    // Test element access
    test_assert(book[100.50].id == 1, "Element access returns correct value");
    test_assert(book.at(100.50).quantity == 100, "at() returns correct value");
    
    // Test find
    auto it = book.find(100.50);
    test_assert(it != book.end(), "find() returns valid iterator for existing key");
    test_assert(it->first == 100.50, "Iterator points to correct key");
    test_assert(it->second.id == 1, "Iterator points to correct value");
    
    auto it_missing = book.find(200.0);
    test_assert(it_missing == book.end(), "find() returns end() for missing key");
}

void test_multiple_elements()
{
    std::cout << "\n=== Testing Multiple Elements ===" << std::endl;
    
    // Create price_map with reasonable price book parameters  
    price_map<Order> book(100.0, 10.0, 10.0, 0.01);
    
    // Insert multiple elements - all prices must be within valid range and tick-aligned
    book.insert(100.0, Order(1, 100, "AAPL"));
    book.insert(100.10, Order(2, 200, "AAPL"));  // Changed from 101.0 to stay in range
    book.insert(99.50, Order(3, 150, "AAPL"));
    book.emplace(100.25, 4, 300, "AAPL");         // Changed from 102.25 to stay in range
    
    test_assert(book.size() == 4, "Size is 4 after multiple insertions");
    
    // Test all elements exist
    test_assert(book.contains(100.0), "Contains 100.0");
    test_assert(book.contains(100.10), "Contains 100.10");
    test_assert(book.contains(99.50), "Contains 99.50");
    test_assert(book.contains(100.25), "Contains 100.25");
    test_assert(!book.contains(50.0), "Does not contain 50.0 (outside range)");
    
    // Test iteration
    int count = 0;
    for (const auto& pair : book) {
        count++;
        test_assert(pair.second.symbol == "AAPL", "All orders have correct symbol");
    }
    test_assert(count == 4, "Iteration visits all elements");
}

void test_deletion()
{
    std::cout << "\n=== Testing Deletion ===" << std::endl;
    
    // Create price_map with reasonable price book parameters
    price_map<Order> book(100.0, 10.0, 10.0, 0.01);
    
    // Add elements - all within valid range and tick-aligned
    book.insert(100.0, Order(1, 100, "AAPL"));
    book.insert(100.10, Order(2, 200, "AAPL"));  // Changed from 101.0
    book.insert(99.50, Order(3, 150, "AAPL"));
    
    test_assert(book.size() == 3, "Size is 3 before deletion");
    
    // Test deletion by key
    size_t erased = book.erase(100.10);
    test_assert(erased == 1, "erase() returns 1 for existing key");
    test_assert(book.size() == 2, "Size is 2 after deletion");
    test_assert(!book.contains(100.10), "Deleted key no longer exists");
    test_assert(book.contains(100.0), "Other keys still exist");
    test_assert(book.contains(99.50), "Other keys still exist");
    
    // Test deletion of non-existing key (outside valid range)
    size_t erased2 = book.erase(120.0);  // Outside range
    test_assert(erased2 == 0, "erase() returns 0 for non-existing key");
    test_assert(book.size() == 2, "Size unchanged after deleting non-existing key");
    
    // Test deletion by iterator
    auto it = book.find(100.0);
    auto next_it = book.erase(it);
    test_assert(book.size() == 1, "Size is 1 after iterator deletion");
    test_assert(!book.contains(100.0), "Deleted key no longer exists by iterator");
    
    // Test iteration after deletion
    int count = 0;
    for (const auto& pair : book) {
        count++;
        test_assert(pair.first == 99.50, "Remaining element has correct key");
    }
    test_assert(count == 1, "Iteration visits remaining element");
}

void test_index_reuse()
{
    std::cout << "\n=== Testing Index Reuse ===" << std::endl;
    
    // Create price_map with reasonable price book parameters
    price_map<Order> book(100.0, 10.0, 10.0, 0.01);
    
    // Add and remove elements to test index reuse
    book.insert(100.0, Order(1, 100, "AAPL"));
    book.insert(100.10, Order(2, 200, "AAPL"));  // Changed from 101.0
    book.insert(100.20, Order(3, 300, "AAPL"));  // Changed from 102.0
    
    // Delete middle element
    book.erase(100.10);
    test_assert(book.size() == 2, "Size is 2 after deletion");
    
    // Add new element (index will be reused automatically)
    book.insert(100.30, Order(4, 400, "AAPL"));  // Changed from 103.0
    test_assert(book.size() == 3, "Size is 3 after reusing index");
    
    // Verify all elements are accessible
    test_assert(book.contains(100.0), "Original element still exists");
    test_assert(book.contains(100.20), "Original element still exists");
    test_assert(book.contains(100.30), "New element exists");
    test_assert(!book.contains(100.10), "Deleted element does not exist");
}

void test_operator_access()
{
    std::cout << "\n=== Testing Operator[] Access ===" << std::endl;
    
    // Create price_map with reasonable price book parameters
    price_map<Order> book(100.0, 10.0, 10.0, 0.01);
    
    // Test operator[] for non-existing key (should create default)
    Order& order = book[100.0];
    test_assert(book.size() == 1, "Size is 1 after operator[] on missing key");
    test_assert(order.id == 0, "Default constructed value");
    
    // Modify through reference
    order.id = 123;
    order.quantity = 500;
    order.symbol = "GOOGL";
    
    test_assert(book[100.0].id == 123, "Modified value persists");
    test_assert(book[100.0].quantity == 500, "Modified value persists");
    test_assert(book[100.0].symbol == "GOOGL", "Modified value persists");
}

void test_clear()
{
    std::cout << "\n=== Testing Clear ===" << std::endl;
    
    // Create price_map with reasonable price book parameters
    price_map<Order> book(100.0, 10.0, 10.0, 0.01);
    
    // Add elements
    book.insert(100.0, Order(1, 100, "AAPL"));
    book.insert(100.10, Order(2, 200, "AAPL"));  // Changed from 101.0
    book.insert(100.20, Order(3, 300, "AAPL"));  // Changed from 102.0
    
    test_assert(book.size() == 3, "Size is 3 before clear");
    
    // Clear
    book.clear();
    test_assert(book.size() == 0, "Size is 0 after clear");
    test_assert(book.empty(), "Container is empty after clear");
    test_assert(book.begin() == book.end(), "Begin equals end after clear");
    
    // Verify elements don't exist
    test_assert(!book.contains(100.0), "Element doesn't exist after clear");
    test_assert(!book.contains(100.10), "Element doesn't exist after clear");
    test_assert(!book.contains(100.20), "Element doesn't exist after clear");
}

void test_copy_construction()
{
    std::cout << "\n=== Testing Copy Construction ===" << std::endl;
    
    // Create two identical price_maps with same parameters
    price_map<Order> book1(100.0, 10.0, 10.0, 0.01);
    book1.insert(100.0, Order(1, 100, "AAPL"));
    book1.insert(100.10, Order(2, 200, "GOOGL"));  // Changed from 101.0
    
    // Note: We haven't implemented copy constructor yet, but let's test assignment
    price_map<Order> book2(100.0, 10.0, 10.0, 0.01);  // Same parameters
    book2.insert(100.0, Order(1, 100, "AAPL"));
    book2.insert(100.10, Order(2, 200, "GOOGL"));      // Changed from 101.0
    
    test_assert(book1 == book2, "Equal containers compare equal");
    
    book2.insert(100.20, Order(3, 300, "MSFT"));       // Changed from 102.0
    test_assert(book1 != book2, "Different containers compare not equal");
}

void test_pointer_values()
{
    std::cout << "\n=== Testing Pointer Values ===" << std::endl;
    
    // Create price_map with reasonable price book parameters
    price_map<std::unique_ptr<Order>> book(100.0, 10.0, 10.0, 0.01);
    
    // Insert with pointer values
    book.emplace(100.0, std::make_unique<Order>(1, 100, "AAPL"));
    book.emplace(100.10, std::make_unique<Order>(2, 200, "GOOGL"));  // Changed from 101.0
    
    test_assert(book.size() == 2, "Size is 2 with pointer values");
    test_assert(book[100.0]->id == 1, "Pointer value accessible");
    test_assert(book[100.10]->symbol == "GOOGL", "Pointer value accessible");
}

// Performance test to verify O(1) characteristics
void test_performance()
{
    std::cout << "\n=== Testing Performance ===" << std::endl;
    
    const int NUM_ELEMENTS = 10000;  // Reduced for reasonable test within price range
    // Create a larger price range for performance test: opening 100, ±50%, tick 0.01
    price_map<int> book(100.0, 50.0, 50.0, 0.01);
    
    std::cout << "Price range: " << book.min_price() << " to " << book.max_price() << std::endl;
    std::cout << "Total capacity: " << book.capacity() << " price levels" << std::endl;
    
    // Insert elements within valid price range
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        double price = book.min_price() + (i * 0.01);
        if (price <= book.max_price()) {
            book.insert(price, i);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto insert_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Inserted " << book.size() << " elements in " << insert_time.count() << " microseconds" << std::endl;
    
    // Lookup elements
    start = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        double price = book.min_price() + (i * 0.01);
        if (price <= book.max_price() && book.contains(price)) {
            sum += book[price];
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto lookup_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Looked up elements in " << lookup_time.count() << " microseconds" << std::endl;
    std::cout << "Sum: " << sum << std::endl;
    
    test_assert(book.size() > 0, "Elements were inserted");
}

int main()
{
    std::cout << "Running price_map tests..." << std::endl;
    
    try {
        test_basic_operations();
        test_multiple_elements();
        test_deletion();
        test_index_reuse();
        test_operator_access();
        test_clear();
        test_copy_construction();
        test_pointer_values();
        test_performance();
        
        std::cout << "\n=== All Tests Passed! ===" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}