CC = gcc

CFLAGS = 

TARGET = output
BUILD_DIR = build
INC_DIR = ../../src/include ../../config
SRC = ./main.c ../../src/TDMS.c


ifeq ($(OS),Windows_NT)
FIXPATH = $(subst /,\,$1)
RMD = rd /s /q
MD = mkdir
EXESUFFIX = .exe
else
FIXPATH = $1
RMD = rm -r
MD = mkdir -p
EXESUFFIX = .elf
endif


SOURCES = $(filter %.c, $(SRC))
INCLUDES = $(patsubst %,-I%, $(INC_DIR:%/=%))
OUTPUT_EXE = $(addsuffix $(EXESUFFIX),$(call FIXPATH,$(BUILD_DIR)/$(TARGET)))


all: $(BUILD_DIR) $(TARGET)

clean:
	$(RMD) $(call FIXPATH,$(BUILD_DIR))

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(call FIXPATH,$(addprefix $(BUILD_DIR)/,$(notdir $@)))

# executable file
$(TARGET): $(SOURCES:.c=.o)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUT_EXE) $(call FIXPATH,$(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.c=.o))))

$(BUILD_DIR):
	$(MD) $(call FIXPATH,$(BUILD_DIR))
