#pragma once

#include "stable_vector.h"
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <functional>
#include <vector>

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
    
    // Internal storage for key-value pairs
    stable_vector<internal_value_type> data_;
    
    // Hash map for O(1) key-to-index lookup  
    std::unordered_map<double, size_type> key_to_index_;
    
    // Track deleted indices for reuse (since stable_vector doesn't support deletion)
    std::vector<size_type> deleted_indices_;

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
            // Skip deleted entries
            skip_deleted();
        }
        
        void skip_deleted()
        {
            while (index_ < container_->data_.size() && is_deleted())
            {
                ++index_;
            }
        }
        
        bool is_deleted() const
        {
            return std::find(container_->deleted_indices_.begin(), 
                           container_->deleted_indices_.end(), 
                           index_) != container_->deleted_indices_.end();
        }
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = price_map::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        
        iterator() : container_(nullptr), index_(0) {}
        
        reference operator*() { return reinterpret_cast<reference>(container_->data_[index_]); }
        pointer operator->() { return reinterpret_cast<pointer>(&container_->data_[index_]); }
        
        iterator& operator++()
        {
            ++index_;
            skip_deleted();
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
            skip_deleted();
        }
        
        void skip_deleted()
        {
            while (index_ < container_->data_.size() && is_deleted())
            {
                ++index_;
            }
        }
        
        bool is_deleted() const
        {
            return std::find(container_->deleted_indices_.begin(), 
                           container_->deleted_indices_.end(), 
                           index_) != container_->deleted_indices_.end();
        }
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = price_map::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        
        const_iterator() : container_(nullptr), index_(0) {}
        const_iterator(const iterator& it) : container_(it.container_), index_(it.index_) {}
        
        reference operator*() const { return reinterpret_cast<reference>(container_->data_[index_]); }
        pointer operator->() const { return reinterpret_cast<pointer>(&container_->data_[index_]); }
        
        const_iterator& operator++()
        {
            ++index_;
            skip_deleted();
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
    price_map() = default;
    
    price_map(std::initializer_list<value_type> init)
    {
        for (const auto& item : init)
        {
            insert(item);
        }
    }
    
    // Capacity
    bool empty() const noexcept
    {
        return size() == 0;
    }
    
    size_type size() const noexcept
    {
        return key_to_index_.size();
    }
    
    size_type max_size() const noexcept
    {
        return data_.max_size();
    }
    
    // Element access
    T& at(const double& key)
    {
        auto it = key_to_index_.find(key);
        if (it == key_to_index_.end())
        {
            throw std::out_of_range("price_map::at");
        }
        return data_[it->second].second;
    }
    
    const T& at(const double& key) const
    {
        auto it = key_to_index_.find(key);
        if (it == key_to_index_.end())
        {
            throw std::out_of_range("price_map::at");
        }
        return data_[it->second].second;
    }
    
    T& operator[](const double& key)
    {
        auto it = key_to_index_.find(key);
        if (it != key_to_index_.end())
        {
            return data_[it->second].second;
        }
        
        // Insert new element
        size_type index;
        if (!deleted_indices_.empty())
        {
            // Reuse deleted index
            index = deleted_indices_.back();
            deleted_indices_.pop_back();
            data_[index] = internal_value_type(key, T{});
        }
        else
        {
            // Add new element
            index = data_.size();
            data_.emplace_back(key, T{});
        }
        
        key_to_index_[key] = index;
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
        auto it = key_to_index_.find(key);
        if (it != key_to_index_.end())
        {
            // Key already exists
            return std::make_pair(iterator(this, it->second), false);
        }
        
        size_type index;
        if (!deleted_indices_.empty())
        {
            // Reuse deleted index
            index = deleted_indices_.back();
            deleted_indices_.pop_back();
            data_[index] = internal_value_type(key, value);
        }
        else
        {
            // Add new element
            index = data_.size();
            data_.emplace_back(key, value);
        }
        
        key_to_index_[key] = index;
        return std::make_pair(iterator(this, index), true);
    }
    
    template<class... Args>
    std::pair<iterator, bool> emplace(const double& key, Args&&... args)
    {
        auto it = key_to_index_.find(key);
        if (it != key_to_index_.end())
        {
            // Key already exists
            return std::make_pair(iterator(this, it->second), false);
        }
        
        size_type index;
        if (!deleted_indices_.empty())
        {
            // Reuse deleted index
            index = deleted_indices_.back();
            deleted_indices_.pop_back();
            data_[index] = internal_value_type(key, T(std::forward<Args>(args)...));
        }
        else
        {
            // Add new element
            index = data_.size();
            data_.emplace_back(key, T(std::forward<Args>(args)...));
        }
        
        key_to_index_[key] = index;
        return std::make_pair(iterator(this, index), true);
    }
    
    size_type erase(const double& key)
    {
        auto it = key_to_index_.find(key);
        if (it == key_to_index_.end())
        {
            return 0;
        }
        
        // Mark index as deleted
        deleted_indices_.push_back(it->second);
        key_to_index_.erase(it);
        return 1;
    }
    
    iterator erase(const_iterator pos)
    {
        if (pos == end())
        {
            return end();
        }
        
        double key = pos->first;
        size_type index = pos.index_;
        
        // Mark index as deleted
        deleted_indices_.push_back(index);
        key_to_index_.erase(key);
        
        // Return iterator to next valid element
        iterator next_it(this, index + 1);
        return next_it;
    }
    
    void clear() noexcept
    {
        key_to_index_.clear();
        deleted_indices_.clear();
        // Note: stable_vector doesn't have clear(), so we just mark all as deleted
        for (size_type i = 0; i < data_.size(); ++i)
        {
            deleted_indices_.push_back(i);
        }
    }
    
    // Lookup
    iterator find(const double& key)
    {
        auto it = key_to_index_.find(key);
        if (it == key_to_index_.end())
        {
            return end();
        }
        return iterator(this, it->second);
    }
    
    const_iterator find(const double& key) const
    {
        auto it = key_to_index_.find(key);
        if (it == key_to_index_.end())
        {
            return end();
        }
        return const_iterator(this, it->second);
    }
    
    size_type count(const double& key) const
    {
        return key_to_index_.count(key);
    }
    
    bool contains(const double& key) const
    {
        return key_to_index_.find(key) != key_to_index_.end();
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