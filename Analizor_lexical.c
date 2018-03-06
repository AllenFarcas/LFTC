#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
                } 
                
                /*else if (ch == '=') {
                    pCrtCh++;
                    state = 3;
                } else if (ch == 0) {
                    addTk(END);
                    return END;
                } else {
                    tkerr(addTk(END),"caracter invalid");
                }*/
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
                } else if (nCh == 2 && !memcmp(pStartCh,"if",2) ) {
                    tk = addTk(IF);
                } else { 
                    //daca nu este cuvant cheie atunci este un ID
                    tk = addTk(ID);
                    tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;
        }
    }
}

int main( int argc, char *args) {
    
}
