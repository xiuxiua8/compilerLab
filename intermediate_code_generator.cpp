#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <iomanip>
#include <functional>

using namespace std;

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

// ==================== 基于AST的代码生成 ====================
// 这里需要与之前的AST节点结合

// 简化的AST节点类型（用于演示）
enum class ASTNodeType {
    BINARY_OP, IDENTIFIER, LITERAL, ASSIGNMENT, IF_STMT, WHILE_STMT, FUNCTION_CALL
};

class SimpleASTNode {
public:
    ASTNodeType type;
    string value;
    vector<shared_ptr<SimpleASTNode>> children;
    
    SimpleASTNode(ASTNodeType t, const string& v = "") : type(t), value(v) {}
    
    void addChild(shared_ptr<SimpleASTNode> child) {
        children.push_back(child);
    }
};

class ASTCodeGenerator {
private:
    IntermediateCodeGenerator& generator;
    
public:
    ASTCodeGenerator(IntermediateCodeGenerator& gen) : generator(gen) {}
    
    // 为AST节点生成中间代码
    string generateCode(shared_ptr<SimpleASTNode> node) {
        if (!node) return "";
        
        switch (node->type) {
            case ASTNodeType::LITERAL:
            case ASTNodeType::IDENTIFIER:
                return node->value;
                
            case ASTNodeType::BINARY_OP: {
                if (node->children.size() >= 2) {
                    string left = generateCode(node->children[0]);
                    string right = generateCode(node->children[1]);
                    return generator.generateArithmeticExpr(node->value, left, right);
                }
                break;
            }
            
            case ASTNodeType::ASSIGNMENT: {
                if (node->children.size() >= 2) {
                    string target = generateCode(node->children[0]);
                    string source = generateCode(node->children[1]);
                    generator.generateAssignment(target, source);
                }
                break;
            }
            
            case ASTNodeType::IF_STMT: {
                if (node->children.size() >= 2) {
                    string condition = generateCode(node->children[0]);
                    auto thenStmt = node->children[1];
                    auto elseStmt = (node->children.size() > 2) ? node->children[2] : nullptr;
                    
                    generator.generateIfStatement(condition,
                        [&]() { generateCode(thenStmt); },
                        elseStmt ? function<void()>([&]() { generateCode(elseStmt); }) : function<void()>()
                    );
                }
                break;
            }
            
            case ASTNodeType::FUNCTION_CALL: {
                vector<string> args;
                for (size_t i = 1; i < node->children.size(); i++) {
                    args.push_back(generateCode(node->children[i]));
                }
                return generator.generateFunctionCall(node->value, args);
            }
            
            default:
                break;
        }
        
        return "";
    }
};

// 测试AST代码生成
void testASTCodeGeneration() {
    cout << "\n\n=== 测试基于AST的代码生成 ===" << endl;
    
    IntermediateCodeGenerator generator;
    ASTCodeGenerator astGen(generator);
    
    // 构建AST: x = a + b * 2
    auto assignment = make_shared<SimpleASTNode>(ASTNodeType::ASSIGNMENT);
    auto x = make_shared<SimpleASTNode>(ASTNodeType::IDENTIFIER, "x");
    auto plus = make_shared<SimpleASTNode>(ASTNodeType::BINARY_OP, "+");
    auto a = make_shared<SimpleASTNode>(ASTNodeType::IDENTIFIER, "a");
    auto mult = make_shared<SimpleASTNode>(ASTNodeType::BINARY_OP, "*");
    auto b = make_shared<SimpleASTNode>(ASTNodeType::IDENTIFIER, "b");
    auto two = make_shared<SimpleASTNode>(ASTNodeType::LITERAL, "2");
    
    mult->addChild(b);
    mult->addChild(two);
    plus->addChild(a);
    plus->addChild(mult);
    assignment->addChild(x);
    assignment->addChild(plus);
    
    // 生成代码
    astGen.generateCode(assignment);
    
    generator.printQuadruples();
}

int main() {
    // 测试基本的中间代码生成
    testIntermediateCodeGeneration();
    
    // 测试基于AST的代码生成
    testASTCodeGeneration();
    
    return 0;
} 