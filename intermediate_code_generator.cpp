#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <iomanip>
#include <functional>
#include "semantic_analyzer.cpp"

using namespace std;


extern bool DEBUG_MODE;  
#define DEBUG_PRINT(x) if(DEBUG_MODE) { x; }

// ==================== 四元式定义 ====================
struct Quadruple {
    string op;      // 操作符
    string arg1;    // 操作数1
    string arg2;    // 操作数2  
    string result;  // 结果
    
    Quadruple(const string& operation, const string& a1, const string& a2, const string& res)
        : op(operation), arg1(a1), arg2(a2), result(res) {}
    
    string toString() const {
        return "(" + op + ", " + arg1 + ", " + arg2 + ", " + result + ")";
    }
};

// ==================== 中间代码生成器 ====================
class IntermediateCodeGenerator {
private:
    vector<Quadruple> quadruples;
    int tempVarCounter;
    int labelCounter;
    map<string, string> symbolAddresses;  // 符号到地址的映射
    
public:
    IntermediateCodeGenerator() : tempVarCounter(0), labelCounter(0) {}
    
    // 生成新的临时变量
    string newTempVar() {
        return "t" + to_string(tempVarCounter++);
    }
    
    // 生成新的标签
    string newLabel() {
        return "L" + to_string(labelCounter++);
    }
    
    // 添加四元式
    void emit(const string& op, const string& arg1, const string& arg2, const string& result) {
        quadruples.emplace_back(op, arg1, arg2, result);
    }
    
    // 生成算术表达式的中间代码
    string generateArithmeticExpr(const string& op, const string& left, const string& right) {
        string temp = newTempVar();
        emit(op, left, right, temp);
        return temp;
    }
    
    // 生成赋值语句的中间代码
    void generateAssignment(const string& target, const string& source) {
        emit("=", source, "", target);
    }
    
    // 生成关系表达式的中间代码
    string generateRelationalExpr(const string& op, const string& left, const string& right) {
        string temp = newTempVar();
        emit(op, left, right, temp);
        return temp;
    }
    
    // 生成条件跳转的中间代码
    void generateConditionalJump(const string& condition, const string& trueLabel, const string& falseLabel) {
        if (!trueLabel.empty()) {
            emit("jnz", condition, "", trueLabel);
        }
        if (!falseLabel.empty()) {
            emit("jz", condition, "", falseLabel);
        }
    }
    
    // 生成无条件跳转的中间代码
    void generateJump(const string& label) {
        emit("jmp", "", "", label);
    }
    
    // 生成标签
    void generateLabel(const string& label) {
        emit("label", "", "", label);
    }
    
    // 生成函数调用的中间代码
    string generateFunctionCall(const string& funcName, const vector<string>& args) {
        // 传递参数
        for (int i = args.size() - 1; i >= 0; i--) {
            emit("param", args[i], "", "");
        }
        
        // 调用函数
        string temp = newTempVar();
        emit("call", funcName, to_string(args.size()), temp);
        return temp;
    }
    
    // 生成return语句的中间代码
    void generateReturn(const string& value = "") {
        emit("return", value, "", "");
    }
    
    // 生成if语句的中间代码
    void generateIfStatement(const string& condition, 
                           function<void()> thenCode, 
                           function<void()> elseCode = nullptr) {
        string falseLabel = newLabel();
        string endLabel = newLabel();
        
        // 条件为假时跳转
        generateConditionalJump(condition, "", falseLabel);
        
        // then部分
        if (thenCode) thenCode();
        
        if (elseCode) {
            generateJump(endLabel);
            generateLabel(falseLabel);
            elseCode();
            generateLabel(endLabel);
        } else {
            generateLabel(falseLabel);
        }
    }
    
    // 生成while循环的中间代码
    void generateWhileLoop(const string& condition, function<void()> bodyCode) {
        string startLabel = newLabel();
        string endLabel = newLabel();
        
        generateLabel(startLabel);
        generateConditionalJump(condition, "", endLabel);
        
        if (bodyCode) bodyCode();
        
        generateJump(startLabel);
        generateLabel(endLabel);
    }
    
    // 打印四元式
    void printQuadruples() const {
        cout << "\n=== 四元式中间代码 ===" << endl;
        cout << setw(4) << "序号" << setw(8) << "操作符" << setw(8) << "操作数1" 
             << setw(8) << "操作数2" << setw(8) << "结果" << endl;
        cout << string(40, '-') << endl;
        
        for (size_t i = 0; i < quadruples.size(); i++) {
            const auto& quad = quadruples[i];
            cout << setw(4) << i << setw(8) << quad.op << setw(8) << quad.arg1 
                 << setw(8) << quad.arg2 << setw(8) << quad.result << endl;
        }
    }
    
    // 获取四元式列表
    const vector<Quadruple>& getQuadruples() const {
        return quadruples;
    }
    
    // 清空四元式
    void clear() {
        quadruples.clear();
        tempVarCounter = 0;
        labelCounter = 0;
    }
};

// ==================== 测试函数 ====================
void testIntermediateCodeGeneration() {
    cout << "=== 测试中间代码生成 ===" << endl;
    
    IntermediateCodeGenerator generator;
    
    // 测试1: 算术表达式 x = a + b * c
    cout << "\n测试1: x = a + b * c" << endl;
    string t1 = generator.generateArithmeticExpr("*", "b", "c");
    string t2 = generator.generateArithmeticExpr("+", "a", t1);
    generator.generateAssignment("x", t2);
    
    // 测试2: if语句 if (x > 0) y = 1; else y = 0;
    cout << "\n测试2: if (x > 0) y = 1; else y = 0;" << endl;
    string cond = generator.generateRelationalExpr(">", "x", "0");
    generator.generateIfStatement(cond,
        [&]() { generator.generateAssignment("y", "1"); },
        [&]() { generator.generateAssignment("y", "0"); }
    );
    
    // 测试3: while循环 while (i < 10) i = i + 1;
    cout << "\n测试3: while (i < 10) i = i + 1;" << endl;
    generator.generateWhileLoop(
        generator.generateRelationalExpr("<", "i", "10"),
        [&]() {
            string temp = generator.generateArithmeticExpr("+", "i", "1");
            generator.generateAssignment("i", temp);
        }
    );
    
    // 测试4: 函数调用 result = add(x, y)
    cout << "\n测试4: result = add(x, y)" << endl;
    string funcResult = generator.generateFunctionCall("add", {"x", "y"});
    generator.generateAssignment("result", funcResult);
    
    // 打印生成的四元式
    generator.printQuadruples();
}

class ASTCodeGenerator {
private:
    IntermediateCodeGenerator& generator;
    
public:
    ASTCodeGenerator(IntermediateCodeGenerator& gen) : generator(gen) {}
    
    // 为AST节点生成中间代码
    string generateCode(shared_ptr<ASTNode> node) {
        if (!node) return "";
        
        switch (node->type) {
            case NodeType::LITERAL: {
                auto literal = static_pointer_cast<LiteralNode>(node);
                return literal->value;
            }
            
            case NodeType::IDENTIFIER: {
                auto identifier = static_pointer_cast<IdentifierNode>(node);
                return identifier->name;
            }
            
            case NodeType::BINARY_OP: {
                auto binaryOp = static_pointer_cast<BinaryOpNode>(node);
                string left = generateCode(binaryOp->left);
                string right = generateCode(binaryOp->right);
                return generator.generateArithmeticExpr(binaryOp->op, left, right);
            }
            
            case NodeType::ASSIGNMENT: {
                auto assignment = static_pointer_cast<AssignmentNode>(node);
                string target = generateCode(assignment->target);
                string source = generateCode(assignment->value);
                generator.generateAssignment(target, source);
                return target;
            }
            
            case NodeType::IF_STMT: {
                auto ifStmt = static_pointer_cast<IfStmtNode>(node);
                string condition = generateCode(ifStmt->condition);
                
                generator.generateIfStatement(condition,
                    [&]() { generateCode(ifStmt->thenStmt); },
                    ifStmt->elseStmt ? function<void()>([&]() { generateCode(ifStmt->elseStmt); }) : function<void()>()
                );
                return "";
            }
            
            case NodeType::WHILE_STMT: {
                auto whileStmt = static_pointer_cast<WhileStmtNode>(node);
                string condition = generateCode(whileStmt->condition);
                
                generator.generateWhileLoop(condition,
                    [&]() { generateCode(whileStmt->body); }
                );
                return "";
            }
            
            case NodeType::FUNCTION_CALL: {
                auto funcCall = static_pointer_cast<FunctionCallNode>(node);
                vector<string> args;
                for (auto& arg : funcCall->arguments) {
                    args.push_back(generateCode(arg));
                }
                return generator.generateFunctionCall(funcCall->functionName, args);
            }
            
            case NodeType::FUNCTION_DEF: {
                auto funcDef = static_pointer_cast<FunctionDefNode>(node);
                
                // 生成函数标签
                generator.generateLabel(funcDef->name);
                
                // 处理函数体
                if (funcDef->body) {
                    generateCode(funcDef->body);
                }
                
                // 如果函数没有显式return，添加默认return
                if (funcDef->returnType == DataType::VOID) {
                    generator.generateReturn();
                } else {
                    // 对于非void函数，如果没有return语句，生成默认返回值
                    generator.generateReturn("0");
                }
                
                return "";
            }
            
            case NodeType::VARIABLE_DECL: {
                auto varDecl = static_pointer_cast<VariableDeclNode>(node);
                
                // 如果有初始化表达式，生成赋值代码
                if (varDecl->initializer) {
                    string initValue = generateCode(varDecl->initializer);
                    generator.generateAssignment(varDecl->name, initValue);
                }
                
                return "";
            }
            
            case NodeType::COMPOUND_STMT: {
                auto compound = static_pointer_cast<CompoundStmtNode>(node);
                
                // 依次处理复合语句中的每个语句
                for (auto& stmt : compound->statements) {
                    generateCode(stmt);
                }
                
                return "";
            }
            
            case NodeType::RETURN_STMT: {
                // 注意：这里假设ReturnStmtNode存在，如果不存在需要添加
                // 暂时处理为简单的return
                generator.generateReturn();
                return "";
            }
            
            case NodeType::EXPRESSION_STMT: {
                // 表达式语句，直接生成表达式的代码
                // 这里需要根据实际的ExpressionStmtNode结构来实现
                return "";
            }
            
            case NodeType::PROGRAM: {
                auto program = static_pointer_cast<ProgramNode>(node);
                
                // 处理全局变量声明
                for (auto& globalVar : program->globalVariables) {
                    generateCode(globalVar);
                }
                
                // 处理函数定义
                for (auto& func : program->functions) {
                    generateCode(func);
                }
                
                return "";
            }
            
            default:
                DEBUG_PRINT(cout << "警告：未处理的AST节点类型: " << nodeTypeToString(node->type) << endl);
                break;
        }
        
        return "";
    }
    
    // 生成完整程序的中间代码
    void generateProgramCode(shared_ptr<ASTNode> ast) {
        if (!ast) {
            cout << "错误：AST为空" << endl;
            return;
        }
        
        cout << "\n=== 开始生成中间代码 ===" << endl;
        
        // 清空之前的代码
        generator.clear();
        
        // 生成代码
        generateCode(ast);
        
        cout << "=== 中间代码生成完成 ===" << endl;
        
        // 打印生成的四元式
        generator.printQuadruples();
    }
};


int main(int argc, char* argv[]) {
    // 处理命令行参数
    bool testMode = false;
    string inputFile = "";
    
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--debug" || string(argv[i]) == "-d") {
            DEBUG_MODE = true;
        } else if (string(argv[i]) == "--test" || string(argv[i]) == "-t") {
            testMode = true;
        } else {
            inputFile = argv[i];
        }
    }
    
    if (testMode) {
        // 运行测试
        cout << "=== 运行中间代码生成测试 ===" << endl;
        testIntermediateCodeGeneration();
        return 0;
    }
    
    if (inputFile.empty()) {
        cout << "用法: " << argv[0] << " [选项] <输入文件>" << endl;
        cout << "选项:" << endl;
        cout << "  --debug, -d    启用调试模式" << endl;
        cout << "  --test, -t     运行测试模式" << endl;
        return 1;
    }
    
    try {
        // 创建语法分析器并解析文件
        SLRParser parser;
        parser.loadSLRTable();
        
        cout << "=== 开始语法分析 ===" << endl;
        shared_ptr<ASTNode> ast = parser.parse(inputFile);
        
        if (!ast) {
            cout << "错误：语法分析失败，无法生成AST" << endl;
            return 1;
        }
        
        cout << "=== 语法分析完成，AST生成成功 ===" << endl;
        
        // 打印AST（可选）
        if (DEBUG_MODE) {
            cout << "\n=== AST结构 ===" << endl;
            ast->printTree();
        }
        
        // 创建中间代码生成器
        IntermediateCodeGenerator generator;
        ASTCodeGenerator astGenerator(generator);
        
        // 生成中间代码
        astGenerator.generateProgramCode(ast);
        
        cout << "\n=== 程序执行完成 ===" << endl;
        
    } catch (const exception& e) {
        cout << "错误：" << e.what() << endl;
        return 1;
    }
    
    return 0;
} 