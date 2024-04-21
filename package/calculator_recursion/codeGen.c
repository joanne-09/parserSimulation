#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

//int tempreg[MAXLEN];
int regptr = 0;

//type 1: +, 2: -, 3: *, 4: /
void outputAss(BTNode *root, int type, int lv, int rv){
    int regL, regR;
    char operator[4];
    if(type == 1)
        strcpy(operator, "ADD");
    else if(type == 2)
        strcpy(operator, "SUB");
    else if(type == 3)
        strcpy(operator, "MUL");
    else if(type == 4)
        strcpy(operator, "DIV");

    if(root->left->data == ID){
        regL = getId(root->left->lexeme);
        if(root->right->data == ID){
            regR = getId(root->right->lexeme);
            //store one side to cache
            regptr++;
            printf("MOV r%d r%d\n", sbcount+regptr-1, regL);

            //add regR to tempreg[regptr]
            printf("%s r%d r%d\n", operator, sbcount+regptr-1, regR);
        }else if(root->right->data == INT){
            //store right side to cache
            regptr++;
            printf("MOV r%d %d\n", sbcount+regptr-1, rv);

            //add left side to right side cache
            printf("%s r%d r%d\n", operator, sbcount+regptr-1, regL);
        }else{
            //add left side to right side cache
            printf("%s r%d r%d\n", operator, sbcount+regptr-1, regL);
        }
    }else if(root->left->data == INT){
        if(root->right->data == ID){
            regR = getId(root->right->lexeme);
            //store left side to cache
            regptr++;
            printf("MOV r%d %d\n", sbcount+regptr-1, lv);

            //add right side to left side cache
            printf("%s r%d r%d\n", operator, sbcount+regptr-1, regR);
        }else if(root->right->data == INT){
            //store left side to cache
            regptr++;
            printf("MOV r%d %d\n", sbcount+regptr-1, lv);

            //add right side to left side cache
            printf("%s r%d %d\n", operator, sbcount+regptr-1, rv);
        }else{
            //add left side to right side cache
            printf("%s r%d %d\n", operator, sbcount+regptr-1, lv);
        }
    }else{
        if(root->right->data == ID){
            regR = getId(root->right->lexeme);

            //add right side id to left side cache
            printf("%s r%d r%d\n", operator, sbcount+regptr-1, regR);
        }else if(root->right->data == INT){
            //add right side int to left side cache
            printf("%s r%d %d\n", operator, sbcount+regptr-1, rv);
        }else{
            printf("%s r%d r%d\n", operator, sbcount+regptr-2, sbcount+regptr-1);
            regptr--;
        }
    }
}

int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;
    int regL, regR;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                break;
            case INT:
                retval = atoi(root->lexeme);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);

                // output assembly
                regL = getId(root->left->lexeme);
                if(root->right->data == ID){
                    regR = getId(root->right->lexeme);
                    printf("MOV r%d r%d\n", regL, regR);
                }else if(root->right->data == INT){
                    printf("MOV r%d %d\n", regL, rv);
                }else{
                    printf("MOV r%d r%d\n", regL, sbcount+regptr-1);
                    regptr--;
                }

                break;
            case ADDSUB:
            case MULDIV:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    //output assembly
                    outputAss(root, 1, lv, rv);
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    outputAss(root, 2, lv, rv);
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                    outputAss(root, 3, lv, rv);
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv == 0)
                        error(DIVZERO);
                    retval = lv / rv;
                    outputAss(root, 4, lv, rv);
                }
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
