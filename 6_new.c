
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR_LEN 500

char Look;

//list to hold references of labels to be freed later
typedef struct node{
    char *label;
    struct node *next;
}Node;

Node *list;

Node *CreateNode(){
    Node *n = malloc(sizeof(Node));
    if(n == NULL) 
    printf("could not create node\n");
    else{
    n->label = NULL;
    n->next = NULL;
    }
    return n;
}

void AppendNode(char *label){
    Node *new = CreateNode();
    if(new){
    new->label = label;
    new->next = list->next;
    list->next = new;
    }
}

void DeleteList(){
    while(list->next != NULL){
        Node *temp = list->next;
        free(temp->label);
        list->next = temp->next;
        free(temp);
    }
    free(list);
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
char GetNum(){
    if(!isdigit(Look)) Expected("Integer");
    int ret = Look;
    GetChar();
    return ret;   
}

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

//output a string with tab
void Emit(char *s){
    printf("\t%s", s);   
}

//output a string with tab and CRLF
void EmitLn(char *s){
    Emit(s);
    printf("\n");   
}

//Generate a Unique Label
char *NewLabel(){
    static int LCount = 0;
    char *label = malloc(MAX_STR_LEN * sizeof(char));
    AppendNode(label);
    snprintf(label, sizeof(label), "L%d", LCount);
    LCount++;
    return label;
}

void PostLabel(char *L){
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "%s :", L);
    printf("%s\n", buf);
}

void Expression();

void Block(char *L);

//Parse and Translate a Do Statement
void DoDo(){
    Match('d');
    char *L1 = NewLabel();
    char *L2 = NewLabel();
    Expression();
    EmitLn("SUBQ #1,D0");
    PostLabel(L1);
    EmitLn("MOVE D0,-(SP)");
    Block(L2);
    EmitLn("MOVE (SP)+,D0");
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "DBRA D0,%s", L1);
    EmitLn(buf);   
    EmitLn("SUBQ #2,SP");
    PostLabel(L2);
    EmitLn("ADDQ #2,SP");
}

//Parse and Translate a FOR Statement
void DoFor(){
    Match('f');
    char *L1 = NewLabel();   
    char *L2 = NewLabel();
    char buf[MAX_STR_LEN];
    char name = GetName();
    snprintf(buf, sizeof(buf), "LEA %c(PC),A0", name);
    EmitLn(buf);
    Match('=');
    Expression();
    EmitLn("SUBQ #1,D0");
    EmitLn("MOVE D0,(A0)");
    Expression();
    EmitLn("MOVE D0,-(SP)");
    PostLabel(L1);
    snprintf(buf, sizeof(buf), "LEA %c(PC),A0", name);
    EmitLn(buf);
    EmitLn("MOVE (A0),D0");
    EmitLn("ADDQ #1,D0");
    EmitLn("MOVE D0,(A0)");
    EmitLn("CMP (SP),D0");
    snprintf(buf, sizeof(buf), "BGT %s", L2);
    EmitLn(buf);
    Block(L2);
    Match('e');
    snprintf(buf, sizeof(buf), "BRA %s", L1);
    EmitLn(buf);
    PostLabel(L2);
    EmitLn("ADDQ #2,SP");
}

void BoolExpression();

//Recognize and Translate a REPEAT Construct
void DoRepeat(){
    Match('r');
    char *L1 = NewLabel();
    char *L2 = NewLabel();
    PostLabel(L1);
    Block(L2);
    Match('u');
    BoolExpression();
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BEQ %s", L1);
    EmitLn(buf);
    PostLabel(L2);
}

//Recognize and Translate a LOOP Construct
void DoLoop(){
    Match('p');
    char *L1 = NewLabel();
    char *L2 = NewLabel();
    PostLabel(L1);
    Block(L2);
    Match('e');
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BRA %s", L1);
    EmitLn(buf);
    PostLabel(L2);
}

//Recognize and Translate a WHILE Construct
void DoWhile(){
    Match('w');
    char *L1 = NewLabel();
    PostLabel(L1);
    BoolExpression();
    char *L2 = NewLabel();
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BEQ %s", L2);
    EmitLn(buf);
    Block(L2);
    Match('e');
    snprintf(buf, sizeof(buf), "BRA %s", L1);
    EmitLn(buf);
    PostLabel(L2);
}

//Recognize and Translate an IF Construct
void DoIf(char *L){
    Match('i');   
    char *L1 = NewLabel();
    char *L2 = L1;
    BoolExpression();
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BEQ %s", L1);
    EmitLn(buf);
    Block(L);
    if(Look == 'l'){
    Match('l');
    L2 = NewLabel();
    snprintf(buf, sizeof(buf), "BRA %s", L2);
        EmitLn(buf);
    PostLabel(L1);
    Block(L);
    }
    Match('e');
    PostLabel(L2);
}

void DoBreak(char *L){
    Match('b');
    if(L != NULL){
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BRA %s", L);
    EmitLn(buf);
    }
    else
    Abort("No loop to break from");
}

//Recognize and Translate an "Other"
void Other(){
    char name[2] = { '\0' };
    name[0] = GetName();
    EmitLn(name);   
}

//Parse and translate an identifier
void Ident(){
    char name = GetName();
    char buf[MAX_STR_LEN];
    if(Look == '('){
        Match('(');
        Match(')');    
        snprintf(buf, sizeof(buf), "BSR %c", name);
    }
    else
        snprintf(buf, sizeof(buf), "MOVE %c(PC),D0", name);
    EmitLn(buf);
}

void Fin(){
    if(Look == '\n') GetChar();
}

void Assignment(){
    char buf[MAX_STR_LEN];
    char name = GetName();
    Match('=');
    BoolExpression();
    snprintf(buf, sizeof(buf), "LEA %c(PC),A0", name);
    EmitLn(buf);
    EmitLn("MOVE D0,(A0)");
}

//Recognize and Translate a Statement Block
void Block(char *L){
    while(Look != 'e' && Look != 'l' && Look != 'u'){
        Fin();
        switch(Look){
            case 'i' : DoIf(L); break;
            case 'w' : DoWhile(); break;
            case 'p' : DoLoop(); break;
            case 'r' : DoRepeat(); break;
            case 'f' : DoFor(); break;
            case 'd' : DoDo(); break;
            case 'b' : DoBreak(L);break;
            default  : Assignment(); break;
        }
        Fin();
    }
}

//Parse and translate a math factor
void Factor(){
    if(Look == '('){
        Match('(');
        Expression();
        Match(')');
    }
    else if(isalpha(Look)) 
        Ident(NULL);
    else {
        char buf[MAX_STR_LEN];
        int num = GetNum() - '0';
        snprintf(buf, sizeof(buf), "MOVE #%d,D0", num);   
        EmitLn(buf);
    }
}

//Parse and Translate the First Math Factor
void SignedFactor() {
    if(Look == '+') GetChar();
    if(Look == '-'){
        GetChar();
        Factor();
        EmitLn("NEG D0");
    }
    else
        Factor();
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
    SignedFactor();
    while(IsMulop(Look)){
        EmitLn("MOVE D0,-(SP)");
        switch(Look){
            case '*' : Multiply(); break;
            case '/' : Divide(); break;
        }
    }
}

//Recognize and Translate an Add
void Add(){
    Match('+');
    Term();
    EmitLn("ADD (SP)+,D0");
}

//Recognize and Translate an Subtract
void Subtract(){
    Match('-');
    Term();
    EmitLn("SUB (SP)+,D0");
    EmitLn("NEG D0");
}

//Parse and Translate an Expression
void Expression(){
    Term();
    while(IsAddop(Look)){
        EmitLn("MOVE D0,-(SP)");
        switch(Look){
            case '+' : Add();      break;
            case '-' : Subtract(); break;
        }
    }
}

//Recognize a Relop
//# denotes 'not equals'
int IsRelop(char c){
    switch(c){
        case '=' :
        case '<' :
        case '>' :
        case '#' : return 1;
        default  : return 0;
    }
}

//Recognize a Boolean Orop
int IsOrop(char c){
    return c == '|' || c == '~';
}

//Recognize a Boolean Literal
int IsBoolean(char c){
    return toupper(c) == 'T' || toupper(c) == 'F';   
}

//Get a Boolean Literal
int GetBoolean(){
    if(!IsBoolean(Look)) Expected("Boolean Literal");
    int bool = toupper(Look) == 'T';
    GetChar();
    return bool;
}

//Recognize and Translate a Relational "Equals"
void Equals(){
    Match('=');
    Expression();
    EmitLn("CMP (SP)+,D0");
    EmitLn("SEQ D0");
}

//Recognize and Translate a Relational "Not Equals"
void NotEquals(){
    Match('#');
    Expression();
    EmitLn("CMP (SP)+,D0");
    EmitLn("SNE D0");
}

//Recognize and Translate a Relational "Less Than"
void Less(){
    Match('<');
    Expression();
    EmitLn("CMP (SP)+,D0");
    EmitLn("SGE D0");
}

//Recognize and Translate a Relational "Greater Than"
void Greater(){
    Match('>');
    Expression();
    EmitLn("CMP (SP)+,D0");
    EmitLn("SLE D0");
}

//Parse and translate a relation
void Relation(){
    Expression();
    if(IsRelop(Look)){
        EmitLn("MOVE D0,-(SP)");
        switch(Look){
            case '=' : Equals(); break;
            case '#' : NotEquals(); break;
            case '<' : Less(); break;
            case '>' : Greater(); break;
        }
        EmitLn("TST D0");
    }
}

void BoolFactor(){
    if(IsBoolean(Look)){
        if(GetBoolean())
            EmitLn("MOVE #-1,D0");
        else
            EmitLn("CLR D0");
    }
    else
        Relation();
}

void NotFactor(){
    if(Look == '!'){
        Match('!');
        BoolFactor();
        EmitLn("EOR #-1,D0");
    }
    else
        BoolFactor();
}

void BoolTerm(){
    NotFactor();
    while(Look == '&'){
        EmitLn("MOVE D0,-(SP)");
        Match('&');
        NotFactor();
        EmitLn("AND (SP)+,D0");
    }
}

//Recognize and Translate an Exclusive OR
void BoolXor(){
    Match('~');
    BoolTerm();
    EmitLn("EOR (SP)+,D0");
}


//Recognize and Translate a Boolean OR
void BoolOr(){
    Match('|');
    BoolTerm();
    EmitLn("OR (SP)+,D0");
}

//Parse and Translate a Boolean Expression
void BoolExpression(){
    BoolTerm();
    while(IsOrop(Look)){
        EmitLn("MOVE D0,-(SP)");
        switch(Look){
            case '|' : BoolOr(); break;
            case '~' : BoolXor(); break;
        }
    }
}

//Parse and Translate a Program
void DoProgram(){
    Block(NULL);
    if(Look != 'e') Expected("End");
    EmitLn("End");
}

void Init(){
    GetChar();
    list = CreateNode();
}

int main(){
    Init();   
    DoProgram();
    //BoolExpression();
    DeleteList();
    return 0;
}