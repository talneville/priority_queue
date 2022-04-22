#include "event.h"
#include "priority_queue.h"
#include "member.h"
#include "date.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct event_t{
    int event_id;
    char* event_name;
    Date event_date;
    PriorityQueue event_members;  
};

/*=========================================================================*/
/* wrapper functions for casting function values to fit PQ:*/

//member wrappers:
static PQElement memberCopyWrapper (PQElement member) {
    return (void*)memberCopy((Member)member);
}

static void memberDestroyWrapper (PQElement member) {
    memberDestroy((Member)member);
}

static bool memberEqualWrapper (PQElement member_1, PQElement member_2) {
    return memberEqual((Member)member_1, (Member)member_2);
}

static PQElementPriority memberCopyPriorityWrapper(PQElementPriority member_priority){
    return (PQElementPriority)memberCopyPriority((MemberPriority)member_priority);
}

static void memberDestroyPriorityWrapper (PQElementPriority priority){
    memberDestroyPriority ((MemberPriority)priority);
}


static int memberComparePrioritiesWrapper (PQElementPriority priority_1, PQElementPriority priority_2){
    return memberComparePriorities ((MemberPriority)priority_1, (MemberPriority)priority_2);
}



/*=========================================================================*/

//assumes date is legal
static EventResult checkLegalEvent (Event event){
    if(event == NULL){
        return EVENT_NULL_ARGUMENT;
    }
    if(event->event_name == NULL || event->event_date == NULL){
        return EVENT_NULL_ARGUMENT;
    }
    if(event->event_id < 0){
        return EVENT_ILEGAL_ID;
    }
    return EVENT_LEGAL;
}


/* this function creates a new event, if NULL was sent, or illegal id - the function returns NULL.
   needs to get a legal ID! */
Event eventCreate(int event_id, char* event_name, Date date){
    if (event_name == NULL || date == NULL){
        return NULL;
    }
    assert(event_id >= 0);
    
    Event event = malloc(sizeof(*event));
    if(event == NULL){
        return NULL;
    }
    
    event->event_name = malloc(strlen(event_name) + 1);
    if(event->event_name == NULL){
        free(event);
        return NULL;
    }
    strcpy(event->event_name, event_name);

    event->event_date = dateCopy(date);
    if(event->event_date == NULL){
        free(event->event_name);
        free(event);
        return NULL;
    }

    event->event_members = pqCreate(memberCopyWrapper, memberDestroyWrapper, memberEqualWrapper, memberCopyPriorityWrapper,
                                      memberDestroyPriorityWrapper, memberComparePrioritiesWrapper);
    if(event->event_members == NULL){
        free(event->event_name);
        dateDestroy(event->event_date);
        free(event);
        return NULL;
    }
    event->event_id = event_id;
    return event;    
}


/* this function creates a new event containing the arguments of a specific event, a copy 
    of the arguments is created in the new event */
Event eventCopy(Event event){
    EventResult legal = checkLegalEvent(event);
    if(legal != EVENT_LEGAL){
        return NULL;
    }
    
    Event event_copy = eventCreate(event->event_id, event->event_name, event->event_date);
    if(event_copy == NULL){
        return NULL;
    }
    pqDestroy(event_copy->event_members);
    event_copy->event_members = pqCopy(event->event_members);
    if(event_copy->event_members == NULL){
        eventDestroy(event_copy);
        return NULL;
    }
    return event_copy;
}


/* this function de-allocate the event & the event's arguments */
void eventDestroy(Event event){
    free(event->event_name);
    dateDestroy(event->event_date);
    pqDestroy(event->event_members);
    free(event);
}


/* this function checks if the elements (events) are the same */
bool eventEqual(Event event1, Event event2){
    return (event1->event_id == event2->event_id);
}


/* this function copies the priority */
Date eventCopyPriority(Date event_priority){
    Date date = dateCopy(event_priority);
    return date;
}


/* this function de-allocates the event's priority */
void eventDestroyPriority(Date event_priority){
    dateDestroy(event_priority);
    return;    
}


/* this function compare 2 priorities
    if the first one is greater - returns 1
    if the priorities are equal - returns 0
    if the first one is less - returns -1*/
int eventComparePriorities(Date event_priority1, Date event_priority2){
    return (-1) * dateCompare(event_priority1, event_priority2);
}



Date eventGetPriority(Event event){
    if(event == NULL){
        return NULL;
    }
    return event->event_date;    
}


char* eventGetName(Event event){
    if(event == NULL){
        return NULL;
    }
    return event->event_name;
}


int eventGetId(Event event){
    if(event == NULL){
        return -1;
    }
    return event->event_id;
}


EventResult eventChangeDate(Event event, Date new_date){
    if(event == NULL){
        return EVENT_NULL_ARGUMENT;
    }
    Date copy = dateCopy(new_date);
    if(copy == NULL){
        return EVENT_OUT_OF_MEMORY;
    }
    dateDestroy(event->event_date);
    event->event_date = copy;
    return EVENT_SUCCESS;
}

PQElement eventGetPQ(Event event){
    if(event == NULL){
        return NULL;
    }
    return event->event_members;
}