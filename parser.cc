#include "lexer.h"
#include "compiler.h"
#include "parser.h"
#include <iostream>
#include <map>

using namespace std;

LexicalAnalyzer lexer;
map<string, int> locationTable;

void syntax_error()
{
    cout << "SYNTAX ERROR !!&%!!\n";
    exit(1);
}

Token expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

//function to add instruction node to node list
void append(InstructionNode** l1, InstructionNode** l2) {
    InstructionNode* i = *l1;
    while(i->next) {
        i = i->next;
    }
    i->next = *l2;
}

//num_list → NUM num_list | NUM
void Parser::parse_num_list() {
    Token t = expect(NUM);
    inputs.push_back(stoi(t.lexeme));
    t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_num_list();
    }
    else if (t.token_type == END_OF_FILE) {
        return;
    }
    else {
        syntax_error();
    }
}

//inputs → num_list
void Parser::parse_inputs() {
    parse_num_list();
}

//default_case → DEFAULT COLON body
InstructionNode* Parser::parse_default_case() {
    InstructionNode* inst = new InstructionNode;
    expect(DEFAULT);
    expect(COLON);
    inst = parse_body();
    return inst;
}

//case → CASE NUM COLON body
InstructionNode* Parser::parse_case(int arg, InstructionNode* inst) {
    InstructionNode* CJMPinst = new InstructionNode;        //CJMP instruction
    CJMPinst->type = CJMP;
    CJMPinst->cjmp_inst.operand1_index = arg;
    CJMPinst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    InstructionNode* JMPinst = new InstructionNode;     //JMP instruction
    JMPinst->type = JMP;
    JMPinst->next = NULL;

    expect(CASE);
    CJMPinst->cjmp_inst.operand2_index = parse_primary();
    expect(COLON);
    CJMPinst->cjmp_inst.target = parse_body();
    CJMPinst->next = NULL;
    JMPinst->jmp_inst.target = inst;
    append(&CJMPinst->cjmp_inst.target, &JMPinst);      //add JMPinst to CJMPinst.target
    return CJMPinst;
}

//case_list → case case_list | case
InstructionNode* Parser::parse_case_list(int arg, InstructionNode* inst) {
    InstructionNode* inst1 = new InstructionNode;
    InstructionNode* inst2 = NULL;
    inst1 = parse_case(arg, inst);
    Token t = lexer.peek(1);
    if(t.token_type == CASE) {
        inst2 = parse_case_list(arg, inst);
        append(&inst1, &inst2);
        return inst1;
    }
    else if(t.token_type == RBRACE || t.token_type == DEFAULT){
        return inst1;
    }
    else {
        syntax_error();
    }
    return inst1;    
}

//for_stmt → FOR LPAREN assign_stmt condition SEMICOLON assign_stmt RPAREN body
InstructionNode* Parser::parse_for_stmt() {
    InstructionNode* ASSIGNinst1 = new InstructionNode;      //ASSIGN (i = 0) inst
    ASSIGNinst1->type = ASSIGN;

    InstructionNode* ASSIGNinst2 = new InstructionNode;     //ASSIGN (i < _ ) inst
    ASSIGNinst2->type = ASSIGN;

    InstructionNode* CJMPinst = new InstructionNode;        //CJMP inst
    CJMPinst->type = CJMP;

    InstructionNode* JMPinst = new InstructionNode;     //JMP inst
    JMPinst->type = JMP;

    InstructionNode* NOOPinst = new InstructionNode;    //NOOP inst
    NOOPinst->type = NOOP;
    NOOPinst->next = NULL;

    JMPinst->jmp_inst.target = CJMPinst;
    CJMPinst->cjmp_inst.target = NOOPinst;

    expect(FOR);
    expect(LPAREN);
    ASSIGNinst1 = parse_assign_stmt();
    ASSIGNinst1->next = CJMPinst;
    parse_condition(&CJMPinst);
    expect(SEMICOLON);
    ASSIGNinst2 = parse_assign_stmt();
    ASSIGNinst2->next = JMPinst;
    expect(RPAREN);
    CJMPinst->next = parse_body();
    ASSIGNinst2->next = JMPinst;

    append(&CJMPinst, &ASSIGNinst2);
    JMPinst->next = NOOPinst;

    return ASSIGNinst1;
}

//switch_stmt → SWITCH ID LBRACE case_list default_case RBRACE
//switch_stmt → SWITCH ID LBRACE case_list RBRACE
InstructionNode* Parser::parse_switch_stmt() {
    InstructionNode* caseList = new InstructionNode;    //caseList instruction

    InstructionNode* defaultCase = new InstructionNode;         //defaultCase instruction

    InstructionNode* NOOPinst = new InstructionNode;     //NOOP instruction  
    NOOPinst->type = NOOP;
    NOOPinst->next = NULL;

    expect(SWITCH);
    Token t = expect(ID);
    int arg = locationTable[t.lexeme];
    expect(LBRACE);
    caseList = parse_case_list(arg, NOOPinst);
    t = lexer.peek(1);
    if (t.token_type == RBRACE) {
        expect(RBRACE);
    }
    else if (t.token_type == DEFAULT) {
        defaultCase = parse_default_case();
        expect(RBRACE);
        append(&caseList, &defaultCase);    //add defaultCase to caseList
    }
    else {
        syntax_error();
    }
    append(&caseList, &NOOPinst);
    return caseList;
}

//relop → GREATER | LESS | NOTEQUAL
ConditionalOperatorType Parser::parse_relop() {
    Token t = lexer.peek(1);
    if(t.token_type == GREATER) {
        expect(GREATER);
        return CONDITION_GREATER;
    }
    else if(t.token_type == LESS) {
        expect(LESS);
        return CONDITION_LESS;
    }
    else if(t.token_type == NOTEQUAL) {
        expect(NOTEQUAL);
        return CONDITION_NOTEQUAL;
    }
    else {
        syntax_error();
    }
}

//condition → primary relop primary
void Parser::parse_condition(InstructionNode** inst) {
    (*inst)->cjmp_inst.operand1_index = parse_primary();
    (*inst)->cjmp_inst.condition_op = parse_relop();
    (*inst)->cjmp_inst.operand2_index = parse_primary();
}

//if_stmt → IF condition body
InstructionNode* Parser::parse_if_stmt() {
    InstructionNode* inst = new InstructionNode; //CJMP
    InstructionNode* inst1 = new InstructionNode; //NOOP
    inst->type = CJMP;
    inst1->type = NOOP;
    inst1->next = NULL;

    expect(IF);
    parse_condition(&inst);
    inst->next = parse_body();
    append(&inst, &inst1);      //add NOOP to CJMP in instruction list
    inst->cjmp_inst.target = inst1;
    return inst;
}

//while_stmt → WHILE condition 
InstructionNode* Parser::parse_while_stmt(){
    InstructionNode* CJMPinst = new InstructionNode;    //CJMP
    CJMPinst->type = CJMP;

    InstructionNode* JMPinst = new InstructionNode;     //JMP
    JMPinst->type = JMP;

    InstructionNode* NOOPinst = new InstructionNode;       //NOOP
    NOOPinst->type = NOOP;
    NOOPinst->next = NULL;

    CJMPinst->cjmp_inst.target = NOOPinst;
    JMPinst->jmp_inst.target = CJMPinst;

    expect(WHILE);
    parse_condition(&CJMPinst);
    CJMPinst->next = parse_body();

    append(&CJMPinst, &JMPinst);           //add JMP to CJMP in list
    JMPinst->next = NOOPinst;

    return CJMPinst;
}

//input_stmt → input ID SEMICOLON
InstructionNode* Parser::parse_input_stmt() {
    InstructionNode* inst = new InstructionNode;
    inst->type = IN;
    inst->next = NULL;
    expect(INPUT);
    Token t = expect(ID);
    inst->input_inst.var_index = locationTable[t.lexeme];
    expect(SEMICOLON);
    return inst;
}

//output_stmt → output ID SEMICOLON
InstructionNode* Parser::parse_output_stmt() {
    InstructionNode* inst = new InstructionNode;
    inst->type = OUT;
    inst->next = NULL;
    expect(OUTPUT);
    Token t = expect(ID);
    inst->input_inst.var_index = locationTable[t.lexeme];
    expect(SEMICOLON);
    return inst;
}

//op → PLUS | MINUS | MULT | DIV
ArithmeticOperatorType Parser::parse_op() {
    Token t = lexer.peek(1);
    if(t.token_type == PLUS) {
        expect(PLUS);
        return OPERATOR_PLUS;
    }
    else if(t.token_type == MINUS) {
        expect(MINUS);
        return OPERATOR_MINUS;
    }
    else if(t.token_type == MULT) {
        expect(MULT);
        return OPERATOR_MULT;
    }
    else if(t.token_type == DIV) {
        expect(DIV);
        return OPERATOR_DIV;
    }
    else {
        syntax_error();
    }
    
}

//primary → ID | NUM
int Parser::parse_primary() {
    Token t = lexer.peek(1);
    if(t.token_type == ID) {
        t = expect(ID);
        return locationTable[t.lexeme];
    }
    else if(t.token_type == NUM) {
        t = expect(NUM);
        locationTable.insert(pair<string, int> (t.lexeme, next_available));
        mem[next_available] = stoi(t.lexeme);
        next_available++;

        return locationTable[t.lexeme];
    }
    else {
        syntax_error();
    }
    return 0;
}

//expr → primary op primary
void Parser::parse_expr(InstructionNode* inst) {
    inst->assign_inst.operand1_index = parse_primary();
    inst->assign_inst.op = parse_op();
    inst->assign_inst.operand2_index = parse_primary();
}

//assign_stmt → ID EQUAL expr SEMICOLON
InstructionNode* Parser::parse_assign_stmt() {
    InstructionNode* inst = new InstructionNode;
    inst->type = ASSIGN;
    inst->next = NULL;
    Token t = expect(ID);
    inst->assign_inst.left_hand_side_index = locationTable[t.lexeme];
    expect(EQUAL);
    t = lexer.peek(2);
    if(t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT || t.token_type == DIV) {
        parse_expr(inst);
    }
    else {
        inst->assign_inst.op = OPERATOR_NONE;
        inst->assign_inst.operand1_index = parse_primary();
    }
    expect(SEMICOLON);
    return inst;
}

//stmt → output stmt | input stmt
//stmt → assign stmt | while stmt | if stmt | switch stmt | for stmt
InstructionNode* Parser::parse_stmt() {
    Token t = lexer.peek(1);
    if (t.token_type == OUTPUT) {
        return parse_output_stmt();
    }
    else if (t.token_type == INPUT) {
        return parse_input_stmt();
    }
    else if (t.token_type == ID) {
        return parse_assign_stmt();
    }
    else if (t.token_type == WHILE) {
        return parse_while_stmt();
    }
    else if (t.token_type == IF) {
        return parse_if_stmt();
    }
    else if (t.token_type == SWITCH) {
        return parse_switch_stmt();
    }
    else if (t.token_type == FOR) {
        return parse_for_stmt();
    }
    else {
        syntax_error();
    }
}

//stmt_list → stmt stmt list | stmt
InstructionNode* Parser::parse_stmt_list() {
    InstructionNode* inst = new InstructionNode;
    InstructionNode* instList = NULL;
    inst = parse_stmt();
    Token t = lexer.peek(1);
    if(t.token_type == INPUT ||t.token_type == OUTPUT || t.token_type == ID || t.token_type == WHILE 
    || t.token_type == IF || t.token_type ==  SWITCH || t.token_type == FOR) {
        instList = parse_stmt_list();
        append(&inst, &instList);       //add instruction to instruction list
        return inst;
    }
    else if (t.token_type == RBRACE) {
        return inst;
    }
    else {
        syntax_error();
    }
    return inst;
}

//body → LBRACE stmt_list RBRACE
InstructionNode* Parser::parse_body() {
    expect(LBRACE);
    InstructionNode* inst = parse_stmt_list();
    expect(RBRACE);
    return inst;
}

//id_list → ID COMMA id_list | ID
void Parser::parse_id_list() {
    Token t = expect(ID);
    locationTable.insert(pair<string, int>(t.lexeme, next_available));  //add variable to location table
    next_available++;   //increment index of memory array
    t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_id_list();
    }
    else if (t.token_type == SEMICOLON) {
        return;
    }
    else {
        syntax_error();
    }
}

//var_section → id_list SEMICOLON 
void Parser::parse_var_section() {
    parse_id_list();
    expect(SEMICOLON);
}

//program → var_section body inputs
InstructionNode* Parser::parse_program() {
    InstructionNode* inst = new InstructionNode;
    parse_var_section();
    inst = parse_body();
    parse_inputs();
    return inst;
}

struct InstructionNode * parse_generate_intermediate_representation(){
    Parser parser;
    return parser.parse_program();
}
