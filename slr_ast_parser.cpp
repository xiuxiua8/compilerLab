#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <stack>
#include <queue>


using namespace std;

// 调试标志
bool DEBUG_MODE = false;
#define DEBUG_PRINT(x) if(DEBUG_MODE) { x; }

// JSON辅助函数
string jsonIndent(int level) {
    return string(level * 2, ' ');
}

string jsonEscape(const string& str) {
    string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

// ===== AST节点定义部分 =====
// 从semantic_analyzer.cpp复制的AST节点定义

// AST节点类型枚举
enum class NodeType {
    PROGRAM,
    FUNCTION_DEF,
    VARIABLE_DECL,
    ASSIGNMENT,
    IF_STMT,
    WHILE_STMT,
    RETURN_STMT,
    EXPRESSION_STMT,
    COMPOUND_STMT,
    BINARY_OP,
    UNARY_OP,
    IDENTIFIER,
    LITERAL,
    FUNCTION_CALL,
    ARRAY_ACCESS
};

// 数据类型枚举
enum class DataType {
    INT,
    FLOAT,
    VOID,
    ARRAY_INT,
    ARRAY_FLOAT,
    UNKNOWN
};

string nodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::PROGRAM: return "Program";
        case NodeType::FUNCTION_DEF: return "FunctionDef";
        case NodeType::VARIABLE_DECL: return "VariableDecl";
        case NodeType::ASSIGNMENT: return "Assignment";
        case NodeType::IF_STMT: return "IfStmt";
        case NodeType::WHILE_STMT: return "WhileStmt";
        case NodeType::RETURN_STMT: return "ReturnStmt";
        case NodeType::EXPRESSION_STMT: return "ExpressionStmt";
        case NodeType::COMPOUND_STMT: return "CompoundStmt";
        case NodeType::BINARY_OP: return "BinaryOp";
        case NodeType::UNARY_OP: return "UnaryOp";
        case NodeType::IDENTIFIER: return "Identifier";
        case NodeType::LITERAL: return "Literal";
        case NodeType::FUNCTION_CALL: return "FunctionCall";
        case NodeType::ARRAY_ACCESS: return "ArrayAccess";
        default: return "Unknown";
    }
}

string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::VOID: return "void";
        case DataType::ARRAY_INT: return "int[]";
        case DataType::ARRAY_FLOAT: return "float[]";
        case DataType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

// 前向声明
class ASTNode;
class ExpressionNode;
class StatementNode;

// AST节点基类
class ASTNode {
public:
    NodeType type;
    DataType dataType;
    int line;
    int column;
    
    ASTNode(NodeType t) : type(t), dataType(DataType::UNKNOWN), line(0), column(0) {}
    virtual ~ASTNode() = default;
    virtual string toJSON(int indent = 0) const = 0;
};

// 表达式节点基类
class ExpressionNode : public ASTNode {
public:
    ExpressionNode(NodeType t) : ASTNode(t) {}
    virtual string getValue() const { return ""; }
};

// 语句节点基类
class StatementNode : public ASTNode {
public:
    StatementNode(NodeType t) : ASTNode(t) {}
};

// 标识符节点
class IdentifierNode : public ExpressionNode {
public:
    string name;
    
    IdentifierNode(const string& n) : ExpressionNode(NodeType::IDENTIFIER), name(n) {}
    
    string getValue() const override {
        return name;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"name\": \"" << jsonEscape(name) << "\"\n";
        json << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 字面量节点
class LiteralNode : public ExpressionNode {
public:
    string value;
    
    LiteralNode(const string& v, DataType dt) : ExpressionNode(NodeType::LITERAL), value(v) {
        dataType = dt;
    }
    
    string getValue() const override {
        return value;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"value\": \"" << jsonEscape(value) << "\",\n";
        json << jsonIndent(indent + 1) << "\"dataType\": \"" << dataTypeToString(dataType) << "\"\n";
        json << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 二元操作节点
class BinaryOpNode : public ExpressionNode {
public:
    string op;
    shared_ptr<ExpressionNode> left;
    shared_ptr<ExpressionNode> right;
    
    BinaryOpNode(const string& operation, shared_ptr<ExpressionNode> l, shared_ptr<ExpressionNode> r)
        : ExpressionNode(NodeType::BINARY_OP), op(operation), left(l), right(r) {}
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"operator\": \"" << jsonEscape(op) << "\",\n";
        if (left) {
            json << jsonIndent(indent + 1) << "\"left\": \n";
            json << left->toJSON(indent + 1) << ",\n";
        }
        if (right) {
            json << jsonIndent(indent + 1) << "\"right\": \n";
            json << right->toJSON(indent + 1) << "\n";
        }
        json << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 其他AST节点类定义...（这里省略了其他节点类的定义，需要从semantic_analyzer.cpp复制）

// ===== SLR分析表结构 =====
struct SLRAction {
    char type;  // 's' for shift, 'r' for reduce, 'a' for accept
    int value;  // state number for shift, production number for reduce
};

struct SLRTable {
    map<int, map<string, SLRAction>> ACTION;
    map<int, map<string, int>> GOTO;
    vector<string> productions;  // 产生式列表
};

// ===== 词法分析器（简化版）=====
enum class TokenType {
    // 关键字
    INT, FLOAT, VOID, IF, ELSE, WHILE, RETURN,
    // 标识符和字面量
    ID, INT_NUM, FLOAT_NUM,
    // 运算符
    ADD, MUL, ASG, REL_OP,
    // 分隔符
    SEMI, COMMA, LPAR, RPAR, LBR, RBR, LBRACK, RBRACK,
    // 特殊
    EOF_TOKEN, UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    string lexeme;
    int line;
    int column;
};

class Lexer {
private:
    string input;
    size_t pos;
    int line;
    int column;
    
    map<string, TokenType> keywords = {
        {"int", TokenType::INT},
        {"float", TokenType::FLOAT},
        {"void", TokenType::VOID},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"return", TokenType::RETURN}
    };
    
public:
    Lexer(const string& input) : input(input), pos(0), line(1), column(1) {}
    
    Token getNextToken() {
        skipWhitespace();
        
        if (pos >= input.length()) {
            return {TokenType::EOF_TOKEN, "#", "", line, column};
        }
        
        // 标识符或关键字
        if (isalpha(input[pos]) || input[pos] == '_') {
            return readIdentifier();
        }
        
        // 数字
        if (isdigit(input[pos])) {
            return readNumber();
        }
        
        // 运算符和分隔符
        char c = input[pos];
        switch (c) {
            case '+': advance(); return {TokenType::ADD, "ADD", "+", line, column - 1};
            case '*': advance(); return {TokenType::MUL, "MUL", "*", line, column - 1};
            case '=': 
                advance();
                if (pos < input.length() && input[pos] == '=') {
                    advance();
                    return {TokenType::REL_OP, "REL_OP", "==", line, column - 2};
                }
                return {TokenType::ASG, "ASG", "=", line, column - 1};
            case '<':
            case '>':
                advance();
                if (pos < input.length() && input[pos] == '=') {
                    advance();
                    return {TokenType::REL_OP, "REL_OP", string(1, c) + "=", line, column - 2};
                }
                return {TokenType::REL_OP, "REL_OP", string(1, c), line, column - 1};
            case ';': advance(); return {TokenType::SEMI, "SEMI", ";", line, column - 1};
            case ',': advance(); return {TokenType::COMMA, "COMMA", ",", line, column - 1};
            case '(': advance(); return {TokenType::LPAR, "LPAR", "(", line, column - 1};
            case ')': advance(); return {TokenType::RPAR, "RPAR", ")", line, column - 1};
            case '{': advance(); return {TokenType::LBR, "LBR", "{", line, column - 1};
            case '}': advance(); return {TokenType::RBR, "RBR", "}", line, column - 1};
            case '[': advance(); return {TokenType::LBRACK, "LBRACK", "[", line, column - 1};
            case ']': advance(); return {TokenType::RBRACK, "RBRACK", "]", line, column - 1};
            default:
                advance();
                return {TokenType::UNKNOWN, "UNKNOWN", string(1, c), line, column - 1};
        }
    }
    
private:
    void advance() {
        if (pos < input.length() && input[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
    
    void skipWhitespace() {
        while (pos < input.length() && isspace(input[pos])) {
            advance();
        }
    }
    
    Token readIdentifier() {
        int startLine = line;
        int startColumn = column;
        string value;
        
        while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) {
            value += input[pos];
            advance();
        }
        
        auto it = keywords.find(value);
        if (it != keywords.end()) {
            string tokenName = value;
            transform(tokenName.begin(), tokenName.end(), tokenName.begin(), ::toupper);
            return {it->second, tokenName, value, startLine, startColumn};
        }
        
        return {TokenType::ID, "ID", value, startLine, startColumn};
    }
    
    Token readNumber() {
        int startLine = line;
        int startColumn = column;
        string value;
        bool isFloat = false;
        
        while (pos < input.length() && (isdigit(input[pos]) || input[pos] == '.')) {
            if (input[pos] == '.') {
                if (isFloat) break;  // 第二个小数点
                isFloat = true;
            }
            value += input[pos];
            advance();
        }
        
        if (isFloat) {
            return {TokenType::FLOAT_NUM, "FLOAT_NUM", value, startLine, startColumn};
        } else {
            return {TokenType::INT_NUM, "INT_NUM", value, startLine, startColumn};
        }
    }
};

// ===== SLR分析器 =====
class SLRParser {
private:
    SLRTable table;
    vector<int> stateStack;
    vector<shared_ptr<ASTNode>> nodeStack;
    
public:
    SLRParser() {
        loadSLRTable();
    }
    
    void loadSLRTable() {
        // 加载产生式规则
        table.productions = {
            "S' -> Prog",  // 0 - 增广文法
            "Prog -> DeclList",  // 1
            "DeclList -> DeclList Decl",  // 2
            "DeclList -> Decl",  // 3
            "Decl -> VarDecl",  // 4
            "Decl -> FunDecl",  // 5
            "VarDecl -> Type ID SEMI",  // 6
            "VarDecl -> Type ID LBRACK INT_NUM RBRACK SEMI",  // 7
            "VarDecl -> Type ID ASG Expr SEMI",  // 8
            "Type -> INT",  // 9
            "Type -> FLOAT",  // 10
            "Type -> VOID",  // 11
            "FunDecl -> Type ID LPAR ParamList RPAR CompStmt",  // 12
            "ParamList -> ParamList COMMA Param",  // 13
            "ParamList -> Param",  // 14
            "ParamList -> ε",  // 15
            "Param -> Type ID",  // 16
            "Param -> Type ID LBRACK RBRACK",  // 17
            "CompStmt -> LBR StmtList RBR",  // 18
            "StmtList -> StmtList Stmt",  // 19
            "StmtList -> ε",  // 20
            "Stmt -> VarDecl",  // 21
            "Stmt -> OtherStmt",  // 22
            "OtherStmt -> ExprStmt",  // 23
            "OtherStmt -> CompStmt",  // 24
            "OtherStmt -> IfStmt",  // 25
            "OtherStmt -> LoopStmt",  // 26
            "OtherStmt -> RetStmt",  // 27
            "ExprStmt -> Expr SEMI",  // 28
            "ExprStmt -> SEMI",  // 29
            "IfStmt -> IF LPAR Expr RPAR CompStmt",  // 30
            "IfStmt -> IF LPAR Expr RPAR CompStmt ELSE CompStmt",  // 31
            "LoopStmt -> WHILE LPAR Expr RPAR Stmt",  // 32
            "RetStmt -> RETURN Expr SEMI",  // 33
            "RetStmt -> RETURN SEMI",  // 34
            "Expr -> ID ASG Expr",  // 35
            "Expr -> ID LBRACK Expr RBRACK ASG Expr",  // 36
            "Expr -> ID LPAR ArgList RPAR",  // 37
            "Expr -> SimpExpr",  // 38
            "SimpExpr -> AddExpr REL_OP AddExpr",  // 39
            "SimpExpr -> AddExpr",  // 40
            "AddExpr -> AddExpr ADD Term",  // 41
            "AddExpr -> Term",  // 42
            "Term -> Term MUL Fact",  // 43
            "Term -> Fact",  // 44
            "Fact -> ID",  // 45
            "Fact -> ID LBRACK Expr RBRACK",  // 46
            "Fact -> INT_NUM",  // 47
            "Fact -> FLOAT_NUM",  // 48
            "Fact -> LPAR Expr RPAR",  // 49
            "ArgList -> ArgList COMMA Expr",  // 50
            "ArgList -> Expr",  // 51
            "ArgList -> ε"  // 52
        };
        
        // TODO: 从文件加载SLR分析表
        // 这里需要解析slr_output.txt文件并填充table.ACTION和table.GOTO
        // 暂时手动填充一些示例数据
    }
    
    shared_ptr<ASTNode> parse(const string& input) {
        Lexer lexer(input);
        vector<Token> tokens;
        
        // 词法分析
        Token token;
        do {
            token = lexer.getNextToken();
            tokens.push_back(token);
        } while (token.type != TokenType::EOF_TOKEN);
        
        // 语法分析
        stateStack.clear();
        nodeStack.clear();
        stateStack.push_back(0);  // 初始状态
        
        size_t tokenIndex = 0;
        
        while (tokenIndex < tokens.size()) {
            int state = stateStack.back();
            Token currentToken = tokens[tokenIndex];
            string symbol = currentToken.value;
            
            // 查找ACTION表
            if (table.ACTION[state].count(symbol) == 0) {
                cerr << "语法错误：状态 " << state << " 没有符号 " << symbol << " 的动作" << endl;
                return nullptr;
            }
            
            SLRAction action = table.ACTION[state][symbol];
            
            if (action.type == 's') {
                // 移进
                stateStack.push_back(action.value);
                nodeStack.push_back(createTerminalNode(currentToken));
                tokenIndex++;
            } else if (action.type == 'r') {
                // 归约
                int prodNum = action.value;
                shared_ptr<ASTNode> newNode = reduce(prodNum);
                if (!newNode) return nullptr;
                
                // 获取产生式左部
                string left = getProductionLeft(prodNum);
                
                // 查找GOTO表
                state = stateStack.back();
                if (table.GOTO[state].count(left) == 0) {
                    cerr << "语法错误：GOTO[" << state << ", " << left << "] 未定义" << endl;
                    return nullptr;
                }
                
                stateStack.push_back(table.GOTO[state][left]);
                nodeStack.push_back(newNode);
            } else if (action.type == 'a') {
                // 接受
                if (!nodeStack.empty()) {
                    return nodeStack.back();
                }
                return nullptr;
            }
        }
        
        return nullptr;
    }
    
private:
    shared_ptr<ASTNode> createTerminalNode(const Token& token) {
        switch (token.type) {
            case TokenType::ID:
                return make_shared<IdentifierNode>(token.lexeme);
            case TokenType::INT_NUM:
                return make_shared<LiteralNode>(token.lexeme, DataType::INT);
            case TokenType::FLOAT_NUM:
                return make_shared<LiteralNode>(token.lexeme, DataType::FLOAT);
            default:
                // 对于其他终结符，暂时返回nullptr或创建一个占位节点
                return nullptr;
        }
    }
    
    shared_ptr<ASTNode> reduce(int prodNum) {
        // 根据产生式编号执行归约操作
        // 这里需要根据具体的产生式创建相应的AST节点
        // TODO: 实现具体的归约逻辑
        
        return nullptr;
    }
    
    string getProductionLeft(int prodNum) {
        if (prodNum < 0 || prodNum >= table.productions.size()) {
            return "";
        }
        
        string prod = table.productions[prodNum];
        size_t pos = prod.find(" -> ");
        if (pos != string::npos) {
            return prod.substr(0, pos);
        }
        
        return "";
    }
};

// 将AST保存为JSON文件
void saveASTtoJSON(shared_ptr<ASTNode> ast, const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << ast->toJSON() << endl;
        file.close();
        cout << "AST已保存到文件: " << filename << endl;
    } else {
        cerr << "无法打开文件: " << filename << endl;
    }
}

int main(int argc, char* argv[]) {
    // 检查命令行参数
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--debug" || string(argv[i]) == "-d") {
            DEBUG_MODE = true;
        }
    }
    
    cout << "SLR语法分析器 - AST生成器" << endl;
    
    // 测试代码
    string testCode = R"(
        int main() {
            int x = 5;
            int y = x + 3;
            return 0;
        }
    )";
    
    SLRParser parser;
    shared_ptr<ASTNode> ast = parser.parse(testCode);
    
    if (ast) {
        cout << "语法分析成功！" << endl;
        cout << "\n=== AST JSON格式 ===" << endl;
        cout << ast->toJSON() << endl;
        
        saveASTtoJSON(ast, "parsed_ast.json");
    } else {
        cout << "语法分析失败！" << endl;
    }
    
    return 0;
} 