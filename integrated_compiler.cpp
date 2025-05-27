#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <sstream>
#include <memory>
#include <iomanip>
#include <cstdlib>
#include <functional>
#include <regex>

using namespace std;

// ==================== Token定义 ====================
struct Token {
    string type;
    string value;
    int line;
    int column;
    
    Token(const string& t, const string& v, int l = 0, int c = 0) 
        : type(t), value(v), line(l), column(c) {}
};

// ==================== AST节点定义（从semantic_analyzer.cpp简化） ====================
enum class NodeType {
    PROGRAM, FUNCTION_DEF, VARIABLE_DECL, ASSIGNMENT, IF_STMT, WHILE_STMT,
    RETURN_STMT, EXPRESSION_STMT, COMPOUND_STMT, BINARY_OP, UNARY_OP,
    IDENTIFIER, LITERAL, FUNCTION_CALL, ARRAY_ACCESS
};

enum class DataType {
    INT, FLOAT, VOID, ARRAY_INT, ARRAY_FLOAT, UNKNOWN
};

class ASTNode {
public:
    NodeType type;
    DataType dataType;
    int line;
    int column;
    
    ASTNode(NodeType t) : type(t), dataType(DataType::UNKNOWN), line(0), column(0) {}
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

// AST节点类定义
class ExpressionNode : public ASTNode {
public:
    ExpressionNode(NodeType t) : ASTNode(t) {}
};

class StatementNode : public ASTNode {
public:
    StatementNode(NodeType t) : ASTNode(t) {}
};

// 标识符节点
class IdentifierNode : public ExpressionNode {
public:
    string name;
    
    IdentifierNode(const string& n) : ExpressionNode(NodeType::IDENTIFIER), name(n) {}
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Identifier: " << name << endl;
    }
};

// 字面量节点
class LiteralNode : public ExpressionNode {
public:
    string value;
    
    LiteralNode(const string& v, DataType dt) : ExpressionNode(NodeType::LITERAL), value(v) {
        dataType = dt;
    }
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Literal: " << value << " (";
        switch(dataType) {
            case DataType::INT: cout << "int"; break;
            case DataType::FLOAT: cout << "float"; break;
            default: cout << "unknown"; break;
        }
        cout << ")" << endl;
    }
};

// 程序节点
class ProgramNode : public ASTNode {
public:
    vector<shared_ptr<ASTNode>> declarations;
    
    ProgramNode() : ASTNode(NodeType::PROGRAM) {}
    
    void addDeclaration(shared_ptr<ASTNode> decl) {
        declarations.push_back(decl);
    }
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Program:" << endl;
        for (const auto& decl : declarations) {
            if (decl) decl->print(indent + 2);
        }
    }
};

// ==================== SLR(1)分析表定义 ====================
struct SLRTable {
    // ACTION[state][terminal] = "sX"(移进), "rY"(归约), "acc"(接受), ""(空)
    map<int, map<string, string>> ACTION;
    // GOTO[state][nonterminal] = 状态编号
    map<int, map<string, int>> GOTO;
};

// ==================== 语义动作定义 ====================
struct Production {
    string left;
    vector<string> right;
    // 语义动作函数指针
    function<shared_ptr<ASTNode>(vector<shared_ptr<ASTNode>>&)> semanticAction;
};

// ==================== 编译器主类 ====================
class IntegratedCompiler {
private:
    vector<Token> tokens;
    SLRTable slrTable;
    vector<Production> productions;
    stack<int> stateStack;
    stack<shared_ptr<ASTNode>> semanticStack;
    shared_ptr<ASTNode> astRoot;
    
public:
    IntegratedCompiler() {
        initializeSLRTable();
        initializeProductions();
    }
    
    // 初始化SLR(1)分析表（从实验四的输出）
    void initializeSLRTable() {
        // 这里需要从实验四的输出文件读取或硬编码SLR表
        // 示例：ACTION表
        slrTable.ACTION[0]["INT"] = "s7";
        slrTable.ACTION[0]["FLOAT"] = "s8";
        slrTable.ACTION[0]["VOID"] = "s9";
        // ... 其他表项
        
        // 示例：GOTO表
        slrTable.GOTO[0]["Prog"] = 1;
        slrTable.GOTO[0]["Decl"] = 2;
        slrTable.GOTO[0]["FunDecl"] = 3;
        // ... 其他表项
        
        // TODO: 完整的表需要从实验四的输出导入
    }
    
    // 初始化产生式和语义动作
    void initializeProductions() {
        // 产生式0: Prog' -> Prog
        productions.push_back({
            "Prog'", {"Prog"},
            [](vector<shared_ptr<ASTNode>>& attrs) {
                return attrs[0]; // 直接返回程序节点
            }
        });
        
        // 产生式1: Prog -> DeclList
        productions.push_back({
            "Prog", {"DeclList"},
            [](vector<shared_ptr<ASTNode>>& attrs) {
                // 创建程序节点
                auto prog = make_shared<ProgramNode>();
                // TODO: 处理声明列表
                return prog;
            }
        });
        
        // 产生式2: DeclList -> Decl DeclList
        productions.push_back({
            "DeclList", {"Decl", "DeclList"},
            [](vector<shared_ptr<ASTNode>>& attrs) {
                // 将Decl添加到DeclList
                // TODO: 实现
                return attrs[1];
            }
        });
        
        // TODO: 添加其他产生式和语义动作
    }
    
    // 从词法分析器读取Token流
    bool readTokensFromLexer(const string& filename) {
        // 调用实验二的词法分析器
        string command = "../lab1/dfa " + filename;
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            cerr << "无法调用词法分析器" << endl;
            return false;
        }
        
        char buffer[256];
        string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        
        // 解析词法分析器的输出
        parseTokenOutput(result);
        
        // 添加结束符
        tokens.push_back(Token("#", "#"));
        
        return true;
    }
    
    // 解析词法分析器输出
    void parseTokenOutput(const string& output) {
        istringstream iss(output);
        string line;
        int lineNum = 1;
        
        while (getline(iss, line)) {
            // 解析格式：(TYPE, value)
            size_t pos = 0;
            while ((pos = line.find("(", pos)) != string::npos) {
                size_t comma = line.find(",", pos);
                size_t end = line.find(")", comma);
                if (comma != string::npos && end != string::npos) {
                    string type = line.substr(pos + 1, comma - pos - 1);
                    string value = line.substr(comma + 2, end - comma - 2);
                    tokens.push_back(Token(type, value, lineNum));
                }
                pos = end + 1;
            }
            lineNum++;
        }
    }
    
    // SLR(1)语法分析主函数
    bool parse() {
        stateStack.push(0);
        semanticStack.push(nullptr); // 初始语义值
        
        size_t index = 0;
        
        while (index < tokens.size()) {
            Token& token = tokens[index];
            int state = stateStack.top();
            
            // 查找ACTION表
            string action = slrTable.ACTION[state][token.type];
            
            if (action.empty()) {
                cerr << "语法错误：状态 " << state << " 遇到 " << token.type << endl;
                return false;
            }
            
            if (action[0] == 's') { // 移进
                int nextState = stoi(action.substr(1));
                stateStack.push(nextState);
                
                // 创建叶子节点（终结符）
                shared_ptr<ASTNode> node = createTerminalNode(token);
                semanticStack.push(node);
                
                index++;
            }
            else if (action[0] == 'r') { // 归约
                int prodNum = stoi(action.substr(1));
                Production& prod = productions[prodNum];
                
                // 弹出产生式右部长度的状态和语义值
                vector<shared_ptr<ASTNode>> childNodes;
                for (size_t i = 0; i < prod.right.size(); i++) {
                    stateStack.pop();
                    childNodes.insert(childNodes.begin(), semanticStack.top());
                    semanticStack.pop();
                }
                
                // 执行语义动作
                shared_ptr<ASTNode> newNode = prod.semanticAction(childNodes);
                semanticStack.push(newNode);
                
                // GOTO
                state = stateStack.top();
                int gotoState = slrTable.GOTO[state][prod.left];
                stateStack.push(gotoState);
            }
            else if (action == "acc") { // 接受
                astRoot = semanticStack.top();
                cout << "语法分析成功！" << endl;
                return true;
            }
        }
        
        return false;
    }
    
    // 创建终结符节点
    shared_ptr<ASTNode> createTerminalNode(const Token& token) {
        if (token.type == "ID") {
            return make_shared<IdentifierNode>(token.value);
        }
        else if (token.type == "NUM" || token.type == "FLO") {
            DataType dt = (token.type == "NUM") ? DataType::INT : DataType::FLOAT;
            return make_shared<LiteralNode>(token.value, dt);
        }
        // TODO: 处理其他终结符
        return nullptr;
    }
    
    // 打印AST
    void printAST() {
        if (astRoot) {
            cout << "\n=== 抽象语法树 ===" << endl;
            astRoot->print();
        }
    }
    
    // 进行语义分析
    void performSemanticAnalysis() {
        if (astRoot) {
            cout << "\n=== 语义分析 ===" << endl;
            // TODO: 调用语义分析器
        }
    }
};

// ==================== 主函数 ====================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <源文件>" << endl;
        return 1;
    }
    
    IntegratedCompiler compiler;
    
    // 1. 词法分析
    cout << "=== 词法分析 ===" << endl;
    if (!compiler.readTokensFromLexer(argv[1])) {
        return 1;
    }
    
    // 2. 语法分析 + AST构建
    cout << "\n=== 语法分析 ===" << endl;
    if (!compiler.parse()) {
        return 1;
    }
    
    // 3. 打印AST
    compiler.printAST();
    
    // 4. 语义分析
    compiler.performSemanticAnalysis();
    
    return 0;
}