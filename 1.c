
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR_LEN 500

char Look;

void GetChar(){
    Look = getchar();
}

//report error
void Error(char *err_msg){
    printf("Error : %s.", err_msg);   
}

//report error and halt
void Abort(char *err_msg){
    Error(err_msg);
    exit(EXIT_FAILURE);
}

//report what was expected
void Expected(char *s){
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "%s Expected", s);
    Abort(buf);   
}

//match a specific input character
void Match(char c){
    if(Look == c) GetChar();
    else {
	char buf[MAX_STR_LEN];
	snprintf(buf, sizeof(buf), "\"%c\"", c);
	Expected(buf);
    }
}

//get an identifier
char GetName(){
    if(!isalpha(Look)) Expected("Name");
    char ret = toupper(Look);
    GetChar();
    return ret;
}

//get a number
char GetNum(){
    if(!isdigit(Look)) Expected("Name");
    int ret = Look;
    GetChar();
    return ret;   
}

int isAddop(char c){
    return c == '+' || c == '-';
}

//output a string with tab
void Emit(char *s){
    printf("\t%s", s);   
}

//output a string with tab and CRLF
void EmitLn(char *s){
    printf("\t%s\n", s);   
}

void Init(){
    GetChar();
}

int main(){
    Init();   
    return 0;
}