
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR_LEN 500
#define KEYWORD_LEN 4
#define TOKEN_LEN 16

char *KWlist[] = {"IF", "ELSE", "ENDIF", "END"};
char KWCode[6] = "xilee";

char Token;
char TokenStr[TOKEN_LEN + 1];
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

//the following function is needed to avoid incompatible pointer type argument to setTokenStr()
int IsAlNum(char c) { return isalnum(c); }

int IsDigit(char c) { return isdigit(c); }


int IsAddop(char c){
    return c == '+' || c == '-';
}

int IsMulop(char c){
    return c == '*' || c == '/';
}

int IsWhite(char c){
    return c == ' ' || c == '\t';
}

void SkipWhite(){
    while(IsWhite(Look)) GetChar();   
}

//match a specific input character
void Match(char c){
    if(Look == c) GetChar();
    else {
        char buf[MAX_STR_LEN];
        snprintf(buf, sizeof(buf), "\"%c\"", c);
        Expected(buf);
    }
    SkipWhite();
}

void Fin(){
    if(Look == '\n') GetChar();
    SkipWhite();
}

//if tok is present in strArr return index else return -1
int Lookup(char **strArr, char *tok, int n){
    int i;
    for(i = 0; i < n; i++){
        if(strcmp(strArr[i], tok) == 0) return i;
    }
    return -1;
}

void setTokenStr(int (*fPtr)(char)){
    int i;
    for(i = 0; i < TOKEN_LEN && fPtr(Look); i++){
        TokenStr[i] = Look;
        GetChar();
    }
    TokenStr[i] = '\0';
    SkipWhite();
}

//get an identifier
void GetName(){
    while(Look == '\n') Fin();
    if(!isalpha(Look)) Expected("Name");
    setTokenStr(&IsAlNum);
    int i; 
    for(i = 0; TokenStr[i] != '\0'; i++) TokenStr[i] = toupper(TokenStr[i]);
}

//get a number
char *GetNum(){
    if(!isdigit(Look)) Expected("Integer");
    setTokenStr(&IsDigit);
    Token = '#';
}

//Get an Identifier and Scan it for Keywords
void *Scan(){
    GetName();
    Token = KWCode[Lookup(KWlist, TokenStr, KEYWORD_LEN) + 1];
}

//Match a Specific Input String
void MatchString(char *str){
    if(strcmp(TokenStr, str) != 0) Expected(str);
}

//Generate a unique label
char *NewLabel(){
    static int LCount = 0;
    int labelSize = 4;
    char *label = malloc(sizeof(char) * labelSize);
    memset(label, '\0', sizeof(char) * labelSize);
    snprintf(label, labelSize, "L%d", LCount);
    LCount++;
    return label;
}

//Post a Label To Output
void PostLabel(char *label){
    printf("%s : \n", label);
}

//output a string with tab
void Emit(char *s){
    printf("\t%s", s);   
}

//output a string with tab and CRLF
void EmitLn(char *s){
    printf("\t%s\n", s);   
}

//Parse and Translate an Identifier
void Ident(){
    GetName();
    char buf[MAX_STR_LEN];
    if(Look == '('){
        Match('(');
        Match(')');
        snprintf(buf, sizeof(buf), "BSR %s", TokenStr);
        EmitLn(buf);
    }
    else{
        snprintf(buf, sizeof(buf), "MOVE %s(PC),D0", TokenStr);
        EmitLn(buf);   
    }
}

void Expression();

//Parse and Translate a Math Factor
void Factor(){
    if(Look == '('){
        Match('(');
        Expression();
        Match(')');
    }
    else if(isalpha(Look))
        Ident();
    else{
        GetNum();
        char buf[MAX_STR_LEN];
        snprintf(buf, sizeof(buf), "MOVE #%s,D0", TokenStr);
        EmitLn(buf);
    }
}

//Parse and Translate the First Math Factor
void SignedFactor(){
    int neg = (Look == '-');
    if(IsAddop(Look)){
        GetChar();
        SkipWhite();
    }
    Factor();
    if(neg) 
        EmitLn("NEG D0");
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
    EmitLn("EXG D1,D0");
    EmitLn("DIVS D1,D0");
}

//Completion of Term Processing  (called by Term and FirstTerm)
void Term1(){
    while(IsMulop(Look)){
        EmitLn("MOVE D0,-(SP)");
        switch(Look){
            case '*' : Multiply(); break;
            case '/' : Divide(); break;
        }
    }
}

//Parse and Translate a Math Term
void Term(){
    Factor();
    Term1();
}

//Parse and Translate a Math Term with Possible Leading Sign
void FirstTerm(){
    SignedFactor();
    Term1();
}

//Recognize and Translate an Add
void Add(){
    Match('+');
    Term();
    EmitLn("ADD (SP)+,D0");
}

//Recognize and Translate a Subtract
void Subtract(){
    Match('-');
    Term();
    EmitLn("SUB (SP)+,D0");
    EmitLn("NEG D0");
}

//Parse and Translate an Expression
void Expression(){
    FirstTerm();
    while(IsAddop(Look)){
        EmitLn("MOVE D0,-(SP)");
        switch(Look){
            case '+' : Add(); break;
            case '-' : Subtract(); break;
        }
    }
}


//Parse and Translate a Boolean Condition
void Condition(){
    EmitLn("Condition");
}

void Block();

//Recognize and Translate an IF Construct
void DoIf(){
    Condition();
    char *L1 = NewLabel();
    char *L2 = L1;
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BEQ %s", L1);
    EmitLn(buf);
    Block();
    if(Token == 'l'){
        L2 = NewLabel();
        snprintf(buf, sizeof(buf), "BRA %s", L2);
        EmitLn(buf);
        PostLabel(L1);
        Block();
    }
    PostLabel(L2);
    MatchString("ENDIF");
}

//Parse and Translate an Assignment Statement
void Assignment(){
    char name[TOKEN_LEN + 1];
    strcpy(name, TokenStr);
    Match('=');
    Expression();
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "LEA %s(PC),A0", name);
    EmitLn(buf);
    EmitLn("MOVE D0,(A0)");
}

//Recognize and Translate a Statement Block
void Block(){
    Scan();
    while(Token != 'e' && Token != 'l'){
        if(Token == 'i') DoIf();
        else Assignment();
        Scan();
    }
}

//Parse and translate a program
void DoProgram(){
    Block();
    MatchString("END");
    EmitLn("END");
}

//Initialize
void Init(){
    GetChar();
}

int main(){
    Init();
    DoProgram();
    return 0;
}

