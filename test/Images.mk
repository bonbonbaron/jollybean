NAMES_1BPP := bleh heck 
NAMES_4BPP := red
NAMES := $(NAMES_1BPP) $(NAMES_4BPP)

IMG_DIR = ${HOME}/jb/src/Image
COLORMAP_DIR = ${IMG_DIR}/Colormap
COLORPAL_DIR = ${IMG_DIR}/ColorPalette
BUILD_DIR = ${HOME}/jb/build

COLORMAP_SRCS := $(addsuffix Colormap.c, $(addprefix $(COLORMAP_DIR)/1bpp/, $(NAMES_1BPP)))
COLORMAP_SRCS := $(COLORMAP_SRCS) $(addsuffix Colormap.c, $(addprefix $(COLORMAP_DIR)/4bpp/, $(NAMES_4BPP)))

COLORPAL_SRCS := $(addsuffix ColorPalette.c, $(addprefix $(COLORPAL_DIR)/1bpp/, $(NAMES_1BPP)))
COLORPAL_SRCS := $(COLORPAL_SRCS) $(addsuffix ColorPalette.c, $(addprefix $(COLORPAL_DIR)/4bpp/, $(NAMES_4BPP)))

IMG_SRCS := $(addsuffix Img.c, $(addprefix $(IMG_DIR)/, $(NAMES_1BPP) $(NAMES_4BPP)))
CPI_SRCS := $(IMG_SRCS) $(COLORMAP_SRCS) $(COLORPAL_SRCS) 

CPI_OBJS := $(foreach src, $(CPI_SRCS), $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(src))))

SDL = $(shell sdl2-config --cflags)

# all: $(CPI_OBJS)

$(BUILD_DIR)/%.o: $(COLORMAP_DIR)/1bpp/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@
$(BUILD_DIR)/%.o: $(COLORMAP_DIR)/2bpp/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@
$(BUILD_DIR)/%.o: $(COLORMAP_DIR)/4bpp/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@

$(BUILD_DIR)/%.o: $(COLORPAL_DIR)/1bpp/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@
$(BUILD_DIR)/%.o: $(COLORPAL_DIR)/2bpp/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@
$(BUILD_DIR)/%.o: $(COLORPAL_DIR)/4bpp/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@

$(BUILD_DIR)/%.o: $(IMG_DIR)/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@
$(BUILD_DIR)/%.o: $(IMG_DIR)/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@
$(BUILD_DIR)/%.o: $(IMG_DIR)/%.c
	$(CC) -c $< -I${HOME}/hack/jollybean/include $(SDL) -o $@

# This stuff requires the repo's root Makefile to be included before this Makefile is included.
$(CPI_SRCS): ${UTL_DIR}/build/body
	${UTL_DIR}/build/body -v $(NAMES)

${UTL_DIR}/build/body:
	make -C ${UTL_DIR}
