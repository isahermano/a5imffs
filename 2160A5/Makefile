CC=clang
CFLAGS=-Wall -g -DNDEBUG

# The default goal is to build all programs

all: a5_test_mm a5_main a5_test_imffs

# Targets to link all programs

a5_test_mm: a5_test_mm.o a4_tests.o a5_multimap.o

a5_main: a5_main.o a5_imffs.o a5_multimap.o

a5_test_imffs: a5_test_imffs.o a4_tests.o a5_imffs.o a5_multimap.o

# Targets to compile all object files

a5_test_mm.o: a5_test_mm.c a4_tests.h a5_multimap.h 

a5_imffs.o: a5_imffs.c a5_imffs.h a5_multimap.h 

a4_tests.o: a4_tests.c a4_tests.h 

a5_main.o: a5_main.c a5_imffs.h a5_multimap.h

a5_test_imffs.o: a5_test_imffs.c a4_tests.h a5_imffs.h a5_multimap.h

clean:
	rm -f *.o a5_test_mm a5_main a5_test_imffs
