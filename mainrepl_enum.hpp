#ifndef ENUM
#define ENUM

//for executing meta commands like .exit
typedef enum {
    META_SUCCESS,
    META_FAILURE
}MetaCmdResult;

//to indicate prepare function results
typedef enum{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_NEGATIVE_ID,
    PREPARE_FAILURE
}PrepareResult;

//for command types
typedef enum{
    STATEMENT_INSERT,
    STATEMENT_SELECT
}StatementType;

//for execution results
typedef enum{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
    EXECUTE_FAILURE,
    EXECUTE_DUPLICATE_KEY
}ExecuteResult;

//for node types
typedef enum{
    NODE_INTERNAL,
    NODE_LEAF
}NodeType;
#endif