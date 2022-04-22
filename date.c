
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "date.h"

#define POS 1
#define NEG -1
#define DAYS_IN_MONTH 30
#define FIRST_DAY 1
#define MONTH_NUM 12

enum {JAN = 1, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC};

struct Date_t{
    unsigned int day;
    unsigned int month;
    int year;
};


Date dateCreate(int day, int month, int year){
    if ((day <= 0) || (day > 30) || (month <= 0) || (month > MONTH_NUM)){
        return NULL;
    }
    Date new_date = malloc(sizeof(*new_date));
    if (!new_date){
        return NULL;
    }
    new_date->day = day;
    new_date->month = month;
    new_date->year = year;
    return new_date;    
}

void dateDestroy(Date date){
    free(date);
}

Date dateCopy(Date date){
    if (date == NULL){
        return NULL;
    }
    return dateCreate(date->day, date->month, date->year);
}

bool dateGet(Date date, int* day, int* month, int* year){
    if(day == NULL || month == NULL || year == NULL){
        return false;
    }
    if ((date->day <= 0) || (date->day > 30) || (date->month <= 0) || (date->month > MONTH_NUM)){
        return false;
    }

    *day = date->day;
    *month = date->month;
    *year = date->year;
    return true;
}


int dateCompare(Date date1, Date date2){
    if (date1 == NULL || date2 == NULL){
        return 0;
    }
    if (date1->year == date2->year){
        if (date1->month == date2->month){
            if (date1->day == date2->day){
                return 0;
            }
            return date1->day > date2->day ? POS : NEG;
        }
        return date1->month > date2->month ? POS : NEG;
    }
    return date1->year > date2->year ? POS : NEG;
}

void dateTick(Date date){
    if(date == NULL){
        return;
    }
    if(date->day == DAYS_IN_MONTH){
        if (date->month == DEC){
            date->day = FIRST_DAY;
            date->month = JAN;
            date->year++;
            return;
        }
        date->day = FIRST_DAY;
        date->month++;
        return;
    }
    date->day++;
    return;
}