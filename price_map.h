#pragma once

#include "stable_vector.h"
#include <utility>
#include <stdexcept>
#include <functional>
#include <vector>
#include <cmath>
#include <limits>

template <typename T>
class price_map
{
public:
    using key_type = double;
    using mapped_type = T;
    using value_type = std::pair<const double, T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

private:
    // Internal storage type - using non-const key for assignment
    using internal_value_type = std::pair<double, T>;
    
    // Price book configuration
    double opening_price_;
    double min_price_;
    double max_price_;
    double tick_size_;
    size_type total_levels_;
    
    // Internal storage for key-value pairs
    stable_vector<internal_value_type> data_;
    
    // Track which indices contain valid data
    std::vector<char> occupied_;  // Using char instead of bool to avoid vector<bool> issues
    size_type size_count_;
    
    // Convert price to array index using mathematical mapping
    size_type price_to_index(double price) const
    {
        if (price < min_price_ || price > max_price_)
        {
            throw std::out_of_range("Price outside valid range");
        }
        
        // Calculate index based on offset from min_price and tick_size
        double offset = price - min_price_;
        size_type index = static_cast<size_type>(std::round(offset / tick_size_));
        
        // Ensure we don't exceed bounds due to floating point precision
        if (index >= total_levels_)
        {
            index = total_levels_ - 1;
        }
        
        return index;
    }
    
    // Convert array index back to price
    double index_to_price(size_type index) const
    {
        if (index >= total_levels_)
        {
            throw std::out_of_range("Index outside valid range");
        }
        
        return min_price_ + (index * tick_size_);
    }
    
    // Validate that a price is properly aligned to tick size
    bool is_valid_price(double price) const
    {
        if (price < min_price_ || price > max_price_)
        {
            return false;
        }
        
        double offset = price - min_price_;
        double ticks = offset / tick_size_;
        return std::abs(ticks - std::round(ticks)) < 1e-9; // Small epsilon for floating point comparison
    }

public:
    // Iterator implementation
    class iterator
    {
        friend class price_map;
        
    private:
        price_map* container_;
        size_type index_;
        
        iterator(price_map* container, size_type index)
            : container_(container), index_(index)
        {
            // Skip unoccupied entries
            skip_unoccupied();
        }
        
        void skip_unoccupied()
        {
            while (index_ < container_->data_.size() && !container_->occupied_[index_])
            {
                ++index_;
            }
        }
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = price_map::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        
        iterator() : container_(nullptr), index_(0) {}
        
        reference operator*() { 
            // Create a reference to the internal pair, but cast the key to const
            auto& internal_pair = container_->data_[index_];
            return *reinterpret_cast<pointer>(&internal_pair);
        }
        pointer operator->() { 
            auto& internal_pair = container_->data_[index_];
            return reinterpret_cast<pointer>(&internal_pair);
        }
        
        iterator& operator++()
        {
            ++index_;
            skip_unoccupied();
            return *this;
        }
        
        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const iterator& other) const
        {
            return container_ == other.container_ && index_ == other.index_;
        }
        
        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }
    };
    
    class const_iterator
    {
        friend class price_map;
        
    private:
        const price_map* container_;
        size_type index_;
        
        const_iterator(const price_map* container, size_type index)
            : container_(container), index_(index)
        {
            skip_unoccupied();
        }
        
        void skip_unoccupied()
        {
            while (index_ < container_->data_.size() && !container_->occupied_[index_])
            {
                ++index_;
            }
        }
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = price_map::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        
        const_iterator() : container_(nullptr), index_(0) {}
        const_iterator(const iterator& it) : container_(it.container_), index_(it.index_) {}
        
        reference operator*() const { 
            auto& internal_pair = container_->data_[index_];
            return *reinterpret_cast<pointer>(&internal_pair);
        }
        pointer operator->() const { 
            auto& internal_pair = container_->data_[index_];
            return reinterpret_cast<pointer>(&internal_pair);
        }
        
        const_iterator& operator++()
        {
            ++index_;
            skip_unoccupied();
            return *this;
        }
        
        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const const_iterator& other) const
        {
            return container_ == other.container_ && index_ == other.index_;
        }
        
        bool operator!=(const const_iterator& other) const
        {
            return !(*this == other);
        }
    };

    // Constructors
    price_map() = delete; // Require price parameters
    
    // Constructor with price book parameters
    price_map(double opening_price, double up_limit_pct, double down_limit_pct, double tick_size)
        : opening_price_(opening_price)
        , tick_size_(tick_size)
        , size_count_(0)
    {
        if (tick_size <= 0.0)
        {
            throw std::invalid_argument("Tick size must be positive");
        }
        if (up_limit_pct < 0.0 || down_limit_pct < 0.0)
        {
            throw std::invalid_argument("Limit percentages must be non-negative");
        }
        
        // Calculate price range
        min_price_ = opening_price * (1.0 - down_limit_pct / 100.0);
        max_price_ = opening_price * (1.0 + up_limit_pct / 100.0);
        
        // Calculate total number of price levels
        double price_range = max_price_ - min_price_;
        total_levels_ = static_cast<size_type>(std::ceil(price_range / tick_size_)) + 1;
        
        // Initialize storage and tracking but don't pre-allocate
        // Let stable_vector grow naturally as needed
        data_.reserve(1000);  // Reserve some reasonable initial capacity
        occupied_.resize(total_levels_, 0);  // 0 = false, 1 = true
    }
    
    price_map(std::initializer_list<value_type> init, double opening_price, double up_limit_pct, double down_limit_pct, double tick_size)
        : price_map(opening_price, up_limit_pct, down_limit_pct, tick_size)
    {
        for (const auto& item : init)
        {
            insert(item);
        }
    }
    
    // Capacity
    bool empty() const noexcept
    {
        return size_count_ == 0;
    }
    
    size_type size() const noexcept
    {
        return size_count_;
    }
    
    size_type max_size() const noexcept
    {
        return total_levels_;
    }
    
    size_type capacity() const noexcept
    {
        return total_levels_;
    }
    
    // Price book specific information
    double min_price() const noexcept { return min_price_; }
    double max_price() const noexcept { return max_price_; }
    double tick_size() const noexcept { return tick_size_; }
    double opening_price() const noexcept { return opening_price_; }
    
    // Element access
    T& at(const double& key)
    {
        if (!is_valid_price(key))
        {
            throw std::out_of_range("Invalid price or price outside range");
        }
        
        size_type index = price_to_index(key);
        if (!occupied_[index])
        {
            throw std::out_of_range("price_map::at - key not found");
        }
        
        return data_[index].second;
    }
    
    const T& at(const double& key) const
    {
        if (!is_valid_price(key))
        {
            throw std::out_of_range("Invalid price or price outside range");
        }
        
        size_type index = price_to_index(key);
        if (!occupied_[index])
        {
            throw std::out_of_range("price_map::at - key not found");
        }
        
        return data_[index].second;
    }
    
    T& operator[](const double& key)
    {
        if (!is_valid_price(key))
        {
            throw std::out_of_range("Invalid price or price outside range");
        }
        
        size_type index = price_to_index(key);
        
        // Ensure the stable_vector has enough elements
        // Use careful growth to avoid string initialization issues
        while (data_.size() <= index)
        {
            // Create a default element with proper initialization
            double dummy_price = index_to_price(data_.size());
            data_.emplace_back(std::make_pair(dummy_price, T{}));
        }
        
        if (!occupied_[index])
        {
            // Initialize the element properly
            data_[index].first = key;
            data_[index].second = T{};
            occupied_[index] = 1;
            ++size_count_;
        }
        
        return data_[index].second;
    }
    
    // Iterators
    iterator begin() noexcept
    {
        return iterator(this, 0);
    }
    
    const_iterator begin() const noexcept
    {
        return const_iterator(this, 0);
    }
    
    const_iterator cbegin() const noexcept
    {
        return begin();
    }
    
    iterator end() noexcept
    {
        return iterator(this, data_.size());
    }
    
    const_iterator end() const noexcept
    {
        return const_iterator(this, data_.size());
    }
    
    const_iterator cend() const noexcept
    {
        return end();
    }
    
    // Modifiers
    std::pair<iterator, bool> insert(const value_type& value)
    {
        return insert(value.first, value.second);
    }
    
    std::pair<iterator, bool> insert(const double& key, const T& value)
    {
        if (!is_valid_price(key))
        {
            throw std::out_of_range("Invalid price or price outside range");
        }
        
        size_type index = price_to_index(key);
        
        // Ensure the stable_vector has enough elements
        while (data_.size() <= index)
        {
            double dummy_price = index_to_price(data_.size());
            data_.emplace_back(std::make_pair(dummy_price, T{}));
        }
        
        if (occupied_[index])
        {
            // Key already exists
            return std::make_pair(iterator(this, index), false);
        }
        
        // Insert new element
        data_[index].first = key;
        data_[index].second = value;
        occupied_[index] = 1;
        ++size_count_;
        
        return std::make_pair(iterator(this, index), true);
    }
    
    template<class... Args>
    std::pair<iterator, bool> emplace(const double& key, Args&&... args)
    {
        if (!is_valid_price(key))
        {
            throw std::out_of_range("Invalid price or price outside range");
        }
        
        size_type index = price_to_index(key);
        
        // Ensure the stable_vector has enough elements
        while (data_.size() <= index)
        {
            double dummy_price = index_to_price(data_.size());
            data_.emplace_back(std::make_pair(dummy_price, T{}));
        }
        
        if (occupied_[index])
        {
            // Key already exists
            return std::make_pair(iterator(this, index), false);
        }
        
        // Insert new element
        data_[index].first = key;
        data_[index].second = T(std::forward<Args>(args)...);
        occupied_[index] = 1;
        ++size_count_;
        
        return std::make_pair(iterator(this, index), true);
    }
    
    size_type erase(const double& key)
    {
        if (!is_valid_price(key))
        {
            return 0;
        }
        
        size_type index = price_to_index(key);
        if (!occupied_[index])
        {
            return 0;
        }
        
        // Mark as unoccupied
        occupied_[index] = 0;
        --size_count_;
        return 1;
    }
    
    iterator erase(const_iterator pos)
    {
        if (pos == end())
        {
            return end();
        }
        
        size_type index = pos.index_;
        
        if (occupied_[index])
        {
            occupied_[index] = 0;
            --size_count_;
        }
        
        // Return iterator to next valid element
        iterator next_it(this, index + 1);
        return next_it;
    }
    
    void clear() noexcept
    {
        std::fill(occupied_.begin(), occupied_.end(), 0);
        size_count_ = 0;
    }
    
    // Lookup
    iterator find(const double& key)
    {
        if (!is_valid_price(key))
        {
            return end();
        }
        
        size_type index = price_to_index(key);
        if (!occupied_[index])
        {
            return end();
        }
        
        return iterator(this, index);
    }
    
    const_iterator find(const double& key) const
    {
        if (!is_valid_price(key))
        {
            return end();
        }
        
        size_type index = price_to_index(key);
        if (!occupied_[index])
        {
            return end();
        }
        
        return const_iterator(this, index);
    }
    
    size_type count(const double& key) const
    {
        if (!is_valid_price(key))
        {
            return 0;
        }
        
        size_type index = price_to_index(key);
        return occupied_[index] ? 1 : 0;
    }
    
    bool contains(const double& key) const
    {
        if (!is_valid_price(key))
        {
            return false;
        }
        
        size_type index = price_to_index(key);
        return occupied_[index];
    }
    
    // Comparison operators
    bool operator==(const price_map& other) const
    {
        if (size() != other.size())
        {
            return false;
        }
        
        for (const auto& pair : *this)
        {
            auto it = other.find(pair.first);
            if (it == other.end() || it->second != pair.second)
            {
                return false;
            }
        }
        return true;
    }
    
    bool operator!=(const price_map& other) const
    {
        return !(*this == other);
    }
};