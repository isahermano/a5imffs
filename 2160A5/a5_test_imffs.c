/**
 * a5_imffs.c
 *
 * COMP 2160 SECTION A02
 * INSTRUCTOR    John Braico
 * ASSIGNMENT    Assignment 5, part c)
 * AUTHOR        Isabella Hermano, 7967075
 * DATE          December 12, 2023
 *
 * PURPOSE: To write test cases
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "a4_tests.h"
#include "a5_imffs.h"

void test_invalid() {
    IMFFSPtr *fs = NULL;
    printf("\n*** Invalid cases:\n\n");

    VERIFY_INT(IMFFS_INVALID, imffs_create(0, fs));
    VERIFY_INT(IMFFS_INVALID, imffs_save(NULL, NULL, NULL));
    VERIFY_INT(IMFFS_INVALID, imffs_load(NULL, NULL, NULL));
    VERIFY_INT(IMFFS_INVALID, imffs_delete(NULL, NULL));
    VERIFY_INT(IMFFS_INVALID, imffs_rename(NULL, NULL, NULL));
    VERIFY_INT(IMFFS_INVALID, imffs_dir(NULL));
    VERIFY_INT(IMFFS_INVALID, imffs_fulldir(NULL));
}

int main() {
  printf("*** Starting tests...\n");

#ifdef NDEBUG
  test_invalid();
#endif
  
  if (0 == Tests_Failed) {
    printf("\nAll %d tests passed.\n", Tests_Passed);
  } else {
    printf("\nFAILED %d of %d tests.\n", Tests_Failed, Tests_Failed+Tests_Passed);
  }
  
  printf("\n*** Tests complete.\n");  
  return 0;
}