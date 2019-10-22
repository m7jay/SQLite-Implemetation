#include "repl_helpers.h"
//g++ mainrepl_enum.h mainrepl_struct.h repl_helpers.h mainrepl.c
int main(int argc, char* argv[]){

    //read filename to store the db through cmd line args
    if(argc < 2){
        printf("Must supply database file name.\n");
        exit(EXIT_FAILURE);
    }

    //create input buffer and db files (if already exists, open that file)
    char* filename = argv[1];
    InputBuffer* input_buffer = new_input_buffer();
    Table* table = open_db(filename);

    while(true){
        print_prompt();
        read_input(input_buffer);

        //meta commands handling (begins with '.')
        if(input_buffer->buffer[0] == '.'){
            switch(do_meta_command(input_buffer, table)){
                case META_SUCCESS:
                    continue;
                case META_FAILURE:
                    printf("Unrecognized command '%s'.\n",input_buffer->buffer);
                    continue;
            }
        }

    //prepare the read statements
    Statement statement;
    switch (prepare_statement(input_buffer,&statement))
    {
    case PREPARE_SUCCESS:
        break;
    case PREPARE_FAILURE:
        printf("Error:Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
        continue;
    case PREPARE_SYNTAX_ERROR:
        printf("Error:Syntax error. Could not parse the statement.\n");
        continue;
    case PREPARE_STRING_TOO_LONG:
        printf("Error:String length too long!.\n");
        continue;
    case PREPARE_NEGATIVE_ID:
        printf("Error:Id must be positive.\n");
    }
    //execute the read statements
    switch(execute_statement(&statement, table)){
        case EXECUTE_SUCCESS:
            printf("Executed.\n");        
            break;
        
        case EXECUTE_TABLE_FULL:
            printf("Error: Table full.\n");
            break;
        }   
    }
}