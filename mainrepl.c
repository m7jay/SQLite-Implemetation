#include "repl_helpers.h"

int main(int argc, char* argv[]){
    InputBuffer* input_buffer = new_input_buffer();
    Table* table = new_table();
    while(true){
        print_prompt();
        read_input(input_buffer);

        if(input_buffer->buffer[0] == '.'){
            switch(do_meta_command(input_buffer)){
                case META_SUCCESS:
                    continue;
                case META_FAILURE:
                    printf("Unrecognized command '%s'.\n",input_buffer->buffer);
                    continue;
            }
        }

    Statement statement;
    switch (prepare_statement(input_buffer,&statement))
    {
    case PREPARE_SUCCESS:
        break;
    case PREPARE_FAILURE:
        printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
        continue;
    case PREPARE_SYNTAX_ERROR:
        printf("Syntax error. Could not parse the statement.\n");
        continue;
    }
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