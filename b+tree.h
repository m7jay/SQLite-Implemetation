#ifndef BTREE
#define BTREE
#include "mainrepl_struct.h"

//return number of cells in a node
uint32_t* leaf_node_num_cells(void* node){
    return (uint32_t*)(node + LEAF_NODE_NUM_CELLS_OFFSET);
}

//returns a pointer to a cell in a node
void* leaf_node_cell(void* node, uint32_t cell_num){
    return node + LEAF_NODE_HEADER_SIZE + (cell_num * LEAF_NODE_CELL_SIZE);
}

//returns pointer to the node key
uint32_t* leaf_node_key(void* node, uint32_t cell_num){
    return (uint32_t*)leaf_node_cell(node, cell_num);
}

//returns a pointer to the node value
void* leaf_node_value(void* node, uint32_t cell_num){
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

//initializes the leaf node 
void initialize_leaf_node(void* node){
    *(leaf_node_num_cells(node)) = 0;
}

//prints the constants for the db
void print_constants(){
    printf("------------------DB Constants------------------\n");
    printf("Row size = %d.\n", ROW_SIZE);
    printf("Common node header size = %d.\n",COMMON_NODE_HEADER_SIZE );
    printf("Leaf node header size = %d.\n", LEAF_NODE_HEADER_SIZE);
    printf("Leaf node cell size = %d.\n", LEAF_NODE_CELL_SIZE);
    printf("Leaf node space for cells = %d.\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("Leaf node max number of cells = %d.\n", LEAF_NODE_MAX_CELLS);
    printf("-------------------------------------------------\n");
}

//prints the leaf node passed
void print_leaf_node(void* node){
    uint32_t num_cells = *(leaf_node_num_cells(node));
    printf("------------------DB Tree------------------------\n");
    printf("Leaf node with %d cells:\n", num_cells);
    printf("    < cell num : key >\n");
    for(uint32_t i=0; i < num_cells; i++){
        uint32_t key = *(leaf_node_key(node,i));
        printf("    [ %3d : %d ]  ", i, key);
        if((i+1)%8 == 0) printf("\n");
    }
    printf("\n-------------------------------------------------\n");

}

#endif