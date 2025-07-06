from metal import Map
from array import array
import random
import time

SIZE = 10000000
keys = list(range(SIZE))
values = list(range(SIZE))
random.shuffle(keys)
random.shuffle(values)
k = array('i', keys)
v = array('i', values)
out_v_gpu = array('i', [0] * SIZE)
out_v_cpu = array('i', [0] * SIZE)
d = dict()

print("CPU")
start = time.perf_counter()
for i in range(SIZE) :
    d[k[i]] = v[i]
for i in range(SIZE) :
    out_v_cpu[i] = d[k[i]]
end = time.perf_counter()
print(f"Done CPU : {end - start}")

a = Map(SIZE)

print("GPU")
start = time.perf_counter()
a.insert(k,v)
a.lookup(k, out_v_gpu)
end = time.perf_counter()
print(f"Done GPU : {end - start}")

assert (v == out_v_cpu and v == out_v_gpu)