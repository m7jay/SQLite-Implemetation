#ifndef BTREE
#define BTREE
#include "mainrepl_struct.hpp"

/*
*function to check if a node is root node
*@param node    pointer to a node
*@returns true if the node is a root node else returns false
*/
bool is_root_node(void* node){
    uint8_t value = *((uint8_t*)(node + IS_ROOT_OFFSET));
    return (bool)value;
}

/*
*function to set a node as root node
*@param node        pointer to a node
*@param is_root     boolean value speceifying if the node is a root node
*/
void set_node_root(void* node, bool is_root){
    uint8_t value = is_root;
    *((uint8_t*)node + IS_ROOT_OFFSET) = value;
}

/*
*function to find the number of cells in a node
*@param  node       pointer to a node
*@returns pointer to  number of cells in a node
*/
uint32_t* leaf_node_num_cells(void* node){
    return (uint32_t*)(node + LEAF_NODE_NUM_CELLS_OFFSET);
}

/*
*function to return a cell
*@param node        pointer to a node
*@param cell_num    cell number to get
*@returns a pointer to a cell in a node
*/void* leaf_node_cell(void* node, uint32_t cell_num){
    return node + LEAF_NODE_HEADER_SIZE + (cell_num * LEAF_NODE_CELL_SIZE);
}
/*
*funtion to get the key for a given cell number
*@param node        pointer to a node
*@param cell_num    cell number to get the key
*@returns pointer to the node key
*/uint32_t* leaf_node_key(void* node, uint32_t cell_num){
    return (uint32_t*)leaf_node_cell(node, cell_num);
}

/*
*function to get the value of a cell
*@param node        pointer to a node
*@param cell_num    cell number to find the value
*@returns a pointer to the node value
*/
void* leaf_node_value(void* node, uint32_t cell_num){
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

/*
*function to get the type of the node, internal or leaf
*@param node    pointer to a node
*@returns type of the node
*/
NodeType get_node_type(void* node){
    uint32_t value = *((uint32_t*)(node + NODE_TYPE_OFFSET));
    return (NodeType)value;
}

/*
*funtoin to set the type of the node, internal or leaf
*@param node    pointer to a node
*@param type    node type to be set
*/
void set_node_type(void* node, NodeType type){
    uint32_t value = type;
    *((uint32_t*)(node + NODE_TYPE_OFFSET)) = value;
}

/*
*funtion to initialize the leaf node 
*@param  node       pointer to a node
*/
void initialize_leaf_node(void* node){
    set_node_type(node, NODE_LEAF);
    set_node_root(node, false);
    *(leaf_node_num_cells(node)) = 0;
}

/*
*function to get the pointer to number of keys in a internal node
*@param node    pointer to a node
*@returns a pointer to the number of keys in an internal node  
*/
uint32_t* internal_node_num_keys(void* node){
    return (uint32_t*)(node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}
/*
*function to get the pointer to the right child of an internal node
*@param node    pointer to a node
*@returns a pointer to the right child of a internal node
*/
uint32_t* internal_node_right_child(void* node){
    return (uint32_t*)(node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

/*
*function to get the internal node cell
*@param node        pointer to a node
*@param cell_num    cell number to read
*@returns a pointer to a cell with node, cell_num 
*/
uint32_t* internal_node_cell(void* node, uint32_t cell_num){
    return (uint32_t*)(node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

/*
*function to get a child node of an internal node
*@param node        pointer to a node
*@param child_num   child node key
*@returns pointer to a child node
*/
uint32_t* internal_node_child(void* node, uint32_t child_num){
    uint32_t num_keys = *(internal_node_num_keys(node));
    if(child_num > num_keys){
        printf("Error:tried to access child_num %d > num_keys %d.\n",child_num, num_keys);
        exit(EXIT_FAILURE);
    }
    else if(child_num == num_keys){
        return internal_node_right_child(node);
    }
    else
        return internal_node_cell(node, child_num);
}

/*
*function to get the key for a given cell in an internal node
*@param node        pointer to a node
*@param key_num     cell number to get the key
*returns a pointer to the key of the passed cell
*/
uint32_t* internal_node_key(void* node, uint32_t key_num){
    return (uint32_t*)(internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE);
}

/*
*function to initialize an internal node
*@param node    pointer to a node
*/
void initialize_internal_node(void* node){
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);
    *(internal_node_num_keys(node)) = 0;
}

/*
*function to get the maximum value in a node
*@param node    pointer to a node
*@returns maximum key value in a node
*/
uint32_t get_node_max_key(void* node){
    switch(get_node_type(node)){
        case NODE_INTERNAL:
            return *internal_node_key(node, *internal_node_num_keys(node)-1);
        case NODE_LEAF:
            return *leaf_node_key(node, *leaf_node_num_cells(node)-1);
    }
}

/*
*funtion to print the constants for the db
*/
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

/*
*function to print the leaf node passed
*@param node    pointer to a node
*/
void print_leaf_node(void* node){//not used
    uint32_t num_cells = *(leaf_node_num_cells(node));
    printf("------------------DB Tree------------------------\n");
    printf("Leaf node with %d cells:\n", num_cells);
    printf("{cell num : key}\n");
    for(uint32_t i=0; i < num_cells; i++){
        uint32_t key = *(leaf_node_key(node,i));
        printf(" - %d : %d\n", i, key);
    }
    printf("\n-------------------------------------------------\n");

}

#endif