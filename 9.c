
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
    if(!isdigit(Look)) Expected("Integer");
    int ret = Look;
    GetChar();
    return ret;   
}

int isAddop(char c){
    return c == '+' || c == '-';
}

int IsWhite(char c){
    return c == ' ' || c == '\t';
}

void SkipWhite(){
    while(IsWhite(Look)) GetChar();   
}

//output a string with tab
void Emit(char *s){
    printf("\t%s", s);   
}

//output a string with tab and CRLF
void EmitLn(char *s){
    printf("\t%s\n", s);   
}

void PostLabel(char label){
    printf("%c : \n", label);
}

void Statements(){
    Match('b');
    while(Look != 'e')
        GetChar();
    Match('e');
}

void Labels(){
    Match('l');
}

void Constants(){
    Match('c');
}

void Types(){
    Match('t');
}

void Variables(){
    Match('v');
}

void DoProcedure(){
    Match('p');
}

void DoFunction(){
    Match('f');
}

void Declarations(){
    int other = 0;
    while(!other){
        switch(Look){
            case 'l' : Labels(); break;
            case 'c' : Constants(); break;
            case 't' : Types(); break;
            case 'v' : Variables(); break;
            case 'p' : DoProcedure(); break;
            case 'f' : DoFunction(); break;
            default : other = 1;
        }
    }
}

void DoBlock(char name){
    Declarations();
    PostLabel(name);
    Statements();
}

void Epilog(char name){
    EmitLn("DC WARMST");
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "END %c", name);
    EmitLn(buf);
}

void Prolog(){
    EmitLn("WARMST EQU $A01E");
}

void Program(){
    Match('p');
    char name = GetName();
    Prolog(name);
    DoBlock(name);
    Match('.');
    Epilog(name);
}

void Init(){
    GetChar();
}

int main(){
    Init();   
    Program();
    return 0;
}