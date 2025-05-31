#include"Parser.h"
#include<iostream>
#include <sstream>
#include <stack>
#include<fstream>
extern vector<vector<string>> keywords;
extern vector<string> operators;
extern vector<string> single_operators;
extern vector<string> double_operators;
int strToInt(const std::string& str) {
	std::stringstream ss(str);
	int num;
	ss >> num;
	return num;
}
void Parser::readTokensFromFile(const std::string& filename) {
	// 构造输入文件名
	std::stringstream ss(filename);
	std::string split_filename;
	std::getline(ss, split_filename, '.');
	std::string inputFilename = split_filename + "_lexer.txt";
	// 打开文件进行读取
	std::ifstream inFile(inputFilename);
	if (!inFile.is_open()) {
		std::cout << "无法打开文件: " << inputFilename << std::endl;
		std::cout << "自动进行词法分析...... "<< std::endl;
		Lexer lexer(filename);
		lexer.main();
		lexer.generateTokens();
		tokens = lexer.getTokens();
		return;
	}
	std::string line;
	while (std::getline(inFile, line)) {
		std::istringstream iss(line);
		std::string type_n, value;

		if (std::getline(iss, type_n, ' ') && std::getline(iss, value)) {
			tokens.push_back({ type_n, value });
		}
		else {
			std::cerr << "解析行失败: " << line << std::endl;
		}
	}
	inFile.close();
}
//Block-->{Sentences}
//Sentences-->Sentence Sentences|ɛ
//Sentence-->Statement|Assignment|FunctionCall|Control|Return
void Parser::_block(SyntaxTree father_tree) {
	index++;
	SyntaxTree sentence_tree;
	sentence_tree.current = sentence_tree.root = std::make_shared<SyntaxTreeNode>("Sentence");
	father_tree.add_child_node(sentence_tree.root, father_tree.root);
	while (index<tokens.size()) {
		string sentence_pattern = _judge_sentence_pattern();
		//处理声明语句
		if (sentence_pattern == "STATEMENT")_statement(sentence_tree.root);
		//处理赋值语句
		else if (sentence_pattern == "ASSIGNMENT")_assignment(sentence_tree.root);
		//处理函数调用
		else if (sentence_pattern == "FUNCTION_CALL")_function_call(sentence_tree.root);
		//控制流语句
		else if (sentence_pattern == "CONTROL")_control(sentence_tree.root);
		//return语句
		else if (sentence_pattern == "RETURN")_return(sentence_tree.root);
		//表达式语句,跳过;
		//目前仅支持自增和自减表达式出现在block中
		else if (sentence_pattern == "EXPRESSION") { _expression(sentence_tree.root); index++; }
		//右大括号
		else if (sentence_pattern == "RB_BRACKET")break;
		else {
			std::cout << "block error!" << std::endl;
			exit(0);
		}
	}
}
//Include-->#include"Identifier"|include<Identifier>
void Parser::_include(std::shared_ptr<SyntaxTreeNode>father) {
	if (father == NULL)father = tree.root;
	SyntaxTree include_tree;
	include_tree.current=include_tree.root= std::make_shared<SyntaxTreeNode>("Include");
	tree.add_child_node(include_tree.root, father);
	//双引号个数cnt
	int cnt = 0;
	//include语句结束标记
	bool flag = false;
	while (!flag) {
		//此处include语句语法分析已在词法分析过程中进行过，故不进行匹配判断
		if (tokens[index].value == "\"")cnt++;
		if (index >= tokens.size() || cnt >= 2 || tokens[index].value == ">")flag = true;
		include_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value,tokens[index].type_n), include_tree.root);
		index++;
	}
}
//FunctionStatement-->Type FunctionName(StateParameterList)Block|Type FunctionName(StateParameterList)
//StateParameterList-->Parameter|Parameter,StateParameterList|ɛ
//Parameter-->Type Identifier
//Type-->int|char|float|double|void
void Parser::_function_statement(std::shared_ptr<SyntaxTreeNode>father) {
	if (father == NULL)father = tree.root;
	SyntaxTree func_statement_tree;
	func_statement_tree.current = func_statement_tree.root = std::make_shared<SyntaxTreeNode>("FunctionStatement");
	tree.add_child_node(func_statement_tree.root, father);
	bool flag = true;
	while (flag && index < tokens.size()) {
		//如果是返回类型
		for (auto p : keywords[0]) {
			if (tokens[index].value == p) {
				func_statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>("Type"),func_statement_tree.root);
				//value type extrainfo
				func_statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "FIELD_TYPE",tokens[index].value));
				index++;
				break;
			}
		}
		if (tokens[index].type_n == "IDENTIFIER") {
			func_statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>("FunctionName"), func_statement_tree.root);
			//value type extrainfo
			func_statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "IDENTIFIER", "FUNCTION_NAME"));
			index++;
		}
		else if (tokens[index].type_n == "LL_BRACKET") {
			std::shared_ptr<SyntaxTreeNode>params_list = std::make_shared<SyntaxTreeNode>("StateParameterList");
			func_statement_tree.add_child_node(params_list, func_statement_tree.root);
			//value type extrainfo
			index++;
			//匹配每个参数
			while (tokens[index].type_n != "RL_BRACKET") {
				for (auto p : keywords[0]) {
					if (tokens[index].value == p) {
						std::shared_ptr<SyntaxTreeNode>param = std::make_shared<SyntaxTreeNode>("Parameter");
						func_statement_tree.add_child_node(param, params_list);
						//标识符类型
						func_statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "FIELD_TYPE",tokens[index].value), param);
					    //标识符
						if (tokens[index + 1].type_n == "IDENTIFIER") {
							//名字和类型（数组还是变量）
							std::shared_ptr<SyntaxTreeNode>new_node = std::make_shared<SyntaxTreeNode>(tokens[index + 1].value, "IDENTIFIER", "VARIABLE");
							new_node->set_extra_info("variable_type", tokens[index].value);
							func_statement_tree.add_child_node(new_node, param);
						}
						else {
							std::cout << "error:function_statement_parameter!" << endl;
							exit(0);
						}
						index++;
					}
				}
				//成功匹配一个参数
				index++;
			}
			//参数列表匹配结束
			index++;
		}
		//遇见左大括号则开始匹配Sentence
		else if (tokens[index].type_n == "LB_BRACKET") {
			_block(func_statement_tree);
			break;
		}
	}
}
//Statement-->Type Identifierlist;|Type Identifierlist [Num];Type Identifierlist [Num]={Constantlist};
//Identifierlist=Identifier|Identifier,Identifierlist
//Constantlist=Num|Num,Constantlist
void Parser::_statement(std::shared_ptr<SyntaxTreeNode> father) {
	if (father == NULL)father = tree.root;
	SyntaxTree statement_tree;
	//保存当前声明语句的类型
	string tmp_variable_type = "";
	//进入该函数前提是已识别Type和IDENTIFIER（judge函数逻辑）,故不需要判断前两个token类型
	//变量类型Type
	for (auto p : keywords[0]) {
		if (p == tokens[index].value) {
			tmp_variable_type = tokens[index].value;
			break;
		}
	}
	index++;
	do {
		//跳过逗号(这里第一次进入循环时必定是IDENTIFIER,故进入该if语句即是多个变量声明)
		if (tokens[index].type_n == "COMMA")index++;
		if (tokens[index].type_n == "IDENTIFIER") {
			SyntaxTree statement_tree;
			statement_tree.current = statement_tree.root = std::make_shared<SyntaxTreeNode>("Statement");
			statement_tree.add_child_node(statement_tree.root, father);
			//类型
			statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>("Type"), statement_tree.root);
			//extra_info
			statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tmp_variable_type, "FIELD_TYPE",tmp_variable_type));
			//变量名
			std::shared_ptr<SyntaxTreeNode>new_node = std::make_shared<SyntaxTreeNode>(tokens[index].value, "IDENTIFIER", "VARIABLE");
			new_node->set_extra_info("variable_type", tmp_variable_type);
			statement_tree.add_child_node(new_node, statement_tree.root);
			index++;
			//如果是数组进入下面if语句，否则进入下一趟while循环判断
			if (tokens[index].type_n == "LM_BRACKET") {
				//记录数组大小
				int array_size = 0;
				if (tokens[++index].type_n == "DIGIT_CONSTANT") {
					array_size = strToInt(tokens[index].value);
					statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "DIGIT_CONSTANT"), statement_tree.root);
					statement_tree.current->left->set_extra_info("type", "LIST");
					statement_tree.current->left->set_extra_info("list_type",tmp_variable_type);
				}
				else {
					std::cout << "error:the size of array is not a digit!" << std::endl;
					exit(0);
				}
				if (tokens[++index].type_n != "RM_BRACKET") {
					std::cout << "error: lack of ] in array statement !" << std::endl;
					exit(0);
				}
				//识别]则进入下一步
				//如果声明时对数组元素赋值
				index++;
				if (tokens[index].type_n == "ASSIGN") {
					index++;
					if (tokens[index].type_n == "LB_BRACKET") {
						index++;
						//常量链表树
						std::shared_ptr<SyntaxTreeNode>constant_list = std::make_shared<SyntaxTreeNode>("ConstantList");
						statement_tree.add_child_node(constant_list, statement_tree.root);
						int constant_list_size = 0;
						while (tokens[index].type_n != "RB_BRACKET") {
							if (tokens[index].type_n == "DIGIT_CONSTANT") {
								statement_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "DIGIT_CONSTANT"), constant_list);
								constant_list_size++;
								//常量链表大小超出声明数组大小
								if (constant_list_size > array_size) {
									std::cout << "error: the size of constant_list is larger than the size of array!" << std::endl;
									exit(0);
								}
								index++;
							}
							//处理
							else if (tokens[index].type_n == "COMMA")index++;
							else {
								std::cout << "error:constant_list grammar" << std::endl;
								exit(0);
							}
						}
					}
				}
				//数组识别结束
				index++;
			}
		}
		else {
			std::cout << "error grammar in multiple statement!" << std::endl;
			exit(0);
		}
	} while (index + 1 < tokens.size()&&tokens[index].type_n == "COMMA");
	//识别分号;不是分号则报错
	if (tokens[index].type_n != "SEMICOLON") {
		std::cout << "error:lack of ; in statement" << std::endl;
		exit(0);
	}
	//声明语句识别结束
	index++;
}
//Assignment->Identifier=Expression;
void Parser::_assignment(std::shared_ptr<SyntaxTreeNode> father ) {
	if (father == NULL)father = tree.root;
	SyntaxTree assign_tree;
	assign_tree.current = assign_tree.root = std::make_shared<SyntaxTreeNode>("Assignment");
	tree.add_child_node(assign_tree.root, father);
	//进入该函数时已识别前两个token为INDENTIFIER和ASSIGN(judge函数实现）
	assign_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "IDENTIFIER"));
	//judge函数已判断存在= 识别=直接跳过，index自增2
	index+=2;
	_expression(assign_tree.root);
	//这里不需要自增因为在_expression函数调用结束时index已自增1
	if (tokens[index].type_n != "SEMICOLON") {
		std::cout << "error:lack of ; in assignment！" << std::endl;
		exit(0);
	}
	//赋值语句识别结束
	index++;
}
//WhileControl->while(Expression) Block |while(Expression);
void Parser::_while(std::shared_ptr<SyntaxTreeNode> father ) {
	SyntaxTree while_tree;
	while_tree.current = while_tree.root = std::make_shared<SyntaxTreeNode>("Control","WhileControl");
	tree.add_child_node(while_tree.root, father);
	//识别while，目前未支持do{}while()结构
	index++;
	if (tokens[index].type_n == "LL_BRACKET") {
		int tmp_index = ++index;
		while (tmp_index < tokens.size() && tokens[tmp_index].type_n != "RL_BRACKET")tmp_index++;
		if (tmp_index < tokens.size())_expression(while_tree.root, tmp_index);
		//未找到)匹配识别
		else {
			std::cout << "error:lack of ) in while sentence!" << std::endl;
			exit(0);
		}
		//跳过)
		index++;
		if (tokens[index].type_n == "LB_BRACKET")_block(while_tree);
		else if (tokens[index].type_n == "SEMICOLON")index++;
		else {
			std::cout << "error:lack of ; in while！" << std::endl;
			exit(0);
		}	
	}
}
//ForControl->for(Assignment;Expression;Expression) Block
void Parser::_for(std::shared_ptr<SyntaxTreeNode> father ) {
	SyntaxTree for_tree;
	for_tree.current=for_tree.root= std::make_shared<SyntaxTreeNode>("Control", "ForControl");
	tree.add_child_node(for_tree.root, father);
	//识别for
	index++;
	//识别(
	if (tokens[index].type_n == "LL_BRACKET") {
			int tmp_index = ++index;
			while (tokens[tmp_index].type_n != "RL_BRACKET")tmp_index++;
			_assignment(for_tree.root);
			_expression(for_tree.root);
			if (tokens[index].type_n == "SEMICOLON")index++;
			else {
				std::cout << "error:lack of ; in for sentence!" << std::endl;
				exit(0);
			}
			_expression(for_tree.root,tmp_index);
			//执行后index自增1,此时token为),故还需要自增1到{
			index++;
		}
	//识别{
	if (tokens[index].type_n == "LB_BRACKET")_block(for_tree);
	else if (tokens[index].type_n == "SEMICOLON")index++;
	//出现其他符号则报错
	else {
			std::cout << "error:invalid character occurs in for sentence!" << std::endl;
			exit(0);
	}	
	//交换for语句下第三个子节点和第四个子节点，因为先执行循环体再改变循环变量
	std::shared_ptr<SyntaxTreeNode>current_node = for_tree.root->first_son->right->right;
	std::shared_ptr<SyntaxTreeNode>next_node = current_node->right;
	for_tree.exchange_lr_tree(current_node, next_node);
}
//IfElseControl->IfControl ElseControl|IfControl
//IfControl->if(Expression)Block
//ElseControl->else Block
void Parser::_if_else(std::shared_ptr<SyntaxTreeNode> father ) {
	SyntaxTree if_else_tree;
	if_else_tree.current = if_else_tree.root = std::make_shared<SyntaxTreeNode>("Control", "IfElseControl");
	tree.add_child_node(if_else_tree.root, father);

	SyntaxTree if_tree;
	if_tree.current=if_tree.root= std::make_shared<SyntaxTreeNode>("IfControl");
	if_else_tree.add_child_node(if_tree.root);

	//if标志
	if (tokens[index].type_n == "IF") {
		index++;
		//左小括号
		if (tokens[index].type_n == "LL_BRACKET") {
			index++;
			//右小括号位置
			int tmp_index = index;
			while (tokens[tmp_index].type_n != "RL_BRACKET")tmp_index++;
			_expression(if_tree.root, tmp_index);
			//此处index自增1到),还需自增1到{
			index++;
		}
		else {
			std::cout << "error:lack of ( in if sentence!" << std::endl;
			exit(0);
		}
		//匹配{
		if (tokens[index].type_n == "LB_BRACKET") {
			_block(if_tree);
		}
		else {
			std::cout << "error:lack of { in if sentence!" << std::endl;
			exit(0);
		}
		//如果有else匹配else
		if (tokens[index].type_n == "ELSE") {
			index++;
			SyntaxTree else_tree;
			else_tree.current=else_tree.root= std::make_shared<SyntaxTreeNode>("ElseControl");
			if_else_tree.add_child_node(else_tree.root, if_else_tree.root);
			if (tokens[index].type_n == "LB_BRACKET")_block(else_tree);
			else {
				std::cout << "error:lack of { in else sentence!" << std::endl;
				exit(0);
			}
		}
	}
}
//Control->WhileControl|IfElseControl|ForControl
void Parser::_control(std::shared_ptr<SyntaxTreeNode> father ) {
	string token_type = tokens[index].type_n;
	//TODO-->修改_while()以支持do{}while()结构
	//if (token_type == "WHILE" || token_type == "DO")_while();
	if (token_type == "WHILE")_while(father);
	else if (token_type == "IF")_if_else(father);
	else if (token_type == "FOR")_for(father);
	else {
		std::cout << "error:control style is not supported!" << std::endl;
		exit(0);
	}

}
//in_index用于标记识别到哪个token为止，默认值为0代表不遇到下一个;不停止
//Expression->(Expression)|Expression Operation Expression|Operation Expression|Digit|Identifier[Expression]|Identifier
//Operation-> >|<|>=|<=|+|-|*|/|++|--|!=|==|！
void Parser::_expression(std::shared_ptr<SyntaxTreeNode> father , int in_index ) {
	if (father == NULL)father = tree.root;
	map<string, int> operator_priority = {
		{">", 0}, {"<", 0}, {">=", 0}, {"<=", 0},
		{"+", 1}, {"-", 1}, {"*", 2}, {"/", 2},
		{"++", 3}, {"--", 3}, {"!", 3},{"==", 0}, {"!=", 0}
	};


	stack<SyntaxTree> operator_stack;
	//逆波兰式表达式
	vector<SyntaxTree> reverse_polish_expression;

	while (index < tokens.size() && tokens[index].type_n != "SEMICOLON" ) {
		if (in_index>0 && index >= in_index)break;
		Token current_token = tokens[index];

		// 处理常量
		if (current_token.type_n == "DIGIT_CONSTANT") {
			SyntaxTree expr_tree;
			auto root = make_shared<SyntaxTreeNode>("Expression", "Constant");
			auto constant_node = make_shared<SyntaxTreeNode>(current_token.value, "_Constant");
			expr_tree.root = expr_tree.current = root;
			expr_tree.add_child_node(constant_node, root);
			reverse_polish_expression.push_back(expr_tree);
		}
		// 处理变量或数组元素或者函数调用
		else if (current_token.type_n == "IDENTIFIER") {
			bool is_variable = true;

			if (index + 1 < tokens.size() && tokens[index + 1].type_n == "LM_BRACKET") {
				// 数组元素处理
				is_variable = false;
				SyntaxTree array_tree;

				auto array_root = make_shared<SyntaxTreeNode>("Expression", "ArrayItem");
				//识别标识符，即数组名
				auto array_name_node = make_shared<SyntaxTreeNode>(current_token.value, "_ArrayName");
				array_tree.root = array_tree.current = array_root;
				array_tree.add_child_node(array_name_node, array_tree.root);
				index += 2; // 跳过标识符和'['，识别下标
				// 数组下标
				if (tokens[index].type_n == "DIGIT_CONSTANT" || tokens[index].type_n == "IDENTIFIER") {
					auto index_node = make_shared<SyntaxTreeNode>(tokens[index].value, "_ArrayIndex");
					array_tree.add_child_node(index_node, array_root);
				}
				else {
					cout<<" error: Array index must be a constant or identifier." << endl;
					exit(0);
				}
				reverse_polish_expression.push_back(array_tree);
			}
			//普通变量
			else if (is_variable&& tokens[index + 1].type_n != "LL_BRACKET") {
				// 变量处理
				SyntaxTree expr_tree;
				auto root = make_shared<SyntaxTreeNode>("Expression", "Variable");
				auto variable_node = make_shared<SyntaxTreeNode>(current_token.value, "_Variable");
				expr_tree.root = expr_tree.current = root;
				expr_tree.add_child_node(variable_node, root);
				reverse_polish_expression.push_back(expr_tree);
			}
			//函数调用值
			else if(is_variable && tokens[index + 1].type_n == "LL_BRACKET"){
				
			}
		}
		// 处理运算符或者()
		else if (find(operators.begin(), operators.end(), current_token.value) != operators.end() ||
			current_token.type_n == "LL_BRACKET" || current_token.type_n == "RL_BRACKET") {
			SyntaxTree op_tree;
			op_tree.root = op_tree.current = std::make_shared<SyntaxTreeNode>("Operator", "Operator");
			shared_ptr<SyntaxTreeNode> op_node = make_shared<SyntaxTreeNode>(current_token.value, "_Operator");
			op_tree.add_child_node(op_node,op_tree.root);
			if (current_token.type_n == "LL_BRACKET") {
				operator_stack.push(op_tree);
			}
			//遇到右括号，弹栈直到遇到左括号为止
			else if (current_token.type_n == "RL_BRACKET") {
				while (!operator_stack.empty() && operator_stack.top().current->node_value != "LL_BRACKET") {
					reverse_polish_expression.push_back(operator_stack.top());
					operator_stack.pop();
				}
				if (!operator_stack.empty()) operator_stack.pop(); // 弹出左括号
			}
			//其他只能是运算符
			else {
				while (!operator_stack.empty() &&
					operator_priority.count(operator_stack.top().current->node_value) &&
					operator_priority[op_node->node_value] <= operator_priority[operator_stack.top().current->node_value]) {
					reverse_polish_expression.push_back(operator_stack.top());
					operator_stack.pop();
				}
				operator_stack.push(op_tree);
			}
		}
		index++;
	}

	// 清空符号栈
	while (!operator_stack.empty()) {
		reverse_polish_expression.push_back(operator_stack.top());
		operator_stack.pop();
	}

	// 使用逆波兰式构造表达式树
	stack<SyntaxTree> operand_stack;

	for (auto& item : reverse_polish_expression) {
		if (item.root->node_type != "Operator") {
			operand_stack.push(item);
		}
		else {
			//处理单目运算符
			if (find(single_operators.begin(), single_operators.end(), item.current->node_value) != single_operators.end()) {
				//缺少操作数
				if (operand_stack.empty()) {
					cout << " error: Not enough operands for unary operator: " << item.current->node_value << endl;
					exit(0);
				}
				SyntaxTree a = operand_stack.top(); operand_stack.pop();
				SyntaxTree new_expr;
				new_expr.root = new_expr.current = make_shared<SyntaxTreeNode>("Expression", "SingleOperand");
				new_expr.add_child_node(item.current, new_expr.root);
				new_expr.add_child_node(a.root, new_expr.root);
				operand_stack.push(new_expr);
			}
			//处理双目运算符
			else if (find(double_operators.begin(), double_operators.end(), item.current->node_value) != double_operators.end()) {
				if (operand_stack.size() < 2) {
					cout << "error: Not enough operands for binary operator: " << item.current->node_value << endl;
					exit(0);
				}
				SyntaxTree b = operand_stack.top(); operand_stack.pop();
				SyntaxTree a = operand_stack.top(); operand_stack.pop();

				SyntaxTree new_expr;
				new_expr.root = new_expr.current = make_shared<SyntaxTreeNode>("Expression", "DoubleOperand");
				new_expr.add_child_node(a.root, new_expr.root);
				new_expr.add_child_node(item.current, new_expr.root);
				new_expr.add_child_node(b.root, new_expr.root);
				operand_stack.push(new_expr);
			}
			else {
				cout << "error: Unsupported operator: " << item.current->node_value << endl;
				exit(0);
			}
		}
	}
	//将处理后的表达式树挂载到父亲节点上
	if (!operand_stack.empty()) {
		tree.add_child_node(operand_stack.top().root, father);
	}
	//Expression识别结束
}
//FunctionCall->Identifier (CallParameterList);
//CallParameterList->Parameter,CallParameterList|Parameter|ɛ
//Parameter->Identifier|Num|"String"|&Identifier
void Parser::_function_call(std::shared_ptr<SyntaxTreeNode> father ) {
	if (father == NULL)father = tree.root;
	SyntaxTree func_call_tree;
	func_call_tree.current = func_call_tree.root = std::make_shared<SyntaxTreeNode>("FunctionCall");
	tree.add_child_node(func_call_tree.root, father);
	//进入该函数已识别Identifier和(
	func_call_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "FUNCTION_NAME"));
	//跳过(处理下一个，下标加2
	index += 2;
	std::shared_ptr<SyntaxTreeNode>params_list = std::make_shared<SyntaxTreeNode>("CallParameterList");
	func_call_tree.add_child_node(params_list, func_call_tree.root);
	int params_size = 0;
	while (tokens[index].type_n != "RL_BRACKET") {
		if (tokens[index].type_n == "IDENTIFIER" || tokens[index].type_n == "DIGIT_CONSTANT") {
			func_call_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, tokens[index].type_n), params_list);
			params_size++;
			index++;
		}
		else if (tokens[index].type_n == "DOUBLE_QUOTE") {
			//字符串常量识别跳过"
			index++;
			func_call_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, tokens[index].type_n), params_list);
			params_size++;
			//跳过后双引号"再自增1
			index+=2;
		}
		//引用传值
		else if (tokens[index].type_n == "ADDRESS") {
			//&后跟标识符
			if (tokens[index + 1].type_n == "IDENTIFIER") {
				func_call_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, "ADDRESS"), params_list);
				index++;
				func_call_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value, tokens[index].type_n), params_list);
				params_size++;
				index++;
			}
			//引用传值缺少标识符
			else {
				std::cout << "error:lack of Identifier after &!" << std::endl;
				exit(0);
			}
		}
		//匹配, 此时参数需要大于等于1否则报错
		else if (tokens[index].type_n == "COMMA" && params_size > 0)index++;
		//TODO-->这里可加指针变量识别
		else {
			std::cout << "error:illegal grammar in function_call parameterlist!" << std::endl;
			exit(0);
		}
		//识别下一个参数
	}
	//识别到),跳过自增1
	index++;
	if (tokens[index].type_n != "SEMICOLON") {
		std::cout << "error:lack of ; in function_call!" << std::endl;
		exit(0);
	}
	//函数调用语句识别结束
	index++;
}
//Return->return Expression;
void Parser::_return(std::shared_ptr<SyntaxTreeNode> father ) {
	if (father == NULL)father = tree.root;
	SyntaxTree return_tree;
	return_tree.current = return_tree.root = std::make_shared<SyntaxTreeNode>("Return");
	tree.add_child_node(return_tree.root, father);
	//已识别return
	return_tree.add_child_node(std::make_shared<SyntaxTreeNode>(tokens[index].value));
	index++;
	//识别Expression
	_expression(return_tree.root);
	//未识别到;报错
	if (tokens[index].type_n!= "SEMICOLON") {
		std::cout << "error:lack of ; in return" << std::endl;
		exit(0);
	}
	//Return识别结束
	index++;
}
string Parser::_judge_sentence_pattern() {
	string token_value = tokens[index].value;
	string token_type = tokens[index].type_n;
	//控制语句
	for (auto p : keywords[1]) {
		if (p == token_value)return"CONTROL";
	}
	//声明语句或者函数声明语句
	if (index + 2 < tokens.size()) {
		for (auto p : keywords[0]) {
			if (p == token_value && tokens[index + 1].type_n == "IDENTIFIER") {
				string index_2_token_type = tokens[index + 2].type_n;
				//函数声明 声明语句 或者报错
				if (index_2_token_type == "LL_BRACKET")return "FUNCTION_STATEMENT";
				else if (index_2_token_type == "SEMICOLON" || index_2_token_type == "LM_BRACKET" || index_2_token_type == "COMMA")return "STATEMENT";
				else return "ERROR";
			}
		}
	}
	//include语句
	if (index + 1 < tokens.size()&&token_type == "SHARP"  && tokens[index + 1].type_n == "INCLUDE") {
		return "INCLUDE";
	}
	//函数调用或者赋值语句，或者表达式语句
	else if (index + 1 < tokens.size()&&token_type == "IDENTIFIER" ) {
		string index_1_token_type = tokens[index + 1].type_n;
		if (index_1_token_type == "LL_BRACKET")return "FUNCTION_CALL";
		else if (index_1_token_type == "ASSIGN")return "ASSIGNMENT";
		else if (index_1_token_type=="SELF_PLUS"|| index_1_token_type == "SELF_MINUS")return "EXPRESSION";
		else return "ERROR";
	}
	//return语句
	else if (token_type == "RETURN")return "RETURN";
	//右大括号，表明函数结束
	else if (token_type == "RB_BRACKET") {
		index++;
		return "RB_BRACKET";
	}
	else return "ERROR";
}
//Program->Sentence1 Program|Sentence1|ɛ
//Sentence1->Include|FunctionCall|Statement|FunctionStatement
void Parser::main() {
	tree.current = tree.root = std::make_shared<SyntaxTreeNode>("Program");
	while (index < tokens.size()) {
		string sentence_pattern = _judge_sentence_pattern();
		if (sentence_pattern == "INCLUDE")_include();
		else if (sentence_pattern == "FUNCTION_STATEMENT")_function_statement();
		else if (sentence_pattern == "STATEMENT")_statement();
		else if (sentence_pattern == "FUNCTION_CALL")_function_call();
		else {
			std::cout << "main error:sentence_pattern recognition!" << std::endl;
			exit(0);
		}
	}
	std::cout << "通过语法分析" << std::endl;
}
//DFS遍历
void Parser::dfs(std::shared_ptr<SyntaxTreeNode> node) {
	if (node == NULL)return;
	std::cout << "(value:" << node->node_value << ",type:" << node->node_type << ")";
	if (node->right != NULL)std::cout << "right:" << node->right->node_value << std::endl;
	else std::cout << "NULL" << endl;
	std::shared_ptr<SyntaxTreeNode> child = node->first_son;
	while (child != NULL) {
		dfs(child);
		child = child->right;
	}
}
//输出语法树调试用
void Parser::display() {
	dfs(tree.root);
}
