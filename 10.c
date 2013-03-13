
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR_LEN 500
#define ST_LEN 100
#define LABEL_SIZE 5
#define TOKEN_LEN 16
#define KW_LEN 11

char KWlist[][TOKEN_LEN + 1] = {"IF", "ELSE", "ENDIF", "WHILE", "ENDWHILE", 
        "READ", "WRITE", "VAR", "BEGIN", "END", "PROGRAM"};
char *KWcode = "xileweRWvbep";

char Token;                     //Encoded Token
char Value[TOKEN_LEN + 1];      //Unencoded Token
char Look;                      //Lookahead character
char ST[ST_LEN][TOKEN_LEN + 1]; //Symbol Table
char SType[ST_LEN];

void Expression();
void Block();
void NewLine();
void SkipWhite();

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

int InTable(char *name){
    return Lookup(ST, name, ST_LEN) != -1;
}

//Add a New Entry to Symbol Table
void AddEntry(char *symbol, char T){
    static int stIndex = 0;
    if(InTable(symbol)){
        char buf[MAX_STR_LEN];
        snprintf(buf, sizeof(buf), "Duplicate Identifier %s", symbol);
        Abort(buf);
    }
    if(stIndex == ST_LEN)
        Abort("Symbol Table full");
    strncpy(ST[stIndex], symbol, TOKEN_LEN);
    SType[stIndex] = T;
    stIndex++;
}

//match a specific input character
void Match(char c){
    NewLine();
    if(Look == c) GetChar();
    else {
        char buf[MAX_STR_LEN];
        snprintf(buf, sizeof(buf), "\"%c\"", c);
        Expected(buf);
    }
    SkipWhite();
}

//output a string with tab
void Emit(char *s){
    printf("\t%s", s);   
}

//output a string with tab and CRLF
void EmitLn(char *s){
    printf("\t%s\n", s);   
}

//Report an Undefined Identifier
void Undefined(char *n){
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "Undefined Identifier %s", n);
    Abort(buf);
}

/***************************/
/*code  generation routines*/
/***************************/

//Clear the Primary Register
void Clear(){
    EmitLn("CLR D0");
}

//Negate the Primary Register
void Negate(){
    EmitLn("NEG D0");
}

//Load a Constant Value to Primary Register
void LoadConst(int n){
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "MOVE #%d,D0", n);
    EmitLn(buf);
}

//Load a Variable to Primary Register
void LoadVar(char *Name){
    if(!InTable(Name)) Undefined(Name);
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "MOVE %s(PC),D0", Name);
    EmitLn(buf);
}

//Push Primary onto Stack
void Push(){
    EmitLn("MOVE D0,-(SP)");
}

//Add Top of Stack to Primary
void PopAdd(){
    EmitLn("ADD (SP)+,D0");
}

//Subtract Primary from Top of Stack
void PopSub(){
    EmitLn("SUB (SP)+,D0");
    EmitLn("NEG D0");
}

//Multiply Top of Stack by Primary
void PopMul(){
    EmitLn("MULS (SP)+,D0");
}

//Divide Top of Stack by Primary
void PopDiv(){
    EmitLn("MOVE (SP)+,D1");
    EmitLn("EXG D1,D0");
    EmitLn("DIVS D1,D0");
}

//Store Primary to Variable
void Store(char *Name){
    if(!InTable(Name)) Undefined(Name);
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "LEA %s(PC),A0", Name);
    EmitLn(buf);
    EmitLn("MOVE D0,(A0)");
}

//Complement the Primary Register
void NotIt(){
    EmitLn("NOT D0");
}

//AND Top of Stack with Primary
void PopAnd(){
    EmitLn("AND (SP)+,D0");
}

//OR Top of Stack with Primary
void PopOr(){
    EmitLn("OR (SP)+,D0");
}

//XOR Top of Stack with Primary
void PopXor(){
    EmitLn("EOR (SP)+,D0");
}

//Compare Top of Stack with Primary
void PopCompare(){
    EmitLn("CMP (SP)+,D0");   
}

//Set D0 If Compare was =
void SetEqual(){
    EmitLn("SEQ D0");
    EmitLn("EXT D0");
}

//Set D0 If Compare was !=
void SetNEqual(){
    EmitLn("SNE D0");
    EmitLn("EXT D0");
}

//Set D0 If Compare was >
void SetGreater(){
    EmitLn("SLT D0");
    EmitLn("EXT D0");
}

//Set D0 If Compare was <
void SetLess(){
    EmitLn("SGT D0");
    EmitLn("EXT D0");
}

//Branch Unconditional
void Branch(char *L){
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BRA %s", L);
    EmitLn(buf);
}

//Branch False
void BranchFalse(char *L){
    EmitLn("TST D0");
    char buf[MAX_STR_LEN];
    snprintf(buf, sizeof(buf), "BEQ %s", L);
    EmitLn(buf);
}

//Set D0 If Compare was <=
void SetLessOrEqual(){
    EmitLn("SGE D0");
    EmitLn("EXT D0");
}

//Set D0 If Compare was >=
void SetGreaterOrEqual(){
    EmitLn("SLE D0");
    EmitLn("EXT D0");
}

//Read Variable to Primary Register
void ReadVar(){
    EmitLn("BSR READ");
    Store(Value);
}

//Write Variable from Primary Register
void WriteVar(){
    EmitLn("BSR WRITE");
}

/**********************************/
/*End of code  generation routines*/
/**********************************/

//get an identifier
void GetName(){
    NewLine();
    if(!isalpha(Look)) Expected("Name");
    int i;
    for(i = 0; i < TOKEN_LEN && isalnum(Look); i++){
        Value[i] = toupper(Look);
        GetChar();
    }
    Value[i] = '\0';
    SkipWhite();
}

//get a number
int GetNum(){
    NewLine();
    if(!isdigit(Look)) Expected("Integer");
    int num = 0;
    while(isdigit(Look)){
        num = num * 10 + (Look - '0');
        GetChar();
    }
    SkipWhite();
    return num;   
}

//Table Lookup, if found return index else -1
int Lookup(char Table[][TOKEN_LEN + 1], char *val, int size){
    int i;
    for(i = 0; i < size; i++){
        if(strcmp(Table[i], val) == 0) return i;
    }
    return -1;
}

//Get an Identifier and Scan it for Keywords
void Scan(){
    GetName();
    Token = KWcode[Lookup(KWlist, Value, KW_LEN) + 1];
}

//Match a Specific Input String
void MatchString(char *x){
    if(strcmp(Value, x) != 0) Expected(x);
}

int IsAddop(char c){
    return c == '+' || c == '-';
}

int IsMulop(char c){
    return c == '*' || c == '/';
}

int IsOrop(char c){
    return c == '|' || c == '~';
}

int IsRelop(char c){
    return c == '=' || c == '#' || c == '<' || c == '>';
}

int IsWhite(char c){
    return c == ' ' || c == '\t';
}

void SkipWhite(){
    while(IsWhite(Look)) GetChar();   
}

char *NewLabel(){
    static int count = 0;
    char *label = malloc(sizeof(char) * LABEL_SIZE);
    snprintf(label, LABEL_SIZE, "L%d", count);
    count++;
    return label;
}

void Postlabel(char *label){
	printf("%s : \n", label);
}

//Skip Over an End-of-Line
void NewLine(){
    while(Look == '\n'){
        GetChar();
        SkipWhite();
    }
}

void Init(){
    memset(ST, '\0', sizeof(ST));
    memset(SType, '\0', ST_LEN);
    GetChar();
    Scan();
}

void Prolog(){
	Postlabel("MAIN");
}

void Epilog(){
	EmitLn("DC WARMST");
   	EmitLn("END MAIN");
}

//write header info
void Header(){
	printf("WARMST\tEQU $A01E\n");
    EmitLn("LIB TINYLIB");
}

//Recognize and Translate a Relational "Equals"
void Equals(){
    Match('=');
    Expression();
    PopCompare();
    SetEqual();
}

//Recognize and Translate a Relational "Not Equals"
void NotEquals(){
    Match('>');
    Expression();
    PopCompare();
    SetNEqual();
}

//Recognize and Translate a Relational "Less Than or Equal"
void LessOrEqual(){
    Match('=');
    Expression();
    PopCompare();
    SetLessOrEqual();
}

//Recognize and Translate a Relational "Less Than"
void Less(){
    Match('<');
    if(Look == '=') LessOrEqual();
    else if(Look == '>') NotEquals();
    else {
        Expression();
        PopCompare();
        SetLess();
    }
}

//Recognize and Translate a Relational "Greater Than"
void Greater(){
    Match('>');
    if(Look == '='){
        Match('=');
        Expression();
        PopCompare();
        SetGreaterOrEqual();
    }
    else{
        Expression();
        PopCompare();
        SetGreater();
    }
}

//Parse and Translate a Relation
void Relation(){
    Expression();
    while(IsRelop(Look)){
        Push();
        switch(Look){
            case '=' : Equals(); break;
            case '#' : NotEquals(); break;
            case '<' : Less(); break;
            case '>' : Greater(); break;
        }
    }
}

//Parse and Translate a Boolean Factor with Leading NOT
void NotFactor(){
    if(Look == '!'){
        Match('!');
        Relation();
        NotIt();
    }
    else
        Relation();
}

//Parse and Translate a Boolean Term
void BoolTerm(){
    NotFactor();
    while(Look == '&'){
        Push();
        Match('&');
        NotFactor();
        PopAnd();
    }
}

//Recognize and Translate a Boolean OR
void BoolOr(){
    Match('|');
    BoolTerm();
    PopOr();
}

//Recognize and Translate an Exclusive Or
void BoolXor(){
    Match('~');
    BoolTerm();
    PopXor();
}

//Parse and Translate a Boolean Expression
void BoolExpression(){
    BoolTerm();
    while(IsOrop(Look)){
        Push();
        switch(Look){
            case '|' : BoolOr(); break;
            case '~' : BoolXor(); break;
        }
    }
}

void Alloc(char *name){
    if(InTable(name)) {
        char buf[MAX_STR_LEN];
        snprintf(buf, sizeof(buf), "Duplicate Variable Name : %s", name);
        Abort(buf);
    }
    AddEntry(name, 'v');
    printf("%s :\tDC ", name);
    if(Look == '='){
        Match('=');
        if(Look == '-') {
            printf("%c", Look);
            Match('-');
        }
        int num = GetNum();
        printf("%d\n", num);
    }
    else
        printf("%d\n", 0);
}

//Process a Data Declaration
void Decl(){
    GetName();
    char *val = malloc(sizeof(char) * (TOKEN_LEN + 1));
    strcpy(val, Value);
    Alloc(val);
    while(Look == ','){
        Match(',');
        GetName();
        strcpy(val, Value);
        Alloc(val);
    }
}

//Parse and Translate Global Declarations
void TopDecls(){
    Scan();
    while(Token != 'b'){
        switch(Token){
            case 'v' : Decl(); break;
            default :{
                char buf[MAX_STR_LEN];
                snprintf(buf, sizeof(buf), "Unrecognized keyword : %c", Token);
                Abort(buf);
            }
        }
        Scan();
        printf("inside TopDecls : token = %c\tvalue = %s\n", Token, Value);
    }
}


//Parse and Translate a Math Factor
void Factor(){
    if(Look == '('){
        Match('(');
        BoolExpression();
        Match(')');
    }
    else if(isdigit(Look)){
        int num = GetNum();
        LoadConst(num);
    }
    else{
        GetName();
        LoadVar(Value);
    }
}

//Parse and Translate a Negative Factor
void NegFactor(){
    Match('-');
    if(isdigit(Look)){
        int num = GetNum();
        LoadConst(num);
    }
    else{
        Factor();
    }
    Negate();
}

//Parse and Translate a Leading Factor
void FirstFactor(){
    if(IsAddop(Look)){
        switch(Look){
            case '+' : Match('+'); Factor(); break;
            case '-' : NegFactor();
        }
    }
    else Factor();
}

//Recognize and Translate a Multiply
void Multiply(){
    Match('*');
    Factor();
    PopMul();
}

//Recognize and Translate a Divide
void Divide(){
    Match('/');
    Factor();
    PopDiv();
}

//Common Code Used by Term and FirstTerm
void Term1(){
    while(IsMulop(Look)){
        Push();
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

//Parse and Translate a Leading Term
void FirstTerm(){
    FirstFactor();
    Term1();
}

//Recognize and Translate an Add
void Add(){
    Match('+');
    Term();
    PopAdd();
}

//Recognize and Translate a Subtract
void Subtract(){
    Match('-');
    Term();
    PopSub();
}

//Parse and Translate an Expression
void Expression(){
    FirstTerm();
    while(IsAddop(Look)){
        Push();
        switch(Look){
            case '+' : Add(); break;
            case '-' : Subtract(); break;
        }
    }
}

//Process a Write Statement
void DoWrite(){
    Match('(');
    Expression();
    WriteVar();
    while(Look == ','){
        Match(',');
        Expression();
        WriteVar();
    }
    Match(')');
}

//Process a Read Statement
void DoRead(){
    Match('(');
    GetName();
    ReadVar();
    while(Look == ','){
        Match(',');
        GetName();
        ReadVar();
    }
    Match(')');
}

//Parse and Translate a WHILE Statement
void DoWhile(){
    char *L1 = NewLabel();
    char *L2 = NewLabel();
    Postlabel(L1);
    BoolExpression();
    BranchFalse(L2);
    Block();
    MatchString("ENDWHILE");
    Branch(L1);
    Postlabel(L2);
}

//Recognize and Translate an IF Construct
void DoIf(){
    char *L1 = NewLabel();
    char *L2 = L1;
    BoolExpression();
    BranchFalse(L1);
    Block();
    if(Token == 'l'){
        L2 = NewLabel();
        Branch(L2);
        Postlabel(L1);
        Block();
    }
    Postlabel(L2);
    MatchString("ENDIF");
}

//Parse and Translate an Assignment Statement
void Assignment(){
    printf("inside assignment : token = %c\tvalue = %s\n", Token, Value);
    char *name = malloc(sizeof(char) * (TOKEN_LEN + 1));
    strcpy(name, Value);
    Match('=');
    BoolExpression();
    Store(name);
}

//Parse and Translate a Block of Statements
void Block(){
    Scan();
    while(Token != 'e' && Token != 'l'){
        printf("inside block : token = %c\tvalue = %s\n", Token, Value);
        switch(Token){
            case 'i' : DoIf(); break;
            case 'w' : DoWhile(); break;
            case 'R' : DoRead(); break;
            case 'W' : DoWrite(); break;
            default : Assignment();    
        }
        Scan();
    }
}

//Parse and Translate a Main Program
void Main(){
    MatchString("BEGIN");
    Prolog();
    Block();
    MatchString("END");
    Epilog();
}

//Parse and Translate a Program
void Prog(){
	MatchString("PROGRAM");
	Header();
    TopDecls();
	Main();
	Match('.');
}

int main(){
    Init(); 
    Prog();  
    return 0;
}