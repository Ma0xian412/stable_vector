# price_map - High-Performance Price Book Container

## Overview

`price_map<T>` is a high-performance container that provides a `std::map`-compatible interface while using `stable_vector` as the underlying storage mechanism. It's specifically designed for financial applications like price books where fast lookups (O(1)) and reference stability are critical.

## Key Features

- **O(1) average-case performance** for insertion, lookup, and deletion operations
- **Reference stability** - pointers and references remain valid when the container grows
- **Cache-friendly storage** - uses `stable_vector` for contiguous memory layout
- **std::map-compatible interface** - drop-in replacement for existing code
- **Double key support** - optimized for price-based lookups

## Usage

### Basic Usage

```cpp
#include "price_map.h"

// Create a price map with Order values
price_map<Order> book;

// Insert elements
book.insert(100.50, Order(1, 1000, "BUY", "AAPL"));
book[101.25] = Order(2, 500, "SELL", "AAPL");

// Lookup (O(1))
auto it = book.find(100.50);
if (it != book.end()) {
    std::cout << "Found order: " << it->second.quantity << std::endl;
}

// Access with operator[]
Order& order = book[100.50];
order.quantity = 1500;  // Modify in-place

// Iteration
for (const auto& pair : book) {
    std::cout << "Price: " << pair.first 
              << ", Quantity: " << pair.second.quantity << std::endl;
}
```

### Using with Pointers

The original requirement mentioned using pointers as values:

```cpp
// Using raw pointers
price_map<Order*> book;
Order* order = new Order(1, 1000, "BUY", "AAPL");
book.insert(100.50, order);

// Using smart pointers (recommended)
price_map<std::shared_ptr<Order>> smart_book;
smart_book.insert(100.50, std::make_shared<Order>(1, 1000, "BUY", "AAPL"));
```

### Template Parameter

You can use any type T as the value:

```cpp
price_map<int> simple_book;              // Simple types
price_map<Order> order_book;             // Custom structs/classes  
price_map<std::string> string_book;      // Standard library types
price_map<Order*> pointer_book;          // Raw pointers
price_map<std::shared_ptr<Order>> smart_book;  // Smart pointers
```

## Interface Compatibility with std::map

`price_map` implements the essential `std::map` interface:

### Element Access
- `T& operator[](const double& key)`
- `T& at(const double& key)`
- `const T& at(const double& key) const`

### Iterators
- `iterator begin()`, `const_iterator begin() const`
- `iterator end()`, `const_iterator end() const`
- `const_iterator cbegin() const`, `const_iterator cend() const`

### Capacity
- `bool empty() const`
- `size_type size() const`
- `size_type max_size() const`

### Modifiers
- `std::pair<iterator, bool> insert(const double& key, const T& value)`
- `std::pair<iterator, bool> insert(const value_type& value)`
- `template<class... Args> std::pair<iterator, bool> emplace(const double& key, Args&&... args)`
- `size_type erase(const double& key)`
- `iterator erase(const_iterator pos)`
- `void clear()`

### Lookup
- `iterator find(const double& key)`
- `const_iterator find(const double& key) const`
- `size_type count(const double& key) const`
- `bool contains(const double& key) const`

### Comparison
- `bool operator==(const price_map& other) const`
- `bool operator!=(const price_map& other) const`

## Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| Insert | O(1) average | Uses hash map for key lookup |
| Find/Lookup | O(1) average | Direct hash table access |
| Erase | O(1) average | Marks as deleted, reuses indices |
| Iteration | O(n) | Skips deleted entries |
| Element Access | O(1) average | Via operator[] or at() |

## Memory Characteristics

- **Reference Stability**: References and pointers to elements remain valid when the container grows
- **Cache Efficiency**: Elements stored contiguously in `stable_vector` chunks
- **Memory Reuse**: Deleted indices are reused for new insertions
- **No Memory Fragmentation**: Uses chunked allocation strategy

## Limitations

1. **No True Deletion**: Elements are marked as deleted rather than physically removed (due to `stable_vector` limitations)
2. **No Ordering**: Unlike `std::map`, iteration order is not guaranteed to be sorted by key
3. **Hash Collisions**: Performance can degrade to O(n) in worst case (rare with good hash function)
4. **Floating Point Keys**: Uses `double` as key type with standard floating-point comparison

## Migration from std::map

To migrate from `std::map<double, T>` to `price_map<T>`:

1. Replace the type declaration:
   ```cpp
   // Before
   std::map<double, Order> book;
   
   // After  
   price_map<Order> book;
   ```

2. Update includes:
   ```cpp
   // Before
   #include <map>
   
   // After
   #include "price_map.h"
   ```

3. The API remains the same for most operations. Key differences:
   - No `lower_bound()`, `upper_bound()`, `equal_range()` (since no ordering)
   - No reverse iterators
   - `clear()` marks elements as deleted rather than removing them

## Example: Financial Price Book

```cpp
#include "price_map.h"

struct Order {
    int id;
    int quantity;
    std::string side;
};

int main() {
    price_map<Order> bid_book;  // Buy orders
    price_map<Order> ask_book;  // Sell orders
    
    // Add orders
    bid_book.insert(100.50, Order{1, 1000, "BUY"});
    bid_book.insert(100.25, Order{2, 500, "BUY"});
    
    ask_book.insert(101.00, Order{3, 800, "SELL"});
    ask_book.insert(101.25, Order{4, 600, "SELL"});
    
    // Fast price lookup
    auto best_bid = bid_book.find(100.50);
    auto best_ask = ask_book.find(101.00);
    
    if (best_bid != bid_book.end() && best_ask != ask_book.end()) {
        double spread = best_ask->first - best_bid->first;
        std::cout << "Bid-Ask Spread: $" << spread << std::endl;
    }
    
    return 0;
}
```

## Building

Compile with C++14 or later:

```bash
g++ -std=c++14 -I. your_code.cpp -o your_program
```

Make sure `stable_vector.h` and `price_map.h` are in your include path.