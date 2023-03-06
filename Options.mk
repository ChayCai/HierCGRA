CXX_FLAGS_DEBUG  := -Wall -std=c++14 -fPIC -lm -fopenmp -I. -g
CXX_FLAGS_NDEBUG := -Wall -std=c++14 -fPIC -lm -fopenmp -I. -Ofast -DNDEBUG
CXX_FLAGS        := $(OPTIONS_NDEBUG)

ifeq ($(DEBUG),1)
	CXX_FLAGS := $(CXX_FLAGS_DEBUG)
endif

ifdef LOGLEVEL
	ifeq ($(LOGLEVEL),0)
		CXX_FLAGS += -DLOGLEVEL0
	endif
	ifeq ($(LOGLEVEL),1)
		CXX_FLAGS += -DLOGLEVEL1
	endif
	ifeq ($(LOGLEVEL),2)
		CXX_FLAGS += -DLOGLEVEL2
	endif
endif

ifndef CXX
	CXX := g++
endif