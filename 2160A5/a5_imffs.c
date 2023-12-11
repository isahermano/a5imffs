/**
 * a5_imffs.c
 *
 * COMP 2160 SECTION A02
 * INSTRUCTOR    John Braico
 * ASSIGNMENT    Assignment 5, part b)
 * AUTHOR        Isabella Hermano, 7967075
 * DATE          December 12, 2023
 *
 * PURPOSE: To implement an in flat file system
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "a5_multimap.h"
#include "a5_imffs.h"

#define BLOCK_SIZE 256

struct KEY { 
    void *file_name;
    u_int32_t file_size;
};

struct IMFFS {
    Multimap *index;
    u_int8_t *device;
    u_int8_t *free_blocks;
    uint32_t block_count;
};


int compare_keys_alphabetically(void *a, void *b) {
    struct KEY *first = (struct KEY *)a;
    struct KEY *second = (struct KEY *)b;
    return strcasecmp(first->file_name, second->file_name);
}

int compare_values_num_part(void *a, void *b) {
  Value *va = a, *vb = b;
  return va->num - vb->num;
}

// helper function that searches the free_blocks array for an index
// correlating to the first spot of a free space, indicated by 0
int get_free_index(IMFFSPtr fs, int curr) {
    assert(fs != NULL);
    assert(curr >= 0);

    if(fs == NULL || curr < 0) {
        return IMFFS_INVALID;
    }

    for(int i = 0; i < fs->block_count; i++) {
        if(fs->free_blocks[i] == 0) {
            return i; // return index if found
        }
    }
    return -1; // return -1
}

// this function will create the filesystem with the given number of blocks;
// it will modify the fs parameter to point to the new file system or set it
// to NULL if something went wrong
IMFFSResult imffs_create(uint32_t block_count, IMFFSPtr *fs) {
    assert(block_count > 0);

    if(block_count <= 0) {
        return IMFFS_INVALID;
    }

    // change pointer to point to struct; malloc it once
    *fs = (IMFFSPtr)malloc(sizeof(struct IMFFS));

    if(NULL == *fs) {
        return IMFFS_NOT_IMPLEMENTED;
    }

    // allocate memory for components of IMFFs
    (*fs)->index = mm_create(block_count, compare_keys_alphabetically, compare_values_num_part);
    (*fs)->free_blocks = (u_int8_t *)malloc(block_count * sizeof(u_int8_t));
    (*fs)->device = (u_int8_t *)malloc(block_count * sizeof(u_int8_t) * BLOCK_SIZE);
    (*fs)->block_count = block_count;

    if(NULL == (*fs)->index || NULL == (*fs)->free_blocks || NULL == (*fs)->device) {
        mm_destroy((*fs)->index);
        free((*fs)->free_blocks);
        free((*fs)->device);
        free(*fs);
        return IMFFS_FATAL;
    }

    // intialize each entry in free_block array as 0 to indicate it is empty
    for(u_int8_t i = 0; i < block_count; i++) {
        (*fs)->free_blocks[i] = 0;
    }

    return IMFFS_OK;
}

// save diskfile imffsfile copy from your system to IMFFS
IMFFSResult imffs_save(IMFFSPtr fs, char *diskfile, char *imffsfile) {
    assert(NULL != fs && NULL != diskfile && NULL != imffsfile);
    
    if(NULL == fs || NULL == diskfile || NULL == imffsfile) {
        return IMFFS_INVALID;
    }

    FILE *in = fopen(diskfile, "rb");
    FILE *out = fopen(imffsfile, "wb");

    // check if error occurs opening file
    if(NULL == in || NULL == out) {
        printf("Error opening files.\n");
        return IMFFS_ERROR;
    }

    // allocate memory for key struct
    struct KEY *key = (struct KEY *)malloc(sizeof(struct KEY));

    if(key == NULL) {
        printf("Error allocating memory.\n");
        free(key);
        fclose(in);
        fclose(out);
        return IMFFS_ERROR;
    }

    // get size of file
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    // assign file name and file size to key
    key->file_name = malloc(strlen(imffsfile) + 1);
    strcpy(key->file_name, imffsfile);
    key->file_size = file_size;

    u_int8_t *chunk_index_ptr;
    size_t bytes_read, bytes_written;
    int num_blocks = 0;
    int start = 0;

    // find index of free block
    int index = get_free_index(fs, start);
    int first_pos = index;

    if(index < 0) {
        printf("IMMFS is full, unable to enter any more data.\n");
        free(key);
        fclose(in);
        fclose(out);
        return IMFFS_ERROR;
    }

    u_int8_t buffer[BLOCK_SIZE];
    bytes_read = fread(buffer, sizeof(u_int8_t), BLOCK_SIZE, in);
    
    //keep reading file until eof
    while(bytes_read > 0) {
        // write data from diskfile to fs->device 
        bytes_written = fwrite(buffer, sizeof(u_int8_t), bytes_read, out);
        memcpy(&fs->device[index * BLOCK_SIZE], buffer, bytes_read);
        
        // mark spot in freeblocks array as occupied
        fs->free_blocks[index] = 1;

        if(bytes_read != bytes_written) {
            printf("Error writing to a file.\n");
            free(key);
            fclose(in);
            fclose(out);
            return IMFFS_ERROR;
        }

        // increment number of blocks
        num_blocks++;

        if(num_blocks > 1) {
            // check if next free_block is empty
            if(fs->free_blocks[index + 1] == 0) {   
                index++;
            } else {
                // pointer to an int that identifies a block by index
                chunk_index_ptr = &fs->device[first_pos];
                num_blocks = 0;
                start++;
                index = get_free_index(fs, start);
                start = index;
            }
        }
        bytes_read = fread(&fs->device[index * BLOCK_SIZE], sizeof(u_int8_t), BLOCK_SIZE, in);
    }
    chunk_index_ptr = &fs->device[first_pos];
    mm_insert_value(fs->index, key, num_blocks, chunk_index_ptr);

    fclose(in);
    fclose(out);

    return IMFFS_OK;
}

// load imffsfile diskfile copy from IMFFS to your system
IMFFSResult imffs_load(IMFFSPtr fs, char *imffsfile, char *diskfile) {
    assert(NULL != fs && NULL != diskfile && NULL != imffsfile);
    
    if(NULL == fs || NULL == diskfile || NULL == imffsfile) {
        return IMFFS_INVALID;
    }

    FILE *out = fopen(diskfile, "wb");
      
    if(NULL == out) {
        printf("Error opening file.\n");
        return IMFFS_ERROR;
    }
    
    struct KEY *key_data;
    int key_found = 0;
    int file_size = 0;

    // search for the key in index
    void *key;
    if (mm_get_first_key(fs->index, &key) > 0) {
      do {
        key_data = (struct KEY *)key;

        if(strcmp((char *)key_data->file_name, imffsfile) == 0) {
            key_found = 1;
            file_size = key_data->file_size;
            break;
        }
      } while (mm_get_next_key(fs->index, &key) > 0);
    }

    if(key_found == 0) {
        printf("Error: Key not found.\n");
        fclose(out);
        return IMFFS_ERROR;
    }

    int num_values = mm_count_values(fs->index, key_data); 
    int num_blocks = 0;
    Value values[num_values];
    int result = mm_get_values(fs->index, key_data, values, num_values);

    if(result == -1) {
        printf("Error getting values.\n");
        fclose(out);
        return IMFFS_ERROR;
    }

    size_t bytes_written = 0;

    for(int i = 0; i < num_values; i++) {
        num_blocks = values[i].num;
        u_int8_t *chunk_index_ptr = values[i].data;
        size_t chunk = (chunk_index_ptr - fs->device) / sizeof(u_int8_t);
        printf("num blocks: %d\n", num_blocks);
        for(int j = 0; j < num_blocks; j++) {
            if(file_size >= BLOCK_SIZE) {
                bytes_written += fwrite(&fs->device[(chunk + j) * BLOCK_SIZE], sizeof(u_int8_t), BLOCK_SIZE, out);
                file_size -= BLOCK_SIZE;
            } else {
                bytes_written += fwrite(&fs->device[(chunk + j) * BLOCK_SIZE], sizeof(u_int8_t), file_size, out);
            }
        }
    }

    if(bytes_written < 0) {
        printf("Error writing to file.\n");
        return IMFFS_ERROR;
    }

    fclose(out);
    return IMFFS_OK;
}


// delete imffsfile remove the IMFFS file from the system, allowing the blocks to be used for other files
IMFFSResult imffs_delete(IMFFSPtr fs, char *imffsfile) {
    assert(NULL != fs && NULL != imffsfile);
    
    if(NULL == fs || NULL == imffsfile) {
        return IMFFS_INVALID;
    }

    int key_found = 0;

    // search for the key in index

    void *key;
    if (mm_get_first_key(fs->index, &key) > 0) {
      do {
        struct KEY *key_data = (struct KEY *)key;

        if(strcmp((char *)key_data->file_name, imffsfile) == 0) {
            key_found = 1;
            break;
        }
      } while (mm_get_next_key(fs->index, &key) > 0);
    }

    if(key_found == 0) {
        printf("Error, key not found\n");
        return IMFFS_ERROR;
    }

    // getting values
    int num_values = mm_count_values(fs->index, key);
    Value values[num_values];

    int result = mm_get_values(fs->index, key, values, num_values);

    if(result == -1) {
        printf("Error getting values.\n");
        return IMFFS_ERROR;
    }

    for(int i = 0; i < num_values; i++) {
        int num_blocks = values[i].num;
        u_int8_t *chunk_index_ptr = values[i].data;
        size_t chunk = (chunk_index_ptr - fs->device) / sizeof(u_int8_t);
        for(int j = 0; j < num_blocks; j++) {
            fs->free_blocks[chunk + j] = 0;
        }
    }

    // remove key from multimap
    int removed = mm_remove_key(fs->index, key);

    if(removed == -1) {
        printf("Error removing the key.\n");
        return IMFFS_ERROR;
    }

    return IMFFS_OK;
}

// rename imffsold imffsnew rename the IMFFS file  from imffsold to imffsnew, keeping all of the data intact
IMFFSResult imffs_rename(IMFFSPtr fs, char *imffsold, char *imffsnew) {
    assert(NULL != fs && NULL != imffsold && NULL != imffsnew);
    
    if(NULL == fs || NULL == imffsold || NULL == imffsnew) {
        return IMFFS_INVALID;
    }

    int key_found = 0;

    // search for the key in index
    void *key;
    if (mm_get_first_key(fs->index, &key) > 0) {
      do {
        struct KEY *key_data = (struct KEY *)key;
        if(strcmp((char *)key_data->file_name, imffsold) == 0) {
            key_data->file_name = imffsnew;
            key_found = 1;
            break;
        }
      } while (mm_get_next_key(fs->index, &key) > 0);
    }

    if(key_found == 0) {
        printf("Error, key not found.\n");
        return IMFFS_ERROR;
    }
    
    return IMFFS_OK;
}

// dir will list all of the files and the number of bytes they occupy
IMFFSResult imffs_dir(IMFFSPtr fs) {
    assert(NULL != fs);

    if(NULL == fs) {
        printf("Error: IMFFS does not exist.\n");
        return IMFFS_INVALID;
    }

    printf("\n----------IMFFS Directory----------\n\n");

    void *key;
    if (mm_get_first_key(fs->index, &key) > 0) {
      do {
        struct KEY *key_data = (struct KEY *)key;
        printf("File Name: %s (%d bytes)\n", (char *)key_data->file_name, key_data->file_size);
      } while (mm_get_next_key(fs->index, &key) > 0);
    }
    return IMFFS_OK;
}

// fulldir is like "dir" except it shows a the files and details about all of the chunks they are stored in (where, and how big)
IMFFSResult imffs_fulldir(IMFFSPtr fs) {
    assert(NULL != fs);

    if(NULL == fs) {
        printf("Error: IMFFS does not exist.\n");
        return IMFFS_INVALID;
    }

    printf("\n----------IMFFS Full Directory----------\n\n");

    void *key;
    if (mm_get_first_key(fs->index, &key) > 0) {
      do {
        int num_values = mm_count_values(fs->index, key);
        Value values[num_values];
        mm_get_values(fs->index, key, values, num_values);

        struct KEY *key_data = (struct KEY *)key;
        for(int i = 0; i < num_values; i++) {
            u_int8_t *chunk_index_ptr = values[i].data;
            size_t chunk = (chunk_index_ptr - fs->device) / sizeof(u_int8_t);

            printf("File Name: %s (%d bytes) Blocks: %d Chunk: %zu\n", (char *)key_data->file_name, key_data->file_size, values[i].num, chunk);
        }
      } while (mm_get_next_key(fs->index, &key) > 0);
    }
    
    return IMFFS_OK;
}

// defrag will defragment the filesystem: if you haven't implemented it, have it print "feature not implemented" and return IMFFS_NOT_IMPLEMENTED
IMFFSResult imffs_defrag(IMFFSPtr fs) {
    printf("Feature not implemented.");

    return IMFFS_NOT_IMPLEMENTED;
}

// quit will quit the program: clean up the data structures
IMFFSResult imffs_destroy(IMFFSPtr fs) {
    assert(NULL != fs);

    if(NULL == fs) {
        printf("Error: IMFFS does not exist.\n");
        return IMFFS_ERROR;
    }
    
    mm_destroy(fs->index);
    free(fs->device);
    free(fs->free_blocks);
    free(fs);

    return IMFFS_OK;
}

