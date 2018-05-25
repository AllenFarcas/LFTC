#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define SAFEALLOC(var,Type) if(((var)=(Type*)malloc(sizeof(Type)))==NULL) myerr("not enough memory");

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
    //NEWLINE
}; // codurile AL

typedef struct s_Token {
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
    struct s_Token *next;// inlantuire la urmatorul AL
}Token;

//=======GLOBAL VARIABLES==========
char *pStartCh;
Token *tokens;
Token *lastToken;
Token *crtTk;
Token *consumedTk;
int lineText=0;
char *pCrtCh;
int limit = 0;

//===========FUNCTIONS=============
struct _Symbol;
typedef struct _Symbol Symbol;
typedef struct{
    Symbol **begin; // the beginning of the symbols, or NULL
    Symbol **end; // the position after the last symbol
    Symbol **after; // the position after the allocated space
}Symbols;

enum{TB_INT,TB_DOUBLE,TB_CHAR,TB_STRUCT,TB_VOID};
typedef struct {
    int typeBase; // TB_*
    Symbol *s; // struct definition for TB_STRUCT
    int nElements; // >0 array of given size, 0=array without size, <0 non array
}Type;

enum{CLS_VAR,CLS_FUNC,CLS_EXTFUNC,CLS_STRUCT};
enum{MEM_GLOBAL,MEM_ARG,MEM_LOCAL};
typedef struct _Symbol{
    const char *name; // a reference to the name stored in a token
    int cls; // CLS_*
    int mem; // MEM_*
    Type type;
    int depth; // 0-global, 1-in function, 2... - nested blocks in function
    union{
        Symbols args; // used only of functions
        Symbols members; // used only for structs
    };
}Symbol;
Symbols symbols;

void initSymbols(Symbols *symbols) {
    symbols->begin=NULL;
    symbols->end=NULL;
    symbols->after=NULL;
}

//================Interface =========================


void myerr(const char *fmt, ...);
void printTk(int tkn);

//================GLOBAL VARIABLES===================
int crtDepth=0;
Symbol* crtStruct=NULL;
Symbol* crtFunc=NULL;



Type createType(int typeBase,int nElements){
    Type t;
    t.typeBase=typeBase;
    t.nElements=nElements;
    return t;
}

typedef union{
    long int i; // int, char
    double d; // double
    const char *str; // char[]
}CtVal;

typedef struct{
    Type type; // type of the result
    int isLVal; // if it is a LVal
    int isCtVal; // if it is a constant value (int, real, char, char[])
    CtVal ctVal; // the constat value
}RetVal;

Symbol *addSymbol(Symbols *symbols,const char *name,int cls) {
    Symbol *s;
    if(symbols->end==symbols->after) { // create more room
        int count = symbols->after - symbols->begin;
        int n=count*2; // double the room
        if(n==0) n=1; // needed for the initial case
        symbols->begin=(Symbol**)realloc(symbols->begin, n*sizeof(Symbol*));
        if(symbols->begin==NULL) myerr("not enough memory");
        symbols->end=symbols->begin+count;
        symbols->after=symbols->begin+n;
    }
    SAFEALLOC(s,Symbol)
    *symbols->end++=s;
    s->name=name;
    s->cls=cls;
    s->depth=crtDepth;
    return s;
}

Symbol *findSymbol(Symbols *symbols, const char *name){
    //printf("\nCautam simbol %s\n",name);
    if(symbols->begin == NULL){
        //printf("Begin este NULL\n");
        return NULL;
    }
    Symbol **iterator = symbols->end-1;
    //printf("Begin nu e null\n");
    //printf("Nume: %s\n",(*iterator)->name);
    while(iterator >= (Symbol **) symbols->begin){
        //printf("iterator !=\n");
        if(strcmp((*iterator)->name,name) == 0){
            //printf("Am gasit simbol: %s\n",(*iterator)->name);
            return *iterator;
        }
        //printf("repetam\n");
        iterator--;
    }
    return NULL;
}

void deleteSymbolsAfter(Symbols *symbols,Symbol *start) {
    Symbol **iterator = symbols->end-1;
    symbols->end--;
    while(*iterator != start){
        //printf("Stergem symbol: %s\n",(*iterator)->name);

        //free(*iterator);
        iterator--;//--;//
    }
    symbols->end ++;//(Symbol **) (iterator + 1);
}

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

void printTokens() {
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

char escapeChar(char c){
    switch(c) {
        case 'a' : return '\a';
        case 'b' : return '\b';
        case 'f' : return '\f';
        case 'n' : return '\n';
        case 'r' : return '\r';
        case 't' : return '\t';
        case 'v' : return '\v';
        case '\'' : return '\'';
        case '\?' : return '\?';
        case '\"' : return '\"';
        case '\\' : return '\\';
        case '0' : return '\0';
        default :
            printf("Eroare nu exista char escaped cu %c\n",c);
            exit(0);
    }
}

char* createString(char *start, char *stop) {
    char *c;
    int n;
    n = (int) (stop - start);
    c = (char *)malloc(sizeof(char)*n+1);
    c = strncpy(c,start,sizeof(char)*n);
    c[n+1] = '\0';
    for(int i=0;i<n;i++) {
        if(c[i]=='\\'){
            c[i] = escapeChar(c[i+1]);
            memmove(c+i+1, c+i+2, strlen(c)-i);
        }
    }
    return c;
}

void myerr(const char *fmt, ...) {
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
    fflush(stdout);
    fflush(stderr);
    exit(-1);
}

void translateTkn(Token *t) {
    switch (t->code) {
        case COMMA : printf("COMMA Line: %d\n",t->line); break;
        case SEMICOLON : printf("SEMICOLON Line: %d\n",t->line); break;
        case LPAR : printf("LPAR Line: %d\n",t->line); break;
        case RPAR : printf("RPAR Line: %d\n",t->line); break;
        case LBRACKET : printf("LBRACKET Line: %d\n",t->line); break;
        case RBRACKET : printf("RBRACKET Line: %d\n",t->line); break;
        case LACC : printf("LACC Line: %d\n",t->line); break;
        case RACC : printf("RACC Line: %d\n",t->line); break;
        case EQUAL : printf("EQUAL Line: %d\n",t->line); break;
        case ASSIGN : printf("ASSIGN Line: %d\n",t->line); break;
        case NOTEQ : printf("NOTEQ Line: %d\n",t->line); break;
        case NOT : printf("NOT Line: %d\n",t->line); break;
        case LESSEQ : printf("LESSEQ Line: %d\n",t->line); break;
        case LESS : printf("LESS Line: %d\n",t->line); break;
        case GREATEREQ : printf("GREATEREQ Line: %d\n",t->line); break;
        case GREATER : printf("GREATER Line: %d\n",t->line); break;
        case ADD : printf("ADD Line: %d\n",t->line); break;
        case SUB : printf("SUB Line: %d\n",t->line); break;
        case MUL : printf("MUL Line: %d\n",t->line); break;
        case DIV : printf("DIV Line: %d\n",t->line); break;
        case AND : printf("AND Line: %d\n",t->line); break;
        case OR : printf("OR Line: %d\n",t->line); break;
        case ID : printf("ID: %s Line: %d\n",t->text ,t->line); break;
        case DOT : printf("DOT Line: %d\n",t->line); break;
        case CT_INT : printf("CT_INT: %ld Line: %d\n",t->i, t->line); break;
        case CT_REAL : printf("CT_REAL: %lf Line: %d\n",t->r, t->line); break;
        case CT_CHAR : printf("CT_CHAR: %li Line: %d\n",t->i, t->line); break;
        case CT_STRING : printf("CT_STRING: %s Line: %d\n",t->text, t->line); break;
        case STRUCT : printf("STRUCT Line: %d\n",t->line); break;
        case DOUBLE : printf("DOUBLE Line: %d\n",t->line); break;
        case RETURN : printf("RETURN Line: %d\n",t->line); break;
        case BREAK : printf("BREAK Line: %d\n",t->line); break;
        case WHILE : printf("WHILE Line: %d\n",t->line); break;
        case CHAR : printf("CHAR Line: %d\n",t->line); break;
        case ELSE : printf("ELSE Line: %d\n",t->line); break;
        case VOID : printf("VOID Line: %d\n",t->line); break;
        case FOR : printf("FOR Line: %d\n",t->line); break;
        case INT : printf("INT Line: %d\n",t->line); break;
        case IF : printf("IF Line: %d\n",t->line); break;
        case END : printf("END Line: %d\n",t->line); break;
            //case NEWLINE : printf("\n"); break;
        default: printf("Eroare tiparire in printTk se cere valoarea %d\n",t->code); break;
    }
}

void printTk(int tkn) {
    switch (tkn) {
        case 0 : printf("COMMA "); break;
        case 1 : printf("SEMICOLON \n"); break;
        case 2 : printf("LPAR "); break;
        case 3 : printf("RPAR \n"); break;
        case 4 : printf("LBRACKET "); break;
        case 5 : printf("RBRACKET "); break;
        case 6 : printf("LACC "); break;
        case 7 : printf("RACC \n"); break;
        case 8 : printf("EQUAL "); break;
        case 9 : printf("ASSIGN "); break;
        case 10 : printf("NOTEQ "); break;
        case 11 : printf("NOT "); break;
        case 12 : printf("LESSEQ "); break;
        case 13 : printf("LESS "); break;
        case 14 : printf("GREATEREQ "); break;
        case 15 : printf("GREATER "); break;
        case 16 : printf("ADD "); break;
        case 17 : printf("SUB "); break;
        case 18 : printf("MUL "); break;
        case 19 : printf("DIV "); break;
        case 20 : printf("AND "); break;
        case 21 : printf("OR "); break;
        case 22 : printf("ID"); break;
        case 23 : printf("DOT "); break;
        case 24 : printf("CT_INT"); break;
        case 25 : printf("CT_REAL"); break;
        case 26 : printf("CT_CHAR"); break;
        case 27 : printf("CT_STRING"); break;
        case 28 : printf("STRUCT "); break;
        case 29 : printf("DOUBLE "); break;
        case 30 : printf("RETURN "); break;
        case 31 : printf("BREAK "); break;
        case 32 : printf("WHILE "); break;
        case 33 : printf("CHAR "); break;
        case 34 : printf("ELSE "); break;
        case 35 : printf("VOID "); break;
        case 36 : printf("FOR "); break;
        case 37 : printf("INT "); break;
        case 38 : printf("IF "); break;
        case 39 : printf("END "); break;
            //case 40 : printf("\n"); break;
        default: printf("Eroare tiparire in printTk se cere valoarea %d\n",tkn); break;
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
                    //printf("Se memoreaza inceputul INT-ului : %s\n",pStartCh);
                    pCrtCh++; //consuma caracter
                } else if ('0' == ch) {
                    state = 32;
                    pStartCh = pCrtCh; //memoreaza inceputul INT-ului
                    //printf("Se memoreaza inceputul INT-ului : %s\n",pStartCh);
                    pCrtCh++; //consuma caracter
                } else if ('\''==ch) {
                    state = 44;
                    pStartCh = pCrtCh+1; //memoreaza inceputul CHAR-ului
                    pCrtCh++; //consuma caracter
                } else if ('\"'==ch) {
                    state = 48;
                    pStartCh = pCrtCh+1;
                    pCrtCh++;//consuma caracter
                } else if (ch == ' ' || ch=='\n' || ch=='\r' || ch=='\t') {
                    state = 0;
                    pCrtCh++; //consuma caracter
                    if( ch == '\n') {
                        lineText++;
                        //addTk(NEWLINE);
                        //printf("===========LINIE NOUA=============");
                        //return NEWLINE;
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
                        //printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
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
                        //printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
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
                        //printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
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
                        //printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
                        tk->text = createString(pStartCh, pCrtCh);
                    }

                } else if (nCh == 2 && !memcmp(pStartCh,"if",2) ) {
                    tk = addTk(IF);
                } else {
                    //daca nu este cuvant cheie atunci este un ID
                    tk = addTk(ID);
                    //printf("Am gasit un ID: %s\n",createString(pStartCh,pCrtCh));
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
                    //printf("EROARE CT_INT: %s\n",int_value);
                }
                break;
            case 34 :
                if (('0' <= ch && '9' >=ch) || ('a'<=ch && 'f'>=ch) || ('A'<=ch && 'F' >= ch)) {
                    state = 34;
                    pCrtCh++;//consuma caracter
                } else {
                    state = 36;
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
                //printf("INT_VALUE: %s\n",int_value);
                tk->i = strtol(int_value, NULL, ct_int);
                //printf("Valoarea token-ului este : %ld\n",strtol(int_value, NULL, ct_int));
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
                    //printf("EROARE CT_INT: %s\n",int_value);
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
                int_value = createString(pStartCh, pCrtCh-1);
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
                    state = 50;
                    pCrtCh++; //consuma caracter
                }
                break;
            case 50 :
                if (ch=='\"') {
                    state = 51;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 48;
                    //nu se consuma caracter
                }
                break;
            case 51 : //============================================CT_STRING========//
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh, pCrtCh-1);
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
                    if(ch=='\0') {
                        state = 57;
                        //printf("EROARE : blocat in comentariu /* \n");
                    }
                    pCrtCh++; //consuma caracter
                } else if(ch == '*') {
                    state = 55;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 57;
                    //printf("EROARE : blocat in comentariu /* \n");
                }
                break;
            case 55 :
                if (ch == '*') {
                    state = 55;
                    pCrtCh++; //consuma caracter
                } else if (ch=='/') {
                    state = 0;
                    pCrtCh++; //consuma caracter
                } else if (ch!='*' || ch!='/') {
                    state = 54;
                    pCrtCh++; //consuma caracter
                } else {
                    state = 57;
                    //printf("EROARE : blocat in comentariu /* \n");
                }
                break;
            case 56 :
                addTk(DIV);
                return DIV;
            case 57 :
                addTk(END);
                return END;
            default:
                addTk(END);
                return END;
        }
        //printf("Character: %c\tState: %d\n",ch,state);
    }
}

int consume(int code){
    if(crtTk->code == code){
        consumedTk = crtTk;
        /*if(crtTk != NULL){
         printf("Token consumed: ");
         translateTkn(crtTk);
        }*/
        crtTk = crtTk->next;
        //printf("Next token is: ");
        //translateTkn(crtTk);
        return 1;
    }
    return 0;
}

void cast(Type *dst,Type *src) {

    if(src->nElements>-1){
        if(dst->nElements>-1){
            if(src->typeBase!=dst->typeBase)
                tkerr(crtTk,"an array cannot be converted to an array of another type");
        }else{
            tkerr(crtTk,"an array cannot be converted to a non-array");
        }
    }else{
        if(dst->nElements>-1){
            tkerr(crtTk,"a non-array cannot be converted to an array");
        }
    }
    switch(src->typeBase){
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
        switch(dst->typeBase){
            case TB_CHAR:
            case TB_INT:
            case TB_DOUBLE:
                return;
        }
        case TB_STRUCT:
            if(dst->typeBase==TB_STRUCT){
                if(src->s!=dst->s)
                    tkerr(crtTk,"a structure cannot be converted to another one");
                return;
            }
    }
    tkerr(crtTk,"incompatible types");
}

Type getArithType(Type *s1,Type *s2) { //char, int double
    Type r;
    r.nElements=-1;
    if(s1->typeBase==TB_DOUBLE || s2->typeBase==TB_DOUBLE) {
        r.typeBase = TB_DOUBLE;
    } else if(s1->typeBase==TB_INT || s2->typeBase==TB_INT) {
        r.typeBase = TB_INT;
    } else if(s1->typeBase==TB_CHAR && s2->typeBase==TB_CHAR){
        r.typeBase = TB_CHAR;
    }
    return r;
}


int unit();
int declStruct();
int declVar();
int typeBase(Type *ret);
int arrayDecl(Type *ret);
int typeName(Type *ret);
int declFuncAux(Type *t);
int declFunc();
int funcArg();
int stm();
int stmCompound();
int expr(RetVal *rv);
int exprAssign(RetVal *rv);
int exprOr(RetVal *rv);
int exprOrPrim(RetVal *rv);
int exprAnd(RetVal *rv);
int exprAndPrim(RetVal *rv);
int exprEq(RetVal *rv);
int exprEqPrim(RetVal *rv);
int exprRel(RetVal *rv);
int exprRelPrim(RetVal *rv);
int exprAdd(RetVal *rv);
int exprAddPrim(RetVal *rv);
int exprMul(RetVal *rv);
int exprMulPrim(RetVal *rv);
int exprCast(RetVal *rv);
int exprUnary(RetVal *rv);
int exprPostfix(RetVal *rv);
int exprPostfixPrim(RetVal *rv);
int exprPrimary(RetVal *rv);


//unit: ( declStruct | declFunc | declVar )* END
int unit() {
    //printf("unit()\n");
    //translateTkn(crtTk);
    for(;;){
        if(declStruct()){continue;}
        if(declFunc()) {continue;}
        if(declVar()) {continue;}
        else break;

    }
    if(consume(END)) {
        return 1;
    } else tkerr(crtTk,"unit: Lipseste END");
    return 0;
}

//declStruct: STRUCT ID LACC declVar* RACC SEMICOLON
/*declStruct:
    STRUCT ID:tkName LACC
        {
        if(findSymbol(&symbols,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        crtStruct=addSymbol(&symbols,tkName->text,CLS_STRUCT);
        initSymbols(&crtStruct->members);
        }
     declVar* RACC SEMICOLON {crtStruct=NULL;} ;*/
int declStruct(){
    //printf("declStruct()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    Token *tkName;
    if(consume(STRUCT)){
        if(consume(ID)){
            tkName = consumedTk;
            if(consume(LACC)){
                //printf("Am cautat simbol: %s\n",tkName->text);
                if(findSymbol(&symbols,tkName->text)) {
                    //printf("Gasit: %s\n",findSymbol(&symbols,tkName->text)->name);
                    tkerr(crtTk, "symbol redefinition: %s", tkName->text);
                }

                crtStruct=addSymbol(&symbols,tkName->text,CLS_STRUCT);
                initSymbols(&(crtStruct->members)); /** Changed from &crtStruct->members */
                for(;;){
                    if(declVar()){continue;}
                    else break;
                }
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        crtStruct = NULL;
                        return 1;
                    } else tkerr(crtTk,"declStruct: dupa STRUCT ID LACC * RACC => Lipseste ;");
                } else tkerr(crtTk,"declStruct: dupa STRUCT ID LACC * => Lipseste }");
            }
        } else tkerr(crtTk,"declStruct: dupa STRUCT => Lipseste ID");
    }
    crtTk = initialTk;
    return 0;
}

void addVar(Token *tkName,Type *t)  {
    Symbol *s;
    if(crtStruct){
        if(findSymbol(&crtStruct->members,tkName->text)) /** Changed from &crtStruct->members */
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(&crtStruct->members,tkName->text,CLS_VAR); /** Changed from &crtStruct->members */
    }
    else if(crtFunc){
        s=findSymbol(&symbols,tkName->text);
        if(s&&s->depth==crtDepth)
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(&symbols,tkName->text,CLS_VAR);
        s->mem=MEM_LOCAL;
    }
    else{
        if(findSymbol(&symbols,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        s=addSymbol(&symbols,tkName->text,CLS_VAR);
        s->mem=MEM_GLOBAL;
    }
    s->type=*t;
}
//declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON
/*declVar:  typeBase:t ID:tkName ( arrayDecl:t | {t.nElements=-1;} ) {addVar(tkName,t);}
    ( COMMA ID:tkName ( arrayDecl:t | {t.nElements=-1;} ) {addVar(tkName,t);} )*
    SEMICOLON ;*/
int declVar(){
    //printf("declVar()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    Token *tkName;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            tkName = consumedTk;
            if (arrayDecl(&t)) {}
            else{
                t.nElements=-1;
            }
            addVar(tkName,&t);
            for(;;){
                if(consume(COMMA)){
                    if(consume(ID)){
                        tkName = consumedTk;
                        if (arrayDecl(&t)) {}
                        else {
                            t.nElements=-1;
                        }
                        addVar(tkName,&t);
                    } else tkerr(crtTk,"declVar: dupa typeBase ID arrayDecl? ( COMMA  => Lipseste ID");
                } else break;
            }
            if(consume(SEMICOLON)){
                return 1;
            } else tkerr(crtTk,"declVar: dupa typeBase ID arrayDecl? => Lipseste ;");
        } else tkerr(crtTk,"declVar: dupa typeBase => Lipseste ID");
    }
    crtTk = initialTk;
    return 0;
}

//typeBase: INT | DOUBLE | CHAR | STRUCT ID
/*
 * typeBase(out ret:Type) : INT {ret->typeBase=TB_INT;}
    | DOUBLE {ret->typeBase=TB_DOUBLE;}
    | CHAR {ret->typeBase=TB_CHAR;}
    | STRUCT ID:tkName
        {
        Symbol      *s=findSymbol(&symbols,tkName->text);
        if(s==NULL)tkerr(crtTk,"undefined symbol: %s",tkName->text);
        if(s->cls!=CLS_STRUCT)tkerr(crtTk,"%s is not a struct",tkName->text);
        ret->typeBase=TB_STRUCT;
        ret->s=s;
        } ;*/
int typeBase(Type *ret){
    //printf("typeBase()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    Token *tkName;
    if(consume(INT)){
        ret->typeBase=TB_INT;
        return 1;
    }
    if(consume(DOUBLE)){
        ret->typeBase=TB_DOUBLE;
        return 1;
    }
    if(consume(CHAR)){
        ret->typeBase=TB_CHAR;
        return 1;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            tkName = consumedTk;
            Symbol *s=findSymbol(&symbols,tkName->text);
            if(s==NULL) {
                tkerr(crtTk, "undefined symbol: %s", tkName->text);
            }
            else {
                if (s->cls != CLS_STRUCT)
                    tkerr(crtTk, "%s is not a struct", tkName->text);
                ret->typeBase = TB_STRUCT;
                ret->s = s;
            }
            //printf("Am consumat ID in typeBase\n");
            return 1;
        } else tkerr(crtTk,"typeBase: dupa INT | DOUBLE | CHAR | STRUCT => Lipseste ID");
    }
    crtTk = initialTk;
    return 0;
}

//arrayDecl: LBRACKET expr? RBRACKET
/*
 * arrayDecl(out ret:Type): LBRACKET ( expr
    {
    ret->nElements=0;       // for now do not compute the real size
    }
)? RBRACKET ;

 arrayDecl(out ret:Type): LBRACKET (expr:rv {    // if an expression, get its value
        if(!rv.isCtVal)tkerr(crtTk,"the array size is not a constant");
        if(rv.type.typeBase!=TB_INT)tkerr(crtTk,"the array size is not an integer");
        ret->nElements=rv.ctVal.i;
        }
    | {ret->nElements=0;/*array without given size } ) RBRACKET ; */
int arrayDecl(Type *ret){
    //printf("arrayDecl()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    RetVal rv;
    if(consume(LBRACKET)) {
        if(expr(&rv)) { // if an expression, get its value
            if(!rv.isCtVal) {
                tkerr(crtTk, "the array size is not a constant");
            }
            if(rv.type.typeBase!=TB_INT) {
                tkerr(crtTk, "the array size is not an integer");
            }
            ret->nElements=rv.ctVal.i;
        } else {
            ret->nElements=0;//array without given size
        }
        ret->nElements=0;       // for now do not compute the real size
        if(consume(RBRACKET)){
            return 1;
        } else tkerr(crtTk,"arrayDecl: dupa LBRACKET expr? Lipseste ]");
    }
    crtTk = initialTk;
    return 0;
}

//typeName: typeBase arrayDecl?
/*typeName(out ret:Type) : typeBase:ret ( arrayDecl:ret | {ret->nElements=-1;} ) ;*/
int typeName(Type *ret){
    //printf("typeName()\n");
    translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(typeBase(ret)){
        if(arrayDecl(ret)){
        } else {
            ret->nElements=-1;
        }
        return 1;
    }
    crtTk = initialTk;
    return 0;
}

//( typeBase MUL? | VOID )
/*( typeBase:t ( MUL {t.nElements=0;} | {t.nElements=-1;} ) | VOID {t.typeBase=TB_VOID;} )*/
int declFuncAux(Type *t){
    //printf("declFuncAux()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(typeBase(t)){
        if (consume(MUL)) {
            t->nElements = 0;
        } else {
            t->nElements=-1;
        }
        return 1;
    } else if (consume(VOID)){
        t->typeBase=TB_VOID;
        return 1;
    }
    crtTk = initialTk;
    return 0;
}

//declFunc: declFuncAux ID LPAR ( funcArg ( COMMA funcArg )* )? RPAR stmCompound
/*
 * declFunc:

    ID:tkName LPAR
        {
        if(findSymbol(&symbols,tkName->text))
            tkerr(crtTk,"symbol redefinition: %s",tkName->text);
        crtFunc=addSymbol(&symbols,tkName->text,CLS_FUNC);
        initSymbols(&crtFunc->args);
        crtFunc->type=t;
        crtDepth++;
        }
    ( funcArg ( COMMA funcArg )* )?
    RPAR {crtDepth--;}
    stmCompound
        {
        deleteSymbolsAfter(&symbols,crtFunc);
        crtFunc=NULL;
        } ;*/
int declFunc(){
    //printf("declFunc()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    Token *tkName;
    Type t;
    if(declFuncAux(&t)){
        if(consume(ID)){
            tkName = consumedTk;
            if(consume(LPAR)){
                if(findSymbol(&symbols,tkName->text))
                    tkerr(crtTk,"symbol redefinition: %s",tkName->text);
                crtFunc=addSymbol(&symbols,tkName->text,CLS_FUNC);
                initSymbols(&crtFunc->args); /** Changed from &crtStruct->args */
                crtFunc->type=t; /** Changed t is of type Type* */
                crtDepth++;
                if(funcArg()){
                    for(;;){
                        if(consume(COMMA)){
                            if(funcArg()){}
                            else tkerr(crtTk,"declFunc: dupa ( typeBase MUL? | VOID ) ID LPAR ( funcArg ( COMMA => Lipseste argument dupa ,");
                        } else break;
                    }
                }
                //printf("Current token to consume: %s\n",crtTk->code);
                /*crtTk=crtTk->next->next;
                if(crtTk != NULL){
                    printf("RPAR?: ");
                    translateTkn(crtTk);
                }*/
                if(consume(RPAR)){
                    crtDepth--;
                    if(stmCompound()){
                        deleteSymbolsAfter(&symbols,crtFunc);
                        crtFunc=NULL;
                        return 1;
                    }else tkerr(crtTk,"declFunc: dupa ( typeBase MUL? | VOID ) ID LPAR ( funcArg ( COMMA funcArg )* )? RPAR => Lipseste stmCompound");
                } else tkerr(crtTk,"declFunc: dupa ( typeBase MUL? | VOID ) ID LPAR ( funcArg ( COMMA funcArg )* )? =>Lipseste )");
            }
        } else tkerr(crtTk,"declFunc: dupa ( typeBase MUL? | VOID ) => Lipseste ID");
    }
    crtTk = initialTk;
    return 0;
}

//funcArg: typeBase ID arrayDecl?
/*
 * funcArg: typeBase:t ID:tkName ( arrayDecl:t | {t.nElements=-1;} )
    {
    Symbol  *s=addSymbol(&symbols,tkName->text,CLS_VAR);
    s->mem=MEM_ARG;
    s->type=t;
    s=addSymbol(&crtFunc->args,tkName->text,CLS_VAR);
    s->mem=MEM_ARG;
    s->type=t;
    } ;*/
int funcArg(){
    //printf("funcArg()\n");
    //translateTkn(crtTk);
    Token *tkName;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            tkName = consumedTk;
            if(arrayDecl(&t)){}
            else {
                t.nElements=-1;
            }
            Symbol *s=addSymbol(&symbols,tkName->text,CLS_VAR);
            s->mem=MEM_ARG;
            s->type=t; /** Changed t is of type Type* */
            s=addSymbol(&crtFunc->args,tkName->text,CLS_VAR); /** Changed from &crtStruct->args */
            s->mem=MEM_ARG;
            s->type=t; /** Changed t is of type Type* */
        } else tkerr(crtTk,"funcArg: dupa typeBase => Lipseste ID");
    }
    return 0;
}

//stm: stmCompound | IF LPAR expr RPAR stm ( ELSE stm )? | WHILE LPAR expr RPAR stm | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm | BREAK SEMICOLON | RETURN expr? SEMICOLON | expr? SEMICOLON
/*stm: stmCompound
| IF LPAR expr:rv {
if(rv.type.typeBase==TB_STRUCT)
tkerr(crtTk,"a structure cannot be logically tested");
}
RPAR stm ( ELSE stm )?
| WHILE LPAR expr:rv {
if(rv.type.typeBase==TB_STRUCT)
tkerr(crtTk,"a structure cannot be logically tested");
}
RPAR stm
| FOR LPAR expr:rv1? SEMICOLON ( expr:rv2 {
if(rv2.type.typeBase==TB_STRUCT)
tkerr(crtTk,"a structure cannot be logically tested");
}
)? SEMICOLON expr:rv3? RPAR stm
| BREAK SEMICOLON
| RETURN ( expr:rv {
if(crtFunc->type.typeBase==TB_VOID)
tkerr(crtTk,"a void function cannot return a value");
cast(&crtFunc->type,&rv.type);
} )? SEMICOLON
| expr:rv? SEMICOLON ;*/
int stm(){
    //printf("stm()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    RetVal rv,rv1,rv2,rv3;
    if(stmCompound()){return 1;}
    else if(consume(IF)){
        if(consume(LPAR)){
            if(expr(&rv)){
                if(rv.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){
                            if(stm()){
                                return 1;
                            } else tkerr(crtTk,"stm: dupa IF LPAR expr RPAR stm ( ELSE => Lipseste stm in ramura ELSE");
                        }
                        return 1;
                    } else tkerr(crtTk,"stm: dupa IF LPAR expr RPAR => Lipseste stm in ramura de IF");
                } else tkerr(crtTk,"stm: dupa IF LPAR expr => Lipseste ) dupa IF");
            } else tkerr(crtTk,"stm: dupa IF LPAR => Lipseste conditia pentru IF");
        } else tkerr(crtTk,"stm: dupa IF => Lipseste ( dupa IF");
    } else if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr(&rv)){
                if(rv.type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"a structure cannot be logically tested");
                if(consume(RPAR)){
                    if(stm()){
                        return 1;
                    } else tkerr(crtTk,"stm: dupa WHILE LPAR expr RPAR => Lipseste stm in WHILE");
                } else tkerr(crtTk,"stm: dupa WHILE LPAR expr => Lipseste ) dupa WHILE");
            } else tkerr(crtTk,"stm: dupa WHILE LPAR => Lipseste conditia pentru WHILE");
        } else tkerr(crtTk,"stm: dupa WHILE => Lipseste ( dupa WHILE");
    } else if(consume(FOR)){
        if(consume(LPAR)){
            expr(&rv1);
            if(consume(SEMICOLON)){
                if(expr(&rv2)) {
                    if (rv2.type.typeBase == TB_STRUCT)
                        tkerr(crtTk, "a structure cannot be logically tested");
                }
                if(consume(SEMICOLON)){
                    expr(&rv3);
                    if(consume(RPAR)){
                        if(stm()){
                            return 1;
                        } else tkerr(crtTk,"stm: dupa FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR => Lipseste stm in FOR");
                    } else tkerr(crtTk,"stm: dupa FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? => Lipseste ) dupa FOR");
                } else tkerr(crtTk,"stm: dupa FOR LPAR expr? SEMICOLON expr? => Lipseste al doilea ; in stm al FOR");
            } else tkerr(crtTk,"stm: dupa FOR LPAR expr? => Lipseste primul ; in stm al FOR");
        } else tkerr(crtTk,"stm: dupa FOR => Lipseste ( dupa FOR");
    } else if(consume(BREAK)){
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(crtTk,"stm: dupa BREAK => Lipseste ;");
    } else if(consume(RETURN)){
        if(expr(&rv)) {
            if (crtFunc->type.typeBase == TB_VOID)
                tkerr(crtTk, "a void function cannot return a value");
            //printf("crtFunc: %d    rv: %d",crtFunc->type.nElements,rv.type.nElements);
            cast(&crtFunc->type, &rv.type);
        }
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(crtTk,"stm: dupa RETURN expr? => Lipseste ;");
    } else if(expr(&rv)) {
        if(consume(SEMICOLON)){
            return 1;
        } else tkerr(crtTk,"stm: dupa expr? => Lipseste ;");
    } else if(consume(SEMICOLON)){
        return 1;
    }
    crtTk = initialTk;
    return 0;
}

//stmCompound: LACC ( declVar | stm )* RACC
/*stmCompound: {Symbol *start=symbols.end[-1];}
    LACC {crtDepth++;} ( declVar | stm )* RACC
    {
    crtDepth--;
    deleteSymbolsAfter(&symbols,start);
    } ;
*/
int stmCompound(){
    //printf("stmCompound()\n");
    //translateTkn(crtTk);
    Symbol *start= symbols.end[-1];
    Token *initialTk = crtTk;
    if(consume(LACC)){
        crtDepth++;
        for(;;){
            if(declVar()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            crtDepth--;
            deleteSymbolsAfter(&symbols,start);
            return 1;
        } else tkerr(crtTk,"stmCompound: dupa LACC ( declVar | stm )* => Lipseste }");
    }
    crtTk = initialTk;
    return 0;
}

//expr: exprAssign
//expr(out rv:RetVal): exprAssign:rv ;
int expr(RetVal *rv){
    //printf("expr()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprAssign(rv)) {
        return 1;
    }
    crtTk = initialTk;
    return 0;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr
/*
 * exprAssign(out rv:RetVal): exprUnary:rv ASSIGN exprAssign:rve {
        if(!rv->isLVal)tkerr(crtTk,"cannot assign to a non-lval");
        if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"the arrays cannot be assigned");
        cast(&rv->type,&rve.type);
        rv->isCtVal=rv->isLVal=0;
        }
    | exprOr:rv ;*/
int exprAssign(RetVal *rv){
    //printf("exprAssign()\n");
    //translateTkn(crtTk);
    RetVal rve;
    Token *initialTk = crtTk;
    if(exprUnary(rv)){
        if(consume(ASSIGN)){
            if(exprAssign(&rve)){
                if(!rv->isLVal)tkerr(crtTk,"cannot assign to a non-lval");
                if(rv->type.nElements>-1||rve.type.nElements>-1)
                    tkerr(crtTk,"the arrays cannot be assigned");
                cast(&rv->type,&rve.type);
                rv->isCtVal=rv->isLVal=0;
                return 1;
            } else tkerr(crtTk,"exprAssign: dupa exprUnary ASSIGN => Lipseste exprAssign");
        }
    }
    crtTk = initialTk;
    if(exprOr(rv)){
        return 1;
    }
    crtTk = initialTk;
    return 0;
}

// exprOr: exprAnd exprOrPrim
/*exprOr(out rv:RetVal): exprOr:rv OR exprAnd:rve {
if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
tkerr(crtTk,"a structure cannot be logically tested");
rv->type=createType(TB_INT,-1);
rv->isCtVal=rv->isLVal=0;
}
| exprAnd:rv ;*/
int exprOr(RetVal *rv){
    //printf("exprOr()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprAnd(rv)){
        if(exprOrPrim(rv)){
            return 1;
        } else tkerr(crtTk,"exprOr: dupa exprAnd => Lipseste exprOrPrim");
    }
    crtTk = initialTk;
    return 0;
}

// exprOrPrim: OR exprAnd exprOrPrim | eps
int exprOrPrim(RetVal *rv){
    //printf("exprOrPrim()\n");
    //translateTkn(crtTk);
    RetVal rve;
    if(consume(OR)){
        if(exprAnd(&rve)){
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be logically tested");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
            if(exprOrPrim(rv)){
                return 1;
            } else tkerr(crtTk,"exprOrPrim: dupa OR exprAnd => Lipseste exprOrPrim");
        } else tkerr(crtTk,"exprOrPrim: dupa OR => Lipseste exprAnd");
    }
    return 1;
}

//exprAnd: exprEq exprAndPrim
/*
 * exprAnd(out rv:RetVal): exprAnd:rv AND exprEq:rve {
        if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be logically tested");
        rv->type=createType(TB_INT,-1);
        rv->isCtVal=rv->isLVal=0;
        }
    | exprEq:rv ;*/
int exprAnd(RetVal *rv){
    //printf("exprAnd()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprEq(rv)){
        if(exprAndPrim(rv)){
            return 1;
        } else tkerr(crtTk,"exprAnd: dupa exprEq => Lipseste exprAndPrim");
    }
    crtTk = initialTk;
    return 0;
}

//exprAndPrim: AND exprEq exprAndPrim | eps
int exprAndPrim(RetVal *rv){
    //printf("exprAndPrim()\n");
    //translateTkn(crtTk);
    RetVal rve;
    if(consume(AND)){
        if(exprEq(&rve)){
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be logically tested");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
            if(exprAndPrim(rv)){
                return 1;
            } else tkerr(crtTk,"exprAndPrim: dupa AND exprEq => Lipseste exprAndPrim");
        } else tkerr(crtTk,"exprAndPrim: dupa AND => Lipseste exprEq");
    }
    return 1;
}

// exprEq: exprRel exprEqPrim
/*
 * exprEq(out rv:RetVal): exprEq:rv ( EQUAL | NOTEQ ):tkop exprRel:rve {
        if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
        rv->type=createType(TB_INT,-1);
        rv->isCtVal=rv->isLVal=0;
        }
    | exprRel:rv ;*/
int exprEq(RetVal *rv){
    //printf("exprEq()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprRel(rv)){
        if(exprEqPrim(rv)){
            return 1;
        } else tkerr(crtTk,"exprEq: dupa exprRel => Lipseste exprEqPrim");
    }
    crtTk = initialTk;
    return 0;
}

//exprEqPrim: (EQUAL|NOTEQ) exprRel exprEqPrim | eps
int exprEqPrim(RetVal *rv){
    //printf("exprEqPrim()\n");
    //translateTkn(crtTk);
    RetVal rve;
    if(consume(EQUAL) || consume(NOTEQ)){
        if(exprRel(&rve)){
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
            if(exprEqPrim(rv)){
                return 1;
            } else tkerr(crtTk,"exprEqPrim: dupa (EQUAL|NOTEQ) exprRel => Lipseste exprEqPrim");
        } else tkerr(crtTk,"exprEqPrim: dupa (EQUAL|NOTEQ) => Lipseste exprRel");
    }
    return 1;
}

//exprRel: exprAdd exprRelPrim
/*
 * exprRel(out rv:RetVal): exprRel:rv
    ( LESS | LESSEQ | GREATER | GREATEREQ ):tkop
    exprAdd:rve {
        if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
        if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
        rv->type=createType(TB_INT,-1);
        rv->isCtVal=rv->isLVal=0;
        }
    | exprAdd:rv ;*/
int exprRel(RetVal *rv){
    //printf("exprRel()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprAdd(rv)){
        if(exprRelPrim(rv)){
            return 1;
        } else tkerr(crtTk,"exprRel: dupa exprAdd => Lipseste exprRelPrim");
    }
    crtTk = initialTk;
    return 0;
}

//exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | eps
int exprRelPrim(RetVal *rv){
    //printf("exprRelPrim()\n");
    //translateTkn(crtTk);
    RetVal rve;
    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)){
        if(exprAdd(&rve)){
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be compared");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be compared");
            rv->type=createType(TB_INT,-1);
            rv->isCtVal=rv->isLVal=0;
            if(exprRelPrim(rv)){
                return 1;
            } else tkerr(crtTk,"exprRelPrim: dupa ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd => Lipseste exprRelPrim");
        } else tkerr(crtTk,"exprRelPrim: dupa ( LESS | LESSEQ | GREATER | GREATEREQ ) => Lipseste exprAdd");
    }
    return 1;
}

//exprAdd: exprMul exprAddPrim
/*
 * exprAdd(out rv:RetVal): exprAdd:rv ( ADD | SUB ):tkop exprMul:rve {
        if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be added or subtracted");
        if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be added or subtracted");
        rv->type=getArithType(&rv->type,&rve.type);
        rv->isCtVal=rv->isLVal=0;
        }
    | exprMul:rv ;*/
int exprAdd(RetVal *rv){
    //printf("exprAdd()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprMul(rv)){
        if(exprAddPrim(rv)){
            return 1;
        } else tkerr(crtTk,"exprAdd: dupa exprMul => Lipseste exprAddPrim");
    }
    crtTk = initialTk;
    return 0;
}

//exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | eps
int exprAddPrim(RetVal *rv){
    //printf("exprAddPrim()\n");
    //translateTkn(crtTk);
    RetVal rve;
    if(consume(ADD) || consume(SUB)){
        if(exprMul(&rve)){
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be added or subtracted");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be added or subtracted");
            rv->type=getArithType(&rv->type,&rve.type);
            rv->isCtVal=rv->isLVal=0;
            if(exprAddPrim(rv)){
                return 1;
            } else tkerr(crtTk,"exprAddPrim: dupa ( ADD | SUB ) exprMul => Lipseste exprAddPrim");
        } else tkerr(crtTk,"exprAddPrim: dupa ( ADD | SUB ) => Lipseste exprMul");
    }
    return 1;
}

//exprMul: exprCast exprMulPrim
/*
 * exprMul(out rv:RetVal): exprMul:rv ( MUL | DIV ):tkop exprCast:rve {
        if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be multiplied or divided");
        if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be multiplied or divided");
        rv->type=getArithType(&rv->type,&rve.type);
        rv->isCtVal=rv->isLVal=0;
        }
    | exprCast:rv ;*/
int exprMul(RetVal *rv){
    //printf("exprMul()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprCast(rv)){
        if(exprMulPrim(rv)){
            return 1;
        } else tkerr(crtTk,"exprMul: dupa exprCast => Lipseste exprMulPrim");
    }
    crtTk = initialTk;
    return 0;
}

//exprMulPrim: ( MUL | DIV ) exprCast exprMulPrim | eps
int exprMulPrim(RetVal *rv){
    //printf("exprMulPrim()\n");
    //translateTkn(crtTk);
    RetVal rve;
    if(consume(MUL) || consume(DIV)){
        if(exprCast(&rve)){
            if(rv->type.nElements>-1||rve.type.nElements>-1)
                tkerr(crtTk,"an array cannot be multiplied or divided");
            if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
                tkerr(crtTk,"a structure cannot be multiplied or divided");
            rv->type=getArithType(&rv->type,&rve.type);
            rv->isCtVal=rv->isLVal=0;
            if(exprMulPrim(rv)){
                return 1;
            } else tkerr(crtTk,"exprMulPrim: dupa ( MUL | DIV ) exprCast => Lipseste exprMulPrim");
        } else tkerr(crtTk,"exprMulPrim: dupa ( MUL | DIV ) => Lipseste exprCast");
    }
    return 1;
}

//exprCast: LPAR typeName RPAR exprCast | exprUnary ;
/*
 * exprCast(out rv:RetVal): LPAR typeName:t RPAR exprCast:rve {
        cast(&t,&rve.type);
        rv->type=t;
        rv->isCtVal=rv->isLVal=0;
        }
    | exprUnary:rv ;*/
int exprCast(RetVal *rv){
    //printf("exprCast()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    Type t;
    RetVal rve;
    if(consume(LPAR)){
        if(typeName(&t)){
            if(consume(RPAR)){
                if(exprCast(&rve)) {
                    cast(&t,&rve.type);
                    rv->type=t;
                    rv->isCtVal=rv->isLVal=0;
                    return 1;
                } else tkerr(crtTk,"exprCast: dupa LPAR typeName RPAR => Lipseste exprCast");
            } else tkerr(crtTk,"exprCast: dupa LPAR typeName => Lipseste )");
        }
    }
    crtTk = initialTk;
    if(exprUnary(rv)){
        return 1;
    }
    crtTk = initialTk;
    return 0;
}

//exprUnary: ( SUB | NOT ) exprUnary | exprPostfix ;
/*
 * exprUnary(out rv:RetVal): ( SUB | NOT ):tkop exprUnary:rv {
        if(tkop->code==SUB){
            if(rv->type.nElements>=0)tkerr(crtTk,"unary '-' cannot be applied to an array");
            if(rv->type.typeBase==TB_STRUCT)
                tkerr(crtTk,"unary '-' cannot be applied to a struct");
            }else{  // NOT
            if(rv->type.typeBase==TB_STRUCT)tkerr(crtTk,"'!' cannot be applied to a struct");
            rv->type=createType(TB_INT,-1);
            }
        rv->isCtVal=rv->isLVal=0;
        }
    | exprPostfix:rv ;*/
int exprUnary(RetVal *rv){
    //printf("exprUnary()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    Token *tkop;
    if(consume(SUB) || consume(NOT)){
        tkop = consumedTk;
        if(exprUnary(rv)){
            if(tkop->code==SUB){
                if(rv->type.nElements>=0)tkerr(crtTk,"unary '-' cannot be applied to an array");
                if(rv->type.typeBase==TB_STRUCT)
                    tkerr(crtTk,"unary '-' cannot be applied to a struct");
            }else{  // NOT
                if(rv->type.typeBase==TB_STRUCT)tkerr(crtTk,"'!' cannot be applied to a struct");
                rv->type=createType(TB_INT,-1);
            }
            rv->isCtVal=rv->isLVal=0;
            return 1;
        } else tkerr(crtTk,"exprUnary: dupa ( SUB | NOT ) => Lipseste exprUnary");
    }
    crtTk = initialTk;
    if(exprPostfix(rv)){
        return 1;
    }
    crtTk = initialTk;
    return 0;
}

//exprPostfix: exprPostfix LBRACKET expr RBRACKET | exprPostfix DOT ID | exprPrimary ;
//exprPostfix: exprPrimary exprPostfixPrim
/*
 * exprPostfix(out rv:RetVal): exprPostfix:rv
        LBRACKET
        expr:rve {
            if(rv->type.nElements<0)tkerr(crtTk,"only an array can be indexed");
            Type typeInt=createType(TB_INT,-1);
            cast(&typeInt,&rve.type);
            rv->type=rv->type;
            rv->type.nElements=-1;
            rv->isLVal=1;
            rv->isCtVal=0;
            }
        RBRACKET
   | exprPostfix DOT ID:tkName {
            Symbol      *sStruct=rv->type.s;
            Symbol      *sMember=findSymbol(&sStruct->members,tkName->text);
            if(!sMember)
                tkerr(crtTk,"struct %s does not have a member %s",sStruct->name,tkName->text);
            rv->type=sMember->type;
            rv->isLVal=1;
            rv->isCtVal=0;
            }
   | exprPrimary:rv ;*/
int exprPostfix(RetVal *rv){
    //printf("exprPostfix()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    if(exprPrimary(rv)){
        if(exprPostfixPrim(rv)){
            return 1;
        } else tkerr(crtTk,"exprPostfix: dupa exprPrimary => Lipseste exprPostfixPrim");
    }
    crtTk = initialTk;
    return 0;
}

//exprPostfixPrim: LBRACKET expr RBRACKET exprPostfixPrim | DOT ID exprPostfixPrim | eps
int exprPostfixPrim(RetVal *rv){
    //printf("exprPostfixPrim()\n");
    //translateTkn(crtTk);
    RetVal rve;
    Token *tkName;
    if(consume(LBRACKET)){
        if(expr(&rve)){
            if(rv->type.nElements<0)tkerr(crtTk,"only an array can be indexed");
            Type typeInt=createType(TB_INT,-1);
            cast(&typeInt,&rve.type);
            rv->type=rv->type;
            rv->type.nElements=-1;
            rv->isLVal=1;
            rv->isCtVal=0;
            if(consume(RBRACKET)){
                if(exprPostfixPrim(rv)){
                    return 1;
                } else tkerr(crtTk,"exprPostfixPrim: dupa LBRACKET expr RBRACKET => Lipseste exprPostfixPrim");
            } else tkerr(crtTk,"exprPostfixPrim: dupa LBRACKET expr => Lipseste ]");
        } else tkerr(crtTk,"exprPostfixPrim: dupa LBRACKET => Lipseste expr");
    }
    if(consume(DOT)){
        if(consume(ID)){
            tkName = consumedTk;
            if(exprPostfixPrim(rv)){
                Symbol *sStruct=rv->type.s;
                Symbol *sMember=findSymbol(&sStruct->members,tkName->text);
                if(!sMember)
                    tkerr(crtTk,"struct %s does not have a member %s",sStruct->name,tkName->text);
                rv->type=sMember->type;
                rv->isLVal=1;
                rv->isCtVal=0;
                return 1;
            } else tkerr(crtTk,"exprPostfixPrim: dupa DOT ID => Lipseste exprPostfixPrim");
        } else tkerr(crtTk,"exprPostfixPrim: dupa DOT => Lipseste ID");
    }
    return 1;
}

//exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )? | CT_INT | CT_REAL | CT_CHAR | CT_STRING | LPAR expr RPAR ;
/*
 * exprPrimary(out rv:RetVal):
    ID:tkName {
        Symbol *s=findSymbol(&symbols,tkName->text);
        if(!s)tkerr(crtTk,"undefined symbol %s",tkName->text);
        rv->type=s->type;
        rv->isCtVal=0;
        rv->isLVal=1;
        }
        ( LPAR {
            Symbol **crtDefArg=s->args.begin;
            if(s->cls!=CLS_FUNC&&s->cls!=CLS_EXTFUNC)
                tkerr(crtTk,"call of the non-function %s",tkName->text);
            }
            ( expr:arg {
                if(crtDefArg==s->args.end)tkerr(crtTk,"too many arguments in call");
                cast(&(*crtDefArg)->type,&arg.type);
                crtDefArg++;
                }
                ( COMMA expr:arg {
                    if(crtDefArg==s->args.end)tkerr(crtTk,"too many arguments in call");
                    cast(&(*crtDefArg)->type,&arg.type);
                    crtDefArg++;
                    }
                )*
            )?
        RPAR {
            if(crtDefArg!=s->args.end)tkerr(crtTk,"too few arguments in call");
            rv->type=s->type;
            rv->isCtVal=rv->isLVal=0;
            }
        | {
            if(s->cls==CLS_FUNC||s->cls==CLS_EXTFUNC)
                tkerr(crtTk,"missing call for function %s",tkName->text);
            } )?
   | CT_INT:tki         {rv->type=createType(TB_INT,-1);rv->ctVal.i=tki->i;
                                    rv->isCtVal=1;rv->isLVal=0;}
   | CT_REAL:tkr     {rv->type=createType(TB_DOUBLE,-1);rv->ctVal.d=tkr->r;
                                    rv->isCtVal=1;rv->isLVal=0;}
   | CT_CHAR:tkc    {rv->type=createType(TB_CHAR,-1);rv->ctVal.i=tkc->i;
                                    rv->isCtVal=1;rv->isLVal=0;}
   | CT_STRING:tks {rv->type=createType(TB_CHAR,0);rv->ctVal.str=tks->text;
                                    rv->isCtVal=1;rv->isLVal=0;}
   | LPAR expr:rv RPAR ;*/
int exprPrimary(RetVal *rv){
    //printf("exprPrimary()\n");
    //translateTkn(crtTk);
    Token *initialTk = crtTk;
    Token *tkName;
    Token *tki;
    Token *tkc;
    Token *tkr;
    Token *tks;
    RetVal arg;
    if(consume(ID)){
        tkName = consumedTk;
        Symbol *s=findSymbol(&symbols,tkName->text);
        if(!s) tkerr(crtTk,"undefined symbol %s",tkName->text);
        rv->type=s->type;
        rv->isCtVal=0;
        rv->isLVal=1;
        if(consume(LPAR)){
            Symbol **crtDefArg=s->args.begin;
            if(s->cls!=CLS_FUNC&&s->cls!=CLS_EXTFUNC)
                tkerr(crtTk,"call of the non-function %s",tkName->text);
            if(expr(&arg)){
                if(crtDefArg==s->args.end)tkerr(crtTk,"too many arguments in call");
                cast(&(*crtDefArg)->type,&arg.type);
                crtDefArg++;
                for(;;){
                    if(consume(COMMA)){
                        if(expr(&arg)){
                            if(crtDefArg==s->args.end)tkerr(crtTk,"too many arguments in call");
                            cast(&(*crtDefArg)->type,&arg.type);
                            crtDefArg++;
                        } else tkerr(crtTk,"exprPrimary: dupa ID ( LPAR ( expr ( COMMA => Lipseste expr");
                    } else break;
                }
            }
            if(consume(RPAR)){
                if(crtDefArg!=s->args.end)tkerr(crtTk,"too few arguments in call");
                rv->type=s->type;
                rv->isCtVal=rv->isLVal=0;
                return 1;
            } else tkerr(crtTk,"exprPrimary: dupa ID ( LPAR ( expr ( COMMA expr )* )? => Lipseste )");
        }
        if(s->cls==CLS_FUNC||s->cls==CLS_EXTFUNC)
            tkerr(crtTk,"missing call for function %s",tkName->text);
        return 1;
    }
    if(consume(CT_INT)){
        tki = consumedTk;
        rv->type=createType(TB_INT,-1);rv->ctVal.i=tki->i;
        rv->isCtVal=1;rv->isLVal=0;
        return 1;
    }
    if(consume(CT_REAL)){
        tkr = consumedTk;
        rv->type=createType(TB_DOUBLE,-1);rv->ctVal.d=tkr->r;
        rv->isCtVal=1;rv->isLVal=0;
        return 1;
    }
    if(consume(CT_CHAR)){
        tkc = consumedTk;
        rv->type=createType(TB_CHAR,-1);rv->ctVal.i=tkc->i;
        rv->isCtVal=1;rv->isLVal=0;
        return 1;
    }
    if(consume(CT_STRING)){
        tks = consumedTk;
        rv->type=createType(TB_CHAR,0);rv->ctVal.str=tks->text;
        rv->isCtVal=1;rv->isLVal=0;
        return 1;
    }
    if(consume(LPAR)){
        if(expr(rv)){
            if(consume(RPAR)){
                return 1;
            } else tkerr(crtTk,"exprPrimary: dupa LPAR expr => Lipseste )");
        }
    }
    crtTk = initialTk;
    return 0;
}

Symbol *addExtFunc(const char *name,Type type) {
    Symbol *s=addSymbol(&symbols,name,CLS_EXTFUNC);
    s->type=type;
    initSymbols(&s->args);
    return s;
}

Symbol *addFuncArg(Symbol *func,const char *name,Type type) {
    Symbol *a=addSymbol(&func->args,name,CLS_VAR);
    a->type=type;
    return a;
}

addDefaultFuncs(){
    Symbol *s;
    s=addExtFunc("put_s",createType(TB_VOID,-1));
    addFuncArg(s,"s",createType(TB_CHAR,0));

    s=addExtFunc("get_s",createType(TB_VOID,-1));
    addFuncArg(s,"s",createType(TB_CHAR,0));

    s=addExtFunc("put_i",createType(TB_VOID,-1));
    addFuncArg(s,"i",createType(TB_INT,-1));

    s=addExtFunc("get_i",createType(TB_INT,-1));

    s=addExtFunc("put_d",createType(TB_VOID,-1));
    addFuncArg(s,"d",createType(TB_DOUBLE,-1));

    s=addExtFunc("get_d",createType(TB_DOUBLE,-1));

    s=addExtFunc("put_c",createType(TB_VOID,-1));
    addFuncArg(s,"c",createType(TB_CHAR,-1));

    s=addExtFunc("get_c",createType(TB_CHAR,-1));

    s=addExtFunc("seconds",createType(TB_DOUBLE,-1));
}

int main() {
    FILE *fin;
    int a;
    char *buff;
    int i=0;
    buff = (char *)malloc(sizeof(char)*10000);
    if ((fin = fopen("/Users/allenpianoman/CLionProjects/LFTC/fisier.txt", "r")) == NULL)
    {
        printf("ERROR opening the file!\n");
        return -1;
    }
    while ( (a=fgetc(fin)) != EOF){
        buff[i++]= (char) a;
    }
    pCrtCh=buff;
    while (getNextToken()!= END) {}
    //printf("%s\nBuffLen = %d\n==========THE END=======\n",buff,i);
    //printf("\n\t Analizatorul lexical:\n\n");
    //printTokens();
    //printf("\n\n\n=================================================\n\n\n");
    crtTk = tokens;
    initSymbols(&symbols);
    addDefaultFuncs();
    //printf("%s\n",symbols.begin[2]->name);
    if(unit()) {
        puts("Sintaxa corecta");
        fclose(fin);
        return 0;
    }
    tkerr(crtTk,"Syntax error");
    fclose(fin);
}
