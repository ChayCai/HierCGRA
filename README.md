# xxxxCGRA
This is a toy. 

## How to install

### Compiler
If you already have `g++`, you can compile directly. 
```sh
mkdir build && cd build 
mkdir object
cd ..
make -j8
```

If you get an error ` g++: error: unrecognized command line option ‘-std=c++14’ `, you can update ` g++ ` or change it to ` c++11 `.

Then you can install the python packages we need. 
```python
pip3 install -r requirement.txt
```

## How to run

1. pack
```sh
./build/pack dfg_name dfg_name dfg_compat
``` 
`dfg_name` is dfg file, ` dfg_compat` is its compat file.
2. partition
```sh
./build/partition dfg_name dfg_compat part_num
```
`part_num` is we need to partition it into parts.
3. placeCore
```sh
./build/placeCore dfg_name_part dfg_name dfg_compat 
```
`dfg_name_part` is the part that needs to be mapped.
3. placeTop
```sh
./build/placeCore  dfg_name dfg_compat 
```

In the meantime, we provide an automated script to map.

We can run `python3 ./testPlace.py ./benchmarks/express/arf/arf_DFG.txt ./benchmarks/express/arf/arf_compat.txt ` or  ` python3 ./testPlace.py  ./benchmarks/express/matinv/matinv_DFG.txt ./benchmarks/express/matinv/matinv_compat.txt`.
