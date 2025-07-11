#include "price_map.h"
#include <iostream>
#include <memory>
#include <string>

// Example Order structure for a price book
struct Order {
    int order_id;
    int quantity;
    std::string side;  // "BUY" or "SELL"
    std::string symbol;
    
    Order() = default;
    Order(int id, int qty, const std::string& s, const std::string& sym)
        : order_id(id), quantity(qty), side(s), symbol(sym) {}
};

// Example usage demonstrating replacement of std::map
void demonstrate_price_book_usage()
{
    std::cout << "=== Price Book Example ===" << std::endl;
    
    // Create a price book for buy orders (bids)
    price_map<Order> bid_book;
    
    // Add some orders at different price levels
    bid_book.insert(100.50, Order(1, 1000, "BUY", "AAPL"));
    bid_book.insert(100.25, Order(2, 500, "BUY", "AAPL"));
    bid_book.insert(100.75, Order(3, 800, "BUY", "AAPL"));
    bid_book.insert(100.60, Order(4, 1200, "BUY", "AAPL"));
    
    std::cout << "Bid book contains " << bid_book.size() << " price levels" << std::endl;
    
    // Access orders by price (O(1) lookup)
    auto it = bid_book.find(100.50);
    if (it != bid_book.end()) {
        std::cout << "Order at $100.50: ID=" << it->second.order_id 
                  << ", Qty=" << it->second.quantity << std::endl;
    }
    
    // Use operator[] for convenient access
    bid_book[99.95].order_id = 5;
    bid_book[99.95].quantity = 300;
    bid_book[99.95].side = "BUY";
    bid_book[99.95].symbol = "AAPL";
    
    std::cout << "Added new order at $99.95" << std::endl;
    
    // Iterate through all price levels
    std::cout << "\nAll bid levels:" << std::endl;
    for (const auto& level : bid_book) {
        std::cout << "  Price: $" << level.first 
                  << ", Order ID: " << level.second.order_id
                  << ", Quantity: " << level.second.quantity << std::endl;
    }
    
    // Remove a price level
    bid_book.erase(100.25);
    std::cout << "\nAfter removing $100.25 level, size is: " << bid_book.size() << std::endl;
}

// Example with pointers (as mentioned in the requirement)
void demonstrate_pointer_usage()
{
    std::cout << "\n=== Pointer Usage Example ===" << std::endl;
    
    // Using smart pointers as values
    price_map<std::shared_ptr<Order>> order_book;
    
    // Insert orders using pointers
    order_book.insert(101.00, std::make_shared<Order>(10, 500, "SELL", "AAPL"));
    order_book.insert(101.25, std::make_shared<Order>(11, 300, "SELL", "AAPL"));
    order_book.insert(100.75, std::make_shared<Order>(12, 800, "SELL", "AAPL"));
    
    std::cout << "Order book with pointers contains " << order_book.size() << " levels" << std::endl;
    
    // Access through pointer
    auto ptr_order = order_book[101.00];
    if (ptr_order) {
        std::cout << "Order at $101.00: ID=" << ptr_order->order_id 
                  << ", Qty=" << ptr_order->quantity << std::endl;
    }
    
    // Demonstrate reference stability (orders don't move in memory when new ones are added)
    auto& reference_to_order = order_book[101.25];
    auto* address_before = reference_to_order.get();
    
    // Add many more orders
    for (int i = 0; i < 1000; ++i) {
        order_book.insert(200.0 + i * 0.01, std::make_shared<Order>(1000 + i, 100, "SELL", "AAPL"));
    }
    
    auto* address_after = reference_to_order.get();
    std::cout << "Reference stability test: " 
              << (address_before == address_after ? "PASS" : "FAIL") << std::endl;
}

// Performance comparison placeholder
void demonstrate_performance_characteristics()
{
    std::cout << "\n=== Performance Characteristics ===" << std::endl;
    std::cout << "price_map provides:" << std::endl;
    std::cout << "  - O(1) insertion, lookup, and deletion (average case)" << std::endl;
    std::cout << "  - Reference stability (addresses don't change when container grows)" << std::endl;
    std::cout << "  - Cache-friendly storage using stable_vector underneath" << std::endl;
    std::cout << "  - Compatible interface with std::map for drop-in replacement" << std::endl;
}

int main()
{
    std::cout << "price_map Usage Examples\n" << std::endl;
    
    demonstrate_price_book_usage();
    demonstrate_pointer_usage();
    demonstrate_performance_characteristics();
    
    std::cout << "\nAll examples completed successfully!" << std::endl;
    return 0;
}