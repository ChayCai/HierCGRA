DEBUG    := 1
LOGLEVEL := 0
include Options.mk

COMMON_OBJECTS := $(addprefix ./build/objects/, Common.o Logger.o HyperGraph.o HierGraph.o)
UTIL_OBJECTS := $(addprefix ./build/objects/, GraphSort.o NetworkAnalyzer.o VanillaMatcher.o NOrderValidator.o VanillaValidator.o Utils.o SimulatedAnnealing.o)
JSON_OBJECTS = $(addprefix ./build/objects/, json_value.o json_reader.o json_writer.o )
GENRTL_OBJECTS = $(addprefix ./build/objects/, GenVerilog.o )
MAPPING_OBJECTS := $(addprefix ./build/objects/, FastPack.o FastPartition.o FastPlace.o )
MODEL_OBJECTS = $(addprefix ./build/objects/, FastPacker.o FastPartitioner.o FastRouter.o FastPlacer.o )
OBJECTS := $(COMMON_OBJECTS) $(UTIL_OBJECTS) $(JSON_OBJECTS) $(GENRTL_OBJECTS) $(MAPPING_OBJECTS) $(MODEL_OBJECTS)
# TEST_EXECS := $(addprefix ./build/, test0 test1)
SCRIPT_OBJECTS := $(addprefix ./build/, init pack partition place)
EXECUTABLES := $(TEST_EXECS) $(SCRIPT_OBJECTS)
# LLVMPASS_LIBS := $(addprefix ./build/, libPassModule2DFG.so)
# LIBRARIES := $(LLVMPASS_LIBS)

all: info $(EXECUTABLES) $(LIBRARIES)

init: ./mapping/script/init.cpp $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $< $(OBJECTS) -o ./build/$@

pack: ./mapping/script/pack.cpp $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $< $(OBJECTS) -o ./build/$@

partition: ./mapping/script/partition.cpp $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $< $(OBJECTS) -o ./build/$@

place: ./mapping/script/place.cpp $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $< $(OBJECTS) -o ./build/$@


info:
	@echo "============================================="
	@echo "INFO: CXX: \t \t $(CXX)"
	@echo "INFO: DEBUG: \t \t $(DEBUG)"
	@echo "INFO: LOGLEVEL: \t $(LOGLEVEL)"
	@echo "INFO: CXX_FLAGS: \t $(CXX_FLAGS)"
	@echo "INFO: COMMON_OBJECTS: \t $(COMMON_OBJECTS)"
	@echo "INFO: SCRIPT_OBJECTS: \t $(SCRIPT_OBJECTS)"
	@echo "INFO: TEST_EXECS: \t $(TEST_EXECS)"
	@echo "============================================="
	@echo ""

$(COMMON_OBJECTS):./build/objects/%.o: ./common/%.cpp ./common/%.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(UTIL_OBJECTS):./build/objects/%.o: ./util/%.cpp ./util/%.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(JSON_OBJECTS):./build/objects/%.o: ./util/tool/json/%.cpp ./util/tool/json/json.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(GENRTL_OBJECTS):./build/objects/%.o: ./util/tool/genrtl/%.cpp ./util/tool/genrtl/%.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(MAPPING_OBJECTS):./build/objects/%.o: ./mapping/%.cpp ./mapping/%.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(MODEL_OBJECTS):./build/objects/%.o: ./mapping/model/%.cpp ./mapping/model/%.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(TEST_EXECS):./build/%: ./test/%.cpp $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $< $(OBJECTS) -o $@

$(SCRIPT_OBJECTS):./build/%: ./mapping/script/%.cpp $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $< $(OBJECTS) -o $@

# $(LLVMPASS_LIBS):./build/%.so: ./dataflow/%.cpp ./dataflow/%.h  $(OBJECTS)
# 	$(CXX) $(CXX_FLAGS) `llvm-config-11 --cxxflags` `llvm-config-11 --ldflags` -Wl,-znodelete -fno-rtti -shared $<  $(OBJECTS) -o $@


.PHONY: clean
clean: 
	@rm $(OBJECTS) $(EXECUTABLES) $(LIBRARIES)
