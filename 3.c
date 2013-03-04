
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR_LEN 500

//Lookahead Character
char Look;
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
    char returnVal = toupper(Look);
    GetChar();
    return returnVal;
}

//get a number
char GetNum(){
    if(!isdigit(Look)) Expected("Integer");
    char returnVal = Look;
    GetChar();
    return returnVal;   
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

//Initialize
void Init(){
    GetChar();
}

//Parse and Translate an Identifier
void Ident(){
    char name = GetName();
    char buf[MAX_STR_LEN];
    if(Look == '('){
	Match('(');
	Match(')');
	snprintf(buf, sizeof(buf), "BSR %c", name);
    }
    else
	snprintf(buf, sizeof(buf), "MOVE %c(PC),DO", name);
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
	snprintf(buf, sizeof(buf), "MOVE #%d,DO", GetNum() - '0');
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
	    case '*' : Multiply();
		       break;
	    case '/' : Divide();
	               break;
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

int isAddop(char c){
    return c == '+' || c == '-';
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
    char name = GetName();
    Match('=');
    Expression();
    //LEA %s(PC),A0
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "LEA %c(PC),A0", name);
    EmitLn(buf);
    EmitLn("MOVE D0,(A0)");
}

//Main Program
int main(){
    Init();   
    Assignment();
    if(Look != '\n') Expected("Newline");
    return 0;
}