#include "mainrepl_struct.h"

/*
* function to create a new input buffer
*
*returns a pointer to new input buffer
*/
InputBuffer* new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

//prints db prompt
void print_prompt(){
    printf("db > ");
}

/*
*function read input from input_buffer
*@param     input_buffer    buffer from which the input should be read
*/
void read_input(InputBuffer* input_buffer){
    //getline(pointer_to_buffer, buffer_length, input_stream)
    unsigned int bytes_read = getline(&input_buffer->buffer,&input_buffer->buffer_length,stdin);
    if(bytes_read < 0){
        printf("Error in reading input\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read-1] = 0;
}

/*
*function to close the input buffer
*@param     input_buffer    buffer to close
*/
void close_input_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

/*
*function to close the pages and store the data to file
*@param     pager       pointer to Pager with file handle and the pages info
*@param     page_num    page number of the page to be removed
*@param     size        size of the page (required for partial pages)
*/
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size){
    if (pager->pages[page_num] == NULL){
        printf("Tried to flush a null page.\n");
        exit(EXIT_FAILURE);
    }

    //seek to the page index on the file
    off_t offset = lseek (pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

    if(offset == -1){
        printf("Error seeking: %d\n.",errno);
        exit(EXIT_FAILURE);
    }

    //write page contents into the file
    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], size);

    if(bytes_written == -1){
        printf("Error writing:%d\n", errno);
        exit(EXIT_FAILURE);
    }

}

/*
*function to close the db
*@param     table       pointer to the Table contents
*/
void db_close(Table* table){
    Pager* pager            = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i< num_full_pages; i++){
      
        if(pager->pages[i] == NULL)
            continue;
        pager_flush (pager, i, PAGE_SIZE);
        free (pager->pages[i]);
        pager->pages[i] = NULL;
    }

    //partial pages in the end
    uint32_t num_additional_pages = table->num_rows % ROWS_PER_PAGE;
    if(num_additional_pages > 0){
        uint32_t page_num = num_full_pages;
        if(pager->pages[page_num] != NULL){

            pager_flush(pager, page_num, num_additional_pages * ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }

    int result = close(pager->file_descriptor);
    if(result == -1){
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i=0; i< TABLE_MAX_PAGES; i++){
        void* page = pager->pages[i];
        if(page){
            free(page);
            pager->pages[i] = NULL;
        }
    }

    free  (pager);
    free (table);
}

/*
*function to get a page
*@param     pager      pointer to Pager, holding file hadles and the pages 
*@param     page_num   page number of the page to be removed 
*returns a pointer to the page
*/
void* get_page(Pager* pager, uint32_t page_num){
    if(page_num > TABLE_MAX_PAGES){
        printf("Tried to fetch page number out of bound. %d > %d\n.",page_num, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    //cache miss, allocate memory and read from file
    if(pager->pages[page_num] == NULL){
        void*       page        = malloc(PAGE_SIZE);
        uint32_t    num_pages   = pager->file_length / PAGE_SIZE;

        //in case of partial pages, add them in the end
        if(pager->file_length % PAGE_SIZE){
            num_pages += 1;
        }

        if(page_num <= num_pages){
            //seek to page offset
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            //read from file to memory
            ssize_t bytes_read  = read(pager->file_descriptor, page, PAGE_SIZE);
            if( bytes_read == -1){
                printf("Error reading file: %d\n.", errno);
                exit(EXIT_FAILURE);
            }
        }

        pager->pages[page_num] = page;

    }

    return pager->pages[page_num];
}

/*
*function to get the cursor at the start of the table
*@param     table      pointer to the Table
*returns a pointer to the Cursor 
*/
Cursor* table_start(Table* table){
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));

    cursor->table           = table;
    cursor->row_num         = 0;
    cursor->end_of_table    = (table->num_rows == 0);

    return cursor;
}
/*
*function to get the cursor at the end of the table
*@param     table      pointer to the Table
*returns a pointer to the Cursor 
*/
Cursor* table_end(Table* table){
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));

    cursor->table        = table;
    cursor->row_num      = table->num_rows;
    cursor->end_of_table = true;

    return cursor; 
}
/*
*function to execute meta commmands
*@param     input_buffer    pointer to the input buffer
*@param     table           pointer to table
*returns an enum of type MetaCmdResult
*/
MetaCmdResult do_meta_command(InputBuffer* input_buffer, Table* table){
    if(strcmp(input_buffer->buffer,".exit")==0){
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    else
        return META_FAILURE;
}

/*
*function for inserting a row
*>>>read from input buffer and inser to end of the table
*@param     source          source pointer of type Row
*@param     destination     destination pointer of type void
*/
void serialize_row(Row* source, void* destination){
    memcpy((destination + ID_OFFSET), &(source->id), ID_SIZE);
//    memcpy((destination + USERNAME_OFFSET), &(source->username),USERNAME_SIZE);
//    memcpy((destination + EMAIL_OFFSET), &(source->email), EMAIL_SIZE);
    //if the enteres string is smaller than the size, then fill the remaining slots as NULL
    strncpy((destination + USERNAME_OFFSET), source->username, USERNAME_SIZE);
    strncpy((destination + EMAIL_OFFSET), source->email, EMAIL_SIZE);
}

/*
*function for read a row
*>>>read from the table and store it in a Row 
*@param     source          source pointer of type void
*@param     destination     destination pointer of type Row
*/
void deserialize_row(void* source, Row* destination){
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email),source + EMAIL_OFFSET, EMAIL_SIZE);
}

/*
*function to get the address of the cursor
*@param     cursor      pointer to the cursor
*returns a pointer to the cursor address
*/
void* cursor_value(Cursor* cursor){
    uint32_t row_num = cursor->row_num;
    int page_num = row_num / ROWS_PER_PAGE;
    void * page = get_page(cursor->table->pager, page_num);
    uint32_t row_offset  = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset; 
}

/*
*function to advance the cursor to the next row
*@param     cursor      pointer to the cursor to be advanced
*/
void advance_cursor(Cursor* cursor){
    cursor->row_num += 1;
    if(cursor->row_num >= cursor->table->num_rows)
        cursor->end_of_table = true;
}

/*
*function to print data from a row
*@param     row     pointer to row to be printed
*/
void print_row(Row* row){
    if(row == NULL)
        printf("Error: Empty Row.\n");
    else
        printf("{ %d %s %s }\n", row->id, row->username, row->email);
}

/*
*function to execute insert 
*@param     statement       pointer to statement to be executed
*@param     table           pointer to the table on which the statements should be executed
*returns an enum of type ExecuteResult
*/
ExecuteResult execute_insert(Statement* statement, Table* table){
    if (table->num_rows >= TABLE_MAX_ROWS ) 
        return EXECUTE_TABLE_FULL;
    
    Cursor* cursor = table_end(table);
    serialize_row(&(statement->row_to_insert),cursor_value(cursor));

    table->num_rows += 1;

    free(cursor);
    return EXECUTE_SUCCESS;
    
}

/*
*function to execute select
*@param     statement       pointer to statement to be executed
*@param     table           pointer to the table on which the statements should be executed
*@returns an enum of type ExecuteResult
*/
ExecuteResult execute_select(Statement* statement, Table* table){
    Row row;
    Cursor* cursor = table_start(table);
    while(cursor->end_of_table != true){
        deserialize_row(cursor_value(cursor),&row);
        print_row(&row);
        advance_cursor(cursor);
    }

    free(cursor);
    return EXECUTE_SUCCESS;
}

/*
*function to open the pager, opens the file passed, initialize the pager & pages
*@param     filename    name of the file to be opened
*returns a pointer to the pager with file handles and the pages
*/
Pager* open_pager(const char* filename){
    //if file exists open it in read/write mode else create the file
    int fd = open(filename, O_RDWR | O_CREAT);

    if(fd == -1){
        printf("Unable to open file\n.");
        exit(EXIT_FAILURE);
    }

    off_t file_legth = lseek(fd,0,SEEK_END);//find the length of the file
    
    Pager* pager    = (Pager*)malloc(sizeof(Pager));
    pager->file_descriptor  = fd;
    pager->file_length      = file_legth;

    for(uint32_t i = 0; i< TABLE_MAX_PAGES; i++)
        pager->pages[i] = NULL;

    return pager;
}

/*
*function to open the db
*@param     filename    name of the file to store the db
*returns to the table 
*/
Table* open_db(const char* filename){
    Pager*      pager       = open_pager(filename);
    uint32_t    num_rows    = pager->file_length / ROW_SIZE; 

    Table* table    = (Table*)malloc(sizeof(Table));
    table->pager    = pager;
    table->num_rows = num_rows;

    return table; 
}

/*
*function to prepare the commands read for execution
*@param     input_buffer        pointer to the input buffer
*@param     statement           pointer to store the statements after preparation
*returns an enum of type PrepareResult
*/
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement){
    if(strncmp(input_buffer->buffer,"insert",6)==0){
        statement->type = STATEMENT_INSERT;
        char* keyword   = strtok(input_buffer->buffer," ");
        char* id_str    = strtok(NULL," ");
        char* username  = strtok(NULL," ");
        char* email     = strtok(NULL," ");    
        if (id_str == NULL || username == NULL || email == NULL)
            return PREPARE_SYNTAX_ERROR;

        int id  = atoi(id_str);
        if(id<0) return PREPARE_NEGATIVE_ID;
        if(strlen(username) > COLUMN_USERNAME_SIZE) return PREPARE_STRING_TOO_LONG;
        if(strlen(email) > COLUMN_EMAIL_SIZE) return PREPARE_STRING_TOO_LONG;

        statement->row_to_insert.id = id;
        strcpy(statement->row_to_insert.username, username);
        strcpy(statement->row_to_insert.email, email);
        return PREPARE_SUCCESS;
    }
    else if(strcmp(input_buffer->buffer,"select")==0){
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    else
        return PREPARE_FAILURE;
}

/*
*function to execute the statements prepared
*@param     statement   pointer to the prepared statements
*@param     table       pointer to the table from which to read
*/
ExecuteResult execute_statement(Statement* statement, Table* table){
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        return execute_insert(statement, table); 
        break;
    
    case STATEMENT_SELECT:
        return execute_select(statement, table);
        break;
    }
}
