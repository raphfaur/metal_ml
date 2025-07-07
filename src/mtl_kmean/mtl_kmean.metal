#include <metal_stdlib>

#include "../common/types.hpp"

using namespace metal;



template<typename T, int dim>
kernel void kmean_compute(device vec<T, dim> * points [[buffer(0)]],
                          device vec<T, dim> * means [[buffer(1)]],
                          uint tid [[thread_position_in_grid]]){
    int N_MEAN = 0;
    vec<T, dim> point = points[tid];
    
    while (true) {
        for (int i = 0; i < N_MEAN; i ++ ) {
            vec<T, dim> mean = means[i];
            auto a = distance(point, mean);
        }
    }
}
