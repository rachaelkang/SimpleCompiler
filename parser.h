#include "compiler.h"

class Parser {
    public:
        void parse_num_list();
        void parse_inputs();
        InstructionNode* parse_default_case();
        InstructionNode* parse_case(int arg, InstructionNode* inst);
        InstructionNode* parse_case_list(int arg, InstructionNode* inst);
        InstructionNode* parse_for_stmt();
        InstructionNode* parse_switch_stmt();
        ConditionalOperatorType parse_relop();
        void parse_condition(InstructionNode** inst);
        InstructionNode* parse_if_stmt();
        InstructionNode* parse_while_stmt();
        InstructionNode* parse_input_stmt();
        InstructionNode* parse_output_stmt();
        ArithmeticOperatorType parse_op();
        int parse_primary();
        void parse_expr(InstructionNode* inst);
        InstructionNode* parse_assign_stmt();
        InstructionNode* parse_stmt();
        InstructionNode* parse_stmt_list();
        InstructionNode* parse_body();
        void parse_id_list();
        void parse_var_section();
        InstructionNode* parse_program();
};