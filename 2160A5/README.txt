
Makefile compiles the following 5 programs:

a5_test_mm.c:
- contains the and runs the test cases for the multimap

a5_test_imffs.c:
- contains and runs the test cases for the imffs 

a5_multimap.c:
- contains the implementation of the multimap 

a5_imffs.c:
- contains the implementation of the imffs

a5_main.c
- handles input from the user

when calling "make", the Makefile compiles the 3 programs (a5_test_mm, a5_main, and a5_test_imffs)
essentially, these 3 programs are what we want to run individually to either
1. print the test case results of the multimap
2. print the test case result of the imffs
3. handle user input (via a5_main).