
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR_LEN 500
#define NO_OF_ALPHABETS 26

char Look;
int Table[NO_OF_ALPHABETS];

void setTableVal(char key, int value){
    Table[toupper(key) - 'A'] = value;   
}

int getTableVal(char key){
    return Table[toupper(key) - 'A'];
}

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
int GetNum(){
    if(!isdigit(Look)) Expected("Integer");
    int value = 0;
    while(isdigit(Look)){
	value = value * 10 + (Look - '0');
	GetChar();
    }
    return value;   
}

int isMulop(char c){
    return c == '*' || c == '/';
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

int Factor(){
    int value;
    if(Look == '('){
	Match('(');
	value = Expression();
	Match(')');
    }
    else if(isalpha(Look)) 
	value = getTableVal(GetName());
    else 
	value = GetNum();;
    return value;
}

int Term(){
    int value = Factor();
    while(isMulop(Look)){
	switch(Look){
	    case '*' : Match('*'); value *= Factor(); break;
	    case '/' : Match('/'); value /= Factor(); break;
	}
    }
    return value;
}

int Expression(){
    int value;
    if(isAddop(Look)) value = 0;
    else value = Term();
    while(isAddop(Look)){
	switch(Look){
	    case '+' : Match('+'); value += Term(); break;
	    case '-' : Match('-'); value -= Term(); break;
	}
    }
    return value;
}

void Assignment(){
    char name = GetName();
    Match('=');
    int value = Expression();
    setTableVal(name, value);
    printf("%c = %d\n", name, getTableVal(name));
}

void Input(){
    Match('?');
    char name = GetName();
    GetChar();
    int value = GetNum();
    setTableVal(name, value);
}

void Output(){
    Match('!');
    printf("%d\n", getTableVal(GetName()));
}

void InitTable(){
    bzero(Table, sizeof(Table));
}

void Init(){
    GetChar();
    InitTable();
}

int main(){
    Init();   
    while(Look != '.'){
	switch(Look){
	    case '?' : Input(); break;
	    case '!' : Output(); break;
	    default  : Assignment();
	}
	GetChar();
    }
    return 0;
}