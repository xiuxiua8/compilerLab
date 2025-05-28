#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>  // 添加文件操作支持
#include "lab3/lr0.cpp"
#include "lexer.cpp"

using namespace std;

extern bool DEBUG_MODE;  
#define DEBUG_PRINT(x) if(DEBUG_MODE) { x; }


// 树状图打印辅助函数
void printTreeHelper(const string& prefix, const string& content, bool isLast) {
    cout << prefix;
    cout << (isLast ? "└── " : "├── ");
    cout << content << endl;
}

string getChildPrefix(const string& prefix, bool isLast) {
    return prefix + (isLast ? "    " : "│   ");
}

// 前向声明
class ASTNode;
class SymbolTable;

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

// AST节点基类
class ASTNode {
public:
    NodeType type;
    DataType dataType;
    int line;
    int column;
    
    ASTNode(NodeType t) : type(t), dataType(DataType::UNKNOWN), line(0), column(0) {}
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
    virtual void printTree(const string& prefix = "", bool isLast = true) const = 0;
    virtual string toString() const = 0;
    virtual string toJSON(int indent = 0) const = 0;  // 新增JSON输出功能
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
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Identifier: " << name << endl;
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "Identifier: " + name, isLast);
    }
    
    string toString() const override {
        return name;
    }
    
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
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Literal: " << value << " (type: ";
        switch(dataType) {
            case DataType::INT: cout << "int"; break;
            case DataType::FLOAT: cout << "float"; break;
            default: cout << "unknown"; break;
        }
        cout << ")" << endl;
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        string typeStr;
        switch(dataType) {
            case DataType::INT: typeStr = "int"; break;
            case DataType::FLOAT: typeStr = "float"; break;
            default: typeStr = "unknown"; break;
        }
        printTreeHelper(prefix, "Literal: " + value + " (" + typeStr + ")", isLast);
    }
    
    string toString() const override {
        return value;
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
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "BinaryOp: " << op << endl;
        if (left) {
            cout << string(indent + 2, ' ') << "Left:" << endl;
            left->print(indent + 4);
        }
        if (right) {
            cout << string(indent + 2, ' ') << "Right:" << endl;
            right->print(indent + 4);
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "BinaryOp: " + op, isLast);
        string childPrefix = getChildPrefix(prefix, isLast);
        
        if (left && right) {
            left->printTree(childPrefix, false);
            right->printTree(childPrefix, true);
        } else if (left) {
            left->printTree(childPrefix, true);
        } else if (right) {
            right->printTree(childPrefix, true);
        }
    }
    
    string toString() const override {
        return "(" + (left ? left->toString() : "") + " " + op + " " + (right ? right->toString() : "") + ")";
    }
    
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

// 变量声明节点
class VariableDeclNode : public StatementNode {
public:
    DataType varType;
    string name;
    shared_ptr<ExpressionNode> initializer;
    bool isArray;
    int arraySize;
    
    VariableDeclNode(DataType dt, const string& n, shared_ptr<ExpressionNode> init = nullptr)
        : StatementNode(NodeType::VARIABLE_DECL), varType(dt), name(n), initializer(init), isArray(false), arraySize(0) {}
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "VariableDecl: ";
        switch(varType) {
            case DataType::INT: cout << "int "; break;
            case DataType::FLOAT: cout << "float "; break;
            case DataType::ARRAY_INT: cout << "int[] "; break;
            case DataType::ARRAY_FLOAT: cout << "float[] "; break;
            default: cout << "unknown "; break;
        }
        cout << name;
        if (isArray) cout << "[" << arraySize << "]";
        cout << endl;
        
        if (initializer) {
            cout << string(indent + 2, ' ') << "Initializer:" << endl;
            initializer->print(indent + 4);
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        string typeStr;
        switch(varType) {
            case DataType::INT: typeStr = "int"; break;
            case DataType::FLOAT: typeStr = "float"; break;
            case DataType::ARRAY_INT: typeStr = "int[]"; break;
            case DataType::ARRAY_FLOAT: typeStr = "float[]"; break;
            default: typeStr = "unknown"; break;
        }
        printTreeHelper(prefix, "VariableDecl: " + typeStr + " " + name, isLast);
        
        if (initializer) {
            string childPrefix = getChildPrefix(prefix, isLast);
            initializer->printTree(childPrefix, true);
        }
    }
    
    string toString() const override {
        string result = name;
        if (isArray) result += "[" + to_string(arraySize) + "]";
        if (initializer) result += " = " + initializer->toString();
        return result;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"varType\": \"" << dataTypeToString(varType) << "\",\n";
        json << jsonIndent(indent + 1) << "\"name\": \"" << jsonEscape(name) << "\",\n";
        json << jsonIndent(indent + 1) << "\"isArray\": " << (isArray ? "true" : "false") << ",\n";
        json << jsonIndent(indent + 1) << "\"arraySize\": " << arraySize;
        if (initializer) {
            json << ",\n" << jsonIndent(indent + 1) << "\"initializer\": \n";
            json << initializer->toJSON(indent + 1);
        }
        json << "\n" << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 赋值节点
class AssignmentNode : public StatementNode {
public:
    shared_ptr<ExpressionNode> target;
    shared_ptr<ExpressionNode> value;
    
    AssignmentNode(shared_ptr<ExpressionNode> t, shared_ptr<ExpressionNode> v)
        : StatementNode(NodeType::ASSIGNMENT), target(t), value(v) {}
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Assignment:" << endl;
        if (target) {
            cout << string(indent + 2, ' ') << "Target:" << endl;
            target->print(indent + 4);
        }
        if (value) {
            cout << string(indent + 2, ' ') << "Value:" << endl;
            value->print(indent + 4);
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "Assignment", isLast);
        string childPrefix = getChildPrefix(prefix, isLast);
        
        if (target && value) {
            target->printTree(childPrefix, false);
            value->printTree(childPrefix, true);
        } else if (target) {
            target->printTree(childPrefix, true);
        } else if (value) {
            value->printTree(childPrefix, true);
        }
    }
    
    string toString() const override {
        return (target ? target->toString() : "") + " = " + (value ? value->toString() : "");
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\"";
        if (target) {
            json << ",\n" << jsonIndent(indent + 1) << "\"target\": \n";
            json << target->toJSON(indent + 1);
        }
        if (value) {
            json << ",\n" << jsonIndent(indent + 1) << "\"value\": \n";
            json << value->toJSON(indent + 1);
        }
        json << "\n" << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 复合语句节点
class CompoundStmtNode : public StatementNode {
public:
    vector<shared_ptr<StatementNode>> statements;
    
    CompoundStmtNode() : StatementNode(NodeType::COMPOUND_STMT) {}
    
    void addStatement(shared_ptr<StatementNode> stmt) {
        statements.push_back(stmt);
    }
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "CompoundStmt {" << endl;
        for (const auto& stmt : statements) {
            if (stmt) stmt->print(indent + 2);
        }
        cout << string(indent, ' ') << "}" << endl;
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "CompoundStmt {}", isLast);
        string childPrefix = getChildPrefix(prefix, isLast);
        
        for (size_t i = 0; i < statements.size(); ++i) {
            bool isLastChild = (i == statements.size() - 1);
            if (statements[i]) {
                statements[i]->printTree(childPrefix, isLastChild);
            }
        }
    }
    
    string toString() const override {
        string result = "{\n";
        for (const auto& stmt : statements) {
            if (stmt) result += "  " + stmt->toString() + "\n";
        }
        result += "}";
        return result;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"statements\": [\n";
        for (size_t i = 0; i < statements.size(); ++i) {
            if (statements[i]) {
                json << statements[i]->toJSON(indent + 2);
                if (i < statements.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }
        }
        json << jsonIndent(indent + 1) << "]\n";
        json << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 条件语句节点
class IfStmtNode : public StatementNode {
public:
    shared_ptr<ExpressionNode> condition;
    shared_ptr<StatementNode> thenStmt;
    shared_ptr<StatementNode> elseStmt;
    
    IfStmtNode(shared_ptr<ExpressionNode> cond, shared_ptr<StatementNode> then_stmt, shared_ptr<StatementNode> else_stmt = nullptr)
        : StatementNode(NodeType::IF_STMT), condition(cond), thenStmt(then_stmt), elseStmt(else_stmt) {}
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "IfStmt:" << endl;
        if (condition) {
            cout << string(indent + 2, ' ') << "Condition:" << endl;
            condition->print(indent + 4);
        }
        if (thenStmt) {
            cout << string(indent + 2, ' ') << "Then:" << endl;
            thenStmt->print(indent + 4);
        }
        if (elseStmt) {
            cout << string(indent + 2, ' ') << "Else:" << endl;
            elseStmt->print(indent + 4);
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "IfStmt", isLast);
        string childPrefix = getChildPrefix(prefix, isLast);
        
        int childCount = 0;
        if (condition) childCount++;
        if (thenStmt) childCount++;
        if (elseStmt) childCount++;
        
        int currentChild = 0;
        if (condition) {
            printTreeHelper(childPrefix, "Condition:", currentChild == childCount - 1);
            string condPrefix = getChildPrefix(childPrefix, currentChild == childCount - 1);
            condition->printTree(condPrefix, true);
            currentChild++;
        }
        
        if (thenStmt) {
            printTreeHelper(childPrefix, "Then:", currentChild == childCount - 1);
            string thenPrefix = getChildPrefix(childPrefix, currentChild == childCount - 1);
            thenStmt->printTree(thenPrefix, true);
            currentChild++;
        }
        
        if (elseStmt) {
            printTreeHelper(childPrefix, "Else:", currentChild == childCount - 1);
            string elsePrefix = getChildPrefix(childPrefix, currentChild == childCount - 1);
            elseStmt->printTree(elsePrefix, true);
            currentChild++;
        }
    }
    
    string toString() const override {
        string result = "if (" + (condition ? condition->toString() : "") + ") " + (thenStmt ? thenStmt->toString() : "");
        if (elseStmt) result += " else " + elseStmt->toString();
        return result;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\"";
        if (condition) {
            json << ",\n" << jsonIndent(indent + 1) << "\"condition\": \n";
            json << condition->toJSON(indent + 1);
        }
        if (thenStmt) {
            json << ",\n" << jsonIndent(indent + 1) << "\"thenStmt\": \n";
            json << thenStmt->toJSON(indent + 1);
        }
        if (elseStmt) {
            json << ",\n" << jsonIndent(indent + 1) << "\"elseStmt\": \n";
            json << elseStmt->toJSON(indent + 1);
        }
        json << "\n" << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 循环语句节点
class WhileStmtNode : public StatementNode {
public:
    shared_ptr<ExpressionNode> condition;
    shared_ptr<StatementNode> body;
    
    WhileStmtNode(shared_ptr<ExpressionNode> cond, shared_ptr<StatementNode> b)
        : StatementNode(NodeType::WHILE_STMT), condition(cond), body(b) {}
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "WhileStmt:" << endl;
        if (condition) {
            cout << string(indent + 2, ' ') << "Condition:" << endl;
            condition->print(indent + 4);
        }
        if (body) {
            cout << string(indent + 2, ' ') << "Body:" << endl;
            body->print(indent + 4);
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "WhileStmt", isLast);
        string childPrefix = getChildPrefix(prefix, isLast);
        
        if (condition && body) {
            printTreeHelper(childPrefix, "Condition:", false);
            string condPrefix = getChildPrefix(childPrefix, false);
            condition->printTree(condPrefix, true);
            
            printTreeHelper(childPrefix, "Body:", true);
            string bodyPrefix = getChildPrefix(childPrefix, true);
            body->printTree(bodyPrefix, true);
        } else if (condition) {
            printTreeHelper(childPrefix, "Condition:", true);
            string condPrefix = getChildPrefix(childPrefix, true);
            condition->printTree(condPrefix, true);
        } else if (body) {
            printTreeHelper(childPrefix, "Body:", true);
            string bodyPrefix = getChildPrefix(childPrefix, true);
            body->printTree(bodyPrefix, true);
        }
    }
    
    string toString() const override {
        return "while (" + (condition ? condition->toString() : "") + ") " + (body ? body->toString() : "");
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\"";
        if (condition) {
            json << ",\n" << jsonIndent(indent + 1) << "\"condition\": \n";
            json << condition->toJSON(indent + 1);
        }
        if (body) {
            json << ",\n" << jsonIndent(indent + 1) << "\"body\": \n";
            json << body->toJSON(indent + 1);
        }
        json << "\n" << jsonIndent(indent) << "}";
        return json.str();
    }
};

// Return语句节点
class ReturnStmtNode : public StatementNode {
public:
    shared_ptr<ExpressionNode> returnValue;  // 返回值表达式，可以为nullptr表示无返回值
    
    ReturnStmtNode(shared_ptr<ExpressionNode> value = nullptr) 
        : StatementNode(NodeType::RETURN_STMT), returnValue(value) {}
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "ReturnStmt";
        if (returnValue) {
            cout << " with value:" << endl;
            returnValue->print(indent + 2);
        } else {
            cout << " (void)" << endl;
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "ReturnStmt", isLast);
        if (returnValue) {
            string childPrefix = getChildPrefix(prefix, isLast);
            returnValue->printTree(childPrefix, true);
        }
    }
    
    string toString() const override {
        if (returnValue) {
            return "return " + returnValue->toString() + ";";
        } else {
            return "return;";
        }
    }
    
    string toJSON(int indent = 0) const override {
        string result = "{\n";
        result += jsonIndent(indent + 1) + "\"type\": \"ReturnStmt\",\n";
        if (returnValue) {
            result += jsonIndent(indent + 1) + "\"returnValue\": " + returnValue->toJSON(indent + 1) + "\n";
        } else {
            result += jsonIndent(indent + 1) + "\"returnValue\": null\n";
        }
        result += jsonIndent(indent) + "}";
        return result;
    }
};

// 函数调用节点
class FunctionCallNode : public ExpressionNode {
public:
    string functionName;
    vector<shared_ptr<ExpressionNode>> arguments;
    
    FunctionCallNode(const string& name) : ExpressionNode(NodeType::FUNCTION_CALL), functionName(name) {}
    
    void addArgument(shared_ptr<ExpressionNode> arg) {
        arguments.push_back(arg);
    }
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "FunctionCall: " << functionName << endl;
        if (!arguments.empty()) {
            cout << string(indent + 2, ' ') << "Arguments:" << endl;
            for (const auto& arg : arguments) {
                if (arg) arg->print(indent + 4);
            }
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "FunctionCall: " + functionName, isLast);
        if (!arguments.empty()) {
            string childPrefix = getChildPrefix(prefix, isLast);
            for (size_t i = 0; i < arguments.size(); ++i) {
                bool isLastArg = (i == arguments.size() - 1);
                if (arguments[i]) {
                    arguments[i]->printTree(childPrefix, isLastArg);
                }
            }
        }
    }
    
    string toString() const override {
        string result = functionName + "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) result += ", ";
            if (arguments[i]) result += arguments[i]->toString();
        }
        result += ")";
        return result;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"functionName\": \"" << jsonEscape(functionName) << "\",\n";
        json << jsonIndent(indent + 1) << "\"arguments\": [\n";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (arguments[i]) {
                json << arguments[i]->toJSON(indent + 2);
                if (i < arguments.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }
        }
        json << jsonIndent(indent + 1) << "]\n";
        json << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 函数定义节点
class FunctionDefNode : public ASTNode {
public:
    DataType returnType;
    string name;
    vector<shared_ptr<VariableDeclNode>> parameters;
    shared_ptr<CompoundStmtNode> body;
    
    FunctionDefNode(DataType rt, const string& n) : ASTNode(NodeType::FUNCTION_DEF), returnType(rt), name(n) {}
    
    void addParameter(shared_ptr<VariableDeclNode> param) {
        parameters.push_back(param);
    }
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "FunctionDef: ";
        switch(returnType) {
            case DataType::INT: cout << "int "; break;
            case DataType::FLOAT: cout << "float "; break;
            case DataType::VOID: cout << "void "; break;
            default: cout << "unknown "; break;
        }
        cout << name << endl;
        
        if (!parameters.empty()) {
            cout << string(indent + 2, ' ') << "Parameters:" << endl;
            for (const auto& param : parameters) {
                if (param) param->print(indent + 4);
            }
        }
        
        if (body) {
            cout << string(indent + 2, ' ') << "Body:" << endl;
            body->print(indent + 4);
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        string typeStr;
        switch(returnType) {
            case DataType::INT: typeStr = "int"; break;
            case DataType::FLOAT: typeStr = "float"; break;
            case DataType::VOID: typeStr = "void"; break;
            default: typeStr = "unknown"; break;
        }
        printTreeHelper(prefix, "FunctionDef: " + typeStr + " " + name + "()", isLast);
        
        string childPrefix = getChildPrefix(prefix, isLast);
        int childCount = 0;
        if (!parameters.empty()) childCount++;
        if (body) childCount++;
        
        int currentChild = 0;
        if (!parameters.empty()) {
            printTreeHelper(childPrefix, "Parameters:", currentChild == childCount - 1);
            string paramPrefix = getChildPrefix(childPrefix, currentChild == childCount - 1);
            for (size_t i = 0; i < parameters.size(); ++i) {
                bool isLastParam = (i == parameters.size() - 1);
                if (parameters[i]) {
                    parameters[i]->printTree(paramPrefix, isLastParam);
                }
            }
            currentChild++;
        }
        
        if (body) {
            body->printTree(childPrefix, currentChild == childCount - 1);
        }
    }
    
    string toString() const override {
        string result = name + "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) result += ", ";
            if (parameters[i]) result += parameters[i]->toString();
        }
        result += ")";
        return result;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"returnType\": \"" << dataTypeToString(returnType) << "\",\n";
        json << jsonIndent(indent + 1) << "\"name\": \"" << jsonEscape(name) << "\",\n";
        json << jsonIndent(indent + 1) << "\"parameters\": [\n";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (parameters[i]) {
                json << parameters[i]->toJSON(indent + 2);
                if (i < parameters.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }
        }
        json << jsonIndent(indent + 1) << "]";
        if (body) {
            json << ",\n" << jsonIndent(indent + 1) << "\"body\": \n";
            json << body->toJSON(indent + 1);
        }
        json << "\n" << jsonIndent(indent) << "}";
        return json.str();
    }
};

// 程序节点（根节点）
class ProgramNode : public ASTNode {
public:
    vector<shared_ptr<FunctionDefNode>> functions;
    vector<shared_ptr<VariableDeclNode>> globalVariables;
    
    ProgramNode() : ASTNode(NodeType::PROGRAM) {}
    
    void addFunction(shared_ptr<FunctionDefNode> func) {
        functions.push_back(func);
    }
    
    void addGlobalVariable(shared_ptr<VariableDeclNode> var) {
        globalVariables.push_back(var);
    }
    
    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Program:" << endl;
        
        if (!globalVariables.empty()) {
            cout << string(indent + 2, ' ') << "Global Variables:" << endl;
            for (const auto& var : globalVariables) {
                if (var) var->print(indent + 4);
            }
        }
        
        if (!functions.empty()) {
            cout << string(indent + 2, ' ') << "Functions:" << endl;
            for (const auto& func : functions) {
                if (func) func->print(indent + 4);
            }
        }
    }
    
    void printTree(const string& prefix = "", bool isLast = true) const override {
        printTreeHelper(prefix, "Program", isLast);
        string childPrefix = getChildPrefix(prefix, isLast);
        
        // 计算总的子节点数
        size_t totalChildren = globalVariables.size() + functions.size();
        size_t currentChild = 0;
        
        // 打印全局变量
        for (const auto& var : globalVariables) {
            bool isLastChild = (currentChild == totalChildren - 1);
            if (var) {
                var->printTree(childPrefix, isLastChild);
            }
            currentChild++;
        }
        
        // 打印函数
        for (const auto& func : functions) {
            bool isLastChild = (currentChild == totalChildren - 1);
            if (func) {
                func->printTree(childPrefix, isLastChild);
            }
            currentChild++;
        }
    }
    
    string toString() const override {
        string result = "Program\n";
        for (const auto& var : globalVariables) {
            if (var) result += var->toString() + "\n";
        }
        for (const auto& func : functions) {
            if (func) result += func->toString() + "\n";
        }
        return result;
    }
    
    string toJSON(int indent = 0) const override {
        ostringstream json;
        json << jsonIndent(indent) << "{\n";
        json << jsonIndent(indent + 1) << "\"type\": \"" << nodeTypeToString(type) << "\",\n";
        json << jsonIndent(indent + 1) << "\"globalVariables\": [\n";
        for (size_t i = 0; i < globalVariables.size(); ++i) {
            if (globalVariables[i]) {
                json << globalVariables[i]->toJSON(indent + 2);
                if (i < globalVariables.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }
        }
        json << jsonIndent(indent + 1) << "],\n";
        json << jsonIndent(indent + 1) << "\"functions\": [\n";
        for (size_t i = 0; i < functions.size(); ++i) {
            if (functions[i]) {
                json << functions[i]->toJSON(indent + 2);
                if (i < functions.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }
        }
        json << jsonIndent(indent + 1) << "]\n";
        json << jsonIndent(indent) << "}";
        return json.str();
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

// 符号表条目
struct SymbolEntry {
    string name;
    DataType type;
    int scope;
    bool isFunction;
    vector<DataType> paramTypes;  // 函数参数类型
    bool isArray;
    int arraySize;
    int address;  // 变量地址（用于中间代码生成）
    
    SymbolEntry(const string& n, DataType t, int s, bool isFunc = false) 
        : name(n), type(t), scope(s), isFunction(isFunc), isArray(false), arraySize(0), address(-1) {}
};

// 符号表类
class SymbolTable {
private:
    vector<SymbolEntry> symbols;
    vector<int> scopeStack;  // 作用域栈
    int currentScope;
    int nextAddress;
    
public:
    SymbolTable() : currentScope(0), nextAddress(0) {
        scopeStack.push_back(0);
    }
    
    // 进入新作用域
    void enterScope() {
        currentScope++;
        scopeStack.push_back(currentScope);
        DEBUG_PRINT(cout << "进入作用域 " << currentScope << endl);
    }
    
    // 退出当前作用域
    void exitScope() {
        if (scopeStack.size() > 1) {
            int exitingScope = scopeStack.back();
            scopeStack.pop_back();
            currentScope = scopeStack.back();
            
            // 移除退出作用域中的符号
            symbols.erase(
                remove_if(symbols.begin(), symbols.end(),
                    [exitingScope](const SymbolEntry& entry) {
                        return entry.scope == exitingScope;
                    }),
                symbols.end()
            );
            
            DEBUG_PRINT(cout << "退出作用域 " << exitingScope << "，回到作用域 " << currentScope << endl);
        }
    }
    
    // 声明变量
    bool declareVariable(const string& name, DataType type, bool isArray = false, int arraySize = 0) {
        // 检查当前作用域是否已存在同名变量
        for (const auto& entry : symbols) {
            if (entry.name == name && entry.scope == currentScope) {
                return false;  // 重复声明
            }
        }
        
        SymbolEntry entry(name, type, currentScope);
        entry.isArray = isArray;
        entry.arraySize = arraySize;
        entry.address = nextAddress++;
        symbols.push_back(entry);
        
        DEBUG_PRINT(cout << "声明变量: " << name << " (类型: "; printDataType(type); cout << ", 作用域: " << currentScope << ", 地址: " << entry.address << ")" << endl);
        
        return true;
    }
    
    // 声明函数
    bool declareFunction(const string& name, DataType returnType, const vector<DataType>& paramTypes) {
        // 检查是否已存在同名函数
        for (const auto& entry : symbols) {
            if (entry.name == name && entry.isFunction) {
                return false;  // 重复声明
            }
        }
        
        SymbolEntry entry(name, returnType, 0, true);  // 函数总是在全局作用域
        entry.paramTypes = paramTypes;
        symbols.push_back(entry);
        
        DEBUG_PRINT(cout << "声明函数: " << name << " (返回类型: "; printDataType(returnType); cout << ", 参数个数: " << paramTypes.size() << ")" << endl);
        
        return true;
    }
    
    // 查找符号
    SymbolEntry* lookup(const string& name) {
        // 从当前作用域向外查找
        for (int i = scopeStack.size() - 1; i >= 0; i--) {
            int scope = scopeStack[i];
            for (auto& entry : symbols) {
                if (entry.name == name && entry.scope == scope) {
                    return &entry;
                }
            }
        }
        
        // 查找全局函数
        for (auto& entry : symbols) {
            if (entry.name == name && entry.isFunction) {
                return &entry;
            }
        }
        
        return nullptr;  // 未找到
    }
    
    // 检查变量是否已声明
    bool isDeclared(const string& name) {
        return lookup(name) != nullptr;
    }
    
    // 获取变量类型
    DataType getType(const string& name) {
        SymbolEntry* entry = lookup(name);
        return entry ? entry->type : DataType::UNKNOWN;
    }
    
    // 打印符号表
    void printSymbolTable() const {
        cout << "\n=== 符号表 ===" << endl;
        cout << "当前作用域: " << currentScope << endl;
        cout << "作用域栈: ";
        for (int scope : scopeStack) {
            cout << scope << " ";
        }
        cout << endl;
        
        cout << "\n符号列表:" << endl;
        cout << setw(15) << "名称" << setw(10) << "类型" << setw(8) << "作用域" 
             << setw(8) << "函数" << setw(8) << "地址" << endl;
        cout << string(55, '-') << endl;
        
        for (const auto& entry : symbols) {
            cout << setw(15) << entry.name;
            cout << setw(10);
            printDataType(entry.type);
            cout << setw(8) << entry.scope;
            cout << setw(8) << (entry.isFunction ? "是" : "否");
            cout << setw(8) << entry.address;
            cout << endl;
        }
    }
    
    // 获取当前作用域
    int getCurrentScope() const {
        return currentScope;
    }
    
private:
    void printDataType(DataType type) const {
        switch(type) {
            case DataType::INT: cout << "int"; break;
            case DataType::FLOAT: cout << "float"; break;
            case DataType::VOID: cout << "void"; break;
            case DataType::ARRAY_INT: cout << "int[]"; break;
            case DataType::ARRAY_FLOAT: cout << "float[]"; break;
            default: cout << "unknown"; break;
        }
    }
};

// 语义错误类
class SemanticError {
public:
    string message;
    int line;
    int column;
    
    SemanticError(const string& msg, int l = 0, int c = 0) 
        : message(msg), line(l), column(c) {}
    
    void print() const {
        cout << "语义错误";
        if (line > 0) cout << " (行 " << line << ", 列 " << column << ")";
        cout << ": " << message << endl;
    }
};

// 语义分析器类
class SemanticAnalyzer {
private:
    SymbolTable symbolTable;
    vector<SemanticError> errors;
    
public:
    // 分析程序节点
    bool analyzeProgram(shared_ptr<ProgramNode> program) {
        if (!program) return false;
        
        DEBUG_PRINT(cout << "\n=== 开始语义分析 ===" << endl);
        
        // 分析全局变量
        for (auto& var : program->globalVariables) {
            if (var) {
                analyzeVariableDecl(var);
            }
        }
        
        // 分析函数
        for (auto& func : program->functions) {
            if (func) {
                analyzeFunction(func);
            }
        }
        
        // 打印符号表
        if (DEBUG_MODE) {
            symbolTable.printSymbolTable();
        }
        
        // 打印错误
        printErrors();
        
        return errors.empty();
    }
    
    // 分析函数定义
    void analyzeFunction(shared_ptr<FunctionDefNode> func) {
        if (!func) return;
        
        DEBUG_PRINT(cout << "\n分析函数: " << func->name << endl);
        
        // 收集参数类型
        vector<DataType> paramTypes;
        for (auto& param : func->parameters) {
            if (param) {
                paramTypes.push_back(param->varType);
            }
        }
        
        // 声明函数
        if (!symbolTable.declareFunction(func->name, func->returnType, paramTypes)) {
            errors.push_back(SemanticError("函数 '" + func->name + "' 重复声明", func->line, func->column));
        }
        
        // 进入函数作用域
        symbolTable.enterScope();
        
        // 声明参数
        for (auto& param : func->parameters) {
            if (param) {
                if (!symbolTable.declareVariable(param->name, param->varType, param->isArray, param->arraySize)) {
                    errors.push_back(SemanticError("参数 '" + param->name + "' 重复声明", param->line, param->column));
                }
            }
        }
        
        // 分析函数体
        if (func->body) {
            analyzeCompoundStmt(func->body);
        }
        
        // 退出函数作用域
        symbolTable.exitScope();
    }
    
    // 分析复合语句
    void analyzeCompoundStmt(shared_ptr<CompoundStmtNode> stmt) {
        if (!stmt) return;
        
        symbolTable.enterScope();
        
        for (auto& s : stmt->statements) {
            if (s) {
                analyzeStatement(s);
            }
        }
        
        symbolTable.exitScope();
    }
    
    // 分析语句
    void analyzeStatement(shared_ptr<StatementNode> stmt) {
        if (!stmt) return;
        
        switch (stmt->type) {
            case NodeType::VARIABLE_DECL:
                analyzeVariableDecl(static_pointer_cast<VariableDeclNode>(stmt));
                break;
            case NodeType::ASSIGNMENT:
                analyzeAssignment(static_pointer_cast<AssignmentNode>(stmt));
                break;
            case NodeType::IF_STMT:
                analyzeIfStmt(static_pointer_cast<IfStmtNode>(stmt));
                break;
            case NodeType::WHILE_STMT:
                analyzeWhileStmt(static_pointer_cast<WhileStmtNode>(stmt));
                break;
            case NodeType::RETURN_STMT:
                analyzeReturnStmt(static_pointer_cast<ReturnStmtNode>(stmt));
                break;
            case NodeType::COMPOUND_STMT:
                analyzeCompoundStmt(static_pointer_cast<CompoundStmtNode>(stmt));
                break;
            default:
                break;
        }
    }
    
    // 分析变量声明
    void analyzeVariableDecl(shared_ptr<VariableDeclNode> decl) {
        if (!decl) return;
        
        // 声明变量
        if (!symbolTable.declareVariable(decl->name, decl->varType, decl->isArray, decl->arraySize)) {
            errors.push_back(SemanticError("变量 '" + decl->name + "' 重复声明", decl->line, decl->column));
        }
        
        // 分析初始化表达式
        if (decl->initializer) {
            DataType initType = analyzeExpression(decl->initializer);
            if (initType != DataType::UNKNOWN && initType != decl->varType) {
                errors.push_back(SemanticError("变量 '" + decl->name + "' 初始化类型不匹配", decl->line, decl->column));
            }
        }
    }
    
    // 分析赋值语句
    void analyzeAssignment(shared_ptr<AssignmentNode> assign) {
        if (!assign) return;
        
        DataType targetType = analyzeExpression(assign->target);
        DataType valueType = analyzeExpression(assign->value);
        
        if (targetType != DataType::UNKNOWN && valueType != DataType::UNKNOWN && targetType != valueType) {
            errors.push_back(SemanticError("赋值类型不匹配", assign->line, assign->column));
        }
    }
    
    // 分析if语句
    void analyzeIfStmt(shared_ptr<IfStmtNode> ifStmt) {
        if (!ifStmt) return;
        
        if (ifStmt->condition) {
            analyzeExpression(ifStmt->condition);
        }
        
        if (ifStmt->thenStmt) {
            analyzeStatement(ifStmt->thenStmt);
        }
        
        if (ifStmt->elseStmt) {
            analyzeStatement(ifStmt->elseStmt);
        }
    }
    
    // 分析while语句
    void analyzeWhileStmt(shared_ptr<WhileStmtNode> whileStmt) {
        if (!whileStmt) return;
        
        if (whileStmt->condition) {
            analyzeExpression(whileStmt->condition);
        }
        
        if (whileStmt->body) {
            analyzeStatement(whileStmt->body);
        }
    }
    
    // 分析return语句
    void analyzeReturnStmt(shared_ptr<ReturnStmtNode> returnStmt) {
        if (!returnStmt) return;
        
        // 分析返回值表达式
        if (returnStmt->returnValue) {
            analyzeExpression(returnStmt->returnValue);
        }
        
        // TODO: 可以在这里添加返回类型检查，确保返回值类型与函数返回类型匹配
    }
    
    // 分析表达式
    DataType analyzeExpression(shared_ptr<ExpressionNode> expr) {
        if (!expr) return DataType::UNKNOWN;
        
        switch (expr->type) {
            case NodeType::IDENTIFIER: {
                auto id = static_pointer_cast<IdentifierNode>(expr);
                if (!symbolTable.isDeclared(id->name)) {
                    errors.push_back(SemanticError("变量 '" + id->name + "' 未声明", id->line, id->column));
                    return DataType::UNKNOWN;
                }
                return symbolTable.getType(id->name);
            }
            case NodeType::LITERAL: {
                auto lit = static_pointer_cast<LiteralNode>(expr);
                return lit->dataType;
            }
            case NodeType::BINARY_OP: {
                auto binOp = static_pointer_cast<BinaryOpNode>(expr);
                DataType leftType = analyzeExpression(binOp->left);
                DataType rightType = analyzeExpression(binOp->right);
                
                if (leftType != DataType::UNKNOWN && rightType != DataType::UNKNOWN && leftType != rightType) {
                    errors.push_back(SemanticError("二元操作类型不匹配", binOp->line, binOp->column));
                }
                
                // 关系操作返回int类型（布尔值）
                if (binOp->op == "<" || binOp->op == "<=" || binOp->op == "==" || 
                    binOp->op == ">" || binOp->op == ">=" || binOp->op == "!=") {
                    return DataType::INT;
                }
                
                return leftType;
            }
            case NodeType::FUNCTION_CALL: {
                auto call = static_pointer_cast<FunctionCallNode>(expr);
                SymbolEntry* func = symbolTable.lookup(call->functionName);
                if (!func || !func->isFunction) {
                    errors.push_back(SemanticError("函数 '" + call->functionName + "' 未声明", call->line, call->column));
                    return DataType::UNKNOWN;
                }
                
                // 检查参数个数
                if (call->arguments.size() != func->paramTypes.size()) {
                    errors.push_back(SemanticError("函数 '" + call->functionName + "' 参数个数不匹配", call->line, call->column));
                }
                
                // 检查参数类型
                for (size_t i = 0; i < min(call->arguments.size(), func->paramTypes.size()); i++) {
                    DataType argType = analyzeExpression(call->arguments[i]);
                    if (argType != DataType::UNKNOWN && argType != func->paramTypes[i]) {
                        errors.push_back(SemanticError("函数 '" + call->functionName + "' 第" + to_string(i+1) + "个参数类型不匹配", call->line, call->column));
                    }
                }
                
                return func->type;
            }
            default:
                return DataType::UNKNOWN;
        }
    }
    
    // 打印错误
    void printErrors() const {
        if (errors.empty()) {
            cout << "\n语义分析完成，无错误。" << endl;
        } else {
            cout << "\n=== 语义错误 ===" << endl;
            for (const auto& error : errors) {
                error.print();
            }
        }
    }
    
    // 获取错误数量
    size_t getErrorCount() const {
        return errors.size();
    }
};


// ===== SLR分析器 =====
class SLRParser {
private:
    SLRTable table;
    vector<int> stateStack;
    vector<shared_ptr<ASTNode>> nodeStack;
    vector<string> productions;
    
public:
    SLRParser() {
        loadSLRTable();
    }
    
    void loadSLRTable() {
        // 加载产生式规则
        productions = {
            "S' -> Prog",
            "Prog -> DeclList",
            "DeclList -> DeclList Decl",
            "DeclList -> Decl",
            "Decl -> VarDecl",
            "Decl -> FunDecl",
            "VarDecl -> Type ID SEMI",
            "VarDecl -> Type ID LBRACK INT_NUM RBRACK SEMI",
            "VarDecl -> Type ID ASG Expr SEMI",
            "Type -> INT",
            "Type -> FLOAT",
            "Type -> VOID",
            "FunDecl -> Type ID LPAR ParamList RPAR CompStmt",
            "ParamList -> ParamList COMMA Param",
            "ParamList -> Param",
            "ParamList -> ε",
            "Param -> Type ID",
            "Param -> Type ID LBRACK RBRACK",
            "CompStmt -> LBR StmtList RBR",
            "StmtList -> StmtList Stmt",
            "StmtList -> ε",
            "Stmt -> VarDecl",
            "Stmt -> OtherStmt",
            "OtherStmt -> ExprStmt",
            "OtherStmt -> CompStmt",
            "OtherStmt -> IfStmt",
            "OtherStmt -> LoopStmt",
            "OtherStmt -> RetStmt",
            "OtherStmt -> PrintStmt",
            "PrintStmt -> PRINT LPAR Expr RPAR SEMI",
            "ExprStmt -> Expr SEMI",
            "ExprStmt -> SEMI",
            "IfStmt -> IF LPAR Expr RPAR CompStmt",
            "IfStmt -> IF LPAR Expr RPAR CompStmt ELSE Stmt",
            "LoopStmt -> WHILE LPAR Expr RPAR Stmt",
            "RetStmt -> RETURN Expr SEMI",
            "RetStmt -> RETURN SEMI",
            "Expr -> ID ASG Expr",
            "Expr -> ID LBRACK Expr RBRACK ASG Expr",
            "Expr -> ID LPAR ArgList RPAR",
            "Expr -> SimpExpr",
            "SimpExpr -> AddExpr REL_OP AddExpr",
            "SimpExpr -> AddExpr",
            "AddExpr -> AddExpr ADD Term",
            "AddExpr -> Term",
            "Term -> Term MUL Fact",
            "Term -> Fact",
            "Fact -> ID",
            "Fact -> ID LBRACK Expr RBRACK",
            "Fact -> INT_NUM",
            "Fact -> FLOAT_NUM",
            "Fact -> LPAR Expr RPAR",
            "ArgList -> ArgList COMMA Expr",
            "ArgList -> Expr",
            "ArgList -> ε"
        };
        
        // 加载SLR分析表
        vector<string> rules = {
            "Prog -> DeclList",
            "DeclList -> DeclList Decl | Decl",
            "Decl -> VarDecl | FunDecl",
            "VarDecl -> Type ID SEMI | Type ID LBRACK INT_NUM RBRACK SEMI | Type ID ASG Expr SEMI",
            "Type -> INT | FLOAT | VOID",
            "FunDecl -> Type ID LPAR ParamList RPAR CompStmt",
            "ParamList -> ParamList COMMA Param | Param | ε",
            "Param -> Type ID | Type ID LBRACK RBRACK",
            "CompStmt -> LBR StmtList RBR",
            "StmtList -> StmtList Stmt | ε",
            "Stmt -> VarDecl | OtherStmt",
            "OtherStmt -> ExprStmt | CompStmt | IfStmt | LoopStmt | RetStmt | PrintStmt",
            "PrintStmt -> PRINT LPAR Expr RPAR SEMI",
            "ExprStmt -> Expr SEMI | SEMI",
            "IfStmt -> IF LPAR Expr RPAR CompStmt | IF LPAR Expr RPAR CompStmt ELSE Stmt",
            "LoopStmt -> WHILE LPAR Expr RPAR Stmt",
            "RetStmt -> RETURN Expr SEMI | RETURN SEMI",
            "Expr -> ID ASG Expr | ID LBRACK Expr RBRACK ASG Expr | ID LPAR ArgList RPAR | SimpExpr",
            "SimpExpr -> AddExpr REL_OP AddExpr | AddExpr",
            "AddExpr -> AddExpr ADD Term | Term",
            "Term -> Term MUL Fact | Fact",
            "Fact -> ID | ID LBRACK Expr RBRACK | INT_NUM | FLOAT_NUM | LPAR Expr RPAR",
            "ArgList -> ArgList COMMA Expr | Expr | ε"
        };

        Grammar g;
        g.parse(rules);
        g.compute_first();
        g.compute_follow();
        CanonicalCollection cc = build_canonical_collection(g);
        table = build_slr_table(g, cc);


        if (DEBUG_MODE) {
            g.print_grammar();  // 打印语法规则，查看产生式编号
            print_canonical_collection(cc, g);
            print_slr_table(table, g, cc.C.size());
        }
        
    }
    
    shared_ptr<ASTNode> parse(const string& filename) {
        Lexer lexer(filename);
        vector<Token> tokens;
        cout << "start parse" << endl;
        // 词法分析
        Token token;
        do {
            token = lexer.getNextToken();
            tokens.push_back(token);
            cout << "token: " << token.value << " (type: " << tokenTypeToString(token.type) << ")" << endl;
        } while (lexer.getPos() < lexer.getTokensSize());
        
        // 添加EOF token
        Token eofToken;
        eofToken.type = TokenType::EOF_TOKEN;
        eofToken.value = "$";
        tokens.push_back(eofToken);
        cout << "token: " << eofToken.value << " (type: " << tokenTypeToString(eofToken.type) << ")" << endl;
        
        // 语法分析
        stateStack.clear();
        nodeStack.clear();
        stateStack.push_back(0);  // 初始状态
        
        size_t tokenIndex = 0;
        
        while (tokenIndex < tokens.size()) {
            int state = stateStack.back();
            Token currentToken = tokens[tokenIndex];
            string symbol = tokenTypeToString(currentToken.type);
            
            // 将EOF_TOKEN转换为#符号
            if (symbol == "EOF_TOKEN") {
                symbol = "#";
            }
            
            DEBUG_PRINT(cout << "处理token[" << tokenIndex << "]: " << symbol << " 在状态 " << state << endl);
            
            // 查找ACTION表
            if (table.ACTION[state].count(symbol) == 0) {
                cerr << "语法错误：状态 " << state << " 没有符号 " << symbol << " 的动作" << endl;
                // 打印当前状态的所有ACTION条目
                cerr << "状态 " << state << " 的ACTION条目：";
                for (const auto& entry : table.ACTION[state]) {
                    cerr << " " << entry.first << "->" << entry.second.type << entry.second.value;
                }
                cerr << endl;
                return nullptr;
            }
            
            SLRAction action = table.ACTION[state][symbol];

            if (DEBUG_MODE) {
                cout << "state: " << state << " symbol: " << symbol << " action: " << action.type << " " << action.value << endl;
            }
            
            
            if (action.type == 's') {
                // 移进
                stateStack.push_back(action.value);
                nodeStack.push_back(createTerminalNode(currentToken));
                tokenIndex++;
            } else if (action.type == 'r') {
                // 归约
                int prodNum = action.value;
                
                // 获取产生式右部长度
                int rightLength = getProductionRightLength(prodNum);
                
                // 弹出状态栈和节点栈
                vector<shared_ptr<ASTNode>> children;
                for (int i = 0; i < rightLength; i++) {
                    if (!nodeStack.empty()) {
                        children.push_back(nodeStack.back());
                        nodeStack.pop_back();
                    }
                    if (!stateStack.empty()) {
                        stateStack.pop_back();
                    }
                }
                reverse(children.begin(), children.end());
                
                // 创建新节点
                shared_ptr<ASTNode> newNode = createNodeFromProduction(prodNum, children);
                if (!newNode) {
                    cerr << "创建节点失败，产生式编号: " << prodNum << endl;
                    return nullptr;
                }
                
                // 获取产生式左部
                string left = getProductionLeft(prodNum);
                
                // 查找GOTO表
                state = stateStack.back();
                if (table.GOTO[state].count(left) == 0) {
                    cerr << "语法错误：GOTO[" << state << ", " << left << "] 未定义" << endl;
                    // 打印当前状态的所有GOTO条目
                    cerr << "状态 " << state << " 的GOTO条目：";
                    for (const auto& entry : table.GOTO[state]) {
                        cerr << " " << entry.first << "->" << entry.second;
                    }
                    cerr << endl;
                    return nullptr;
                }
                
                stateStack.push_back(table.GOTO[state][left]);
                nodeStack.push_back(newNode);
            } else if (action.type == 'a') {
                // 接受
                cout << "语法分析成功完成！" << endl;
                if (!nodeStack.empty()) {
                    auto ast = nodeStack.back();
                    cout << "\n=== AST结构 ===" << endl;
                    if (ast) {
                        ast->printTree();
                        saveASTtoJSON(ast, "ast.json");
                    }
                    return ast;
                }
                return nullptr;
            }
        }
        
        cout << "所有token处理完成，但没有遇到接受状态" << endl;
        cout << "最终状态栈大小: " << stateStack.size() << endl;
        cout << "最终节点栈大小: " << nodeStack.size() << endl;
        if (!nodeStack.empty()) {
            cout << "返回最后的节点" << endl;
            return nodeStack.back();
        }
        
        return nullptr;
    }
    
private:
    shared_ptr<ASTNode> createTerminalNode(const Token& token) {
        switch (token.type) {
            case TokenType::ID:
                return make_shared<IdentifierNode>(token.value);
            case TokenType::INT_NUM:
                return make_shared<LiteralNode>(token.value, DataType::INT);
            case TokenType::FLOAT_NUM:
                return make_shared<LiteralNode>(token.value, DataType::FLOAT);
            default:
                // 对于其他终结符，暂时返回nullptr或创建一个占位节点
                return nullptr;
        }
    }
    
    string getProductionLeft(int prodNum) {
        // 根据SLR表构建时的产生式编号返回左部
        switch(prodNum) {
            case 0: return "S'";
            case 1: return "Prog";
            case 2: case 3: return "DeclList";
            case 4: case 5: return "Decl";
            case 6: case 7: case 8: return "VarDecl";
            case 9: case 10: case 11: return "Type";
            case 12: return "FunDecl";
            case 13: case 14: case 15: return "ParamList";
            case 16: case 17: return "Param";
            case 18: return "CompStmt";
            case 19: case 20: return "StmtList";
            case 21: case 22: return "Stmt";
            case 23: case 24: case 25: case 26: case 27: case 28: return "OtherStmt";
            case 29: return "PrintStmt";
            case 30: case 31: return "ExprStmt";
            case 32: case 33: return "IfStmt";
            case 34: return "LoopStmt";
            case 35: case 36: return "RetStmt";
            case 37: case 38: case 39: case 40: return "Expr";
            case 41: case 42: return "SimpExpr";
            case 43: case 44: return "AddExpr";
            case 45: case 46: return "Term";
            case 47: case 48: case 49: case 50: case 51: return "Fact";
            case 52: case 53: case 54: return "ArgList";
            default: return "";
        }
    }
    
    int getProductionRightLength(int prodNum) {
        // 根据实际的产生式编号返回右部长度
        switch(prodNum) {
            case 0: return 1; // S' → Prog
            case 1: return 1; // Prog → DeclList
            case 2: return 2; // DeclList → DeclList Decl
            case 3: return 1; // DeclList → Decl
            case 4: return 1; // Decl → VarDecl
            case 5: return 1; // Decl → FunDecl
            case 6: return 3; // VarDecl → Type ID SEMI
            case 7: return 6; // VarDecl → Type ID LBRACK INT_NUM RBRACK SEMI
            case 8: return 5; // VarDecl → Type ID ASG Expr SEMI
            case 9: return 1; // Type → INT
            case 10: return 1; // Type → FLOAT
            case 11: return 1; // Type → VOID
            case 12: return 6; // FunDecl → Type ID LPAR ParamList RPAR CompStmt
            case 13: return 3; // ParamList → ParamList COMMA Param
            case 14: return 1; // ParamList → Param
            case 15: return 0; // ParamList → ε
            case 16: return 2; // Param → Type ID
            case 17: return 4; // Param → Type ID LBRACK RBRACK
            case 18: return 3; // CompStmt → LBR StmtList RBR
            case 19: return 2; // StmtList → StmtList Stmt
            case 20: return 0; // StmtList → ε
            case 21: return 1; // Stmt → VarDecl
            case 22: return 1; // Stmt → OtherStmt
            case 23: return 1; // OtherStmt → ExprStmt
            case 24: return 1; // OtherStmt → CompStmt
            case 25: return 1; // OtherStmt → IfStmt
            case 26: return 1; // OtherStmt → LoopStmt
            case 27: return 1; // OtherStmt → RetStmt
            case 28: return 1; // OtherStmt → PrintStmt
            case 29: return 5; // PrintStmt → PRINT LPAR Expr RPAR SEMI
            case 30: return 2; // ExprStmt → Expr SEMI
            case 31: return 1; // ExprStmt → SEMI
            case 32: return 5; // IfStmt → IF LPAR Expr RPAR CompStmt
            case 33: return 7; // IfStmt → IF LPAR Expr RPAR CompStmt ELSE CompStmt
            case 34: return 5; // LoopStmt → WHILE LPAR Expr RPAR Stmt
            case 35: return 3; // RetStmt → RETURN Expr SEMI
            case 36: return 2; // RetStmt → RETURN SEMI
            case 37: return 3; // Expr → ID ASG Expr
            case 38: return 6; // Expr → ID LBRACK Expr RBRACK ASG Expr
            case 39: return 4; // Expr → ID LPAR ArgList RPAR
            case 40: return 1; // Expr → SimpExpr
            case 41: return 3; // SimpExpr → AddExpr REL_OP AddExpr
            case 42: return 1; // SimpExpr → AddExpr
            case 43: return 3; // AddExpr → AddExpr ADD Term
            case 44: return 1; // AddExpr → Term
            case 45: return 3; // Term → Term MUL Fact
            case 46: return 1; // Term → Fact
            case 47: return 1; // Fact → ID
            case 48: return 4; // Fact → ID LBRACK Expr RBRACK
            case 49: return 1; // Fact → INT_NUM
            case 50: return 1; // Fact → FLOAT_NUM
            case 51: return 3; // Fact → LPAR Expr RPAR
            case 52: return 3; // ArgList → ArgList COMMA Expr
            case 53: return 1; // ArgList → Expr
            case 54: return 0; // ArgList → ε
            default: return 0;
        }
    }
    
    shared_ptr<ASTNode> createNodeFromProduction(int prodNum, const vector<shared_ptr<ASTNode>>& children) {
        // 根据产生式编号创建对应的AST节点
        // 每个case对应一个语法产生式规则
        switch(prodNum) {
            case 0: // S' -> Prog (增广文法的开始符号)
                return children.empty() ? nullptr : children[0];
            
            case 1: // Prog -> DeclList (程序由声明列表组成)
                return children.empty() ? nullptr : children[0];
            
            case 2: { // DeclList -> DeclList Decl
                if (children.size() < 2) return nullptr;
                auto program = static_pointer_cast<ProgramNode>(children[0]);
                auto decl = children[1];
                
                if (decl->type == NodeType::VARIABLE_DECL) {
                    program->addGlobalVariable(static_pointer_cast<VariableDeclNode>(decl));
                } else if (decl->type == NodeType::FUNCTION_DEF) {
                    program->addFunction(static_pointer_cast<FunctionDefNode>(decl));
                }
                
                return program;
            }
            
            case 3: { // DeclList -> Decl
                if (children.empty()) return nullptr;
                auto program = make_shared<ProgramNode>();
                auto decl = children[0];
                
                if (decl->type == NodeType::VARIABLE_DECL) {
                    program->addGlobalVariable(static_pointer_cast<VariableDeclNode>(decl));
                } else if (decl->type == NodeType::FUNCTION_DEF) {
                    program->addFunction(static_pointer_cast<FunctionDefNode>(decl));
                }
                
                return program;
            }
            
            case 4: // Decl -> VarDecl
            case 5: // Decl -> FunDecl
                return children.empty() ? nullptr : children[0];
            
            case 6: { // VarDecl -> Type ID SEMI
                if (children.size() < 3) return nullptr;
                DataType varType = getDataTypeFromNode(children[0]);
                string varName = getIdentifierName(children[1]);
                return make_shared<VariableDeclNode>(varType, varName);
            }
            
            case 7: { // VarDecl -> Type ID LBRACK INT_NUM RBRACK SEMI
                if (children.size() < 6) return nullptr;
                DataType baseType = getDataTypeFromNode(children[0]);
                string varName = getIdentifierName(children[1]);
                int arraySize = stoi(getLiteralValue(children[3]));
                
                DataType arrayType = (baseType == DataType::INT) ? DataType::ARRAY_INT : DataType::ARRAY_FLOAT;
                auto varDecl = make_shared<VariableDeclNode>(arrayType, varName);
                varDecl->isArray = true;
                varDecl->arraySize = arraySize;
                
                return varDecl;
            }
            
            case 8: { // VarDecl -> Type ID ASG Expr SEMI
                if (children.size() < 5) return nullptr;
                DataType varType = getDataTypeFromNode(children[0]);
                string varName = getIdentifierName(children[1]);
                auto initExpr = static_pointer_cast<ExpressionNode>(children[3]);
                
                return make_shared<VariableDeclNode>(varType, varName, initExpr);
            }
            
            case 9: // Type -> INT
                return make_shared<LiteralNode>("int", DataType::INT);
            
            case 10: // Type -> FLOAT
                return make_shared<LiteralNode>("float", DataType::FLOAT);
            
            case 11: // Type -> VOID
                return make_shared<LiteralNode>("void", DataType::VOID);
            
            case 12: { // FunDecl -> Type ID LPAR ParamList RPAR CompStmt
                if (children.size() < 6) return nullptr;
                DataType returnType = getDataTypeFromNode(children[0]);
                string funcName = getIdentifierName(children[1]);
                auto paramList = children[3]; // ParamList节点
                auto body = static_pointer_cast<CompoundStmtNode>(children[5]);
                
                auto func = make_shared<FunctionDefNode>(returnType, funcName);
                func->body = body;
                
                // 处理参数列表
                auto params = getParameterList(paramList);
                for (auto param : params) {
                    func->addParameter(param);
                }
                
                return func;
            }
            
            case 13: { // ParamList -> ParamList COMMA Param
                if (children.size() < 3) return nullptr;
                // 创建参数列表节点（使用CompoundStmtNode临时表示）
                auto paramList = make_shared<CompoundStmtNode>();
                auto existingParams = getParameterList(children[0]);
                auto newParam = static_pointer_cast<VariableDeclNode>(children[2]);
                
                // 添加现有参数
                for (auto param : existingParams) {
                    paramList->addStatement(param);
                }
                // 添加新参数
                paramList->addStatement(newParam);
                
                return paramList;
            }
            
            case 14: { // ParamList -> Param
                if (children.empty()) return nullptr;
                // 创建只包含一个参数的参数列表
                auto paramList = make_shared<CompoundStmtNode>();
                auto param = static_pointer_cast<VariableDeclNode>(children[0]);
                paramList->addStatement(param);
                return paramList;
            }
            
            case 15: { // ParamList -> ε (空产生式)
                // 返回一个空的复合语句节点来表示空参数列表
                return make_shared<CompoundStmtNode>();
            }
            
            case 16: { // Param -> Type ID
                if (children.size() < 2) return nullptr;
                DataType paramType = getDataTypeFromNode(children[0]);
                string paramName = getIdentifierName(children[1]);
                return make_shared<VariableDeclNode>(paramType, paramName);
            }
            
            case 17: { // Param -> Type ID LBRACK RBRACK
                if (children.size() < 4) return nullptr;
                DataType baseType = getDataTypeFromNode(children[0]);
                string paramName = getIdentifierName(children[1]);
                DataType arrayType = (baseType == DataType::INT) ? DataType::ARRAY_INT : DataType::ARRAY_FLOAT;
                auto param = make_shared<VariableDeclNode>(arrayType, paramName);
                param->isArray = true;
                return param;
            }
            
            case 18: { // CompStmt -> LBR StmtList RBR
                if (children.size() < 3) return nullptr;
                return children[1]; // 返回StmtList
            }
            
            case 19: { // StmtList -> StmtList Stmt
                if (children.size() < 2) return nullptr;
                auto stmtList = static_pointer_cast<CompoundStmtNode>(children[0]);
                auto stmt = static_pointer_cast<StatementNode>(children[1]);
                stmtList->addStatement(stmt);
                return stmtList;
            }
            
            case 20: { // StmtList -> ε (空产生式)
                return make_shared<CompoundStmtNode>();
            }
            
            case 21: // Stmt -> VarDecl
            case 22: // Stmt -> OtherStmt
            case 23: // OtherStmt -> ExprStmt
            case 24: // OtherStmt -> CompStmt
            case 25: // OtherStmt -> IfStmt
            case 26: // OtherStmt -> LoopStmt
            case 27: // OtherStmt -> RetStmt
                return children.empty() ? nullptr : children[0];
            
            case 28: { // ExprStmt -> Expr SEMI
                if (children.size() < 2) return nullptr;
                return children[0]; // 返回表达式
            }
            
            case 29: { // ExprStmt -> SEMI
                return nullptr; // 空语句
            }
            
            case 30: { // ExprStmt -> Expr SEMI
                if (children.size() < 2) return nullptr;
                return children[0]; // 返回表达式
            }
            
            case 31: { // ExprStmt -> SEMI
                return nullptr; // 空语句
            }
            
            case 32: { // IfStmt -> IF LPAR Expr RPAR CompStmt
                if (children.size() < 5) return nullptr;
                auto condition = static_pointer_cast<ExpressionNode>(children[2]);
                auto thenStmt = static_pointer_cast<StatementNode>(children[4]);
                return make_shared<IfStmtNode>(condition, thenStmt);
            }
            
            case 33: { // IfStmt -> IF LPAR Expr RPAR CompStmt ELSE CompStmt
                if (children.size() < 7) return nullptr;
                auto condition = static_pointer_cast<ExpressionNode>(children[2]);
                auto thenStmt = static_pointer_cast<StatementNode>(children[4]);
                auto elseStmt = static_pointer_cast<StatementNode>(children[6]);
                return make_shared<IfStmtNode>(condition, thenStmt, elseStmt);
            }
            
            case 34: { // LoopStmt -> WHILE LPAR Expr RPAR Stmt
                if (children.size() < 5) return nullptr;
                auto condition = static_pointer_cast<ExpressionNode>(children[2]);
                auto body = static_pointer_cast<StatementNode>(children[4]);
                return make_shared<WhileStmtNode>(condition, body);
            }
            
            case 35: { // RetStmt -> RETURN Expr SEMI
                if (children.size() < 3) return nullptr;
                auto returnExpr = static_pointer_cast<ExpressionNode>(children[1]);
                return make_shared<ReturnStmtNode>(returnExpr);
            }
            
            case 36: { // RetStmt -> RETURN SEMI
                if (children.size() < 2) return nullptr;
                return make_shared<ReturnStmtNode>(nullptr);
            }
            
            case 37: { // Expr -> ID ASG Expr (赋值表达式)
                if (children.size() < 3) return nullptr;
                auto target = static_pointer_cast<ExpressionNode>(children[0]);
                auto value = static_pointer_cast<ExpressionNode>(children[2]);
                return make_shared<AssignmentNode>(target, value);
            }
            
            case 38: { // Expr -> ID LBRACK Expr RBRACK ASG Expr (数组赋值)
                if (children.size() < 6) return nullptr;
                auto arrayId = static_pointer_cast<IdentifierNode>(children[0]);
                auto index = static_pointer_cast<ExpressionNode>(children[2]);
                auto value = static_pointer_cast<ExpressionNode>(children[5]);
                auto arrayAccess = make_shared<BinaryOpNode>("[]", arrayId, index);
                arrayAccess->type = NodeType::ARRAY_ACCESS;
                return make_shared<AssignmentNode>(arrayAccess, value);
            }
            
            case 39: { // Expr -> ID LPAR ArgList RPAR (函数调用)
                if (children.size() < 4) return nullptr;
                string funcName = getIdentifierName(children[0]);
                auto argList = getArgumentList(children[2]);
                auto funcCall = make_shared<FunctionCallNode>(funcName);
                for (auto arg : argList) {
                    funcCall->addArgument(arg);
                }
                return funcCall;
            }
            
            case 40: // Expr -> SimpExpr
            case 42: // SimpExpr -> AddExpr
            case 44: // AddExpr -> Term
            case 46: // Term -> Fact
            case 47: // Fact -> ID
                return children.empty() ? nullptr : children[0];
            
            case 41: { // SimpExpr -> AddExpr REL_OP AddExpr
                if (children.size() < 3) return nullptr;
                auto left = static_pointer_cast<ExpressionNode>(children[0]);
                string op = getOperatorValue(children[1]);
                auto right = static_pointer_cast<ExpressionNode>(children[2]);
                return make_shared<BinaryOpNode>(op, left, right);
            }
            
            case 43: { // AddExpr -> AddExpr ADD Term
                if (children.size() < 3) return nullptr;
                auto left = static_pointer_cast<ExpressionNode>(children[0]);
                auto right = static_pointer_cast<ExpressionNode>(children[2]);
                return make_shared<BinaryOpNode>("+", left, right);
            }
            
            case 45: { // Term -> Term MUL Fact
                if (children.size() < 3) return nullptr;
                auto left = static_pointer_cast<ExpressionNode>(children[0]);
                auto right = static_pointer_cast<ExpressionNode>(children[2]);
                return make_shared<BinaryOpNode>("*", left, right);
            }
            
            case 48: { // Fact -> ID LBRACK Expr RBRACK
                if (children.size() < 4) return nullptr;
                auto arrayId = static_pointer_cast<ExpressionNode>(children[0]);
                auto index = static_pointer_cast<ExpressionNode>(children[2]);
                auto arrayAccess = make_shared<BinaryOpNode>("[]", arrayId, index);
                arrayAccess->type = NodeType::ARRAY_ACCESS;
                return arrayAccess;
            }
            
            case 49: { // Fact -> INT_NUM
                if (children.size() < 1) {
                    cerr << "错误：产生式49 (Fact -> INT_NUM) 缺少子节点" << endl;
                    return nullptr;
                }
                return children[0]; // 返回数字字面量
            }
            
            case 50: { // Fact -> FLOAT_NUM
                if (children.size() < 1) return nullptr;
                return children[0]; // 返回浮点数字面量
            }
            
            case 51: { // Fact -> LPAR Expr RPAR
                if (children.size() < 3) return nullptr;
                return children[1]; // 返回括号中的表达式
            }
            
            case 52: { // ArgList -> ArgList COMMA Expr
                if (children.size() < 3) return nullptr;
                auto argList = make_shared<CompoundStmtNode>();
                auto existingArgs = getArgumentList(children[0]);
                auto newArg = static_pointer_cast<ExpressionNode>(children[2]);
                for (auto arg : existingArgs) {
                    argList->addStatement(make_shared<AssignmentNode>(nullptr, arg));
                }
                argList->addStatement(make_shared<AssignmentNode>(nullptr, newArg));
                return argList;
            }
            
            case 53: { // ArgList -> Expr
                if (children.size() < 1) return nullptr;
                auto argList = make_shared<CompoundStmtNode>();
                auto expr = static_pointer_cast<ExpressionNode>(children[0]);
                argList->addStatement(make_shared<AssignmentNode>(nullptr, expr));
                return argList;
            }
            
            case 54: { // ArgList -> ε
                return make_shared<CompoundStmtNode>();
            }
            
            default:
                cerr << "错误：未实现的产生式编号 " << prodNum << endl;
                cerr << "子节点数量: " << children.size() << endl;
                for (size_t i = 0; i < children.size(); ++i) {
                    if (children[i]) {
                        cerr << "  子节点[" << i << "]: " << nodeTypeToString(children[i]->type) << endl;
                    } else {
                        cerr << "  子节点[" << i << "]: nullptr" << endl;
                    }
                }
                return nullptr;
        }
    }
    
    // 辅助函数实现
    DataType getDataTypeFromNode(shared_ptr<ASTNode> node) {
        if (!node) {
            cerr << "警告：getDataTypeFromNode 收到空节点" << endl;
            return DataType::UNKNOWN;
        }
        if (auto literal = static_pointer_cast<LiteralNode>(node)) {
            return literal->dataType;
        }
        cerr << "警告：无法从节点类型 " << nodeTypeToString(node->type) << " 获取数据类型" << endl;
        return DataType::UNKNOWN;
    }
    
    string getIdentifierName(shared_ptr<ASTNode> node) {
        if (!node) {
            cerr << "警告：getIdentifierName 收到空节点" << endl;
            return "";
        }
        if (auto id = static_pointer_cast<IdentifierNode>(node)) {
            return id->name;
        }
        cerr << "警告：节点类型 " << nodeTypeToString(node->type) << " 不是标识符" << endl;
        return "";
    }
    
    string getLiteralValue(shared_ptr<ASTNode> node) {
        if (!node) {
            cerr << "警告：getLiteralValue 收到空节点" << endl;
            return "";
        }
        if (auto literal = static_pointer_cast<LiteralNode>(node)) {
            return literal->value;
        }
        cerr << "警告：节点类型 " << nodeTypeToString(node->type) << " 不是字面量" << endl;
        return "";
    }
    
    string getOperatorValue(shared_ptr<ASTNode> node) {
        // 操作符可能以不同方式表示，这里提供更完善的处理
        if (auto literal = static_pointer_cast<LiteralNode>(node)) {
            return literal->value;
        }
        if (auto identifier = static_pointer_cast<IdentifierNode>(node)) {
            // 某些操作符可能被识别为标识符
            string name = identifier->name;
            if (name == "ADD" || name == "+") return "+";
            if (name == "SUB" || name == "-") return "-";
            if (name == "MUL" || name == "*") return "*";
            if (name == "DIV" || name == "/") return "/";
            if (name == "LT" || name == "<") return "<";
            if (name == "LE" || name == "<=") return "<=";
            if (name == "GT" || name == ">") return ">";
            if (name == "GE" || name == ">=") return ">=";
            if (name == "EQ" || name == "==") return "==";
            if (name == "NE" || name == "!=") return "!=";
            return name;
        }
        return "";
    }
    
    vector<shared_ptr<VariableDeclNode>> getParameterList(shared_ptr<ASTNode> node) {
        vector<shared_ptr<VariableDeclNode>> params;
        if (auto compound = static_pointer_cast<CompoundStmtNode>(node)) {
            for (auto stmt : compound->statements) {
                if (auto param = static_pointer_cast<VariableDeclNode>(stmt)) {
                    params.push_back(param);
                }
            }
        }
        return params;
    }
    
    vector<shared_ptr<ExpressionNode>> getArgumentList(shared_ptr<ASTNode> node) {
        vector<shared_ptr<ExpressionNode>> args;
        if (auto compound = static_pointer_cast<CompoundStmtNode>(node)) {
            for (auto stmt : compound->statements) {
                if (auto assign = static_pointer_cast<AssignmentNode>(stmt)) {
                    if (assign->value) {
                        args.push_back(assign->value);
                    }
                }
            }
        }
        return args;
    }
};

#ifdef SEMANTIC_ANALYZER_MAIN
int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--debug" || string(argv[i]) == "-d") {
            DEBUG_MODE = true;
        }
    }
    
    SLRParser slrparser;
    slrparser.loadSLRTable();
    slrparser.parse(argv[1]);

    return 0;
}
#endif