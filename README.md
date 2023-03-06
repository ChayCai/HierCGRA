# HierCGRA
HierCGRA, a flexible and scalable CGRA framework, that allows complex interconnection generation and hierarchical CGRA modeling.

## How to install

### dependencies

xmltodict==0.13.0
networkx==2.8.6
optuna==3.0.2
clang==11.0.0

### make
```sh
cd .
mkdir build
mkdir build/object
make clean
make / make -j8 
```

If you get an error ` g++: error: unrecognized command line option ‘-std=c++14’ `, you can update ` g++ ` or change it to ` c++11 `.

Then you can install the python packages we need. 
```python
pip3 install -r requirement.txt
```

## How to run