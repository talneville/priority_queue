
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

char* duplicateString(char* str, int times){
    if(str == NULL){
        return NULL;
    }
    if(times <= 0){
        return NULL;
    }
    int len = strlen(str);
    char* out = malloc(sizeof(char) * len * times + 1);
    if (out == NULL){
        return NULL;
    }
    char* tmp = out;
    for (int i = 0; i < times; i++){
        strcpy(tmp, str);
        tmp = tmp + len;
    }
    return out;
}

// int main(){
//     char* str = "hello";
//     char* str_copy = duplicateString(str, 3);
//     printf("orig: %s\n new: %s\n", str, str_copy);
//     return 0;
// }