# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
	OF_ROOT=$(realpath ../../../../openframeworks)
endif

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

# Build and refresh the shared lightgraph core before linking simulator targets.
.PHONY: lightgraph_core
lightgraph_core:
	+cmake -S $(LIGHTGRAPH_ROOT) -B $(LIGHTGRAPH_BUILD_DIR) \
		-DLIGHTGRAPH_CORE_BUILD_TESTS=OFF \
		-DLIGHTGRAPH_CORE_BUILD_EXAMPLES=OFF \
		-DLIGHTGRAPH_CORE_BUILD_BENCHMARKS=OFF \
		-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
	+cmake --build $(LIGHTGRAPH_BUILD_DIR) --target lightgraph --parallel

bin/simulator: lightgraph_core
bin/simulator_debug: lightgraph_core
