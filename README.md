# metal-ml
metal-ml is an Apple Silicon GPU accelerated library offering various data structures and algorithms.  
It is built on the Metal shading language as well as the metal-cpp bindings.

## Python
The library is also intended to be ported to python, through a standard C python extension.

## Data structures
- (WIP) `mtl_map` a parallelized hashmap supporting integer keys and values up to 32 bits each.

## Algorithms
- (TBD) `mtl_kmean` a kmean algorithm for arbitrary dimension spaces.s


## C++ Performances

### Map
| Map Type                                 | Insert Time (ms) | Lookup Time (ms) | Size      |
| ---------------------------------------- | ---------------- | ---------------- | --------- |
| `unordered_map<uint16_t, uint16_t>`      | 3.3              | 10               | 65,536    |
| `mtl_map<uint16_t, uint16_t, 65,536>`    | 2.7              | 0.2              | 65,536    |
| `unordered_map<uint32_t, uint32_t>`      | 677.8            | 1188.4           | 5,000,000 |
| `mtl_map<uint32_t, uint32_t, 5,000,000>` | 52.3             | 35.6             | 5,000,000 |
