#include "stable_vector.h"
#include <iostream>

int main() {
    stable_vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    
    std::cout << "Size: " << v.size() << std::endl;
    std::cout << "Elements: ";
    for (const auto& i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    
    return 0;
}