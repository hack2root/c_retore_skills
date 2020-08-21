#define DEBUG
#define LIST_LIGHT

#define DIRTY

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef LIST_LIGHT
// address type (for debugging printf function)
typedef long long unsigned int ADDR;

/* Define a custom `malloc` function. */
void* my_calloc(size_t nmemb, size_t size)
{
    void* ptr = calloc(nmemb, size);
#ifdef DEBUG
    printf("!alloc: 0x%llx :%ld\n", (ADDR)ptr, size);
#endif
    return ptr;
}

/* Define a custom `malloc` function. */
void my_free(void* ptr)
{
    if (ptr != 0) {
#ifdef DEBUG
        printf("!free: 0x%llx\n", (ADDR)ptr);
#endif
    }
    free(ptr);
}

/* Override the default `malloc` function used by Rexo with ours. */
#define _LIST_CALLOC my_calloc

/* Override the default `free` function used by Rexo with ours. */
#define _LIST_FREE my_free

#endif //LIST_LIGHT

#ifndef _LIST_CALLOC
    #include <stdlib.h>
    #define _LIST_CALLOC calloc
#endif

#ifndef _LIST_FREE
    #include <stdlib.h>
    #define _LIST_FREE free
#endif

#define ALLOC(size, type) (type*)_LIST_CALLOC(1, sizeof(type))
#define FREE(ptr) _LIST_FREE(ptr)

#include "list-light-api.h"

// default list methods
void list_init(struct list_context* const ctx);
void list_push(struct list_context* const ctx, void* payload);
void* list_pop(struct list_context* const ctx);
void* list_peek(struct list_context* const ctx);
void list_destroy(struct list_context* const ctx);

// list vtable
const struct list_light_vtable list_light_vt = {
    .push = list_push,
    .pop = list_pop,
    .peek = list_peek,
    .init = list_init,
    .destroy = list_destroy
};

// initializes the new context's root element
// as a result, new memory block will be allocated
// current context pointer set to zero
void list_init(struct list_context* const ctx) {
    // sets current context's root element
    ctx->root = ctx->head = ALLOC(1, struct list);
    // sets current context's counter to zero
    ctx->count = 0;
}

// allocates a memory for provided payload 
// at current context, data payload stored at allocated memory buffer
// as a result, items counter will increase
void list_push(struct list_context* const ctx, void* payload) {
    // stores into pre-allocated value newly allocated memory buffer pointer
    struct list* item = ALLOC(1, struct list);
    // sets the new data into allocated memory buffer
    item->payload = payload;
    // pushes new item on top of the stack in current context
    // get current context's head
    struct list* head = ctx->head;
    // assigns item pointer to head's next pointer value
    head->next = item;
    // assigns item's prev pointer to head pointer
    item->prev = ctx->head;
    // advances position of head pointer to the new head
    ctx->head = item;
    // increment current context's counter by one
    ctx->count++;    
}

// pop existing element at the top of the stack/queue/list
// at current context, existing head will be removed out of stack
// for the new stack header, correcponding values will be fixed
// as a result, header will be rewinded to previous position, represented as head's reference to previos head
void* list_pop(struct list_context* const ctx) {
    // get current context's head
    struct list* head = ctx->head;
    // if we call method on empty stack, do not return root element, return null element by convention
    if (head == 0 || head->prev == 0) {
        // returns default element as null element
        return 0;
    }
    // gets previos pointer
    struct list* prev = head->prev;
    // detouches prev pointer to next to it
    prev->next = 0;
    // rewinds head pointer to previous pointer value
    ctx->head = prev;
    // assigns current stack head pointer to temporary
    struct list* ptr = head;
    // gets temporary pointer value
    void* payload = ptr->payload;
    // detouches the pointer from the list
#ifndef DIRTY
    ptr->prev = 0;
    ptr->next = 0;
    ptr->payload = 0;
#endif
    // decrement current context counter
    ctx->count--;
    // free temporary pointer value
    FREE(ptr);
    // returns removed element
    return payload;
}

// peek existing element at the top of the stack/queue/list
// at current context, existing head
void* list_peek(struct list_context* const ctx) {
    // get current context's head
    struct list* head = ctx->head;
    struct list* tmp = head;
    // if we call method on empty stack, do not return root element, return null element by convention
    if (head == 0 || head->prev == 0) {
        // returns default element as null element
        return 0;
    }
    // assigns current stack head pointer to temporary
    tmp = head;
    // returns head element
    return tmp->payload;
}

// destroys the memory stack
// frees all memory elements
// as a result, memory will be freed
void list_destroy(struct list_context* const ctx) {
    // get current context's head
    struct list** item = &ctx->root;
    // assigns currently selected item pointer to temporary
    struct list* tmp = *item;
    // until we run out of stack or stop at root element
    while (tmp != 0) {
        // gets temporary pointer value
        struct list* ptr = tmp;
        // gets next pointer value
        struct list* next = tmp->next;
        // zero all pointers
#ifndef DIRTY
        ptr->prev = 0;
        ptr->next = 0;
        ptr->payload = 0;
#endif
        // free temporary pointer value
        FREE(ptr);
        // advances temporary pointer value to the next item
        tmp = next;
    }
    // all stack items are processed
    *item = 0;
}
