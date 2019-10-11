#include "mainrepl_struct.h"
InputBuffer* new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void print_prompt(){
    printf("db > ");
}

void read_input(InputBuffer* input_buffer){
    ssize_t bytes_read = getline(&input_buffer->buffer,&input_buffer->buffer_length,stdin);
    if(bytes_read < 0){
        printf("Error in reading input\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read-1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

MetaCmdResult do_meta_command(InputBuffer* input_buffer){
    if(strcmp(input_buffer->buffer,".exit")==0)
        exit(EXIT_SUCCESS);
    else
        return META_FAILURE;
}

void serialize_row(Row* source, void* destination){
    memcpy(destination + ID_OFFSET, &source->id, ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &source->username,USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &source->email, EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination){
    memcpy(&destination->id, source + ID_OFFSET, ID_SIZE);
    memcpy(&destination->username, source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&destination->email,source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* row_slot(Table* table, uint32_t row_num){
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void * page = table->pages[page_num];
    if(page == NULL){
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }

    uint32_t row_offset  = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset; 
}

void print_row(Row* row){
    if(row == NULL)
        printf("Error: Empty Row.\n");
    else
        printf("{ %d %s %s }\n", row->id, row->username, row->email);
}
ExecuteResult execute_insert(Statement* statement, Table* table){
    if (table->num_rows >= TABLE_MAX_ROWS ) 
        return EXECUTE_TABLE_FULL;
    
    serialize_row(&statement->row_to_insert, row_slot(table,table->num_rows));

    return EXECUTE_SUCCESS;
    
}

ExecuteResult execute_select(Statement* statement, Table* table){
    Row row;
    for(uint32_t i=0; i<table->num_rows;i++){
        deserialize_row(row_slot(table,i),&row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

Table* new_table(){
    Table* table = malloc(sizeof(Table));
    table->num_rows = 0;
    for(uint32_t i=0; i<TABLE_MAX_PAGES;i++)
    table->pages[i] = NULL;
    
    return table;
}

void free_table(Table* table){
    for(int i=0; table->pages[i] && i < TABLE_MAX_PAGES;i++)
        free(table->pages[i]);

    free(table);
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement){
    if(strncmp(input_buffer->buffer,"insert",6)==0){
        statement->type = STATEMENT_INSERT;
        int args = sscanf(input_buffer->buffer,"insert %d %s %s",&statement->row_to_insert.id,&statement->row_to_insert.username,&statement->row_to_insert.email);
        if (args < 3) return PREPARE_SYNTAX_ERROR;
        return PREPARE_SUCCESS;
    }
    else if(strcmp(input_buffer->buffer,"select")==0){
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    else
        return PREPARE_FAILURE;
}

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

