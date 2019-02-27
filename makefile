SRC=./src
BUILD=./build
LIB=./lib/bin
INCLUDE=./lib/include

LIBS=-lm
ARGS=-O3 -Wall
OBJECTS=$(BUILD)/string.o $(BUILD)/object.o $(BUILD)/list.o $(BUILD)/number.o $(BUILD)/pair.o $(BUILD)/bool.o $(BUILD)/prime.o\
$(BUILD)/dictionary.o $(BUILD)/environment.o $(BUILD)/error.o $(BUILD)/function.o $(BUILD)/macro.o $(BUILD)/operation.o $(BUILD)/struct.o \
$(BUILD)/variabletable.o $(BUILD)/tokenlist.o $(BUILD)/program.o $(BUILD)/token.o #langallocator.o
TARGET=./wakan
LIBTARGET=libwakan.a
CC=gcc
CLEAN=rm -f
COPY=cp -R

all: ./lib $(TARGET)

./lib: $(OBJECTS)
	$(COPY) $(SRC)/bool.h $(SRC)/dictionary.h $(SRC)/environment.h $(SRC)/error.h $(SRC)/function.h $(SRC)/langallocator.h $(SRC)/list.h $(SRC)/struct.h $(SRC)/tokenlist.h $(SRC)/variabletable.h \
$(SRC)/macro.h $(SRC)/number.h $(SRC)/object.h $(SRC)/operation.h $(SRC)/pair.h $(SRC)/prime.h $(SRC)/program.h $(SRC)/string.h $(SRC)/token.h $(SRC)/types.h $(INCLUDE)/
	ar rcs $(LIB)/$(LIBTARGET) $(OBJECTS)

$(TARGET): $(OBJECTS) $(BUILD)/main.o
	$(CC) -o $(TARGET) $(ARGS) $(OBJECTS) $(BUILD)/main.o $(LIBS)

$(BUILD)/main.o: $(SRC)/main.c $(SRC)/object.h $(SRC)/types.h $(SRC)/program.h
	$(CC) -c -o $(BUILD)/main.o $(ARGS) $(SRC)/main.c

$(BUILD)/string.o: $(SRC)/string.c $(SRC)/string.h $(SRC)/types.h $(SRC)/langallocator.h $(SRC)/bool.h
	$(CC) -c -o $(BUILD)/string.o $(ARGS) $(SRC)/string.c

$(BUILD)/object.o: $(SRC)/object.c $(SRC)/object.h $(SRC)/string.h $(SRC)/pair.h $(SRC)/number.h $(SRC)/list.h $(SRC)/dictionary.h $(SRC)/function.h\
$(SRC)/macro.h $(SRC)/types.h $(SRC)/langallocator.h $(SRC)/bool.h
	$(CC) -c -o $(BUILD)/object.o $(ARGS) $(SRC)/object.c

$(BUILD)/list.o: $(SRC)/list.c $(SRC)/list.h $(SRC)/object.h $(SRC)/types.h $(SRC)/langallocator.h $(SRC)/bool.h
	$(CC) -c -o $(BUILD)/list.o $(ARGS) $(SRC)/list.c

$(BUILD)/number.o: $(SRC)/number.c $(SRC)/number.h $(SRC)/types.h $(SRC)/bool.h
	$(CC) -c -o $(BUILD)/number.o $(ARGS) $(SRC)/number.c

$(BUILD)/pair.o: $(SRC)/pair.c $(SRC)/pair.h $(SRC)/object.h $(SRC)/types.h $(SRC)/bool.h
	$(CC) -c -o $(BUILD)/pair.o $(ARGS) $(SRC)/pair.c

$(BUILD)/bool.o: $(SRC)/bool.c $(SRC)/bool.h $(SRC)/types.h
	$(CC) -c -o $(BUILD)/bool.o $(ARGS) $(SRC)/bool.c

$(BUILD)/prime.o: $(SRC)/prime.c $(SRC)/prime.h
	$(CC) -c -o $(BUILD)/prime.o $(ARGS) $(SRC)/prime.c

$(BUILD)/dictionary.o: $(SRC)/dictionary.c $(SRC)/dictionary.h $(SRC)/prime.h $(SRC)/object.h $(SRC)/types.h $(SRC)/bool.h
	$(CC) -c -o $(BUILD)/dictionary.o $(ARGS) $(SRC)/dictionary.c

$(BUILD)/environment.o: $(SRC)/environment.c $(SRC)/environment.h $(SRC)/variabletable.h $(SRC)/types.h $(SRC)/object.h $(SRC)/prime.h $(SRC)/string.h
	$(CC) -c -o $(BUILD)/environment.o $(ARGS) $(SRC)/environment.c

$(BUILD)/error.o: $(SRC)/error.c $(SRC)/error.h
	$(CC) -c -o $(BUILD)/error.o $(ARGS) $(SRC)/error.c

$(BUILD)/function.o: $(SRC)/function.c $(SRC)/function.h $(SRC)/object.h $(SRC)/environment.h $(SRC)/prime.h $(SRC)/object.h $(SRC)/operation.h $(SRC)/types.h
	$(CC) -c -o $(BUILD)/function.o $(ARGS) $(SRC)/function.c

#$(BUILD)/langallocator.o: $(SRC)/langallocator.c $(SRC)/langallocator.h
#	$(CC) -c -o $(BUILD)/langallocator.o $(ARGS) $(SRC)/langallocator.c

$(BUILD)/macro.o: $(SRC)/macro.c $(SRC)/macro.h $(SRC)/operation.h
	$(CC) -c -o $(BUILD)/macro.o $(ARGS) $(SRC)/macro.c

$(BUILD)/operation.o: $(SRC)/operation.c $(SRC)/operation.h $(SRC)/object.h
	$(CC) -c -o $(BUILD)/operation.o $(ARGS) $(SRC)/operation.c

$(BUILD)/struct.o: $(SRC)/struct.c $(SRC)/struct.h $(SRC)/environment.h
	$(CC) -c -o $(BUILD)/struct.o $(ARGS) $(SRC)/struct.c

$(BUILD)/variabletable.o: $(SRC)/variabletable.c $(SRC)/variabletable.h $(SRC)/object.h $(SRC)/types.h
	$(CC) -c -o $(BUILD)/variabletable.o $(ARGS) $(SRC)/variabletable.c

$(BUILD)/token.o: $(SRC)/token.c $(SRC)/token.h $(SRC)/operation.h $(SRC)/types.h $(SRC)/string.h $(SRC)/number.h
	$(CC) -c -o $(BUILD)/token.o $(ARGS) $(SRC)/token.c

$(BUILD)/tokenlist.o: $(SRC)/tokenlist.c $(SRC)/tokenlist.h $(SRC)/token.h $(SRC)/types.h $(SRC)/string.h $(SRC)/error.h
	$(CC) -c -o $(BUILD)/tokenlist.o $(ARGS) $(SRC)/tokenlist.c

$(BUILD)/program.o: $(SRC)/program.c $(SRC)/program.h $(SRC)/types.h $(SRC)/operation.h
	$(CC) -c -o $(BUILD)/program.o $(ARGS) $(SRC)/program.c

clean:
	$(CLEAN) $(OBJECTS)

cleanall:
	$(CLEAN) $(OBJECTS) $(TARGET)
