#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// for lex
#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, END, ENDFILE,
    INT, ID,
    ADDSUB, MULDIV, INCDEC,
    ASSIGN,
    LPAREN, RPAREN,
    AND, OR, XOR, ADDSUB_ASSIGN
} TokenSet;

TokenSet getToken(void);
TokenSet curToken = UNKNOWN;
char lexeme[MAXLEN];

// Test if a token matches the current token
int match(TokenSet token);
// Get the next token
void advance(void);
// Get the lexeme of the current token
char *getLexeme(void);


// for parser
#define TBLSIZE 64
// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    printf("EXIT 1\n"); \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left;
    struct _Node *right;
    struct _Node *prev;
} BTNode;

int sbcount = 0;
Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
void initTable(void);
// Get the value of a variable
int getval(char *str);
// Set the value of a variable
int setval(char *str, int val);
int getId(char *str);
int checkVarValid(char *str);
// Make a new node according to token type and lexeme
BTNode *makeNode(TokenSet tok, const char *lexe);
// Free the syntax tree
void freeTree(BTNode *root);
BTNode *factor(void);
BTNode *assign_expr(void);
BTNode *or_expr(void);
BTNode *or_expr_tail(BTNode *left);
BTNode *xor_expr(void);
BTNode *xor_expr_tail(BTNode *left);
BTNode *and_expr(void);
BTNode *and_expr_tail(BTNode *left);
BTNode *addsub_expr(void);
BTNode *addsub_expr_tail(BTNode *left);
BTNode *muldiv_expr(void);
BTNode *muldiv_expr_tail(BTNode *left);
BTNode *unary_expr(void);
void statement(void);
// Print error message and exit the program
void err(ErrorType errorNum);


// for codeGen
// Evaluate the syntax tree
int evaluateTree(BTNode *root, int reg);
int treeWithID(BTNode *root);
// Print the syntax tree in prefix
void printPrefix(BTNode *root);


// TODO
/*============================================================================================
lex implementation
============================================================================================*/

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
        c = fgetc(stdin);
        i = 1;
        while ((isalpha(c) || c == '_' || isdigit(c)) && i < MAXLEN) {
            lexeme[i++] = c;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
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


// TODO
/*============================================================================================
parser implementation
============================================================================================*/

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    error(NOTFOUND);

    /*strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;*/
    return 0;
}

int setval(char *str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

int getId(char *str){
    for(int i=0; i<sbcount; i++){
        if(strcmp(str, table[i].name) == 0){
            return i;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    error(NOTFOUND);

    /*strcpy(table[sbcount].name, str);
    table[sbcount++].val = 0;
    return sbcount-1;*/
    return 0;
}

int checkVarValid(char *str){
    for(int i=0; i<sbcount; i++){
        if(strcmp(str, table[i].name) == 0){
            return 1;
        }
    }

    return 0;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    node->prev = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        freeTree(root->prev);
        free(root);
    }
}

// factor := INT | ID | 
//           INCDEC ID | 
//           LPAREN assign_expr RPAREN(new)
BTNode *factor(void) {
    BTNode *retp = NULL, *left = NULL;
    
    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) {
        retp = makeNode(ID, getLexeme());
        advance();
    } else if (match(INCDEC)) {
        retp = makeNode(INCDEC, getLexeme());
        advance();
        if(match(ID)){
            retp->left = NULL;
            retp->right = makeNode(ID, getLexeme());
            advance();
        }else{
            error(NOTNUMID);
        }
    } else if (match(LPAREN)) {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
}

//assign_expr := ID ASSIGN assign_expr |
//               ID ADDSUB_ASSIGN assign_expr |
//               or_expr
BTNode *assign_expr(void){
    BTNode *retp = NULL , *left = NULL;
    left = or_expr();
    if(match(ASSIGN)){
        retp = makeNode(ASSIGN, getLexeme());
        advance();
        retp->left = left;
        retp->right = assign_expr();
    }else if(match(ADDSUB_ASSIGN)){
        retp = makeNode(ADDSUB_ASSIGN, getLexeme());
        advance();
        retp->left = left;
        retp->right = assign_expr();
    }else{
        retp = left;
    }
    return retp;
}

BTNode *or_expr(void){
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

BTNode *or_expr_tail(BTNode *left){
    BTNode *node = NULL;

    if(match(OR)){
        node = makeNode(OR, getLexeme());
        advance();
        node->left = left;
        node->right = xor_expr();
        return or_expr_tail(node);
    } else {
        return left;
    }
}

BTNode *xor_expr(void){
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

BTNode *xor_expr_tail(BTNode *left){
    BTNode *node = NULL;

    if(match(XOR)){
        node = makeNode(XOR, getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        return xor_expr_tail(node);
    } else {
        return left;
    }
}

BTNode *and_expr(void){
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

BTNode *and_expr_tail(BTNode *left){
    BTNode *node = NULL;

    if(match(AND)){
        node = makeNode(AND, getLexeme());
        advance();
        node->left = left;
        node->right = addsub_expr();
        return and_expr_tail(node);
    } else {
        return left;
    }
}

// expr := term expr_tail
BTNode *addsub_expr(void) {
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

// expr_tail := ADDSUB term expr_tail | OR expr expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance(); 
        node->left = left;
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    } else {
        return left;
    }
}

// term := factor term_tail
BTNode *muldiv_expr(void) {
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

// term_tail := MULDIV factor term_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    } else {
        return left;
    }
}

//unary_expr := ADDSUB unary_expr | factor
BTNode *unary_expr(void){
    BTNode *node = NULL;
    if(match(ADDSUB)){
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = makeNode(INT,"0");
        node->right = unary_expr();
        return node;
    }else{
        return factor();
    }
}

// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode *retp = NULL;

    if (match(ENDFILE)) {
        for(int i=0; i<3; i++)
            printf("MOV r%d [%d]\n", i, i*4);
        printf("EXIT 0\n");
        exit(0);
    } else if (match(END)) {
        advance();
    } else {
        retp = assign_expr();
        if (match(END)) {
            evaluateTree(retp, 0);
            //printf("Prefix traversal: ");
            //printPrefix(retp);
            //printf("\n");
            freeTree(retp);
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum) {
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    exit(0);
}


// TODO
/*============================================================================================
codeGen implementation
============================================================================================*/

//type 1: +, 2: -, 3: *, 4: /
void outputAss(BTNode *root, int reg){
    char operator[4];
    if(strcmp(root->lexeme, "+") == 0)
        strcpy(operator, "ADD");
    else if(strcmp(root->lexeme, "-") == 0)
        strcpy(operator, "SUB");
    else if(strcmp(root->lexeme, "*") == 0)
        strcpy(operator, "MUL");
    else if(strcmp(root->lexeme, "/") == 0)
        strcpy(operator, "DIV");
    else if(root->data == AND)
        strcpy(operator, "AND");
    else if(root->data == OR)
        strcpy(operator, "OR");
    else if(root->data == XOR)
        strcpy(operator, "XOR");
    else if(root->data == ASSIGN){
        int regL = getId(root->left->lexeme);

        printf("MOV [%d] r%d\n", regL*4, reg);
        return;
    }
    
    printf("%s r%d r%d\n", operator, reg, reg+1);
}

int evaluateTree(BTNode *root, int reg) {
    int retval = 0, lv = 0, rv = 0;
    int regL, regR;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);

                regR = getId(root->lexeme);
                printf("MOV r%d [%d]\n", reg, regR*4);
                break;
            case INT:
                retval = atoi(root->lexeme);

                printf("MOV r%d %d\n", reg, retval);
                break;
            case ASSIGN:
                if(root->left->data != ID) error(NOTFOUND);
                rv = evaluateTree(root->right, reg);
                retval = setval(root->left->lexeme, rv);

                // output assembly
                outputAss(root, reg);

                break;
            case ADDSUB: case MULDIV:
            case AND: case OR: case XOR:
                lv = evaluateTree(root->left, reg);
                rv = evaluateTree(root->right, reg+1);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    //output assembly
                    outputAss(root, reg);
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    outputAss(root, reg);
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                    outputAss(root, reg);
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv == 0){
                        if(!treeWithID(root->right)){
                            error(DIVZERO);
                        }else retval = lv;
                    }else
                        retval = lv / rv;

                    outputAss(root, reg);
                } else if(root->data == AND) {
                    retval = lv & rv;
                    outputAss(root, reg);
                } else if(root->data == OR) {
                    retval = lv | rv;
                    outputAss(root, reg);
                } else if(root->data == XOR) {
                    retval = lv ^ rv;
                    outputAss(root, reg);
                }
                break;
            case INCDEC:
                rv = evaluateTree(root->right, reg);
                regR = getId(root->right->lexeme);

                printf("MOV r%d 1\n", reg+1);
                if(strcmp(root->lexeme, "++") == 0){
                    retval = setval(root->right->lexeme, rv+1);
                    printf("ADD r%d r%d\n", reg, reg+1);

                    printf("MOV [%d] r%d\n", regR*4, reg);
                }else if(strcmp(root->lexeme, "--") == 0){
                    retval = setval(root->right->lexeme, rv-1);
                    printf("SUB r%d r%d\n", reg, reg+1);

                    printf("MOV [%d] r%d\n", regR*4, reg);
                }
                break;
            case ADDSUB_ASSIGN:
                lv = evaluateTree(root->left, reg);
                rv = evaluateTree(root->right, reg+1);

                regL = getId(root->left->lexeme);
                if(strcmp(root->lexeme, "+=") == 0){
                    retval = setval(root->left->lexeme, rv+lv);
                    printf("ADD r%d r%d\n", reg, reg+1);
                }else if(strcmp(root->lexeme, "-=") == 0){
                    retval = setval(root->left->lexeme, lv-rv);
                    printf("SUB r%d r%d\n", reg, reg+1);
                }

                printf("MOV [%d] r%d\n", regL*4, reg);
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

int treeWithID(BTNode *root){
    if(root == NULL) return 0;

    int rv, lv;

    switch(root->data){
        case ID:
            return 1;
        case INT:
            return 0;
        default:
            rv = treeWithID(root->right);
            lv = treeWithID(root->left);

            return lv + rv;
    }
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}


/*============================================================================================
main
============================================================================================*/

// This package is a calculator
// It works like a Python interpretor
// Example:
// >> y = 2
// >> z = 2
// >> x = 3 * y + 4 / (2 * z)
// It will print the answer of every line
// You should turn it into an expression compiler
// And print the assembly code according to the input

// This is the grammar used in this package
// You can modify it according to the spec and the slide
// statement  :=  ENDFILE | END | expr END
// expr    	  :=  term expr_tail
// expr_tail  :=  ADDSUB term expr_tail | NiL
// term 	  :=  factor term_tail
// term_tail  :=  MULDIV factor term_tail| NiL
// factor	  :=  INT | ADDSUB INT |
//		   	      ID  | ADDSUB ID  |
//		   	      ID ASSIGN expr |
//		   	      LPAREN expr RPAREN |
//		   	      ADDSUB LPAREN expr RPAREN

int main() {
    // generate output file of this calculator
    //freopen("input.txt", "w" ,stdout);

    initTable();

    while (1) {
        //printf("EXIT 1\n");
        statement();
    }
    return 0;
}
