#include"Assembler.h"
#include<sstream>
#include<iostream>

extern vector<string> single_operators;
extern vector<string> double_operators;
extern vector<string>reg;
extern map<string, string> operator_map;

string Assembler::_sizeof(const std::string& _type) {
    int size = -1;
    if (_type == "int" || _type == "float" || _type == "long")size = 4;
    else if (_type == "char")size = 1;
    else if (_type == "double")size = 8;
    return to_string(size);
}
//�ж��Ƿ���float����
bool Assembler::_is_float(OperandItem& operand) {
    return operand.type == "VARIABLE" && symbol_table[operand.operand].data_name == "field_type" && symbol_table[operand.operand].data == "float";
}
bool Assembler::_contain_float(OperandItem& operand_a, OperandItem& operand_b) {
    return _is_float(operand_a) || _is_float(operand_b);
}
void Assembler::_include(std::shared_ptr<SyntaxTreeNode> node) {
    //��ʱ�����κδ���
}
bool Assembler::_is_contain_sentence(const std::string& sentence) {
    for (auto p : sentence_type) {
        if (p == sentence)return true;
    }
    return false;
}
bool Assembler::_is_contain_function(const std::string& function_name) {
    auto func = symbol_table.find(function_name);
    if (func == symbol_table.end())return false;
    return true;
}
//��������ָ���ָ�������
string Assembler::join(const std::vector<std::string>& list, const std::string& sep) {
    if (list.empty()) return "";
    std::string result = list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        result += sep + list[i];
    }
    return result;
}
//��ʱֻ֧��main������Ŀǰû�д����������ͷ���ֵ
void Assembler::_function_statement(std::shared_ptr<SyntaxTreeNode> node) {
    std::shared_ptr<SyntaxTreeNode>current_node = node->first_son;
    std::vector<string>para_list;
    std::string func_name = "";
    std::string func_type = "";
    int para_count = 0;
    while (current_node != NULL) {
        //������ֵ����
        if (current_node->node_value == "Type")func_type = current_node->first_son->node_value;
        //��������
        else if (current_node->node_value == "FunctionName") {
            func_name = current_node->first_son->node_value;
            // ���ɺ�����ǩ
            if (func_name != "main") {
                ass_file_handler.insert(func_name + ":", "TEXT");
                //std::cout << "other function statement except for main is not supported!" << std::endl;
                //exit(0);
            }
            else {
                ass_file_handler.insert(".global main", "TEXT");
                ass_file_handler.insert("main:", "TEXT");
                ass_file_handler.insert("finit", "TEXT");
            }
        }
        // ����������
        else if (current_node->node_value == "StateParameterList") {
            // ����������������Ӧ�Ļ�����
            //��¼����������
            std::string line = "pushl %ebp";
            ass_file_handler.insert(line, "TEXT");
            line = "movl %eap,%ebp";
            ass_file_handler.insert(line, "TEXT");
            std::shared_ptr<SyntaxTreeNode>parameter_node = current_node->first_son;
            while (parameter_node != NULL) {
                para_count++;
                //����ֵ����
                std::string para_field_type = parameter_node->first_son->node_value;
                std::string para_type = parameter_node->first_son->right->extra_info["type"];
                std::string para_name =parameter_node->first_son->right->node_value;
                para_list.push_back(para_name);
                if (para_field_type != "int") {
                    std::cout << "not supported type in stateparameterlist!" << std::endl;
                    exit(0);
                }
                //ͳһʹ��ȫ�ֱ���
                //�������뵽����ļ���Ӧ����
                line = ".lcomm " + para_name + ", " + _sizeof(para_field_type);
                std::string section = (para_type == "VARIABLE") ? "BSS" : "DATA";
                ass_file_handler.insert(line, section);
                // ������������ű�
                symbol_table[para_name] = SymbolTableItem{
                    "VARIABLE","field_type", para_field_type
                };

                //���븳ֵ��䣬��ʼ����ڲ���
                line = "movl %" + to_string(4*(para_count+1))+"(%ebp)" + "," + para_name;
                ass_file_handler.insert(line, "TEXT");

                parameter_node = parameter_node->right;
            }
        }
        //��������
        else if (current_node->node_value == "Sentence") { 
            traverse(current_node->first_son); 
            if (func_name != "main") {
                std::string line = "popl %ebp";
                ass_file_handler.insert(line, "TEXT");
                //���뷵��ָ��
                ass_file_handler.insert("ret", "TEXT");
            }
        }
        current_node = current_node->right;
    }

    //����ֲ���������
    for (auto p : para_list) {
        symbol_table.erase(p);
    }
    //main�����ڽ�β����call exit���ó����˳����ָ��
    if (func_name == "main") {
        std::string line = "call exit";
        ass_file_handler.insert(line, "TEXT");
    }
    function_table[func_name] = func_type;
}
//ֱ�ӱ���Ϊȫ�ֱ���
void Assembler::_statement(std::shared_ptr<SyntaxTreeNode> node) {
    std::string line;
    std::string variable_field_type; // �������ͣ�int, float ��
    std::string variable_type;       // �������ͣ�VARIABLE �� ARRAY
    std::string variable_name;       // ������

    auto current_node = node->first_son;

    while (current_node != NULL) {
        if (current_node->node_value == "Type") {
            // ��ȡ��������
            variable_field_type = current_node->first_son->node_value;
        }
        else if (current_node->node_type == "IDENTIFIER") {
            // ��ȡ�����������ͣ����� extra_info��
            variable_name = current_node->node_value;
            variable_type = current_node->extra_info["type"]; 
            line = ".lcomm " + variable_name + ", " + _sizeof(variable_field_type);
        }
        //�����б�
        else if (current_node->node_value == "ConstantList") {
            // �����ʼ��
            line = variable_name + ": ." + variable_field_type + " ";
            std::vector<std::string> array_values;
            auto tmp_node = current_node->first_son;
            while (tmp_node != nullptr) {
                array_values.push_back(tmp_node->node_value);
                tmp_node = tmp_node->right;
            }
            line += join(array_values, ", ");
        }
        current_node = current_node->right;
    }

    // ���뵽����ļ���Ӧ����
    std::string section = (variable_type == "VARIABLE") ? "BSS" : "DATA";
    ass_file_handler.insert(line, section);

    // ������������ű�
    symbol_table[variable_name] = SymbolTableItem{
        variable_type,"field_type", variable_field_type
    };
}
void Assembler::_function_call(std::shared_ptr<SyntaxTreeNode> node) {
    if (!node || !node->first_son) return;

    auto current_node = node->first_son;
    std::string func_name;
    std::vector<std::string> parameter_list;
    int para_count = 0;
    while (current_node != NULL) {
        if (current_node->node_type == "FUNCTION_NAME") {
            func_name = current_node->node_value;
            if (func_name != "printf" && func_name != "scanf") {
                //std::cout << "Function call except scanf and printf not supported yet!" << std::endl;
                //exit(0);
                std::string line = "call " + func_name;
            }
        }
        else if (current_node->node_value == "CallParameterList") {
            auto tmp_node = current_node->first_son;
            if (func_name == "printf" || func_name == "scanf") {
                while (tmp_node != NULL) {
                    if (tmp_node->node_type == "DIGIT_CONSTANT" || tmp_node->node_type == "STRING_CONSTANT") {
                        std::string label = "var_" + std::to_string(variable_cnt++);
                        //������ַ�������
                        if (tmp_node->node_type == "STRING_CONSTANT") {
                            std::string line = label + ": .asciz \"" + tmp_node->node_value + "\"";
                            ass_file_handler.insert(line, "DATA");
                            symbol_table[label] = { "STRING_CONSTANT", "value", tmp_node->node_value };
                        }
                        else {
                            std::cout << "Digital constant parameter is not supported yet!" << std::endl;
                            exit(0);
                        }
                        parameter_list.push_back(label);
                    }
                    else if (tmp_node->node_type == "IDENTIFIER")parameter_list.push_back(tmp_node->node_value);
                    else if (tmp_node->node_type == "ADDRESS") {
                        // TODO-> Handle address parameters
                    }
                    else {
                        //�������͵Ĳ����ݲ�֧��
                        std::cout << "Unsupported parameter type: " << tmp_node->node_type << std::endl;
                        exit(0);
                    }

                    tmp_node = tmp_node->right;
                }
            }
            //���������Ĳ�������
            //Ŀǰֻ֧�����Ͳ���
            else {
                //���淵��ֵ
                //std::string line = "pushl %eax";
                //ass_file_handler.insert(line, "TEXT");
                //caller��������
                while (tmp_node != NULL) {
                    if (tmp_node->node_type == "DIGIT_CONSTANT") {
                        std::string line = "pushl $" + tmp_node->node_value ;
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (tmp_node->node_type == "IDENTIFIER") {
                        auto sym = symbol_table.find(tmp_node->node_value);
                        if (sym == symbol_table.end()) {
                            std::cout << "Symbol not found in table: " << tmp_node->node_value << std::endl;
                            exit(0);
                        }
                        std::string line = "pushl " + tmp_node->node_value ;
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else {
                        //�������͵Ĳ����ݲ�֧��
                        std::cout << "Unsupported parameter type: " << tmp_node->node_type << std::endl;
                        exit(0);
                    }
                    para_count++;
                    tmp_node = tmp_node->right;
                }
            }
         }       
        current_node = current_node->right;
    }
    //������ǿ⺯����ֱ��call����
    if (func_name != "printf" && func_name != "scanf") {
        std::string line = "call " + func_name;
        ass_file_handler.insert(line, "TEXT");

        line = "addl $" +to_string(4*para_count)+",%esp";
        ass_file_handler.insert(line, "TEXT");
    }
    // ����printf��������
    if (func_name == "printf") {
        //%espҪ+��ֵ
        int num_bytes = 0;
        for (auto it = parameter_list.rbegin(); it != parameter_list.rend(); ++it) {
            const std::string& param = *it;
            //ȥ���ű�����Ѱ�Ҳ���
            auto sym = symbol_table.find(param);
            if (sym == symbol_table.end()) {
                std::cout << "Symbol not found in table: " << param << std::endl;
                exit(0);
            }

            if (sym->second.type == "STRING_CONSTANT") {
                std::string line = "pushl $" + param;
                ass_file_handler.insert(line, "TEXT");
                num_bytes += 4;
            }
            else if (sym->second.type == "VARIABLE") {
                //field_type
                std::string field_type = sym->second.data;
                if (field_type == "int") {
                    std::string line = "pushl " + param;
                    ass_file_handler.insert(line, "TEXT");
                    num_bytes += 4;
                }
                else if (field_type == "float") {
                    std::string line = "flds " + param;
                    ass_file_handler.insert(line, "TEXT");

                    line = "subl $8, %esp";
                    ass_file_handler.insert(line, "TEXT");

                    line = "fstpl (%esp)";
                    ass_file_handler.insert(line, "TEXT");

                    num_bytes += 8;
                }
                else {
                    std::cout << "Field type except int and float not supported in printf!" << std::endl;
                    exit(0);
                }
            }
            else {
                std::cout << "Parameter type not supported in printf!" << std::endl;
                exit(0);
            }
        }
        //���뺯������
        ass_file_handler.insert("call printf", "TEXT");
        if (num_bytes > 0) {
            ass_file_handler.insert("add $" + std::to_string(num_bytes) + ", %esp", "TEXT");
        }

    }
    else if (func_name == "scanf") {
        int num_bytes = 0;
        for (auto it = parameter_list.rbegin(); it != parameter_list.rend(); ++it) {
            const std::string& param = *it;
            auto sym = symbol_table.find(param);
            if (sym == symbol_table.end()) {
                std::cout << "Symbol not found in table: " << param << std::endl;
                exit(0);
            }

            if (sym->second.type == "STRING_CONSTANT" || sym->second.type == "VARIABLE") {
                std::string line = "pushl $" + param;
                ass_file_handler.insert(line, "TEXT");
                num_bytes += 4;
            }
            else {
                std::cout << "Parameter type not supported in scanf!" << std::endl;
                exit(0);
            }
        }

        ass_file_handler.insert("call scanf", "TEXT");
        if (num_bytes > 0) {
            ass_file_handler.insert("add $" + std::to_string(num_bytes) + ", %esp", "TEXT");
        }
    }
}
void Assembler::_assignment(std::shared_ptr<SyntaxTreeNode> node) {
    if (!node || !node->first_son) {
        std::cout << "Invalid assignment node." << std::endl;
        exit(0);
    }

    auto current_node = node->first_son;

    // ����Ƿ��Ǹ�ֵ��ʽ����������� IDENTIFIER���Ҳ������� Expression
    if (current_node->node_type == "IDENTIFIER" && current_node->right && current_node->right->node_value == "Expression") {
        ExpressionResult expres = _expression(current_node->right);

        // ��ȡĿ�����������
        std::string variable_name = current_node->node_value;
        std::string field_type = symbol_table[variable_name].data;

        if (field_type == "int") {
            if (expres.type == "CONSTANT") {
                std::string line = "movl $" + expres.value + ", " + variable_name;
                ass_file_handler.insert(line, "TEXT");
            }
            else if (expres.type == "VARIABLE") {
                std::string line = "movl " + expres.value + ", %edi";
                ass_file_handler.insert(line, "TEXT");

                line = "movl %edi, " + variable_name;
                ass_file_handler.insert(line, "TEXT");
            }
        }
        else if (field_type == "float") {
            if (expres.type == "CONSTANT") {
                std::string line = "movl $" + expres.value + "," + variable_name;
                ass_file_handler.insert(line, "TEXT");

                //ѹջ
                line = "filds " + variable_name;
                ass_file_handler.insert(line, "TEXT");
                line = "fstps " + variable_name;
                ass_file_handler.insert(line, "TEXT");
            }
            else if (expres.type == "VARIABLE"){
                // ֱ�Ӵ洢 FPU ջ����ֵ��Ŀ�����
                std::string line = "fstps " + variable_name;
                ass_file_handler.insert(line, "TEXT");
            }
        }
        else {
            std::cout << "Field type except int and float not supported!" << std::endl;
            exit(0);
        }
    }
    else {
        std::cout << "Assignment format error." << std::endl;
        exit(0);
    }
}
void Assembler::_control_for(std::shared_ptr<SyntaxTreeNode> node) {
    if (!node || !node->first_son) return;

    auto current_node = node->first_son;
    int cnt = 2; // ������һ�����������ʽ��cntΪ2��Ϊ�����ж�ʽ
    int if_jmp_index = -1;
    ExpressionResult ex_tmp;
    // ��һ���֣���ʼ������ֵ��䣩
    if (current_node && current_node->node_value == "Assignment") {
        _assignment(current_node);
        current_node = current_node->right;
    }
    // �ڶ����֣��������ʽ������ѭ�����ж�������
    if (current_node && current_node->node_value == "Expression"&&cnt==2) {
        cnt++;
        std::string line = "label_" + std::to_string(jump_cnt++) + ":";
        ass_file_handler.insert(line, "TEXT");
        ex_tmp=_expression(current_node);
        //������תָ��,��ת��ַ����
        if (ex_tmp.type == "BOOLEAN")if_jmp_index=ass_file_handler.insert("", "TEXT");
        current_node = current_node->right;
    }
    // �������֣�ѭ����
    if (current_node && current_node->node_value == "Sentence") {
        traverse(current_node->first_son);
        current_node = current_node->right;
    }
    // ���Ĳ��֣����±��ʽ���� i++ �� i--��
    if (current_node && current_node->node_value == "Expression") {
        _expression(current_node);
        // ���ص�ѭ����ʼ��
        ass_file_handler.insert("jmp label_" + std::to_string(jump_cnt-1), "TEXT");
    }
    // ����forѭ��������ǩ
    std::string end_label = "label_" + std::to_string(jump_cnt++) + ":";
    ass_file_handler.insert(end_label, "TEXT");
    //ȷ����תָ���ַ���޸���תָ��
    std::string line = operator_map[ex_tmp.value] + " " + end_label;
    ass_file_handler.change(line, if_jmp_index);
}
void Assembler::_control_if(std::shared_ptr<SyntaxTreeNode> node) {
    if (!node || !node->first_son) return;
    auto current_node = node->first_son;

    //else�ꡢend��ǩ��д����ָ��
    std::string label_else, label_end,line;
    int else_jmp_index = -1,end_jmp_index=-1;
    while (current_node != NULL) {
        if (current_node->node_value == "IfControl") {
            //ȷ�� if ���ƽṹ�ĵ�һ���ӽڵ���һ�����ʽ���������Ҳ��ֵܽڵ���һ������
            if (current_node->first_son->node_value != "Expression" || current_node->first_son->right->node_value != "Sentence") {
                std::cout << "control_if error!" << std::endl;
                exit(0);
            }
            ExpressionResult ex_tmp= _expression(current_node->first_son);
            //������תelse����ָ��
            else_jmp_index=ass_file_handler.insert("", "TEXT");
            //����if�����ڲ����
            traverse(current_node->first_son->right->first_son);
            //����else��֧,������תend����ָ��
            end_jmp_index=ass_file_handler.insert("", "TEXT");
            //����else�ı�ǩ
            label_else = "label_" + std::to_string(jump_cnt++);
            line =label_else + ":";
            ass_file_handler.insert(line, "TEXT");
            //����else��ַ
            line = operator_map[ex_tmp.value]+" " +label_else;
            ass_file_handler.change(line, else_jmp_index);
        }
        else if (current_node->node_value == "ElseControl") {
            traverse(current_node->first_son);
            //���������ǩ
            label_end= "label_" + std::to_string(jump_cnt++);
            line = label_end + ":";
            ass_file_handler.insert(line, "TEXT");
            //����end��ַ
            line = "jmp " + label_end;
            ass_file_handler.change(line, end_jmp_index);
        }
        current_node = current_node->right;
    }
}
void Assembler::_control_while(std::shared_ptr<SyntaxTreeNode> node) {
    if (!node || !node->first_son) return;

    auto current_node = node->first_son;
    std::string label_begin= "label_" + std::to_string(jump_cnt++);
    std::string label_end = "label_" + std::to_string(jump_cnt++);
    ExpressionResult ex_tmp;
    int end_jmp_index;
    if (current_node->node_value == "Expression") {
        ass_file_handler.insert(label_begin+":", "TEXT");
        ex_tmp=_expression(current_node);
        //����block,������תend����ָ��
        end_jmp_index = ass_file_handler.insert("", "TEXT");

        current_node = current_node->right;
    }
    else {
        std::cout << "control_while_expression error!" << std::endl;
        exit(0);
    }
    //ѭ������
    if (current_node && current_node->node_value == "Sentence") {
        traverse(current_node->first_son);
        std::string line = "jmp " + label_begin;
        ass_file_handler.insert(line, "TEXT");
        line = label_end + ":";
        ass_file_handler.insert(line, "TEXT");

        //����end��ַ
        line = operator_map[ex_tmp.value] + " " + label_end;
        ass_file_handler.change(line, end_jmp_index);
    }
    else {
        std::cout << "control_while_block error!" << std::endl;
        exit(0);
    }
}
void Assembler::_return(std::shared_ptr<SyntaxTreeNode> node) {
    if (!node || !node->first_son) return;

    auto current_node = node->first_son;
    //return Expresion�ṹ�����㱨��
    if (current_node->node_value != "return" || current_node->right->node_value != "Expression") {
        std::cout << "return error!" << std::endl;
        exit(0);
    }
    else {
        //��ȡ������
        std::string func_name = node->father->left->left->first_son->node_value;
        std::string func_return_field_type = node->father->left->left->left->first_son->node_value;
        std::string line;
        current_node = current_node->right;
        ExpressionResult expres = _expression(current_node);
        if (func_name == "main") {
            //main����ֻ�ܷ��س���
            if (expres.type != "CONSTANT") {
                std::cout << "not supported type of return!" << std::endl;
                exit(0);
            }
            line = "pushl $" + expres.value;
        }
        else {
             if (func_return_field_type == "int")line = "movl " + expres.value+",%eax";
             else if (func_return_field_type == "float")line = "filds "+expres.value;
             else {
                    std::cout << "not supported type of return!" << std::endl;
                    exit(0);
             }
        }
       ass_file_handler.insert(line, "TEXT");
    }
}
void Assembler::_traverse_expression(std::shared_ptr<SyntaxTreeNode> node) {
    if (node == NULL)return;
    if (node->node_type == "_Variable") { 
       /* if(!_is_contain_function(node->node_value))*/operand_stack.push({ "VARIABLE",node->node_value });
        ////��������
        //else {
        //    //ȡ��eax����
        //    std::string line = "movl %eax,bss_tmp";
        //    ass_file_handler.insert(line, "TEXT");
        //    //�ָ�eax�Ĵ���
        //    line = "popl %eax";
        //    ass_file_handler.insert(line, "TEXT");
        //    operand_stack.push({ "VARIABLE","bss_tmp"});
        //}
    }
    else if (node->node_type == "_Constant")operand_stack.push({ "CONSTANT",node->node_value });
    else if (node->node_type == "_Operator")operator_stack.push(node->node_value);
    else if (node->node_type == "_ArrayName") {
        //�����������ƺ������±�
        OperandItem new_item("ARRAY_ITEM", node->node_value);
        new_item.set_operand_array_index(node->right->node_value);
        operand_stack.push(new_item);
        return;
    }
    auto current_node = node->first_son;
    while (current_node != NULL) {
        _traverse_expression(current_node);
        current_node = current_node->right;
    }
}
ExpressionResult Assembler::_expression(std::shared_ptr<SyntaxTreeNode> node) {
    if (node==NULL) return {"", ""};
    //����ֱ�ӷ���ֵ
    if (node->node_type == "Constant") {
        return { "CONSTANT", node->first_son->node_value };
    }

    // ��ղ�����ջ�Ͳ��������ȼ�ջ
    while (!operator_stack.empty())operator_stack.pop();
    while (!operand_stack.empty())operand_stack.pop();
    // �������ʽ�������������Ͳ�����ѹջ
    _traverse_expression(node);
    // ˫Ŀ�����


    while (!operator_stack.empty()) {
        std::string op = operator_stack.top();
        operator_stack.pop();

        if (std::find(double_operators.begin(), double_operators.end(), op) != double_operators.end()) {
            OperandItem operand_b = operand_stack.top();
            operand_stack.pop();
            OperandItem operand_a = operand_stack.top();
            operand_stack.pop();

            bool contain_float = _contain_float(operand_a, operand_b);

            if (op == "+") {
                if (contain_float) {
                    //flds�����ظ��㳣��������� FPU ����Ĵ���ջ��
                    //filds�������������ݲ�ת��Ϊ��������ѹ�� FPU ջ��
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a�Ѽ��ص�ջ��
                    line = (_is_float(operand_b) ? "fadd " : "fiadd ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //���������浽bss_tmp��
                    line = "fstps bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //������ѹջ
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] =SymbolTableItem("IDENTIFIER","field_type","float");
                }
                else {
                    std::string line;
                    // �������ͼӷ�
                    //��һ��������
                    if (operand_a.type == "ARRAY_ITEM") {
                        //���±�ͻ�ַ
                        line = "movl " + operand_a.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "movl (" + operand_a.operand + ",%edi,4), %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "VARIABLE") {
                        line = "movl " + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "CONSTANT") {
                        line = "movl $" + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    if (operand_b.type == "ARRAY_ITEM") {
                        //���±�ͻ�ַ
                        line = "movl " + operand_b.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "addl (" + operand_b.operand + ",%edi,4), %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "VARIABLE") {
                        line = "addl " + operand_b.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "CONSTANT") {
                        line = "addl $" + operand_b.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    //������ʱ������
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //������ѹջ
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }
            }
            else if (op == "-") {
                if (contain_float) {
                    //flds�����ظ��㳣��������� FPU ����Ĵ���ջ��
                    //filds�������������ݲ�ת��Ϊ��������ѹ�� FPU ջ��
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a�Ѽ��ص�ջ��
                    line = (_is_float(operand_b) ? "fsub " : "fisub ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //���������浽bss_tmp��
                    line = "fstps bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //������ѹջ
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "float");
                }
                else {
                    std::string line;
                    // �������ͼ���
                    //��һ��������
                    if (operand_a.type == "ARRAY_ITEM") {
                        //���±�ͻ�ַ
                        line = "movl " + operand_a.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "movl (" + operand_a.operand + ",%edi,4), %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "VARIABLE") {
                        line = "movl " + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "CONSTANT") {
                        line = "movl $" + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    if (operand_b.type == "ARRAY_ITEM") {
                        //���±�ͻ�ַ
                        line = "movl " + operand_b.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "subl (" + operand_b.operand + ", ,%edi,4), %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "VARIABLE") {
                        line = "subl " + operand_b.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "CONSTANT") {
                        line = "subl $" + operand_b.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    //������ʱ������
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //������ѹջ
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }
            }
            //Ŀǰֻ���������˷�
            else if (op == "*") {
                if (contain_float) {
                    //��ʱ��֧�ָ������˷�
                }
                //�����˷�,mullֻ��һ�������� ��mull %ecx  edx:eax->eax*ecx
                else {
                    std::string line;
                    if (operand_a.type == "ARRAY_ITEM") {
                        line = "movl " + operand_a.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "movl (" + operand_a.operand + ",%edi,4), %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "VARIABLE") {
                        line = "movl " + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "CONSTANT") {
                        line = "movl $" + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    if (operand_b.type == "ARRAY_ITEM") {
                        //���±�ͻ�ַ
                        line = "movl " + operand_b.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "mull (" + operand_b.operand + ",%edi,4)";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "VARIABLE") {
                        line = "mull " + operand_b.operand ;
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "CONSTANT") {
                        line = "mull $" + operand_b.operand;
                        ass_file_handler.insert(line, "TEXT");
                    }
                    //������ʱ������
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //������ѹջ
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }
            }
            else if (op == "/") {
                //����������
                if (contain_float) {
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a�Ѽ��ص�ջ��
                    line = (_is_float(operand_b) ? "fdiv " : "fidiv ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //���������浽bss_tmp��
                    line = "fstps bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //������ѹջ
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "float");
                }
                //��������
                else {
                    std::string line;
                    if (operand_a.type == "ARRAY_ITEM") {
                        line = "movl " + operand_a.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "movl (" + operand_a.operand + ",%edi,4), %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "VARIABLE") {
                        line = "movl " + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "CONSTANT") {
                        line = "movl $" + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    if (operand_b.type == "ARRAY_ITEM") {
                        //���±�ͻ�ַ
                        line = "movl " + operand_b.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "divl (" + operand_b.operand + ",%edi,4)";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "VARIABLE") {
                        line = "divl " + operand_b.operand;
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "CONSTANT") {
                        line = "divl $" + operand_b.operand;
                        ass_file_handler.insert(line, "TEXT");
                    }
                    //������ʱ������
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //������ѹջ
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }     
            }
            //�Ƚ����������ʱ���رȽ��������������{BOOLEAN,>=}
            else if (op == ">="|| op == "<"|| op == "<="|| op == ">"|| op == "=="||op=="!=") {
                if (contain_float) {
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a�Ѽ��ص�ջ��
                    line = (_is_float(operand_b) ? "flds " : "filds ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    line = "fcom";
                    ass_file_handler.insert(line, "TEXT");

                    //�޸�EFLAGS�Ĵ�����״̬��־λ
                    line = "fstsw %eax";
                    ass_file_handler.insert(line, "TEXT");

                    //���Ʊ�־�Ĵ�����CPU��־�Ĵ���
                    line = "sahf";
                    ass_file_handler.insert(line, "TEXT");

                    return { "BOOLEAN",op};
                }
                else {
                    std::string line;
                    if (operand_a.type == "ARRAY_ITEM") {
                        line = "movl " + operand_a.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "movl (" + operand_a.operand + ",%edi,4), %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "VARIABLE") {
                        line = "movl " + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_a.type == "CONSTANT") {
                        line = "movl $" + operand_a.operand + ", %eax";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    if (operand_b.type == "ARRAY_ITEM") {
                        //���±�ͻ�ַ
                        line = "movl " + operand_b.operand_array_index + ", %edi";
                        ass_file_handler.insert(line, "TEXT");
                        line = "movl (" + operand_b.operand + ",%edi,4), %ebx";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "VARIABLE") {
                        line = "movl " + operand_b.operand+", %ebx";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    else if (operand_b.type == "CONSTANT") {
                        line = "movl $" + operand_b.operand+", %ebx";
                        ass_file_handler.insert(line, "TEXT");
                    }
                    line = "cmp %eax,%ebx";
                    ass_file_handler.insert(line, "TEXT");


                    return { "BOOLEAN",op };
                }
            }
        }
        // ��Ŀ���������
        else if (std::find(single_operators.begin(), single_operators.end(), op) != single_operators.end()) {
            OperandItem operand = operand_stack.top();
            operand_stack.pop();
            if (op == "++")ass_file_handler.insert("incl " + operand.operand, "TEXT");
            else if (op == "--")ass_file_handler.insert("decl " + operand.operand, "TEXT");
            //δ����!�����
        }
        else {
            std::cout << "Unsupported operator: " << op << std::endl;
            exit(0);
        }
    }

    //����typeΪ�������߱���������������
    if (!operand_stack.empty()) {
        auto p = operand_stack.top();
        return { p.type,p.operand};
    }

    return { "", "" };
}
void Assembler::_handler_block(std::shared_ptr<SyntaxTreeNode> node) {
    if (node == NULL)return;
    if (_is_contain_sentence(node->node_value)) {
        if (node->node_value == "Program" || node->node_value == "Sentence")traverse(node->first_son);
        else if (node->node_value == "Include")_include(node);
        else if (node->node_value == "FunctionStatement")_function_statement(node);
        else if (node->node_value == "Statement")_statement(node);
        else if (node->node_value == "FunctionCall")_function_call(node);
        else if (node->node_value == "Assignment")_assignment(node);
        else if (node->node_value == "Control") {
            if (node->node_type == "IfElseControl")_control_if(node);
            else if (node->node_type == "ForControl")_control_for(node);
            else if (node->node_type == "WhileControl")_control_while(node);
            else {
                std::cout << "control type not supported!" << std::endl;
                exit(0);
            }
        }
        else if (node->node_value == "Expression")_expression(node);
        else if (node->node_value == "Return")_return(node);
        else {
            std::cout << "sentence type not supported yet!" << std::endl;
            exit(0);
        }
    }
}
void Assembler::traverse(std::shared_ptr<SyntaxTreeNode> node) {
    _handler_block(node);
    std::shared_ptr<SyntaxTreeNode>next_node = node->right;
    while (next_node != NULL) {
        _handler_block(next_node);
        next_node = next_node->right;
    }
}
void Assembler::main() {
    traverse(tree.root);
    stringstream ss(filename);
    string split_filename;
    getline(ss, split_filename, '.');
    ass_file_handler.generateAssFile(split_filename);
}