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
```sh
pip3 install -r requirement.txt
```

## How to run

### run architecture generation
Write your own HADL language description and run it with python. Example usage can be found in ` ./genarch/genarchX `. Customize your own CGRA interconnect and PE patterns in ` ./genarch/genarchX/utils.py `.
Example Usage:
```sh
python3 ./genarch/genarch3/genarch3.py
#example
python3 ./genarch/genarch5/genarch5.py
```

### run RTL Generation
$adl$ file can be found in the path set up by HADL. Create and Specify the lib file $lib. Dump the RTL File to $path

Example Usage:

```sh
python3 ./script/script.py genrtl \$adl \$lib \$path
#example
python3 ./script/script.py genrtl ./arch/arch5/Core.json ./arch/arch5/CoreLib.ini ./arch/arch5/rtl
```

Or:
```sh
./build/genrtl \$adl \$lib \$path
#example
python3 ./script/script.py init ./arch/arch3/Top_RRG.txt ./arch/arch3/Top_FUs.txt ./arch/arch3/ ./arch/arch3/
```

### run Mapping Flow

Before run Mapping, run:
```sh
python3 ./script/script.py init \$fus \$rrg \$rrgpath \$linkpath
#example
python3 ./script/script.py init  
```
This is a non-essential step for contracting RRG and dump the links, but can be a significant time saver for larg-scale RRG and requires only one run for each RRG. 


Example Usage of running mapping for hierarchical CGRA:
```sh
python3 ./script/script.py mapping \$fileDFG \$fileCompat \$arch \$threadNum
#example
python3 ./script/script.py mapping ./benchmarks/express/arf/arf_DFG.txt ./benchmarks/express/arf/arf_compat.txt ./arch/arch3/arch.ini 4
```

Or runnng step by step
```sh
#packing
./build/pack \$dfg \$dfgGlobal(\$dfg) \$compat \$arch
#example
./build/pack ./benchmarks/express/arf/arf_DFG.txt ./benchmarks/express/arf/arf_DFG.txt ./benchmarks/express/arf/arf_compat.txt ./arch/arch3/arch.ini 
  
#partition
./build/partition \$dfg  \$compat \$arch
#example
./build/partition ./benchmarks/express/arf/arf_DFG.txt ./benchmarks/express/arf/arf_compat.txt ./arch/arch3/arch.ini 
  
#placeCore
./build/place placeCore \$dfg \$dfgGlobal  \$compat \$arch
#example
./build/place placeCore ./benchmarks_copy/express/arf/arf_DFG_part0.txt ./benchmarks/express/arf/arf_DFG.txt ./benchmarks/express/arf/arf_compat.txt ./arch/arch3/arch.ini
  
#placeTop
./build/place placeTop \$dfg  \$compat \$arch
#example
./build/place placeTop ./benchmarks/express/arf/arf_DFG.txt ./benchmarks/express/arf/arf_compat.txt ./arch/arch3/arch.ini
```  

use 2>/dev/null or 2>XX.log to get operation information clearly

Example Usage of running mapping for small scale CGRA:
```sh
./build/place placeCoreII \$dfg  \$compat \$rrg \$fus \$ii 
#example
./build/place placeCoreII ./benchmarks/cgrame/accumulate/accumulate_DFG.txt ./benchmarks/cgrame/accumulate/accumulate_compat.txt ./arch/arch5/Core_RRG.txt ./arch/arch5/Core_FUs.txt 3
```

### run DFG Generation

LLVM PASS:
```sh
cd ./benchmarks/standard/
make
```

DFDL PASS:
run dataflow/exampleDFG.py