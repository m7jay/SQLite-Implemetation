#ifndef HELPERS
#define HELPERS
#include "mainrepl_struct.hpp"
#include "b+tree.hpp"
/*
*function to create a new input buffer
*@returns a pointer to new input buffer
*/
InputBuffer* new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

/*
*prints db prompt
*/
void print_prompt(){
    printf("db > ");
}

/*
*function read input from input_buffer
*@param     input_buffer    buffer from which the input should be read
*/
void read_input(InputBuffer* input_buffer){
    //getline(pointer_to_buffer, buffer_length, input_stream)
    size_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
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
*function to flush the data to the db file
*@param     pager       pointer to Pager with file handle and the pages info
*@param     page_num    page number of the page to be flushed
*/
void pager_flush(Pager* pager, uint32_t page_num){
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
    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);

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

    for(uint32_t i = 0; i < pager->num_pages; i++){
      
        if(pager->pages[i] == NULL)
            continue;
        pager_flush (pager, i);
        free (pager->pages[i]);
        pager->pages[i] = NULL;
    }

  /*  //partial pages in the end
    uint32_t num_additional_pages = table->num_rows % ROWS_PER_PAGE;
    if(num_additional_pages > 0){
        uint32_t page_num = num_full_pages;
        if(pager->pages[page_num] != NULL){

            pager_flush(pager, page_num, num_additional_pages * ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }
*/
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
*@param     page_num   page number of the page to read 
*@returns a pointer to the page
*/
void* get_page(Pager* pager, uint32_t page_num){

    printf("page-num = %d.\n", page_num);
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

        printf("adding-%d\n", page_num);
        pager->pages[page_num] = page;
        printf("page\n");

        if(page_num >= pager->num_pages)
            pager->num_pages = page_num + 1;

    }

    return pager->pages[page_num];
}

/*
*function to get the cursor at the end of the table
*@param     table      pointer to the Table
*@returns a pointer to the Cursor 
*/
Cursor* table_end(Table* table){ //not used
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));

    cursor->table        = table;
    cursor->page_num     = table->root_page_num;

    void* root_node      = get_page(table->pager, table->root_page_num);
    uint32_t num_cells   = *(leaf_node_num_cells(root_node));

    cursor->cell_num     = num_cells;    
    cursor->end_of_table = true;

    return cursor; 
}

/*
*function to print the correct indentations
*/
void indent(uint32_t level){
    for(uint32_t i = 0 ; i < level; i++)
        printf("  ");
}

/*
*function to recursively print a node and its childrens
*@param pager
*@param page_num
*@param indentation_level
*/
void print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level){
    void* node = get_page(pager, page_num);
    uint32_t num_keys, child;

    switch(get_node_type(node)){
        case NODE_LEAF:
            num_keys = *(leaf_node_num_cells(node));
            indent(indentation_level);
            printf("- leaf %d\n", num_keys);
            for(uint32_t i = 0; i < num_keys; i++){
                indent(indentation_level+1);
                printf("- %d\n", *leaf_node_key(node, i));
            }
            break;
        case NODE_INTERNAL:
            num_keys = *(internal_node_num_keys(node));
            indent(indentation_level);
            printf("- internal %d\n", num_keys);
            for(uint32_t i = 0; i < num_keys; i++){
                child = *internal_node_child(node, i);
                print_tree(pager, child, indentation_level+1);
                indent(indentation_level+1);
                printf("- key %d\n", *internal_node_key(node, i));
            }
            child = *(internal_node_right_child(node));
            print_tree(pager, child, indentation_level+1);
            break;
    }
}

/*
*function to execute meta commmands
*@param     input_buffer    pointer to the input buffer
*@param     table           pointer to table
*@returns an enum of type MetaCmdResult
*/
MetaCmdResult do_meta_command(InputBuffer* input_buffer, Table* table){

    if(strcmp(input_buffer->buffer,".exit") == 0){
        db_close(table);
        exit(EXIT_SUCCESS);
    }

    else if(strcmp(input_buffer->buffer, ".constants") == 0){
        print_constants();
        return META_SUCCESS;
    }

    else if(strcmp(input_buffer->buffer,".btree") == 0){
        //print_leaf_node(get_page(table->pager,0));
        print_tree(table->pager, 0, 1);
        return META_SUCCESS;
    }

    else
        return META_FAILURE;
}

/*
*function for inserting a row
*reads from input buffer and insert to the table
*@param     source          source pointer of type Row
*@param     destination     destination pointer of type void
*/
void serialize_row(Row* source, void* destination){
    memcpy((destination + ID_OFFSET), &(source->id), ID_SIZE);
    memcpy((destination + USERNAME_OFFSET), &(source->username),USERNAME_SIZE);
    memcpy((destination + EMAIL_OFFSET), &(source->email), EMAIL_SIZE);
    //if the enteres string is smaller than the size, then fill the remaining slots as NULL
//    strncpy((destination + USERNAME_OFFSET), source->username, USERNAME_SIZE);
//    strncpy((destination + EMAIL_OFFSET), source->email, EMAIL_SIZE);
}

/*
*function for read a row
*read from the table and store it in a Row 
*@param     source          source pointer of type void
*@param     destination     destination pointer of type Row
*/
void deserialize_row(void* source, Row* destination){
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email),source + EMAIL_OFFSET, EMAIL_SIZE);
}

/*
*function to get the value of the node pointed by the cursor
*@param     cursor      pointer to the cursor
*@returns a pointer to node value
*/
void* cursor_value(Cursor* cursor){
    void * page = get_page(cursor->table->pager, cursor->page_num);
    return leaf_node_value(page, cursor->cell_num);
}

/*
*function to advance the cursor to the next row
*@param     cursor      pointer to the cursor to be advanced
*/
void advance_cursor(Cursor* cursor){
    
    void* node = get_page(cursor->table->pager, cursor->page_num);
    cursor->cell_num += 1;

    if(cursor->cell_num >= *(leaf_node_num_cells(node))){
        uint32_t next_page_num = *leaf_node_next_leaf(node);
        if(next_page_num == 0)
            cursor->end_of_table = true;
        else{
            cursor->page_num = next_page_num;
            cursor->cell_num = 0;
        }
    }
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
*until recycling of the free pages is implemented, new pages will be added to the end of the db file
*@param pager   pointer to pager
*@returns next page number
*/
uint32_t get_unused_page_num(Pager* pager){
    return pager->num_pages;
}

/*
*we have already allocated the rigth child, 
*this function takes the rigth child as input 
*and allocates a new page to store the left child
*@param table                   pointer to table
*@param right_child_page_num    right child page number
*/
void create_new_root(Table* table, uint32_t right_child_page_num){
    /*
    *Splitting the root is handled by
    *old root is copied to the new page, becomes the left child
    *address of the rigth child is passed in 
    *reinitialize the root page to contain the new root node
    *new root node points to two children 
    */
   void* root                   = get_page(table->pager, table->root_page_num);
   void* right_child            = get_page(table->pager, right_child_page_num);
   uint32_t left_child_page_num = get_unused_page_num(table->pager);
   void* left_child             = get_page(table->pager, left_child_page_num); 

    //copy old node as left child
    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    //root node is new internal node with one key and two children
    initialize_internal_node(root);
    set_node_root(root, true);
    *(internal_node_num_keys(root))    = 1;
    *(internal_node_child(root, 0))    = left_child_page_num;
    *(internal_node_key(root, 0))      = get_node_max_key(left_child);
    *(internal_node_right_child(root)) = right_child_page_num; 
}

/*
*Create a new node and move half of the cells over,
*Insert the new value to one of the two nodes,
*update parent or create a new parent
*@param cursor      cursor pointing to the position where the new node to be inserted
*@param key         key of the new node to be inserted
*@param value       pointert to the value to be inserted
*/
void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value){

    void* old_node          = get_page(cursor->table->pager, cursor->page_num);
    uint32_t new_page_num   = get_unused_page_num(cursor->table->pager);
    void* new_node          = get_page(cursor->table->pager, new_page_num);
    initialize_leaf_node(new_node);

    //the old leafs sibling becomes the new leaf 
    //and the new leaf's sibling becomes whatever used to be the old leaf's sibling
    *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
    *leaf_node_next_leaf(old_node) = new_page_num;

    /*
    *All exisiting keys + new key should be 
    *divided evenly between old (left) and the new(right) nodes.
    * starting from the right, move each key to correct position. 
    */
    for(int32_t i =LEAF_NODE_MAX_CELLS; i >= 0; i--){
        void* destination_node;
        if(i >= LEAF_NODE_LEFT_SPLIT_COUNT)
            destination_node = new_node;
        else
            destination_node = old_node;

        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void* destination = leaf_node_cell(destination_node, index_within_node);

        if(i == cursor->cell_num){
            serialize_row(value, leaf_node_value(destination_node, index_within_node));
            *leaf_node_key(destination_node, index_within_node) = key;
        }
        else if(i > cursor->cell_num)
            memcpy(destination, leaf_node_cell(old_node, i-1), LEAF_NODE_CELL_SIZE);
        else
            memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }
    //update the cell count on left and rigth child nodes
    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    //update the parent node, if the parent was a root node, create a new root node
    if(is_root_node(old_node))
        return create_new_root(cursor->table, new_page_num);
    else{
        printf("Need to implement updating parent after split\n");
        exit(EXIT_FAILURE);
    }

}

/*
*insert a row based on the key
*@param cursor  pointer the the cursor
*@param key     key to be inserted
*@param value   value of type Row to be inserted
*/
void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value){
    void* node = get_page(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *(leaf_node_num_cells(node));

    //check if the node is already having max cells
    if(num_cells >= LEAF_NODE_MAX_CELLS){
        //Node full
//        printf("Need to implement splitting a leaf node.\n");
//        exit(EXIT_FAILURE);
        leaf_node_split_and_insert(cursor, key, value);
        return;
    }

    //check if the cell num is less than the number of cells in the node
    //if yes shift the cells to right by leaf node size
    if(cursor->cell_num < num_cells){
        for(uint32_t i=num_cells; i > cursor->cell_num; i--){
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i-1), LEAF_NODE_CELL_SIZE);
        }
    }

    //increment the number of cells for the node
    *(leaf_node_num_cells(node)) += 1;
    //set the key for the cell
    *(leaf_node_key(node, cursor->cell_num)) = key;

    serialize_row(value, leaf_node_value(node, cursor->cell_num));

}

/*
*function to find the position of the key or the position of the key to be moved or the one past the last key
*@param table       pointer to the table
*@param page_num    page number of the page to search
*@param key         key to be find
*/
Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key){
    
    printf("in leaf node find\n");
    void* node          = get_page(table->pager, page_num);
    uint32_t num_cells  = *(leaf_node_num_cells(node));

    Cursor* cursor      = (Cursor*)malloc(sizeof(Cursor));
    cursor->table       = table;
    cursor->page_num    = page_num;

    uint32_t min        = 0;
    uint32_t max        = num_cells;
    //binary search
    while(min != max){
        uint32_t index          = (min + max)/2;
        uint32_t key_at_index   = *(leaf_node_key(node, index));

        if(key_at_index == key){
            cursor->cell_num = index;
            return cursor;
        }
        else if(key_at_index < key)
            max = index;
        else
            min = index + 1; 
//        printf("stuck");
    }

    cursor->cell_num = min;
    return cursor;
}
/*
*function to search the internal nodes and find a position for insertion
*@param table       pointer to table
*@param page_num    page number of the page to search
*@param key         key to search
*/
Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key){
    printf("in internal node find\n");
    void*       node          = get_page(table->pager, page_num);
    uint32_t    num_keys      = *internal_node_num_keys(node);
    uint32_t    min           = 0;
    uint32_t    max           = num_keys;   
    //binary search
    while(min != max){
        uint32_t index = (min + max)/2;
        uint32_t key_at_index = *internal_node_key(node, index);
        if(key_at_index >= key)
            max = index;
        else
            min = index + 1;
    }

    uint32_t child_num = *internal_node_child(node, min);
    void*    child     = get_page(table->pager, child_num);
    //node can have either left or right child or more internal nodes
    switch(get_node_type(child)){
        case NODE_LEAF:
            return leaf_node_find(table, child_num, key);
        case NODE_INTERNAL:
            return internal_node_find(table, child_num, key);
    }
}
/*
*function to find the position to insert a node
*@param table   pointer to the table
*@key   key     key to be inserted
*@returns the positions of the key passed, if key not present returns the position where it should be inserted.
*/
Cursor* table_find(Table* table, uint32_t key){
    uint32_t root_page_num = table->root_page_num;
    void* root_node = get_page(table->pager, root_page_num);
    printf("get page in table find\n");
    if(get_node_type(root_node) == NODE_LEAF)
        return leaf_node_find(table, root_page_num, key);
    else
        return internal_node_find(table, root_page_num, key);
}

/*
*function to get the cursor at the start of the table
*@param     table      pointer to the Table
*@returns a pointer to the Cursor 
*/
Cursor* table_start(Table* table){
    Cursor* cursor = table_find(table, 0);
    printf("table find in table start\n");
    void* node = get_page(table->pager, cursor->page_num);
    printf("get page in table start\n");
    uint32_t num_cells = *leaf_node_num_cells(node);
    cursor->end_of_table = (num_cells == 0);
    printf(" page num :%d  cell num:%d end of table:%d",cursor->page_num, cursor->cell_num, cursor->end_of_table );
    return cursor;
}

/*
*function to execute insert operation
*@param     statement       pointer to statement to be executed
*@param     table           pointer to the table on which the statements should be executed
*@returns an enum of type ExecuteResult
*/
ExecuteResult execute_insert (Statement* statement, Table* table){

    void*       node        = get_page (table->pager, table->root_page_num);
    uint32_t    num_cells   = *(leaf_node_num_cells (node));

    //check if table is full
//    if (num_cells >= LEAF_NODE_MAX_CELLS) 
//        return EXECUTE_TABLE_FULL;

    //Cursor* cursor = table_end (table);
    uint32_t    key     = statement->row_to_insert.id;
    Cursor*     cursor  =  table_find(table, key);

    if(cursor->cell_num < num_cells){
        uint32_t ket_at_cursor = *leaf_node_key(node, cursor->cell_num);
        if(ket_at_cursor == key)
            return EXECUTE_DUPLICATE_KEY;
    }

    leaf_node_insert (cursor, key, &(statement->row_to_insert));

    free (cursor);
    return EXECUTE_SUCCESS;
    
}

/*
*function to execute select operation
*@param     statement       pointer to statement to be executed
*@param     table           pointer to the table on which the statements should be executed
*@returns an enum of type ExecuteResult
*/
ExecuteResult execute_select(Statement* statement, Table* table){
    Row row;
    Cursor* cursor = table_start(table);
    printf("table start\n");
    while(cursor->end_of_table != true){
        deserialize_row(cursor_value(cursor),&row);
        print_row(&row);
        printf("adv cur\n");
        advance_cursor(cursor);
    }

    free(cursor);
    return EXECUTE_SUCCESS;
}

/*
*function to open the pager, opens the file passed, initialize the page handle & pages
*@param     filename    name of the file to be opened
*@returns a pointer to the pager with file handles and the pages
*/
Pager* open_pager(const char* filename){

    //if file exists open it in read/write mode else create the file
    int fd = open(filename, O_RDWR | O_CREAT);

    if(fd == -1){//failed to open/create file
        printf("Unable to open file\n.");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd,0,SEEK_END);//find the length of the file
    
    Pager* pager    = (Pager*)malloc(sizeof(Pager));
    pager->file_descriptor  = fd;
    pager->file_length      = file_length;
    pager->num_pages        = file_length/PAGE_SIZE;

    if(file_length % PAGE_SIZE != 0){
        printf("Number of pages in the db is not a whole number.\n Corrupt file.\n");
        exit(EXIT_FAILURE);
    }
    
    for(uint32_t i = 0; i< TABLE_MAX_PAGES; i++)
        pager->pages[i] = NULL;

    return pager;
}

/*
*function to open the db
*@param     filename    name of the db file
*@returns pointer to the table 
*/
Table* open_db(const char* filename){
    Pager*      pager    = open_pager(filename);
    Table*      table    = (Table*)malloc(sizeof(Table));
    table->pager         = pager;
    table->root_page_num = 0;

    if(pager->num_pages == 0){
        //new db file, intialize the page 0 as the leaf node
        void* root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
    }

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
*@param     table       pointer to the table
*@returns execution results of type ExecuteResult
*/
ExecuteResult execute_statement(Statement* statement, Table* table){
    printf("root-page-num: %d.\n", table->root_page_num);
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

#endif