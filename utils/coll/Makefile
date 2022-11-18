# Target: anim
./bin/anim: ./build/anim.o ./bin
	gcc -g ./build/anim.o ${HOME}/hack/jollybean/utils/file/build/fileUtils.o -lfastjson -L${HOME}/hack/botox/lib -lbotox -o $@

./build/anim.o: ./src/anim.c ./src/include/anim.h ./build
	gcc -g -lfastjson -I${HOME}/hack/jollybean/src/include -I${HOME}/hack/jollybean/utils/file/src/include -I/usr/include/libfastjson -I./src/include -I${HOME}/hack/botox/src/include $(shell sdl2-config --cflags ) ./src/anim.c -c -o $@

# Build directory (dependency)
./build:
	mkdir -p ./build

./bin:
	mkdir -p ./bin

# Cleaner
.PHONY: clean
clean:
	rm -rf ./bin
	rm -rf ./build
