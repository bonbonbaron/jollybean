./o: ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o ./render.o ../../build/xRender.o ../../build/x.o ../../build/data.o ../../build/interface.o ../../build/data.o
	gcc ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o ./render.o ../../build/interface.o $(shell sdl2-config --libs) ../../utils/body/build/previewImg.o ../../build/xRender.o ../../build/x.o ../../build/data.o -o $@
	touch $@

./render.o: ./render.c
	gcc -g -c render.c -I../../src/include/ -I../../utils/body/src/include -I${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/include/ -I${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/include/ $(shell sdl2-config --cflags --cflags) -o $@
	touch $@

${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o: ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/redColormap.c
	gcc -g -I../../src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o: ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/src/redColorPalette.c
	gcc -g -I../../src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

#../../build/data.o: ../../src/data.c
	#make -C ../../

#../../build/xRender.o: ../../src/xRender.c: 
	#make -C ../../

${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/src/redColorPalette.c:
	../../utils/body/bin/body red

${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/redColormap.c:
	../../utils/body/bin/body red

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./render.o 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/obj/redColorPalette.o 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/obj/redColormap.o 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Palette/Color/src/redColorPalette.c 2>/dev/null
	rm -f ${HOME}/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/redColormap.c 2>/dev/null
