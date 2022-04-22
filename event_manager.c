/*=========================================================================*/
// Include files:

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "priority_queue.h"
#include "date.h"
#include "event_manager.h"
#include "member.h"
#include "event.h"

/*=========================================================================*/
// Constants and definitions:
#define NULL_VALUE -1

struct EventManager_t {
    Date system_date;
    PriorityQueue members_pq;
    PriorityQueue events;
};


static bool checkLegalMemberID(int id);

/*=========================================================================*/
// main code: 

static EventManagerResult changePQResultToEventResult(PriorityQueueResult result){
    switch (result)
    {
        case PQ_SUCCESS:
            return EM_SUCCESS;

        case PQ_OUT_OF_MEMORY:
            return EM_OUT_OF_MEMORY;

        case PQ_NULL_ARGUMENT:
            return EM_NULL_ARGUMENT;

        case PQ_ELEMENT_DOES_NOT_EXISTS:
            return EM_EVENT_NOT_EXISTS;

        case PQ_ITEM_DOES_NOT_EXIST:
            return EM_EVENT_ID_NOT_EXISTS;

        case PQ_ERROR:
            return EM_ERROR;
            
        default:
            break;
    }
    assert(NULL);
    return EM_ERROR;
}




static EventManagerResult changePQResultToMemberResult(PriorityQueueResult result){
    switch (result)
    {
        case PQ_SUCCESS:
            return EM_SUCCESS;

        case PQ_OUT_OF_MEMORY:
            return EM_OUT_OF_MEMORY;

        case PQ_NULL_ARGUMENT:
            return EM_NULL_ARGUMENT;

        case PQ_ELEMENT_DOES_NOT_EXISTS:
            return EM_MEMBER_ID_NOT_EXISTS;

        case PQ_ITEM_DOES_NOT_EXIST:
            return EM_MEMBER_ID_NOT_EXISTS;

        case PQ_ERROR:
            return EM_ERROR;

        default:
            break;
    }
    assert(NULL);
    return EM_ERROR;
}


static bool checkLegalDate(Date date, Date system_date){
    if(date == NULL || system_date == NULL){
        return false;
    }
    return (dateCompare(date, system_date) >= 0);
}

static bool checkLegalEventID(int event_id){
    return (event_id >= 0);
}

//check
static bool checkEventNameExist(PriorityQueue events, const char* name, Date date){
    PQ_FOREACH(Event, iterator, events){
        if ((strcmp(name, eventGetName(iterator)) == 0) &&
                 (dateCompare(date, eventGetPriority(iterator)) == 0)){
            return true;
        }
    }
    return false;
}




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





/*=========================*/
//event wrappers:
static PQElement eventCopyWrapper (PQElement event) {
    return (void*)eventCopy((Event)event);
}

static void eventDestroyWrapper (PQElement event) {
    eventDestroy((Event)event);
}

static bool eventEqualWrapper (PQElement event_1, PQElement event_2) {
    return eventEqual((Event)event_1, (Event)event_2);
}

static PQElementPriority eventCopyPriorityWrapper(PQElementPriority event_priority){
    return (PQElementPriority)eventCopyPriority((Date)event_priority);
}

// typedef void(*FreePQElementPriority)(PQElementPriority);
static void eventDestroyPriorityWrapper (PQElementPriority priority){
    eventDestroyPriority ((Date)priority);
}


static int eventComparePrioritiesWrapper (PQElementPriority priority_1, PQElementPriority priority_2){
    return eventComparePriorities ((Date)priority_1, (Date)priority_2);
}


/*=========================================================================*/


EventManager createEventManager(Date date){
    if(date == NULL){
        return NULL;
    }
    EventManager event_manager = malloc(sizeof(*event_manager));
    if(event_manager == NULL){
        return NULL;
    }
    event_manager->system_date = dateCopy(date);
    if(event_manager->system_date == NULL){
        free(event_manager);
        return NULL;
    }
    event_manager->members_pq = pqCreate (memberCopyWrapper,
                                          memberDestroyWrapper,
                                          memberEqualWrapper,
                                          memberCopyPriorityWrapper,
                                          memberDestroyPriorityWrapper,
                                          memberComparePrioritiesWrapper);


    if (event_manager->members_pq == NULL){
        dateDestroy (event_manager->system_date);
        free(event_manager);
        return NULL;
    }
    event_manager->events = pqCreate (eventCopyWrapper,
                                      eventDestroyWrapper,
                                      eventEqualWrapper,
                                      eventCopyPriorityWrapper,
                                      eventDestroyPriorityWrapper,
                                      eventComparePrioritiesWrapper);
    if(event_manager->events == NULL){
        dateDestroy (event_manager->system_date);
        pqDestroy(event_manager->members_pq);
        free(event_manager);
        return NULL;
    }
    return event_manager;
}



void destroyEventManager(EventManager em){
    if (em == NULL){
        return;
    }
    dateDestroy(em->system_date);    
    pqDestroy(em->members_pq);
    pqDestroy(em->events);
    free(em);
    return;
}



EventManagerResult emAddEventByDate(EventManager em, char* event_name, Date date, int event_id){

    if(em == NULL || event_name == NULL || date == NULL){
        return EM_NULL_ARGUMENT;
    }
    if(checkLegalDate(date, em->system_date) == false){
        return EM_INVALID_DATE;
    }
    if (checkLegalEventID(event_id) == false){
        return EM_INVALID_EVENT_ID;
    }
    if(checkEventNameExist(em->events, event_name, date)){
        return EM_EVENT_ALREADY_EXISTS;
    }
    

    Event event = eventCreate(event_id, event_name, date);
    if(event == NULL){
        destroyEventManager(em);
        return EM_OUT_OF_MEMORY;
    }
    if(pqContains(em->events, event)){
        eventDestroy(event);
        return EM_EVENT_ID_ALREADY_EXISTS;
    }
    PriorityQueueResult result = pqInsert(em->events, event, date);
    eventDestroy(event);
    if(result == PQ_OUT_OF_MEMORY){
        destroyEventManager(em);
        return EM_OUT_OF_MEMORY;
    }
    return changePQResultToEventResult(result);
}

static Date createDateByDifference (Date date, int days){
    if (date == NULL){
        return NULL;
    } 
    Date date_copy = dateCopy(date);
    if (date_copy == NULL){
        return NULL;
    }
    assert(days >= 0);
    for(int i = 0; i < days; i++){
        dateTick(date_copy);
    }
    return date_copy;
}

EventManagerResult emAddEventByDiff(EventManager em, char* event_name, int days, int event_id){
    if(em == NULL || event_name == NULL){
        return EM_NULL_ARGUMENT;
    }
    if(days < 0){
        return EM_INVALID_DATE;
    }   
    Date date = createDateByDifference(em->system_date, days);
    if(date == NULL){
        destroyEventManager(em);
        return EM_OUT_OF_MEMORY;
    }
    
    EventManagerResult result = emAddEventByDate(em, event_name, date, event_id);
    dateDestroy(date);
    return result;
}

static Member getMemberByID(PriorityQueue members_pq, int member_id){
    //legality checks:
    if(members_pq == NULL){
        return NULL;
    }
    if(!checkLegalMemberID(member_id)){
        return NULL;
    }
    //get the member:
    PQ_FOREACH(Member, iterator, members_pq){
        if(memberGetID(iterator) == member_id){
            return iterator;
        }
    }
    return NULL;
}

//memberChangeEventsNum
static void removeLinkedMembersEventsNum(PriorityQueue members_pq, PriorityQueue members_linkes_to_event){
    Member tmp;
    PQ_FOREACH(Member, iterator, members_linkes_to_event){
        tmp = getMemberByID(members_pq, memberGetID(iterator));
        memberChangeEventsNum(tmp, memberGetEventsNum(tmp) - 1);
    }
}

static Event getEventByID(PriorityQueue events, int id){
    if(events == NULL){
        return NULL;
    }
    PQ_FOREACH(Event, iterator, events){
        if(eventGetId(iterator) == id){
           return iterator;
        }
    }
    return NULL;
} 

EventManagerResult emRemoveEvent(EventManager em, int event_id){
    if (em == NULL){
        return EM_NULL_ARGUMENT;
    }
    if(!checkLegalEventID(event_id)){
        return EM_INVALID_EVENT_ID;
    }
    Event event = getEventByID(em->events, event_id);
    if(event == NULL){
        return EM_EVENT_NOT_EXISTS;
    }
  
    removeLinkedMembersEventsNum(em->members_pq, eventGetPQ(event));
    PriorityQueueResult result = pqRemoveElement(em->events, event);
    return changePQResultToEventResult(result);
}



static Date getDateByEventID(PriorityQueue events, int id){
    if(events == NULL){
        return NULL;
    }
    Event event = getEventByID(events, id);
    return eventGetPriority(event);
}

static Event getEventByNameAndDate(PriorityQueue events, Date date, char* event_name){
    PQ_FOREACH(Event, iterator, events){
        if((strcmp(eventGetName(iterator), event_name) == 0) && 
                (dateCompare(date, eventGetPriority(iterator)) == 0)){
                    return iterator;
                }
    }
    return NULL;  
}

//need to checkif event exist in the same date
EventManagerResult emChangeEventDate(EventManager em, int event_id, Date new_date){
    //legality check:
    if(em == NULL || new_date == NULL){
        return EM_NULL_ARGUMENT;
    }
    if(!checkLegalDate(new_date, em->system_date)){
        return EM_INVALID_DATE;
    }
    if(!checkLegalEventID(event_id)){
        return EM_INVALID_EVENT_ID;
    }

    Event event = getEventByID(em->events, event_id);
    if(event == NULL){
        return EM_EVENT_ID_NOT_EXISTS;
    }
    Date old_date = getDateByEventID(em->events, event_id);

    //check if event with this name is already in destination date:
    Event check_new_date = getEventByNameAndDate(em->events, new_date, eventGetName(event));
    if(check_new_date != NULL){
        return EM_EVENT_ALREADY_EXISTS;
    }


    //change priority:
    PriorityQueueResult priority_result = pqChangePriority(em->events, event, old_date, new_date);
    if(priority_result == PQ_OUT_OF_MEMORY){
        destroyEventManager(em);
        return EM_OUT_OF_MEMORY;
    }

    //change event date:
    event = getEventByID(em->events, event_id);
    EventResult event_result = eventChangeDate(event, new_date);
    if(event_result == EVENT_OUT_OF_MEMORY){
        destroyEventManager(em);
        return EM_OUT_OF_MEMORY;
    }

    return changePQResultToEventResult(priority_result);
}


EventManagerResult emAddMember(EventManager em, char* member_name, int member_id) {
    if(em == NULL || member_name == NULL){
        return EM_NULL_ARGUMENT;
    }
    if(member_id < 0){
        return EM_INVALID_MEMBER_ID;
    }
    Member member = memberCreate(member_id, member_name);
    if(member == NULL){
        destroyEventManager(em);
        return EM_OUT_OF_MEMORY;        
    }
    if (pqContains(em->members_pq, member)){
        memberDestroy(member);
        return EM_MEMBER_ID_ALREADY_EXISTS;
    }

    PriorityQueueResult pq_result = pqInsert(em->members_pq, member, &member_id);
    memberDestroy(member);
    if(pq_result == PQ_OUT_OF_MEMORY){
            destroyEventManager(em);
            return EM_OUT_OF_MEMORY;
    }
    return changePQResultToMemberResult(pq_result);
}


static bool checkLegalMemberID(int id){
    return (id >=0);
}


EventManagerResult emAddMemberToEvent(EventManager em, int member_id, int event_id){
    //legality checks:
    if(em == NULL){
        return EM_NULL_ARGUMENT;
    }
    if(!checkLegalEventID(event_id)){
        return EM_INVALID_EVENT_ID;
    }
    if(!checkLegalMemberID(member_id)){
        return EM_INVALID_MEMBER_ID;
    }

    Event event = getEventByID(em->events, event_id);
    if(event == NULL){
        return EM_EVENT_ID_NOT_EXISTS;
    }    
    Member member = getMemberByID(em->members_pq, member_id);
    if(member == NULL){
        return EM_MEMBER_ID_NOT_EXISTS;
    }    
    
    //change event linked members pq
    PriorityQueue linked_members = eventGetPQ(event);
    if(pqContains(linked_members, member)){
        return EM_EVENT_AND_MEMBER_ALREADY_LINKED;
    }
    
    PriorityQueueResult result = pqInsert(linked_members, member, &member_id);
    if(result == PQ_OUT_OF_MEMORY){
        destroyEventManager(em);
        return EM_OUT_OF_MEMORY;
    }
    memberChangeEventsNum(member, memberGetEventsNum(member) + 1);
    member = getMemberByID(eventGetPQ(event), member_id);
    memberChangeEventsNum(member, memberGetEventsNum(member) + 1);
    return changePQResultToEventResult(result);
}



EventManagerResult emRemoveMemberFromEvent (EventManager em, int member_id, int event_id){
    //legality check:
    if(em == NULL){
        return EM_NULL_ARGUMENT;
    }    
    if(!checkLegalEventID(event_id)){
        return EM_INVALID_EVENT_ID;
    }
    if(!checkLegalMemberID(member_id)){
        return EM_INVALID_MEMBER_ID;
    }

    //get the event
    Event event = getEventByID(em->events, event_id);
    if(event == NULL){
        return EM_EVENT_ID_NOT_EXISTS;
    }  
    Member member = getMemberByID(em->members_pq, member_id);
    if(member == NULL){
        return EM_MEMBER_ID_NOT_EXISTS;
    }   

    //get linked_members pq
    PriorityQueue linked_members = eventGetPQ(event);

    //remove member
    PriorityQueueResult result = pqRemoveElement(linked_members, member);
    if(result == PQ_ELEMENT_DOES_NOT_EXISTS){
        return EM_EVENT_AND_MEMBER_NOT_LINKED;
    }
    memberChangeEventsNum(member, memberGetEventsNum(member) - 1);
        
    return EM_SUCCESS;
}





static Date dateTickLoop(Date date, int days){
    if(days < 0){
        return NULL;
    }
    for(int i = 0; i < days; i++){
        dateTick(date);
    }
    return date;
}

EventManagerResult emTick(EventManager em, int days){
    //legality checks:
    if(em == NULL){
        return EM_NULL_ARGUMENT;
    }
    if(days <= 0){
        return EM_INVALID_DATE;
    }

    //change system date:
    em->system_date = dateTickLoop(em->system_date, days);

    // delete past events:
    Event first_event = pqGetFirst(em->events);
    while((first_event != NULL) && (dateCompare(eventGetPriority(first_event), em->system_date) < 0)){
        int first_event_id = eventGetId(first_event);
        EventManagerResult result = emRemoveEvent(em, first_event_id);
        if(result == EM_OUT_OF_MEMORY){
            destroyEventManager(em);
            return result;
        }
        first_event = pqGetFirst(em->events);
    }
    return EM_SUCCESS;
}


int emGetEventsAmount(EventManager em){
    if(em == NULL){
        return NULL_VALUE;
    }
    return (pqGetSize(em->events));
}


char* emGetNextEvent(EventManager em){
    if(em == NULL){
        return NULL;
    }
    return eventGetName(pqGetFirst(em->events));
}


static void printEvent (Event event, FILE* file){
    assert (file && event);
    Date date = eventGetPriority(event);
    int day, month, year;
    dateGet(date, &day, &month, &year);
    fprintf(file, "%s,%d.%d.%d",eventGetName(event), day, month, year);

    PQ_FOREACH(Member, iterator, eventGetPQ(event)){
        fprintf(file, ",%s", memberGetName(iterator));
    }
    fprintf(file, "\n");
    
    return;
}


void emPrintAllEvents(EventManager em, const char* file_name){
    //legality checks:
    if(em == NULL || file_name == NULL){
        return;
    }
    //open file:
    FILE* file = fopen (file_name, "w");
    if(file == NULL){
        return;
    }
    //print into file:
    PQ_FOREACH(Event, iterator, em->events){
        printEvent(iterator, file);
    }
    //close file and return:
    fclose(file);
    return;
}

static void printMembersByAmount(FILE* file, int num_of_events, PriorityQueue members, bool first_print){
    PQ_FOREACH(Member, iterator, members){
        // if(memberGetEventsNum(iterator) == num_of_events){
        //     if(first_print){
        //         fprintf(file, "%s,%d", memberGetName(iterator), num_of_events);
        //         first_print = false;  
        //     }else{
        //         fprintf(file, "\n%s,%d", memberGetName(iterator), num_of_events);
        //     }
        // }       
        if(memberGetEventsNum(iterator) == num_of_events){
            fprintf(file, "%s,%d\n", memberGetName(iterator), num_of_events);
        }     
    }
}



void emPrintAllResponsibleMembers(EventManager em, const char* file_name){
    //legality checks:
    if(em == NULL || file_name == NULL){
        return;
    }
    //open file:
    FILE* file = fopen (file_name, "w");
    if(file == NULL){
        return;
    }
    //get max event num
    int max = pqGetSize(em->events);
    for(int amount = max; amount > 0; amount--){
        //print members by amount of events
        printMembersByAmount(file, amount, em->members_pq, amount == max);
    }
    
    //close file
    fclose(file);
    return;
}
