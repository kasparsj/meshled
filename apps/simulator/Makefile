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

# Build and refresh the shared lightpath core before linking simulator targets.
.PHONY: lightpath_core
lightpath_core:
	+cmake -S $(LIGHTPATH_ROOT) -B $(LIGHTPATH_BUILD_DIR) \
		-DLIGHTPATH_CORE_BUILD_TESTS=OFF \
		-DLIGHTPATH_CORE_BUILD_EXAMPLES=OFF \
		-DLIGHTPATH_CORE_BUILD_BENCHMARKS=OFF \
		-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
	+cmake --build $(LIGHTPATH_BUILD_DIR) --target lightpath --parallel

bin/simulator: lightpath_core
bin/simulator_debug: lightpath_core
