#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

enum {COMMA,SEMICOLON,LPAR,RPAR,LBRACKET,RBRACKET,LACC,RACC,EQUAL,ASSIGN,NOTEQ,NOT,LESSEQ,LESS,GREATEREQ, GREATER,ADD,SUB,MUL,DOT,AND,OR,ID,CT_INT,CT_REAL,CT_CHAR,CT_STRING}; // codurile AL
typedef struct _Token {
    int code; // codul (numele)
    union
    {
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
        long int i; // folosit pentru CT_INT, CT_CHAR
        double r; // folosit pentru CT_REAL
    };
int line; // linia din fisierul de intrare
struct _Token *next;// inlantuire la urmatorul AL
}Token;

Token *addTk(int code) {
    Token *tk;
    SAFEALLOC(tk,Token)
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
    } else {
        tokens=tk;
    }
    lastToken = tk;
    return tk;
}

void err(const char *fmt,...) {
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc()'\n',stderr);
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

int getNextToken() {
    int state = 0;
    int nCh;
    char ch;
    const char *pStartCh;
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
                    pCrtCh++; //consuma caracter
                } else if ('0' == ch) {
                    state = 32;
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
                //teste cuvinte cheie
                if (nCh == 6) {
                    if (!memcmp(pStartCh,"struct",6)) {
                        tk = addTk(STRUCT);
                    } else if ( !memcmp(pStartCh,"double",6) {
                        tk = addTk(DOUBLE);
                    } else if ( !memcmp(pStartCh,"return",6) {
                        tk = addTk(RETURN);
                    }
                } else if (nCh == 5) {
                    if (!memcmp(pStartCh,"break",5)) {
                        tk = addTk(BREAK);
                    } else if ( !memcmp(pStartCh,"while",5) {
                        tk = addTk(WHILE);
                    }
                } else if (nCh == 4) {
                    if (!memcmp(pStartCh,"char",4)) {
                        tk = addTk(CHAR);
                    } else if ( !memcmp(pStartCh,"else",4) {
                        tk = addTk(ELSE);
                    } else if ( !memcmp(pStartCh,"void",4) {
                        tk = addTk(VOID);
                    }
                } else if (nCh == 3) {
                    if (!memcmp(pStartCh,"for",3)) {
                        tk = addTk(FOR);
                    } else if ( !memcmp(pStartCh,"int",3) {
                        tk = addTk(INT); 
                    }
                } else if (nCh == 2 && !memcmp(pStartCh,"if",2) ) {
                        tk = addTk(IF);
                } else { 
                        //daca nu este cuvant cheie atunci este un ID
                        tk = addTk(ID);
                        tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;
            case 31 :
                if ('0' <= ch && '9' >= ch) {
                    state = 31;
                    pCrtCh++; //consuma caracter
                } else if (ch == '.') {
                    state = 38;
                    pCrtCh++; //consuma caracter
                } else if (ch == 'e' || ch == 'E') {
                    state = 40;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 36;
                    // nu se consuma caracter
                }
                break;
            case 32 : 
                if (ch == 'x') {
                    state = 33;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 35;
                }
                break;
            case 33 :
                if (('0' <= ch && '9' >=ch) || ('a'<=ch && 'f'>=ch) || ('A'<=ch && 'F' >= ch)) {
                    state = 34;
                    pCrtCh++;//consuma caracter
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
                tk->text = createString(pStartCh, pCrtCh);
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
                tk->text = createString(pStartCh, pCrtCh);
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
                tk->text = createString(pStartCh, pCrtCh);
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
    }
}

int main( int argc, char *args) {
    
}
