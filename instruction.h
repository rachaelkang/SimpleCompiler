#include "compiler.h"

struct AssignmentInstruction {
    int left_hand_side_index;
    int operand1_index;
    int operand2_index;
    ArithmeticOperatorType op;
    struct AssignmentStatement* next;
};

struct OutputInstruction {
    int var_index;
};

// struct InstructionNode {
//     InstructionType type;

//     union {
//         struct {
//             int left_hand_side_index;
//             int operand1_index;
//             int operand2_index;
//             ArithmeticOperatorType op;
//         } assign_inst;

//         struct {

//         } jmp_inst;

//         struct {

//         } cjmp_inst;

//         struct {
//             int var_index;
//         } input_inst;

//         struct {
//             int var_index;
//         } output_inst;
//     };
//     struct InstructionNode* next;
// }

struct CJMP {
    ConditionalOperatorType condition_op;
    int operand1_index;
    int operand2_index;
    struct InstructionNode * target;
};