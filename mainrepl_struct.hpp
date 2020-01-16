#ifndef HEADERS
#define HEADERS
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<stdint.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include "mainrepl_enum.hpp"
#endif

#ifndef STRUCT
#define STRUCT
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE    255
#define TABLE_MAX_PAGES      100

//input buffer to read the cmds
typedef struct{
    char* buffer;
    size_t buffer_length;
    size_t input_length;
}InputBuffer;

//row to store- user ID, user name, email ID
typedef struct{
    uint32_t    id;
    char        username[COLUMN_USERNAME_SIZE + 1];
    char        email[COLUMN_EMAIL_SIZE + 1];
}Row;

//read cmds >> statements (insert, select)
typedef struct{
    StatementType   type;
    Row             row_to_insert;
}Statement;

//struct to hold page info, file info
typedef struct {
    int         file_descriptor;
    uint32_t    file_length;
    uint32_t    num_pages;
    void*       pages[TABLE_MAX_PAGES];
}Pager;

//struct to hold db table
typedef struct {
    uint32_t root_page_num;
    Pager*   pager;
}Table;

//struct to store the cursor
typedef struct{
    Table*      table;
    uint32_t    page_num;
    uint32_t    cell_num;
    bool        end_of_table;
}Cursor;

//need to use g++ compiler as in C below definitions are not supported
//c requires them to be compile time constants
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
const uint32_t ID_SIZE          = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE    = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE       = size_of_attribute(Row, email);
const uint32_t ID_OFFSET        = 0;
const uint32_t USERNAME_OFFSET  = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET     = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE         = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE        = 4096;
const uint32_t ROWS_PER_PAGE    = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS   = TABLE_MAX_PAGES * ROWS_PER_PAGE;


//db constants with btree
//common node header format
const uint32_t NODE_TYPE_SIZE           = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET         = 0;
const uint32_t IS_ROOT_SIZE             = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET           = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE      = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET    = IS_ROOT_OFFSET + IS_ROOT_SIZE; 
const uint8_t  COMMON_NODE_HEADER_SIZE  = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

//leaf node header format
const uint32_t LEAF_NODE_NUM_CELLS_SIZE     = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET   = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE        = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

//leaf node body format
const uint32_t LEAF_NODE_KEY_SIZE           = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET         = 0;
const uint32_t LEAF_NODE_VALUE_SIZE         = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET       = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE          = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS    = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS          = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;
#endif