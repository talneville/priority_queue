
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "priority_queue.h"




typedef struct node {
    PQElement element;
    PQElementPriority priority;
    struct node* next_node;
}*Node;

struct PriorityQueue_t {
    Node first_node;
    Node iterator;

    PQElement(*CopyPQElement)(PQElement);
    PQElementPriority(*CopyPQElementPriority)(PQElementPriority);
    void(*FreePQElement)(PQElement);
    void(*FreePQElementPriority)(PQElementPriority);
    bool(*EqualPQElements)(PQElement, PQElement);
    int(*ComparePQElementPriorities)(PQElementPriority, PQElementPriority);
};

PriorityQueue pqCreate(CopyPQElement copy_element,
                       FreePQElement free_element,
                       EqualPQElements equal_elements,
                       CopyPQElementPriority copy_priority,
                       FreePQElementPriority free_priority,
                       ComparePQElementPriorities compare_priorities){

                           PriorityQueue queue;
                           queue = malloc(sizeof(*queue));
                            if (!queue) {
                                return NULL;
                            }

                            queue->first_node = NULL;
                            queue->iterator = NULL;

                            queue->CopyPQElement = copy_element;
                            queue->CopyPQElementPriority = copy_priority;
                            queue->FreePQElement = free_element;
                            queue->FreePQElementPriority = free_priority;
                            queue->EqualPQElements = equal_elements;
                            queue->ComparePQElementPriorities = compare_priorities;
                            return queue;
                       }

// new func - satatic - delete node
static Node deleteNode(Node to_delete, FreePQElement free_element, FreePQElementPriority free_priority){
    if(to_delete == NULL){
        return NULL;
    }
    Node next = to_delete->next_node;
    free_element(to_delete->element);
    free_priority(to_delete->priority);
    free(to_delete);
    return next;
}

static void destroyLinkedList(Node node, FreePQElement free_element, FreePQElementPriority free_priority){
    while (node){
       node = deleteNode(node, free_element, free_priority);
    }
}

void pqDestroy(PriorityQueue queue){
    if(queue == NULL){
        return;
    }
    destroyLinkedList(queue->first_node, queue->FreePQElement, queue->FreePQElementPriority);
    free(queue);
    return;
}

static Node nodeCreate(PQElement element, PQElementPriority priority){
    Node node = malloc(sizeof(*node));
    if (node == NULL){
        return NULL;
    }
    node->element = element;
    node->priority = priority;
    node->next_node = NULL;
    return node;
}


static Node nodeCopy (Node node, CopyPQElement copy_element, CopyPQElementPriority copy_priority,
                            FreePQElement free_element, FreePQElementPriority free_priority){
    assert(node);
    PQElement element = copy_element(node->element);
    if(!element){
        return NULL;
    }
    PQElement priority = copy_priority(node->priority);
    if(!priority){
        free_element(element);
        return NULL;
    }
    Node node_copy = nodeCreate(element, priority);
    if (node_copy == NULL){
        free_element(element);
        free_priority(priority);
        return NULL;
    }
    return node_copy;
}

static Node copyLinkedList(Node first_node, CopyPQElement copy_element, CopyPQElementPriority copy_priority,
                                FreePQElement free_element, FreePQElementPriority free_priority){
    assert (first_node);
    Node first_copy = nodeCopy(first_node, copy_element, copy_priority, free_element, free_priority);
    if(!first_copy){
        return NULL;
    }
    Node current_copy = first_copy;
    Node current_node = first_node;
    while(current_node->next_node){

        current_copy->next_node = nodeCopy(current_node->next_node, copy_element, copy_priority,
                                                    free_element, free_priority);
        if(!current_copy->next_node){
            destroyLinkedList(first_copy, free_element, free_priority);
            return NULL;
        }
        current_copy = current_copy->next_node;
        current_node = current_node->next_node;
    }
    return first_copy;
}


PriorityQueue pqCopy(PriorityQueue queue){
    if(queue == NULL){
        return NULL;
    }
    queue->iterator = NULL;
    PriorityQueue queue_copy =
        pqCreate(queue->CopyPQElement, queue->FreePQElement, queue->EqualPQElements,
            queue->CopyPQElementPriority, queue->FreePQElementPriority, queue->ComparePQElementPriorities);

    if (queue_copy == NULL){
        return NULL;
    }
    if(queue->first_node == NULL){
        return queue_copy;
    }    
    queue_copy->first_node = copyLinkedList(queue->first_node, queue->CopyPQElement, queue->CopyPQElementPriority,
                                queue->FreePQElement, queue->FreePQElementPriority);
    
    if (queue_copy->first_node == NULL){
        pqDestroy(queue_copy);
        return NULL;
    }
    return queue_copy;
}

int pqGetSize(PriorityQueue queue){
    int counter = 0;
    if(queue == NULL){
        return -1;
    }
    Node current = queue->first_node;
    while (current != NULL){
        counter++;
        current = current->next_node;
    }
    return counter;
}
bool pqContains(PriorityQueue queue, PQElement element){
    if(queue == NULL || element == NULL){
        return false;
    }
    Node current = queue->first_node;
    while (current){
        if(queue->EqualPQElements(current->element, element)){
            return true;
        }
        current = current->next_node;
    }
    return false;
}


/* this function deletes a node from the pq but not from the memory
    this function returns the node that has been deleted from the pq list*/
static Node RemoveAndConnect (Node parent){
    assert(parent);
    Node r_value = parent->next_node;
    parent->next_node = r_value->next_node;
    return r_value;    
}


PriorityQueueResult pqInsert(PriorityQueue queue, PQElement element, PQElementPriority priority){
     if(queue == NULL){
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator = NULL;
    if(element == NULL || priority == NULL){
        return PQ_NULL_ARGUMENT;
    }
    PQElementPriority priority_copy = queue->CopyPQElementPriority(priority);
    if(priority_copy == NULL){
        return PQ_OUT_OF_MEMORY;
    }
    PQElement element_copy = queue->CopyPQElement(element);
    if(element_copy == NULL){
        queue->FreePQElementPriority(priority_copy);
        return PQ_OUT_OF_MEMORY;
    }    
    Node new_node = nodeCreate(element_copy, priority_copy);
    if(new_node == NULL){
        queue->FreePQElementPriority(priority_copy);
        queue->FreePQElement(element_copy);
        return PQ_OUT_OF_MEMORY;
    }  

    if (queue->first_node == NULL){
        queue->first_node = new_node;
        return PQ_SUCCESS;
    }

    if(queue->ComparePQElementPriorities(priority_copy, queue->first_node->priority) > 0){
        new_node->next_node = queue->first_node;
        queue->first_node = new_node;
        return PQ_SUCCESS;
    }

    Node parent = queue->first_node;
   
    while (parent->next_node && 
           queue->ComparePQElementPriorities(priority_copy, parent->next_node->priority) <= 0){
        parent = parent->next_node;
    }
    new_node->next_node = parent->next_node;
    parent->next_node = new_node;
    return PQ_SUCCESS;
}


PriorityQueueResult pqRemove(PriorityQueue queue){
    if (!queue){
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator = NULL;
    if (!queue->first_node){
        return PQ_SUCCESS;
    }
    queue->first_node = deleteNode(queue->first_node, queue->FreePQElement, queue->FreePQElementPriority);
    return PQ_SUCCESS;
}



PriorityQueueResult pqRemoveElement(PriorityQueue queue, PQElement element){
    if(queue == NULL){
        return PQ_NULL_ARGUMENT;
    }    
    queue->iterator = NULL;
    if(element == NULL){
        return PQ_NULL_ARGUMENT;
    }
    if(!pqContains(queue, element)){
        return PQ_ELEMENT_DOES_NOT_EXISTS;
    }    
    
    if(queue->EqualPQElements(queue->first_node->element,element)){
        queue->first_node = deleteNode(queue->first_node, queue->FreePQElement, queue->FreePQElementPriority);
        return PQ_SUCCESS;
    }
    
    Node parent = queue->first_node;
    assert(parent);
    while ((parent->next_node != NULL) && (!queue->EqualPQElements(parent->next_node->element, element))){
        parent = parent->next_node;
    }
    parent->next_node = deleteNode(parent->next_node, queue->FreePQElement, queue->FreePQElementPriority);
    return PQ_SUCCESS;   
}



// redo!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
PriorityQueueResult pqChangePriority(PriorityQueue queue, PQElement element,
                                     PQElementPriority old_priority, PQElementPriority new_priority){
    if (!queue){
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator = NULL;
    if (!element || !old_priority || !new_priority){
        return PQ_NULL_ARGUMENT;
    }
    if (queue->first_node==NULL){
        return PQ_ELEMENT_DOES_NOT_EXISTS;
    }
    if(pqGetSize(queue)==1){
        //malloc
        //matybe save old priority
        queue->FreePQElementPriority(queue->first_node->priority);
        queue->first_node->priority = queue->CopyPQElementPriority(new_priority);
        if(queue->first_node->priority == NULL){
            
            return PQ_OUT_OF_MEMORY;
        }
        
        return PQ_SUCCESS;
    }
    PriorityQueueResult result;
    if (queue->EqualPQElements(queue->first_node->element, element) &&
        queue->ComparePQElementPriorities(queue->first_node->priority, old_priority) == 0){
            Node tmp = queue->first_node;
            queue->first_node = queue->first_node->next_node;
            result = pqInsert(queue, tmp->element, new_priority);
            if(result != PQ_SUCCESS){
                queue->first_node = tmp;
                return result;
            }
            deleteNode(tmp, queue->FreePQElement, queue->FreePQElementPriority);
            return PQ_SUCCESS;
    }
    
    Node parent = queue->first_node;
    Node to_change;
    while(parent->next_node) {
        if (queue->EqualPQElements(parent->next_node->element, element) &&
            queue->ComparePQElementPriorities(parent->next_node->priority, old_priority) == 0){
                    to_change = RemoveAndConnect(parent);
                    result = pqInsert(queue, to_change->element, new_priority);
                    if(result != PQ_SUCCESS){
                        parent->next_node = to_change;
                        return result;
                    }
                    deleteNode(to_change, queue->FreePQElement, queue->FreePQElementPriority);
                    return PQ_SUCCESS;
        }
        parent = parent->next_node;
    }
    return PQ_ELEMENT_DOES_NOT_EXISTS;
}


PQElement pqGetFirst(PriorityQueue queue){
    if(queue == NULL){
        return NULL;
    }
    if(queue->first_node == NULL){
        return NULL;
    }
    queue->iterator = queue->first_node;    
    return queue->iterator->element;
}


PQElement pqGetNext(PriorityQueue queue){
    if(queue == NULL){
        return NULL;
    }
    if(queue->first_node == NULL){
        return NULL;
    }
    if(queue->iterator == NULL){
        return NULL;
    }
    queue->iterator = queue->iterator->next_node;
    if(queue->iterator == NULL){
        return NULL;
    }
    return queue->iterator->element;
}

PriorityQueueResult pqClear(PriorityQueue queue){
    if(queue == NULL){
        return PQ_NULL_ARGUMENT;
    }
    queue->iterator = NULL;
    destroyLinkedList(queue->first_node, queue->FreePQElement, queue->FreePQElementPriority);
    queue->first_node = NULL;
    return PQ_SUCCESS;
}