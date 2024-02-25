// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <iostream>
#include <vector>
#include "hal/nfc.hpp"

int main() {
    NFCBoard pn532;
    std::vector<unsigned char> uid = pn532.getUid();
    std::cout << "[ ";
    for(auto it = uid.begin(); it != uid.end(); ++it) {
        std::cout << *it << std::endl;
    }
    std::cout << "]" << std::endl;;
    std::cout << "Hello, World!" << std::endl;
    return 0;
}