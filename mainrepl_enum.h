#ifndef ENUM
#define ENUM
typedef enum {
    META_SUCCESS,
    META_FAILURE
}MetaCmdResult;

typedef enum{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_FAILURE
}PrepareResult;

typedef enum{
    STATEMENT_INSERT,
    STATEMENT_SELECT
}StatementType;

typedef enum{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
    EXECUTE_FAILURE
}ExecuteResult;

#endif