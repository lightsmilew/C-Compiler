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
//判断是否是float类型
bool Assembler::_is_float(OperandItem& operand) {
    return operand.type == "VARIABLE" && symbol_table[operand.operand].data_name == "field_type" && symbol_table[operand.operand].data == "float";
}
bool Assembler::_contain_float(OperandItem& operand_a, OperandItem& operand_b) {
    return _is_float(operand_a) || _is_float(operand_b);
}
void Assembler::_include(std::shared_ptr<SyntaxTreeNode> node) {
    //暂时不做任何处理
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
//将数组用指定分隔符连接
string Assembler::join(const std::vector<std::string>& list, const std::string& sep) {
    if (list.empty()) return "";
    std::string result = list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        result += sep + list[i];
    }
    return result;
}
//暂时只支持main函数，目前没有处理函数参数和返回值
void Assembler::_function_statement(std::shared_ptr<SyntaxTreeNode> node) {
    std::shared_ptr<SyntaxTreeNode>current_node = node->first_son;
    std::vector<string>para_list;
    std::string func_name = "";
    std::string func_type = "";
    int para_count = 0;
    while (current_node != NULL) {
        //处理返回值类型
        if (current_node->node_value == "Type")func_type = current_node->first_son->node_value;
        //处理函数名
        else if (current_node->node_value == "FunctionName") {
            func_name = current_node->first_son->node_value;
            // 生成函数标签
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
        // 处理函数参数
        else if (current_node->node_value == "StateParameterList") {
            // 解析参数并生成相应的汇编代码
            //记录参数的数量
            std::string line = "pushl %ebp";
            ass_file_handler.insert(line, "TEXT");
            line = "movl %eap,%ebp";
            ass_file_handler.insert(line, "TEXT");
            std::shared_ptr<SyntaxTreeNode>parameter_node = current_node->first_son;
            while (parameter_node != NULL) {
                para_count++;
                //变量值类型
                std::string para_field_type = parameter_node->first_son->node_value;
                std::string para_type = parameter_node->first_son->right->extra_info["type"];
                std::string para_name =parameter_node->first_son->right->node_value;
                para_list.push_back(para_name);
                if (para_field_type != "int") {
                    std::cout << "not supported type in stateparameterlist!" << std::endl;
                    exit(0);
                }
                //统一使用全局变量
                //变量插入到汇编文件对应段中
                line = ".lcomm " + para_name + ", " + _sizeof(para_field_type);
                std::string section = (para_type == "VARIABLE") ? "BSS" : "DATA";
                ass_file_handler.insert(line, section);
                // 将变量加入符号表
                symbol_table[para_name] = SymbolTableItem{
                    "VARIABLE","field_type", para_field_type
                };

                //插入赋值语句，初始化入口参数
                line = "movl %" + to_string(4*(para_count+1))+"(%ebp)" + "," + para_name;
                ass_file_handler.insert(line, "TEXT");

                parameter_node = parameter_node->right;
            }
        }
        //处理函数体
        else if (current_node->node_value == "Sentence") { 
            traverse(current_node->first_son); 
            if (func_name != "main") {
                std::string line = "popl %ebp";
                ass_file_handler.insert(line, "TEXT");
                //插入返回指令
                ass_file_handler.insert("ret", "TEXT");
            }
        }
        current_node = current_node->right;
    }

    //清除局部变量声明
    for (auto p : para_list) {
        symbol_table.erase(p);
    }
    //main函数在结尾插入call exit调用程序退出汇编指令
    if (func_name == "main") {
        std::string line = "call exit";
        ass_file_handler.insert(line, "TEXT");
    }
    function_table[func_name] = func_type;
}
//直接保存为全局变量
void Assembler::_statement(std::shared_ptr<SyntaxTreeNode> node) {
    std::string line;
    std::string variable_field_type; // 数据类型：int, float 等
    std::string variable_type;       // 变量类型：VARIABLE 或 ARRAY
    std::string variable_name;       // 变量名

    auto current_node = node->first_son;

    while (current_node != NULL) {
        if (current_node->node_value == "Type") {
            // 获取数据类型
            variable_field_type = current_node->first_son->node_value;
        }
        else if (current_node->node_type == "IDENTIFIER") {
            // 获取变量名和类型（来自 extra_info）
            variable_name = current_node->node_value;
            variable_type = current_node->extra_info["type"]; 
            line = ".lcomm " + variable_name + ", " + _sizeof(variable_field_type);
        }
        //常量列表
        else if (current_node->node_value == "ConstantList") {
            // 数组初始化
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

    // 插入到汇编文件对应段中
    std::string section = (variable_type == "VARIABLE") ? "BSS" : "DATA";
    ass_file_handler.insert(line, section);

    // 将变量加入符号表
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
                        //如果是字符串常量
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
                        //其他类型的参数暂不支持
                        std::cout << "Unsupported parameter type: " << tmp_node->node_type << std::endl;
                        exit(0);
                    }

                    tmp_node = tmp_node->right;
                }
            }
            //其他函数的参数处理
            //目前只支持整型参数
            else {
                //保存返回值
                //std::string line = "pushl %eax";
                //ass_file_handler.insert(line, "TEXT");
                //caller参数传递
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
                        //其他类型的参数暂不支持
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
    //如果不是库函数则直接call调用
    if (func_name != "printf" && func_name != "scanf") {
        std::string line = "call " + func_name;
        ass_file_handler.insert(line, "TEXT");

        line = "addl $" +to_string(4*para_count)+",%esp";
        ass_file_handler.insert(line, "TEXT");
    }
    // 处理printf函数调用
    if (func_name == "printf") {
        //%esp要+的值
        int num_bytes = 0;
        for (auto it = parameter_list.rbegin(); it != parameter_list.rend(); ++it) {
            const std::string& param = *it;
            //去符号表里面寻找参数
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
        //插入函数调用
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

    // 检查是否是赋值格式：左操作数是 IDENTIFIER，右操作数是 Expression
    if (current_node->node_type == "IDENTIFIER" && current_node->right && current_node->right->node_value == "Expression") {
        ExpressionResult expres = _expression(current_node->right);

        // 获取目标变量的类型
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

                //压栈
                line = "filds " + variable_name;
                ass_file_handler.insert(line, "TEXT");
                line = "fstps " + variable_name;
                ass_file_handler.insert(line, "TEXT");
            }
            else if (expres.type == "VARIABLE"){
                // 直接存储 FPU 栈顶的值到目标变量
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
    int cnt = 2; // 控制哪一个是条件表达式，cnt为2则为条件判断式
    int if_jmp_index = -1;
    ExpressionResult ex_tmp;
    // 第一部分：初始化（赋值语句）
    if (current_node && current_node->node_value == "Assignment") {
        _assignment(current_node);
        current_node = current_node->right;
    }
    // 第二部分：条件表达式（进入循环的判断条件）
    if (current_node && current_node->node_value == "Expression"&&cnt==2) {
        cnt++;
        std::string line = "label_" + std::to_string(jump_cnt++) + ":";
        ass_file_handler.insert(line, "TEXT");
        ex_tmp=_expression(current_node);
        //生成跳转指令,跳转地址待定
        if (ex_tmp.type == "BOOLEAN")if_jmp_index=ass_file_handler.insert("", "TEXT");
        current_node = current_node->right;
    }
    // 第三部分：循环体
    if (current_node && current_node->node_value == "Sentence") {
        traverse(current_node->first_son);
        current_node = current_node->right;
    }
    // 第四部分：更新表达式（如 i++ 或 i--）
    if (current_node && current_node->node_value == "Expression") {
        _expression(current_node);
        // 返回到循环开始处
        ass_file_handler.insert("jmp label_" + std::to_string(jump_cnt-1), "TEXT");
    }
    // 插入for循环结束标签
    std::string end_label = "label_" + std::to_string(jump_cnt++) + ":";
    ass_file_handler.insert(end_label, "TEXT");
    //确定跳转指令地址，修改跳转指令
    std::string line = operator_map[ex_tmp.value] + " " + end_label;
    ass_file_handler.change(line, if_jmp_index);
}
void Assembler::_control_if(std::shared_ptr<SyntaxTreeNode> node) {
    if (!node || !node->first_son) return;
    auto current_node = node->first_son;

    //else标、end标签和写入汇编指令
    std::string label_else, label_end,line;
    int else_jmp_index = -1,end_jmp_index=-1;
    while (current_node != NULL) {
        if (current_node->node_value == "IfControl") {
            //确保 if 控制结构的第一个子节点是一个表达式，并且其右侧兄弟节点是一个语句块
            if (current_node->first_son->node_value != "Expression" || current_node->first_son->right->node_value != "Sentence") {
                std::cout << "control_if error!" << std::endl;
                exit(0);
            }
            ExpressionResult ex_tmp= _expression(current_node->first_son);
            //生成跳转else语句块指令
            else_jmp_index=ass_file_handler.insert("", "TEXT");
            //处理if语句块内部语句
            traverse(current_node->first_son->right->first_son);
            //跳过else分支,插入跳转end语句块指令
            end_jmp_index=ass_file_handler.insert("", "TEXT");
            //插入else的标签
            label_else = "label_" + std::to_string(jump_cnt++);
            line =label_else + ":";
            ass_file_handler.insert(line, "TEXT");
            //回填else地址
            line = operator_map[ex_tmp.value]+" " +label_else;
            ass_file_handler.change(line, else_jmp_index);
        }
        else if (current_node->node_value == "ElseControl") {
            traverse(current_node->first_son);
            //插入结束标签
            label_end= "label_" + std::to_string(jump_cnt++);
            line = label_end + ":";
            ass_file_handler.insert(line, "TEXT");
            //回填end地址
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
        //跳过block,插入跳转end语句块指令
        end_jmp_index = ass_file_handler.insert("", "TEXT");

        current_node = current_node->right;
    }
    else {
        std::cout << "control_while_expression error!" << std::endl;
        exit(0);
    }
    //循环部分
    if (current_node && current_node->node_value == "Sentence") {
        traverse(current_node->first_son);
        std::string line = "jmp " + label_begin;
        ass_file_handler.insert(line, "TEXT");
        line = label_end + ":";
        ass_file_handler.insert(line, "TEXT");

        //回填end地址
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
    //return Expresion结构不满足报错
    if (current_node->node_value != "return" || current_node->right->node_value != "Expression") {
        std::cout << "return error!" << std::endl;
        exit(0);
    }
    else {
        //获取函数名
        std::string func_name = node->father->left->left->first_son->node_value;
        std::string func_return_field_type = node->father->left->left->left->first_son->node_value;
        std::string line;
        current_node = current_node->right;
        ExpressionResult expres = _expression(current_node);
        if (func_name == "main") {
            //main函数只能返回常量
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
        ////函数调用
        //else {
        //    //取出eax变量
        //    std::string line = "movl %eax,bss_tmp";
        //    ass_file_handler.insert(line, "TEXT");
        //    //恢复eax寄存器
        //    line = "popl %eax";
        //    ass_file_handler.insert(line, "TEXT");
        //    operand_stack.push({ "VARIABLE","bss_tmp"});
        //}
    }
    else if (node->node_type == "_Constant")operand_stack.push({ "CONSTANT",node->node_value });
    else if (node->node_type == "_Operator")operator_stack.push(node->node_value);
    else if (node->node_type == "_ArrayName") {
        //保存数组名称和数组下标
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
    //常量直接返回值
    if (node->node_type == "Constant") {
        return { "CONSTANT", node->first_son->node_value };
    }

    // 清空操作数栈和操作符优先级栈
    while (!operator_stack.empty())operator_stack.pop();
    while (!operand_stack.empty())operand_stack.pop();
    // 遍历表达式树，将操作数和操作符压栈
    _traverse_expression(node);
    // 双目运算符


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
                    //flds：加载浮点常量或变量到 FPU 浮点寄存器栈。
                    //filds：加载整型数据并转换为浮点数后压入 FPU 栈。
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a已加载到栈顶
                    line = (_is_float(operand_b) ? "fadd " : "fiadd ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果保存到bss_tmp中
                    line = "fstps bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果压栈
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] =SymbolTableItem("IDENTIFIER","field_type","float");
                }
                else {
                    std::string line;
                    // 处理整型加法
                    //第一个操作数
                    if (operand_a.type == "ARRAY_ITEM") {
                        //存下标和基址
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
                        //存下标和基址
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
                    //存入临时操作数
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果压栈
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }
            }
            else if (op == "-") {
                if (contain_float) {
                    //flds：加载浮点常量或变量到 FPU 浮点寄存器栈。
                    //filds：加载整型数据并转换为浮点数后压入 FPU 栈。
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a已加载到栈顶
                    line = (_is_float(operand_b) ? "fsub " : "fisub ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果保存到bss_tmp中
                    line = "fstps bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果压栈
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "float");
                }
                else {
                    std::string line;
                    // 处理整型减法
                    //第一个操作数
                    if (operand_a.type == "ARRAY_ITEM") {
                        //存下标和基址
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
                        //存下标和基址
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
                    //存入临时操作数
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果压栈
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }
            }
            //目前只考虑整数乘法
            else if (op == "*") {
                if (contain_float) {
                    //暂时不支持浮点数乘法
                }
                //整数乘法,mull只有一个操作数 如mull %ecx  edx:eax->eax*ecx
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
                        //存下标和基址
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
                    //存入临时操作数
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果压栈
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }
            }
            else if (op == "/") {
                //浮点数除法
                if (contain_float) {
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a已加载到栈顶
                    line = (_is_float(operand_b) ? "fdiv " : "fidiv ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果保存到bss_tmp中
                    line = "fstps bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果压栈
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "float");
                }
                //整数除法
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
                        //存下标和基址
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
                    //存入临时操作数
                    line = "movl %eax,bss_tmp";
                    ass_file_handler.insert(line, "TEXT");

                    //计算结果压栈
                    operand_stack.push({ "VARIABLE", "bss_tmp" });
                    symbol_table["bss_tmp"] = SymbolTableItem("IDENTIFIER", "field_type", "int");
                }     
            }
            //比较运算符，此时返回比较运算符的类型如{BOOLEAN,>=}
            else if (op == ">="|| op == "<"|| op == "<="|| op == ">"|| op == "=="||op=="!=") {
                if (contain_float) {
                    std::string line = (_is_float(operand_a) ? "flds " : "filds ") + operand_a.operand;
                    ass_file_handler.insert(line, "TEXT");

                    //a已加载到栈顶
                    line = (_is_float(operand_b) ? "flds " : "filds ") + operand_b.operand;
                    ass_file_handler.insert(line, "TEXT");

                    line = "fcom";
                    ass_file_handler.insert(line, "TEXT");

                    //修改EFLAGS寄存器的状态标志位
                    line = "fstsw %eax";
                    ass_file_handler.insert(line, "TEXT");

                    //复制标志寄存器到CPU标志寄存器
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
                        //存下标和基址
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
        // 单目运算符处理
        else if (std::find(single_operators.begin(), single_operators.end(), op) != single_operators.end()) {
            OperandItem operand = operand_stack.top();
            operand_stack.pop();
            if (op == "++")ass_file_handler.insert("incl " + operand.operand, "TEXT");
            else if (op == "--")ass_file_handler.insert("decl " + operand.operand, "TEXT");
            //未处理!运算符
        }
        else {
            std::cout << "Unsupported operator: " << op << std::endl;
            exit(0);
        }
    }

    //返回type为常量或者变量，无数组类型
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