./o: ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o ./render.o ${HOME}/hack/jollybean/build/xRender.o ${HOME}/hack/jollybean/build/x.o ${HOME}/hack/jollybean/build/data.o ${HOME}/hack/jollybean/build/interface.o ${HOME}/hack/jollybean/build/data.o
	gcc ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o ./render.o ${HOME}/hack/jollybean/build/interface.o $(shell sdl2-config --libs) ${HOME}/hack/jollybean/build/xRender.o ${HOME}/hack/jollybean/build/x.o ${HOME}/hack/jollybean/build/data.o -o $@
	touch $@

./render.o: ./render.c
	gcc -g -c render.c -I../src/include/ -I${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/include/ -I${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/include/ $(shell sdl2-config --cflags --cflags) -o $@
	touch $@

${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o: ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/redColormap.c
	gcc -g -I${HOME}/hack/jollybean/src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o: ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/src/redColorPalette.c
	gcc -g -I${HOME}/hack/jollybean/src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${HOME}/hack/jollybean/build/data.o: ${HOME}/hack/jollybean/src/data.c
	make -C ../

${HOME}/hack/jollybean/build/xRender.o: ${HOME}/hack/jollybean/src/xRender.c
	make -C ../

${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/src/redColorPalette.c:
	${HOME}/hack/jollybean/utils/body/bin/body red

${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/redColormap.c:
	${HOME}/hack/jollybean/utils/body/bin/body red

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./render.o 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/src/redColorPalette.c 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/redColormap.c 2>/dev/null
