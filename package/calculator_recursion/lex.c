#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i++] = c;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        char ad = fgetc(stdin);
        lexeme[0] = c;

        if(ad == '+' || ad == '-'){
            lexeme[1] = ad;
            lexeme[2] = '\0';
            return INCDEC;
        }else if(ad == '='){
            lexeme[1] = ad;
            lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        }else {
            lexeme[1] = '\0';
            ungetc(ad, stdin);
            return ADDSUB;
        }
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isalpha(c)) {
        // TODO:accept variable names with multiple characters
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ID;
    } else if (c == EOF) {
        return ENDFILE;
    } else if (c == '&') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    } else if(c == '|') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    } else if(c == '^') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    } else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}
