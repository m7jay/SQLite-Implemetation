#ifndef HEADERS
#define HEADERS
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<stdint.h>
#include "mainrepl_enum.h"
#endif

#ifndef STRUCT
#define STRUCT
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE    255
#define TABLE_MAX_PAGES      100


typedef struct{
    char* buffer;
    size_t buffer_length;
    size_t input_length;
}InputBuffer;

typedef struct{
    uint32_t    id;
    char        username[COLUMN_USERNAME_SIZE];
    char        email[COLUMN_EMAIL_SIZE];
}Row;

typedef struct{
    StatementType   type;
    Row             row_to_insert;
}Statement;

typedef struct {
    uint32_t num_rows;
    void*    pages[TABLE_MAX_PAGES];
}Table;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
// const uint32_t ID_SIZE          = size_of_attribute(Row, id);
// const uint32_t USERNAME_SIZE    = size_of_attribute(Row, username);
// const uint32_t EMAIL_SIZE       = size_of_attribute(Row, email);
// const uint32_t ID_OFFSET        = 0;
// const uint32_t USERNAME_OFFSET  = ID_OFFSET + ID_SIZE;
// const uint32_t EMAIL_OFFSET     = USERNAME_OFFSET + USERNAME_SIZE;
// const uint32_t ROW_SIZE         = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
// const uint32_t PAGE_SIZE        = 4096;
// const uint32_t ROWS_PER_PAGE    = PAGE_SIZE / ROW_SIZE;
// const uint32_t TABLE_MAX_ROWS   = TABLE_MAX_PAGES * ROWS_PER_PAGE;
#define ID_SIZE           size_of_attribute(Row, id)
#define USERNAME_SIZE     size_of_attribute(Row, username)
#define EMAIL_SIZE        size_of_attribute(Row, email)
#define ID_OFFSET         0
#define USERNAME_OFFSET   ID_OFFSET + ID_SIZE
#define EMAIL_OFFSET      USERNAME_OFFSET + USERNAME_SIZE
#define ROW_SIZE          ID_SIZE + USERNAME_SIZE + EMAIL_SIZE
#define PAGE_SIZE         4096
#define ROWS_PER_PAGE     PAGE_SIZE / ROW_SIZE
#define TABLE_MAX_ROWS    TABLE_MAX_PAGES * ROWS_PER_PAGE

#endif