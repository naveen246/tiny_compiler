
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR_LEN 500

char Look;    //Lookahead Character
char tokenStr[MAX_STR_LEN];
void Expression();

//Read New Character From Input Stream
void GetChar(){
    Look = getchar();
}

//report error
void Error(char *err_msg){
    printf("Error : %s\n", err_msg);   
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

int isAddop(char c){
    return c == '+' || c == '-';
}

//Recognize White Space
int IsWhite(char c){
    return c == ' ' || c == '\t';   
}

void SkipWhite(){
    while(IsWhite(Look)) GetChar();   
}

//match a specific input character
void Match(char c){
    if(Look != c) {
    	char buf[MAX_STR_LEN];
    	snprintf(buf, sizeof(buf), "\"%c\"", c);
    	Expected(buf);
    }
    else{
    	GetChar();
    	SkipWhite();
    }
}

//get an identifier
char *GetName(){
    if(!isalpha(Look)) Expected("Name");
    int i;
    for(i = 0; i < MAX_STR_LEN && isalnum(Look); i++){
    	tokenStr[i] = toupper(Look);
    	GetChar();
    }
    tokenStr[i] = '\0';
    SkipWhite();
    return tokenStr;
}

//get a number
char *GetNum(){
    if(!isdigit(Look)) Expected("Integer");
    int i;
    for(i = 0; i < MAX_STR_LEN && isdigit(Look); i++){
    	tokenStr[i] = Look;
    	GetChar();
    }
    tokenStr[i] = '\0';
    SkipWhite();
    return tokenStr;   
}

//output a string with tab
void Emit(char *s){
    printf("\t%s", s);   
}

//output a string with tab and newline
void EmitLn(char *s){
    Emit(s);
    printf("\n");   
}

//Parse and Translate an Identifier
void Ident(){
    char name[MAX_STR_LEN];
    strcpy(name, GetName());
    char buf[MAX_STR_LEN];
    if(Look == '('){
    	Match('(');
    	Match(')');
    	snprintf(buf, sizeof(buf), "BSR %s", name);
    }
    else
    	snprintf(buf, sizeof(buf), "MOVE %s(PC),DO", name);
    EmitLn(buf);
}

//Parse and Translate a Math Factor
void Factor(){
    if(Look == '('){
    	Match('(');
    	Expression();
    	Match(')');
    }
    else if(isalpha(Look)){
	   Ident();   
    }
    else{
    	char buf[MAX_STR_LEN];
    	snprintf(buf, sizeof(buf), "MOVE #%s,DO", GetNum());
    	EmitLn(buf);   
    }
}

//Recognize and Translate a Multiply
void Multiply(){
    Match('*');
    Factor();
    EmitLn("MULS (SP)+,D0");
}

//Recognize and Translate a Divide
void Divide(){
    Match('/');
    Factor();
    EmitLn("MOVE (SP)+,D1");
    EmitLn("EXG D0,D1");
    EmitLn("DIVS D1,D0");
}

//Parse and Translate a Math Term
void Term(){
    Factor();
    while(Look == '*' || Look == '/'){
    	EmitLn("MOVE D0,-(SP)");
    	switch(Look){
    	    case '*' : Multiply(); break;
    	    case '/' : Divide();   break;
    	}
    }
}

//recognize and translate an add
void Add(){
    Match('+');
    Term();
    EmitLn("ADD (SP)+,D0");
}

//recognize and translate a subtract
void Subtract(){
    Match('-');
    Term();
    EmitLn("SUB (SP)+,D0");
    EmitLn("NEG D0");
}

//Parse and Translate a Math Expression
void Expression(){
    if(isAddop(Look)) EmitLn("CLR D0");
    else Term();   
    while(isAddop(Look)){
    	EmitLn("MOVE D0,-(SP)");
    	switch(Look){
    	    case '+' : Add();
    		       break;
    	    case '-' : Subtract();
    		       break;
    	}
    }
}

//Parse and Translate an Assignment Statement
void Assignment(){
    char name[MAX_STR_LEN];
    strcpy(name, GetName());
    Match('=');
    Expression();
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "LEA %s(PC),A0", name);
    EmitLn(buf);
    EmitLn("MOVE D0,(A0)");
}

//Initialize
void Init(){
    GetChar();
    SkipWhite();
}

//Main Program
int main(){
    Init();   
    Assignment();
    if(Look != '\n') Expected("Newline");
    return 0;
}