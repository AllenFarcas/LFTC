#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* createString(char *start, char *stop) {
    char *c;
    int n;
    n = stop-start;
    c = (char *)malloc(sizeof(char)*n+1);
    c = strncpy(c,start,sizeof(char)*n);
    c[n+1] = '\0';
    return c;
}

int main() {
    char s[50] = "abcde00010fghij";
    char *point = s+5;
    char *stop = s+10;
    char *c = createString(point,stop);
    puts(c);
    stop = stop+1;
    long b2 = strtol(c,NULL,8);
    int a = atoi(c);
    int b = 10+a;
    printf("%d\n",a);
    printf("%ld\n",b2);
}
