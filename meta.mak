# MSVC compiler and linker
CC = cl
LINK = link

# Compiler and linker flags
CFLAGS = /nologo /MD /I D:\Programs\C\meta\include
LDFLAGS = /nologo

# Source and object directories
SRC_DIR = D:\Programs\C\meta\src
OBJ_DIR = D:\Programs\C\meta\build
BIN_DIR = D:\Programs\C\meta

# List of source files
SOURCES = $(SRC_DIR)\main.c $(SRC_DIR)\metadata.c $(SRC_DIR)\config.c $(SRC_DIR)\filelist.c

# Object files (manually list object files corresponding to source files)
OBJECTS = $(OBJ_DIR)\main.obj $(OBJ_DIR)\metadata.obj $(OBJ_DIR)\config.obj $(OBJ_DIR)\filelist.obj

# Target executable
TARGET = $(BIN_DIR)\meta.exe

# Build rule for the executable
all: $(TARGET)

$(TARGET): $(OBJECTS)
    $(LINK) $(LDFLAGS) /OUT:$(TARGET) $(OBJECTS)

# Build rule for object files
$(OBJ_DIR)\main.obj: $(SRC_DIR)\main.c
    $(CC) $(CFLAGS) /c /Fo$@ $(SRC_DIR)\main.c

$(OBJ_DIR)\metadata.obj: $(SRC_DIR)\metadata.c
    $(CC) $(CFLAGS) /c /Fo$@ $(SRC_DIR)\metadata.c

$(OBJ_DIR)\config.obj: $(SRC_DIR)\config.c
    $(CC) $(CFLAGS) /c /Fo$@ $(SRC_DIR)\config.c

$(OBJ_DIR)\filelist.obj: $(SRC_DIR)\filelist.c
    $(CC) $(CFLAGS) /c /Fo$@ $(SRC_DIR)\filelist.c

# Clean rule
clean:
    del /q $(OBJECTS) $(TARGET)
