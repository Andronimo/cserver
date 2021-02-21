#VERSION = 3.02
VERBOSE  = 0
CXX      = gcc

OBJ+=hash_table.o

.DEFAULT_GOAL :=
elib: libtools.so

libtools.so: $(OBJ)
	@$(CXX) --shared -fPIC ${OBJ} -o libtools.so

all: elib test_elib

%.o: %.c
	@echo "compiling.." $<
	@$(CXX) -c -fPIC $< -o $@

test_elib: libtools.so test_elib.o
	@$(CXX) -o test_elib test_elib.o -L. -ltools

.PHONY: clean

clean:
	rm -f *.o libtools.so test_elib