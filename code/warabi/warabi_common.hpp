/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef WARABI_COMMON_HPP
#define WARABI_COMMON_HPP

#include <warabi/Client.hpp>
#include <sstream>
#include <iomanip>
#include <string>

// Convert warabi::RegionID (16-byte array) to UUID string format
// Format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
inline std::string regionid_to_string(const warabi::RegionID& rid) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    // First 4 bytes
    for(int i = 0; i < 4; i++) {
        oss << std::setw(2) << static_cast<unsigned>(static_cast<unsigned char>(rid[i]));
    }
    oss << "-";

    // Next 2 bytes
    for(int i = 4; i < 6; i++) {
        oss << std::setw(2) << static_cast<unsigned>(static_cast<unsigned char>(rid[i]));
    }
    oss << "-";

    // Next 2 bytes
    for(int i = 6; i < 8; i++) {
        oss << std::setw(2) << static_cast<unsigned>(static_cast<unsigned char>(rid[i]));
    }
    oss << "-";

    // Next 2 bytes
    for(int i = 8; i < 10; i++) {
        oss << std::setw(2) << static_cast<unsigned>(static_cast<unsigned char>(rid[i]));
    }
    oss << "-";

    // Last 6 bytes
    for(int i = 10; i < 16; i++) {
        oss << std::setw(2) << static_cast<unsigned>(static_cast<unsigned char>(rid[i]));
    }

    return oss.str();
}

#endif // WARABI_COMMON_HPP
