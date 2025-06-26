//
//  main.cpp
//  metal-project
//
//  Created by Raphaël Faure on 23/06/2025.
//

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>
#include "fox_map/fox_map.hpp"

void fill_buffer (MTL::Buffer * buffer, float value) {
    float* content = (float *) buffer->contents();
    for (size_t index = 0; index < buffer->length() / 4 ; index++) {
        content[index] = (float) index;
    }
}

void dump_buffer(MTL::Buffer * buffer) {
    float * content = (float *) buffer->contents();
    std::cout << "[ " ;
    for (size_t index = 0; index < buffer->length() / 4; index++) {
        std::cout << content[index] << " , ";
    }
    std::cout << "]" << std::endl;
}

#define DATA_SIZE 16

int main(int argc, const char * argv[]) {
    auto map = fox_map();
    if (!map.init()) {
        std::cout << "Error initializing map" << std::endl;
    } else {
        std::cout << "Map initialized" << std::endl;
    }
    if (!map.insert(4, 3)){
        std::cout << "Could not insert" << std::endl;
    };
    if (!map.insert(4 + MAP_ENTRY_N, 7)){
        std::cout << "Could not insert" << std::endl;
    };
    if (!map.insert(4 + 128, 5)){
        std::cout << "Could not insert" << std::endl;
    };
    std::cout << map.lookup(132) << std::endl;
    
    return 0;
}
