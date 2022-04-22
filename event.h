
#ifndef EVENT_H
#define EVENT_H

#include <stdbool.h>
#include "date.h"
#include "priority_queue.h"



typedef struct event_t *Event;

/** Type used for returning error codes from event functions */
typedef enum event_result {
    EVENT_SUCCESS,
    EVENT_LEGAL,
    EVENT_ILEGAL_ID,
    EVENT_NULL_ARGUMENT,
    EVENT_ILEGAL_DATE,
    EVENT_OUT_OF_MEMORY,

} EventResult;


/* this function creates a new event, if NULL was sent, or illegal id - the function returns NULL */
Event eventCreate(int event_id, char* event_name, Date date);


/* this function creates a new event containing the arguments of a specific event, a copy 
    of the arguments is created in the new event */
Event eventCopy(Event event);


/* this function de-allocate the event & the event's arguments */
void eventDestroy(Event event);


/* this function checks if the elements (events) are the same */
bool eventEqual(Event event1, Event event2);


/* this function copies the priority */
Date eventCopyPriority(Date event_priority);


/* this function de-allocates the event's priority */
void eventDestroyPriority(Date event_priority);


/* this function compare 2 priorities
    if the first one is greater - returns 1
    if the priorities are equal - returns 0
    if the first one is less - returns -1*/
int eventComparePriorities(Date event_priority1, Date event_priority2);


/* this function returns the event's priority (date) */
Date eventGetPriority(Event event);


/* this function returns the event's name*/
char* eventGetName(Event event);


/* this function return -1 in case of null argument was sent, the id - in case of succes */
int eventGetId(Event event);


EventResult eventChangeDate(Event event, Date new_date);


PQElement eventGetPQ(Event);


#endif //EVENT_H




