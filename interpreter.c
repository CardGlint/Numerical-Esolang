#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"

static int mem[MAX_MEMORY];

static void reset_memory() {
    for (int i = 0; i < MAX_MEMORY; i++)
        mem[i] = 0;
}

void run_file(const char *filename) {

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    reset_memory();

    char c;
    int varNum = 0;
    int num = 0;
    int numR = 0;
    int numF = 0;
    int comp = 0;

    long loopStack[MAX_LOOPS];
    int loopDepth = -1;

    while ((c = fgetc(fp)) != EOF) {

        // <var>
        if (c == '<') {
            varNum = 0;
            while ((c = fgetc(fp)) != '>' && c != EOF)
                if (c >= '0' && c <= '9')
                    varNum = varNum * 10 + (c - '0');

            if (varNum < 0 || varNum >= MAX_MEMORY) {
                printf("Invalid memory index\n");
                fclose(fp);
                return;
            }
        }

        // [number]
        else if (c == '[') {
            num = 0;
            while ((c = fgetc(fp)) != ']' && c != EOF)
                if (c >= '0' && c <= '9')
                    num = num * 10 + (c - '0');
        }

        // {var reference}
        else if (c == '{') {
            int temp = 0;
            while ((c = fgetc(fp)) != '}' && c != EOF)
                if (c >= '0' && c <= '9')
                    temp = temp * 10 + (c - '0');

            if (temp >= 0 && temp < MAX_MEMORY)
                num = mem[temp];
            else
                printf("Invalid memory reference\n");
        }

        // (loop end value)
        else if (c == '(') {
            numR = 0;
            while ((c = fgetc(fp)) != ')' && c != EOF)
                if (c >= '0' && c <= '9')
                    numR = numR * 10 + (c - '0');
        }

        // |if compare value|
        else if (c == '|') {
            numF = 0;
            while ((c = fgetc(fp)) != '|' && c != EOF)
                if (c >= '0' && c <= '9')
                    numF = numF * 10 + (c - '0');
        }

        // operations
        else if (c == '=') mem[varNum] = num;
        else if (c == '+') mem[varNum] += num;
        else if (c == '-') mem[varNum] -= num;
        else if (c == '*') mem[varNum] *= num;
        else if (c == '/') {
            if (num == 0) printf("Division by zero\n");
            else mem[varNum] /= num;
        }

        // comparison operator
        else if (c == '#') {
            c = fgetc(fp);
            if (c == '=') comp = 1;
            else if (c == '!') comp = 2;
            else if (c == '>') comp = 3;
            else if (c == '<') comp = 4;
        }

        // print
        else if (c == 'p') printf("%d", mem[varNum]);
        else if (c == 'a') printf("%c", mem[varNum]);
        else if (c == 'n') printf("\n");

        // user input
        else if (c == 'i') {    
            char input[50];
            printf(">> ");
            if (fgets(input, sizeof(input), stdin)) {
                sscanf(input, "%d", &mem[varNum]);
            }
        }

        // loop start
        else if (c == 'r') {
            if (loopDepth + 1 >= MAX_LOOPS) {
                printf("Loop stack overflow\n");
                break;
            }
            loopStack[++loopDepth] = ftell(fp);
        }

        // loop end
        else if (c == 'd') {
            if (loopDepth < 0) {
                printf("Loop stack underflow\n");
                break;
            }

            if (mem[varNum] != numR)
                fseek(fp, loopStack[loopDepth], SEEK_SET);
            else
                loopDepth--;
        }

        // IF start
        else if (c == 'f') {
            int condition = 0;

            if ((comp == 1 && mem[varNum] == numF) ||
                (comp == 2 && mem[varNum] != numF) ||
                (comp == 3 && mem[varNum] >  numF) ||
                (comp == 4 && mem[varNum] <  numF))
                condition = 1;

            if (!condition) {
                int depth = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (c == 'f') depth++;
                    else if (c == 'e') {
                        if (depth == 0) break;
                        depth--;
                    }
                }
            }
        }

        // IF end marker
        else if (c == 'e') {
            // just marker
        }
    }

    fclose(fp);
}
