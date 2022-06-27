cmake_args += -DCMAKE_BUILD_TYPE=Debug
ifdef dbg
run = gdb --args
cmake_args += -DCMAKE_BUILD_TYPE=Debug
else
run =
cmake_args += -DCMAKE_BUILD_TYPE=Release
endif

examples: example/sf_/Boomwhacker.sf2

#ifdef sf3
examples: example/sf_/tloz.sf3
#examples: example/sf_/default.sf3
cmake_args += -DENABLE_SF3=YES
#ifeq ($(sf3),stb)
cmake_args += -DSTB_VORBIS=YES
#endif
#endif

example/%: out/example
	@echo Run example
	@LD_LIBRARY_PATH=out $(run) $^ $* $*.sf3
	#@LD_LIBRARY_PATH=out $(run) $^ $* $*.pcm

clean:
	@rm -rf out

out/example: src/main.c out/libfluidlite.a
	@$(CC) -g -o $@ $< -I../include -I../libbotox/src/include -Lout -lfluidlite -L../libbotox/lib -lbotox

.PHONY: $(addprefix out/libfluidlite.,a so)
$(addprefix out/libfluidlite.,a so):
	@mkdir -p out
	@cd out && make
	#@cd out && cmake $(cmake_args) ../../ && make
