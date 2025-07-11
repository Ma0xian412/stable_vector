#include "price_map.h"
#include <map>
#include <iostream>
#include <chrono>
#include <random>

struct Order {
    int id;
    int quantity;
    std::string symbol;
    
    Order() = default;
    Order(int id, int qty, const std::string& sym) 
        : id(id), quantity(qty), symbol(sym) {}
};

// Performance comparison between std::map and price_map
void performance_comparison()
{
    const int NUM_OPERATIONS = 50000;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> price_dist(90.0, 110.0);
    
    std::cout << "=== Performance Comparison ===" << std::endl;
    std::cout << "Testing with " << NUM_OPERATIONS << " operations" << std::endl;
    
    // Test std::map
    {
        std::map<double, Order> std_book;
        auto start = std::chrono::high_resolution_clock::now();
        
        // Insert operations
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            double price = price_dist(gen);
            std_book[price] = Order(i, 100 + i, "AAPL");
        }
        
        auto mid = std::chrono::high_resolution_clock::now();
        
        // Lookup operations
        int found_count = 0;
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            double price = price_dist(gen);
            auto it = std_book.find(price);
            if (it != std_book.end()) {
                found_count++;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        
        auto insert_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
        auto lookup_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);
        
        std::cout << "\nstd::map results:" << std::endl;
        std::cout << "  Size: " << std_book.size() << std::endl;
        std::cout << "  Insert time: " << insert_time.count() << " μs" << std::endl;
        std::cout << "  Lookup time: " << lookup_time.count() << " μs" << std::endl;
        std::cout << "  Found: " << found_count << " elements" << std::endl;
    }
    
    // Test price_map
    {
        price_map<Order> price_book;
        auto start = std::chrono::high_resolution_clock::now();
        
        // Insert operations
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            double price = price_dist(gen);
            price_book[price] = Order(i, 100 + i, "AAPL");
        }
        
        auto mid = std::chrono::high_resolution_clock::now();
        
        // Lookup operations
        int found_count = 0;
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            double price = price_dist(gen);
            auto it = price_book.find(price);
            if (it != price_book.end()) {
                found_count++;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        
        auto insert_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
        auto lookup_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);
        
        std::cout << "\nprice_map results:" << std::endl;
        std::cout << "  Size: " << price_book.size() << std::endl;
        std::cout << "  Insert time: " << insert_time.count() << " μs" << std::endl;
        std::cout << "  Lookup time: " << lookup_time.count() << " μs" << std::endl;
        std::cout << "  Found: " << found_count << " elements" << std::endl;
    }
}

// Test API compatibility
void api_compatibility_test()
{
    std::cout << "\n=== API Compatibility Test ===" << std::endl;
    
    // Test that the same code works for both containers
    auto test_container = [](auto& container, const std::string& name) {
        std::cout << "\nTesting " << name << ":" << std::endl;
        
        // Insert
        container[100.50] = Order(1, 1000, "AAPL");
        container.insert(std::make_pair(101.25, Order(2, 500, "GOOGL")));
        
        // Access
        std::cout << "  Size: " << container.size() << std::endl;
        std::cout << "  Empty: " << (container.empty() ? "yes" : "no") << std::endl;
        
        // Find
        auto it = container.find(100.50);
        if (it != container.end()) {
            std::cout << "  Found order at $100.50: ID=" << it->second.id << std::endl;
        }
        
        // Iterate
        std::cout << "  All orders:" << std::endl;
        for (const auto& pair : container) {
            std::cout << "    $" << pair.first << " -> Order " << pair.second.id << std::endl;
        }
        
        // Erase
        container.erase(101.25);
        std::cout << "  Size after erase: " << container.size() << std::endl;
    };
    
    std::map<double, Order> std_container;
    price_map<Order> price_container;
    
    test_container(std_container, "std::map");
    test_container(price_container, "price_map");
}

// Test reference stability
void reference_stability_test()
{
    std::cout << "\n=== Reference Stability Test ===" << std::endl;
    
    price_map<Order> book;
    
    // Insert initial order
    book[100.0] = Order(1, 1000, "AAPL");
    Order& order_ref = book[100.0];
    Order* order_ptr = &book[100.0];
    
    std::cout << "Initial order address: " << order_ptr << std::endl;
    std::cout << "Initial order ID: " << order_ref.id << std::endl;
    
    // Add many more orders to trigger potential reallocation
    for (int i = 0; i < 10000; ++i) {
        book[200.0 + i * 0.01] = Order(i + 2, 100, "OTHER");
    }
    
    std::cout << "After adding 10000 orders:" << std::endl;
    std::cout << "Order address: " << &book[100.0] << std::endl;
    std::cout << "Order ID: " << order_ref.id << std::endl;
    std::cout << "Reference stable: " << (order_ptr == &book[100.0] ? "YES" : "NO") << std::endl;
    std::cout << "Book size: " << book.size() << std::endl;
}

int main()
{
    std::cout << "price_map Comparison and Compatibility Tests\n" << std::endl;
    
    performance_comparison();
    api_compatibility_test();
    reference_stability_test();
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}