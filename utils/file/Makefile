# Target: fileUtils
./build/fileUtils.o: ./src/fileUtils.c ./src/include/fileUtils.h ./build
	gcc -g -lfastjson -I/usr/include/libfastjson -I./src/include -I${HOME}/hack/botox/src/include ./src/fileUtils.c -c -o $@

# Build directory (dependency)
./build:
	mkdir -p ./build

# Cleaner
.PHONY: clean
clean:
	rm -rf ./build
