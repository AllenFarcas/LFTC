#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

void err(const char *fmt,...);
void printTk(int tkn);

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum { COMMA, SEMICOLON,
       LPAR, RPAR,
       LBRACKET, RBRACKET,
       LACC, RACC,
       EQUAL, ASSIGN,
       NOTEQ, NOT,
       LESSEQ, LESS,
       GREATEREQ, GREATER,
       ADD, SUB,
       MUL, DIV,
       AND, OR,
       ID, DOT,
       CT_INT, CT_REAL,
       CT_CHAR, CT_STRING,
       STRUCT,DOUBLE,RETURN,
       BREAK,WHILE,
       CHAR,ELSE,VOID,
       FOR,INT,
       IF,
       END
    }; // codurile AL

typedef struct _Token {
    int code; // codul (numele)
    union
    {
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
        //char *c = createString(point,stop);
        //long int b = strtol(c,NULL,16); //8 sau 2
        long int i; // folosit pentru CT_INT, CT_CHAR
        double r; // folosit pentru CT_REAL
    };
    int line; // linia din fisierul de intrare
    struct _Token *next;// inlantuire la urmatorul AL
}Token;

//=======GLOBAL VARIABLES==========
char *pStartCh;
Token *tokens;
Token *lastToken;
int lineText=0;
char *pCrtCh;

//===========FUNCTIONS=============
Token *addTk(int code) {
    Token *tk;
    SAFEALLOC(tk,Token)
    tk->code=code;
    tk->line=lineText;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
    } else {
        tokens=tk;
    }
    lastToken = tk;
    return tk;
}

void printTokens()
{
    Token *atom = tokens;
    while (atom != NULL) {
        if (atom->code == CT_INT) {
            printTk(atom->code);
            printf(":%ld ",atom->i);
        }
        else if (atom->code == CT_REAL) {
            printTk(atom->code);
            printf(":%lf ",atom->r);
        }
        else if (atom->code == ID) {
            printTk(atom->code);
            printf(":%s ",atom->text);
        }
        else if (atom->code == CT_STRING) {
            printTk(atom->code);
            printf(":%s ",atom->text);
        }
        else if (atom->code == CT_CHAR) {
            printTk(atom->code);
            printf(":%c ",(char)atom->i);
        }
        else {
            printTk(atom->code);
        }
        atom = atom->next;
    }
}

char* createString(char *start, char *stop) {
    char *c;
    int n;
    n = stop-start;
    c = (char *)malloc(sizeof(char)*n+1);
    c = strncpy(c,start,sizeof(char)*n);
    c[n+1] = '\0';
    return c;
}

void err(const char *fmt,...) {
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk, const char *fmt,...) {
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

void printTk(int tkn) {
    switch (tkn) {
        case 0 :
            printf("COMMA ");
            break;
        case 1 :
            printf("SEMICOLON\n");
            break;
        case 2 :
            printf("LPAR ");
            break;
        case 3 :
            printf("RPAR\n");
            break;
        case 4 :
            printf("LBRACKET ");
            break;
        case 5 :
            printf("RBRACKET ");
            break;
        case 6 :
            printf("LACC ");
            break;
        case 7 :
            printf("RACC\n");
            break;
        case 8 :
            printf("EQUAL ");
            break;
        case 9 :
            printf("ASSIGN ");
            break;
        case 10 :
            printf("NOTEQ ");
            break;
        case 11 :
            printf("NOT ");
            break;
        case 12 :
            printf("LESSEQ ");
            break;
        case 13 :
            printf("LESS ");
            break;
        case 14 :
            printf("GREATEREQ ");
            break;
        case 15 :
            printf("GREATER ");
            break;
        case 16 :
            printf("ADD ");
            break;
        case 17 :
            printf("SUB ");
            break;
        case 18 :
            printf("MUL ");
            break;
        case 19 :
            printf("DIV ");
            break;
        case 20 :
            printf("AND ");
            break;
        case 21 :
            printf("OR ");
            break;
        case 22 :
            printf("ID");
            break;
        case 23 :
            printf("DOT ");
            break;
        case 24 :
            printf("CT_INT");
            break;
        case 25 :
            printf("CT_REAL");
            break;
        case 26 :
            printf("CT_CHAR");
            break;
        case 27 :
            printf("CT_STRING");
            break;
        case 28 :
            printf("STRUCT ");
            break;
        case 29 :
            printf("DOUBLE ");
            break;
        case 30 :
            printf("RETURN ");
            break;
        case 31 :
            printf("BREAK ");
            break;
        case 32 :
            printf("WHILE ");
            break;
        case 33 :
            printf("CHAR ");
            break;
        case 34 :
            printf("ELSE ");
            break;
        case 35 :
            printf("VOID ");
            break;
        case 36 :
            printf("FOR ");
            break;
        case 37 :
            printf("INT ");
            break;
        case 38 :
            printf("IF ");
            break;
        case 39 :
            printf("END ");
            break;
        default:
            printf("Eroare tiparire in printTk se cere valoarea %d\n",tkn);
            break;
    }
}

int getNextToken() {
    int state = 0;
    char *int_value;
    char *double_value;
    int nCh;
    char ch;
    int ct_int = 10; // baza 10 initiala
    Token *tk;
    while(1) {
        ch = *pCrtCh;
        switch(state){
            case 0 :
                if (ch == ',') {
                    state = 1; //COMMA
                    pCrtCh++; //consuma caracter
                } else if (ch == ';') {
                    state = 2; //SEMICOLON
                    pCrtCh++; //consuma caracter
                } else if (ch == '(') {
                    state = 3; //LPAR
                    pCrtCh++; //consuma caracter
                } else if (ch == ')') {
                    state = 4; //RPAR
                    pCrtCh++; //consuma caracter
                } else if (ch == '[') {
                    state = 5; //LBRACKET
                    pCrtCh++; //consuma caracter
                } else if (ch == ']') {
                    state = 6; //RBRACKET
                    pCrtCh++; //consuma caracter
                } else if (ch == '{') {
                    state = 7; //LACC
                    pCrtCh++; //consuma caracter
                } else if (ch == '}') {
                    state = 8; //RACC
                    pCrtCh++; //consuma caracter
                } else if (ch == '=') {
                    state = 9;
                    pCrtCh++; //consuma caracter
                } else if (ch == '!') {
                    state = 12;
                    pCrtCh++; //consuma caracter
                } else if (ch == '<') {
                    state  = 15;
                    pCrtCh++; //consuma caracters
                } else if (ch == '>') {
                    state = 18;
                    pCrtCh++; //consuma caracter
                } else if (ch == '+') {
                    state = 21; // ADD
                    pCrtCh++; //consuma caracter
                } else if (ch == '-') {
                    state = 22; //SUB
                    pCrtCh++; //consuma caracter
                } else if (ch == '*') {
                    state = 23; //MUL
                    pCrtCh++; //consuma caracter
                } else if (ch == '.') {
                    state = 24; //DOT
                    pCrtCh++; //consuma caracter
                } else if (ch == '&') {
                    state = 25;
                    pCrtCh++; //consuma caracter
                } else if (ch == '|') {
                    state = 27;
                    pCrtCh++; //consuma caracter
                } else if (isalpha(ch) || ch == '_') {
                    state = 29; //trece la noua stare
                    pStartCh = pCrtCh; //memoreaza inceputul ID-ului
                    pCrtCh++; //consuma caracter
                } else if ( '1' <= ch && '9' >= ch) {
                    state = 31; //trece la noua stare
                    pStartCh = pCrtCh; //memoreaza inceputul INT-ului
                    printf("Se memoreaza inceputul INT-ului : %s\n",pStartCh);
                    pCrtCh++; //consuma caracter
                } else if ('0' == ch) {
                    state = 32;
                    pStartCh = pCrtCh; //memoreaza inceputul INT-ului
                    printf("Se memoreaza inceputul INT-ului : %s\n",pStartCh);
                    pCrtCh++; //consuma caracter
                } else if ('\''==ch) {
                    state = 44;
                    pStartCh = pCrtCh; //memoreaza inceputul CHAR-ului
                    pCrtCh++; //consuma caracter
                } else if ('\"'==ch) {
                    state = 48;
                    pCrtCh++;//consuma caracter
                } else if (ch == ' ' || ch=='\n' || ch=='\r' || ch=='\t') {
                    state = 0;
                    pCrtCh++; //consuma caracter
                    if( ch == '\n' || ch == '\r') {
                        lineText++;
                    }
                } else if (ch=='/') {
                    state = 52;
                    pCrtCh++; //consuma caracter
                } else if (ch=='\0') {// pentru EOF ch==-1
                    state = 57;
                    pCrtCh++;
                } else {
                    tkerr(addTk(END),"caracter invalid");
                }
                break;
            case 1 : 
                addTk(COMMA);
                return COMMA;
            case 2 :
                addTk(SEMICOLON);
                return SEMICOLON;
            case 3 :
                addTk(LPAR);
                return LPAR;
            case 4 :
                addTk(RPAR);
                return RPAR;
            case 5 :
                addTk(LBRACKET);
                return LBRACKET;
            case 6 :
                addTk(RBRACKET);
                return RBRACKET;
            case 7 :
                addTk(LACC);
                return LACC;
            case 8 :
                addTk(RACC);
                return RACC;
            case 9 :
                if (ch == '=') {
                    state = 10; //EQUAL
                    pCrtCh++; // consuma caracter
                } else {
                    state = 11; //ASSIGN
                    //nu se consuma caracter
                }
                break;
            case 10 :
                addTk(EQUAL);
                return EQUAL;
            case 11 : 
                addTk(ASSIGN);
                return ASSIGN;
            case 12 : 
                if (ch == '=') {
                    state = 13; //NOTEQ
                    pCrtCh++; // consuma caracter
                } else {
                    state = 14; //NOT
                    //nu se consuma caracter
                }
                break;
            case 13 : 
                addTk(NOTEQ);
                return NOTEQ;
            case 14 :
                addTk(NOT);
                return NOT;
            case 15 : 
                if (ch == '=') {
                    state = 16; //LESSEQ
                    pCrtCh++; // consuma caracter
                } else {
                    state = 17; //LESS
                    //nu se consuma caracter
                }
                break;
            case 16 :
                addTk(LESSEQ);
                return LESSEQ;
            case 17 :
                addTk(LESS);
                return LESS;
            case 18 :
                if (ch == '=') {
                    state = 19; //GREATEREQ
                    pCrtCh++; // consuma caracter
                } else {
                    state = 20; //GREATER
                    //nu se consuma caracter
                }
                break;
            case 19 :
                addTk(GREATEREQ);
                return GREATEREQ;
            case 20 :
                addTk(GREATER);
                return GREATER;
            case 21 :
                addTk(ADD);
                return ADD;
            case 22 :
                addTk(SUB);
                return SUB;
            case 23 :
                addTk(MUL);
                return MUL;
            case 24 :
                addTk(DOT);
                return DOT;
            case 25 :
                if (ch == '&') {
                    state = 26; //AND
                    pCrtCh++; // consuma caracter
                }
                break;
            case 26 :
                addTk(AND);
                return AND;
            case 27 :
                if (ch == '|') {
                    state = 28; //OR
                    pCrtCh++; // consuma caracter
                }
                break;
            case 28 :
                addTk(OR);
                return OR;
            case 29 :
                if (isalnum(ch) || ch == '_') {
                    state = 29;
                    pCrtCh++; //consuma caracter
                } else  {
                    state = 30;
                    //nu se consuma caracter;
                }
                break;
            case 30 :
                nCh = pCrtCh - pStartCh; // lungimea cuvantului gasit
                //printf("Lungimea cuvantului este : %d\n",nCh);
                //teste cuvinte cheie
                if (nCh == 6) {
                    if (!memcmp(pStartCh,"struct",6)) {
                        tk = addTk(STRUCT);
                    } else if ( !memcmp(pStartCh,"double",6)) {
                        tk = addTk(DOUBLE);
                    } else if ( !memcmp(pStartCh,"return",6)) {
                        tk = addTk(RETURN);
                    } else {
                        //daca nu este cuvant cheie atunci este un ID
                        tk = addTk(ID);
                        printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
                        tk->text = createString(pStartCh, pCrtCh);
                    }

                } else if (nCh == 5) {
                    if (!memcmp(pStartCh,"break",5)) {
                        tk = addTk(BREAK);
                    } else if ( !memcmp(pStartCh,"while",5)) {
                        tk = addTk(WHILE);
                    } else {
                        //daca nu este cuvant cheie atunci este un ID
                        tk = addTk(ID);
                        printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
                        tk->text = createString(pStartCh, pCrtCh);
                    }

                } else if (nCh == 4) {
                    if (!memcmp(pStartCh,"char",4)) {
                        tk = addTk(CHAR);
                    } else if ( !memcmp(pStartCh,"else",4)) {
                        tk = addTk(ELSE);
                    } else if ( !memcmp(pStartCh,"void",4)) {
                        tk = addTk(VOID);
                    } else {
                        //daca nu este cuvant cheie atunci este un ID
                        tk = addTk(ID);
                        printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
                        tk->text = createString(pStartCh, pCrtCh);
                    }

                } else if (nCh == 3) {
                    if (!memcmp(pStartCh,"for",3)) {
                        tk = addTk(FOR);
                    } else if ( !memcmp(pStartCh,"int",3)) {
                        tk = addTk(INT); 
                    } else {
                        //daca nu este cuvant cheie atunci este un ID
                        tk = addTk(ID);
                        printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
                        tk->text = createString(pStartCh, pCrtCh);
                    }

                } else if (nCh == 2 && !memcmp(pStartCh,"if",2) ) {
                        tk = addTk(IF);
                } else { 
                        //daca nu este cuvant cheie atunci este un ID
                        tk = addTk(ID);
                        printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
                        tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;
            case 31 :
                if ('0' <= ch && '9' >= ch) {
                    state = 31;//CT_INT normal
                    pCrtCh++; //consuma caracter
                } else if (ch == '.') {
                    state = 38; //CT_REAL
                    pCrtCh++; //consuma caracter
                } else if (ch == 'e' || ch == 'E') {
                    state = 40; //CT_REAL
                    pCrtCh++; //consuma caracter
                } else {
                    state = 36; // CT_INT normal
                    ct_int = 10;
                    // nu se consuma caracter
                }
                break;
            case 32 : 
                if (ch == 'x') {
                    state = 33;//CT_INT hexa
                    ct_int = 16;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 35;//CT_INT octal
                    ct_int = 8;
                }
                break;
            case 33 :
                if (('0' <= ch && '9' >=ch) || ('a'<=ch && 'f'>=ch) || ('A'<=ch && 'F' >= ch)) {
                    state = 34;
                    pCrtCh++;//consuma caracter
                } else {
                    state = 57;
                    int_value = createString(pStartCh, pCrtCh);
                    printf("EROARE CT_INT: %s\n",int_value);
                }
                break;
            case 34 :
                if (('0' <= ch && '9' >=ch) || ('a'<=ch && 'f'>=ch) || ('A'<=ch && 'F' >= ch)) {
                    state = 34;
                    pCrtCh++;//consuma caracter
                } else {
                    state = 36; 
                    pCrtCh++; //consuma caracter
                }
                break;
            case 35 :
                if ('0' <= ch && '7' >=ch) {
                    state = 35;
                    pCrtCh++;//consuma caracter
                } else if (ch == '8' || ch == '9') {
                    state = 37;
                    pCrtCh++; //consuma caracter
                } else if (ch == '.') {
                    state = 38;
                    pCrtCh++;//consuma caracter
                } else if (ch == 'e' || ch == 'E') {
                    state = 40;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 36;
                    // nu consuma caracter
                }
                break;
            case 36 : //============================================CT_INT========//
                tk = addTk(CT_INT);
                int_value = createString(pStartCh, pCrtCh);
                printf("INT_VALUE: %s\n",int_value);
                tk->i = strtol(int_value, NULL, ct_int);
                printf("Valoarea token-ului este : %ld\n",strtol(int_value, NULL, ct_int));
                return CT_INT;
            case 37 :
                if ('0'<=ch && '9' >=ch) {
                    state = 37;
                    pCrtCh++; //consuma caracter
                } else if ('.' == ch) {
                    state = 38;
                    pCrtCh++; //consuma caracter
                } else if (ch == 'e' || ch == 'E') {
                    state = 40;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 57;
                    int_value = createString(pStartCh, pCrtCh);
                    printf("EROARE CT_INT: %s\n",int_value);
                }
                break;
            case 38 :
                if ('0'<=ch && '9' >=ch) {
                    state = 39;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 39 :
                if ('0'<=ch && '9' >=ch) {
                    state = 39;
                    pCrtCh++; //consuma caracter
                } else if (ch == 'e' || ch == 'E') {
                    state = 40;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 43;
                    //nu se consuma caracter
                }
                break;
            case 40 :
                if (ch == '-' || ch == '+') {
                    state = 41;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 41;
                    //nu consuma caracter
                }
                break;
            case 41 :
                if ('0'<=ch && '9' >=ch) {
                    state = 42;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 42 :
                if ('0'<=ch && '9' >=ch) {
                    state = 42;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 43;
                    //nu se consuma caracter
                }
                break;
            case 43 : //============================================CT_REAL========//
                tk = addTk(CT_REAL);
                double_value = createString(pStartCh, pCrtCh);
                tk->r = atof(double_value);
                return CT_REAL;
            case 44 :
                if (ch =='\\') {
                    state = 45;
                    pCrtCh++; //consuma caracter
                } else if (ch != '\'' && ch != '\\') {
                    state = 46;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 45 :
                if (ch=='a' || ch=='b' || ch=='f' || ch=='n' || ch=='r' || ch=='t' || ch=='v' || ch=='\'' || ch=='?' || ch=='\"' || ch=='\\' || ch=='0' ) {
                    state = 46;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 46 :
                if (ch =='\'') {
                    state = 47;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 47 : //============================================CT_CHAR========//
                tk = addTk(CT_CHAR);
                int_value = createString(pStartCh, pCrtCh);
                tk->i = *int_value;
                return CT_CHAR;
            case 48 : 
                if (ch=='\\') {
                    state = 49;
                    pCrtCh++; //consuma caracter
                } else if (ch != '\"' && ch != '\\') {
                    state = 50;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 50;
                    //nu se consuma caracter
                }
                break;
            case 49 :
                if (ch=='a' || ch=='b' || ch=='f' || ch=='n' || ch=='r' || ch=='t' || ch=='v' || ch=='\'' || ch=='?' || ch=='\"' || ch=='\\' || ch=='0' ) {
                    state = 49;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 50 :
                if (ch=='\"') {
                    state = 51;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 51 : //============================================CT_STRING========//
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh, pCrtCh);
                return CT_STRING;
            case 52 :
                if (ch=='/'){
                    state = 53;
                    pCrtCh++; //consuma caracter
                } else if (ch=='*') {
                    state = 54;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 56;
                    //nu se consuma caracter
                }
                break;
            case 53 :
                if (ch!='\n' && ch!='\r' && ch!='\0') {
                    state = 53;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 0;
                    //nu se consuma caracter
                }
                break;
            case 54 :
                if(ch!='*') {
                    state = 54;
                    pCrtCh++; //consuma caracter
                } else if(ch == '*') {
                    state = 55;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 55 :
                if (ch == '*') {
                    state = 55;
                    pCrtCh++; //consuma caracter
                } else if (ch!='*') {
                    state = 54;
                    pCrtCh++; //consuma caracter
                } else if (ch=='/') {
                    state = 0;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 56 :
                addTk(DIV);
                return DIV;
            case 57 :
                addTk(END);
                return END;
        }
        printf("Character: %c\tState: %d\n",ch,state);
    }
}

int main() {
    FILE *fin;
    int a;
    char *buff;
    int i=0;
    buff = (char *)malloc(sizeof(char)*1000);
    if ((fin = fopen("fisier.txt", "r")) == NULL)
    {
        printf("ERROR opening the file!\n");
        return -1;
    }
    while ( (a=fgetc(fin)) != EOF){
        buff[i++]=a;
    }
    pCrtCh=buff;
    while (getNextToken()!= END) {}
    printf("%s\nBuffLen = %d\n==========THE END=======\n",buff,i);
    printf("\n\t Analizatorul lexical:\n\n");
    printTokens();
    printf("\n");
    fclose(fin);
}
