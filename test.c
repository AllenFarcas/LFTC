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
    char s[50] = "abcde15.30fghij";
    char *point = s+5;
    char *stop = s+10;
    char *c = createString(point,stop);
    puts(c); // 00010
    stop = stop+1;
    int b2 = strtol(c,NULL,16);
    char c1 = *c;
    double doub = atof(c);
    //putchar(c1);
    printf("%lf",doub);
    //int a = atoi(c);
   // printf("%d\n",a); //10
    //printf("%d\n",b2); //8
}
