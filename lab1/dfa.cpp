#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>

using namespace std;

class DFA {
    set<string> alphabet;
    set<string> states;
    string startState;
    set<string> acceptStates;
    map<pair<string, string>, string> transitions;
    map<string, string> stateTypes; // 存储状态和对应的类型
    set<string> keywords; // 存储关键字集合

public:
    bool loadFromFile(const string& filename);
    bool validate();
    bool simulate(const string& input);
    void generateLanguage(int maxLength);
    set<string> getAcceptStates() const { return acceptStates; }
    string getEndState(const string& input) const;
    string getStateType(const string& state) const;
    string classifyToken(const string& type, const string& token) const;
    void initKeywords();

private:
    void generateAllStrings(const string& current, int maxLength, vector<string>& results);
};

// 读取 DFA 配置
bool DFA::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file) return false;

    string line;
    while (getline(file, line)) {
        if (line.find("alphabet:") == 0) {
            istringstream iss(line.substr(9));
            string symbol;
            while (iss >> symbol) alphabet.insert(symbol);
        } else if (line.find("states:") == 0) {
            istringstream iss(line.substr(7));
            string state;
            while (iss >> state) states.insert(state);
        } else if (line.find("start:") == 0) {
            startState = line.substr(6);
            startState.erase(0, startState.find_first_not_of(" \t"));
        } else if (line.find("accept:") == 0) {
            istringstream iss(line.substr(7));
            string state, type;
            while (iss >> state) {
                // 尝试读取状态对应的类型
                if (iss.peek() == ':') {
                    iss.ignore();
                    iss >> type;
                    stateTypes[state] = type;
                }
                acceptStates.insert(state);
            }
        } else if (line.find("transition:") == 0) {
            while (getline(file, line) && !line.empty()) {
                istringstream iss(line);
                string from, symbol, to;
                if (iss >> from >> symbol >> to) {
                    transitions[{from, symbol}] = to;
                }
            }
        } else if (line.find("types:") == 0) {
            // 额外的类型定义部分
            while (getline(file, line) && !line.empty()) {
                istringstream iss(line);
                string state, type;
                if (iss >> state >> type) {
                    stateTypes[state] = type;
                }
            }
        }
    }
    return true;
}

// 检查 DFA 合法性
bool DFA::validate() {
    if (states.find(startState) == states.end()) {
        cout << "错误：开始状态不在状态集中。\n";
        return false;
    }
    if (acceptStates.empty()) {
        cout << "错误：接受状态集为空。\n";
        return false;
    }
    for (const auto& state : acceptStates) {
        if (states.find(state) == states.end()) {
            cout << "错误：接受状态 " << state << " 不在状态集中。\n";
            return false;
        }
    }
    return true;
}

// 模拟 DFA 识别过程
bool DFA::simulate(const string& input) {
    string current = startState;
    for (char c : input) {
        string symbol(1, c);
        if (alphabet.find(symbol) == alphabet.end()) return false;
        auto it = transitions.find({current, symbol});
        if (it == transitions.end()) return false;
        current = it->second;
    }
    return acceptStates.count(current) > 0;
}

// 构造语言集中所有长度≤N的规则字符串
void DFA::generateLanguage(int maxLength) {
    vector<string> results;
    generateAllStrings("", maxLength, results);

    cout << "语言集中长度≤" << maxLength << "的字符串：\n";
    for (const auto& str : results) {
        if (simulate(str)) {
            cout << str << "\n";
        }
    }
}

void DFA::generateAllStrings(const string& current, int maxLength, vector<string>& results) {
    if (current.length() > maxLength) return;
    results.push_back(current);
    for (const auto& symbol : alphabet) {
        generateAllStrings(current + symbol, maxLength, results);
    }
}

// 模拟DFA并返回最终状态
string DFA::getEndState(const string& input) const {
    string current = startState;
    for (char c : input) {
        string symbol(1, c);
        if (alphabet.find(symbol) == alphabet.end()) return "ERROR";
        auto it = transitions.find({current, symbol});
        if (it == transitions.end()) return "ERROR";
        current = it->second;
    }
    return current;
}

// 获取状态对应的类型
string DFA::getStateType(const string& state) const {
    auto it = stateTypes.find(state);
    if (it != stateTypes.end()) {
        return it->second;
    }
    // 这里可以根据状态名称推断类型
    // 例如：如果状态名包含"ID"，则返回"ID"
    //0 2 4A 5 6 8 A AB C CD EF F FG
    if (state.find("0") != string::npos) return "0";
    if (state.find("2") != string::npos) return "SCO";   
    if (state.find("4A") != string::npos) return "ADD";
    if (state.find("5") != string::npos) return "AAS";
    if (state.find("6") != string::npos) return "AAA";
    //if (state.find("8") != string::npos) return "ID/INT";
    if (state.find("8") != string::npos) return "ID";
    //if (state.find("A") != string::npos) return "SUB";
    if (state.find("AB") != string::npos) return "NUM";
    //if (state.find("C") != string::npos) return "C";
    if (state.find("CD") != string::npos) return "FLO";
    if (state.find("EF") != string::npos) return "EF";
    //if (state.find("F") != string::npos) return "F";
    if (state.find("FG") != string::npos) return "FLO";
    //DIV MUL ASG LPA RPA LBK RBK LBR RBR CMA SCO ROP
    if (state.find("DIV") != string::npos) return "DIV";
    if (state.find("MUL") != string::npos) return "MUL";
    if (state.find("ASG") != string::npos) return "ASG";
    if (state.find("LPA") != string::npos) return "LPA";
    if (state.find("RPA") != string::npos) return "RPA";
    if (state.find("LBK") != string::npos) return "LBK";
    if (state.find("RBK") != string::npos) return "RBK";
    if (state.find("LBR") != string::npos) return "LBR";
    if (state.find("RBR") != string::npos) return "RBR";
    if (state.find("CMA") != string::npos) return "CMA";
    //if (state.find("SCO") != string::npos) return "SCO";
    if (state.find("ROP") != string::npos) return "ROP";
    
    return "UNKNOWN";
}

// 初始化关键字集合
void DFA::initKeywords() {
    // 添加C语言常见关键字
    keywords.insert("if");
    keywords.insert("else");
    keywords.insert("while");
    keywords.insert("for");
    keywords.insert("do");
    keywords.insert("int");
    keywords.insert("float");
    keywords.insert("double");
    keywords.insert("char");
    keywords.insert("void");
    keywords.insert("return");
    keywords.insert("break");
    keywords.insert("continue");
    keywords.insert("switch");
    keywords.insert("case");
    keywords.insert("default");
    keywords.insert("typedef");
    keywords.insert("struct");
    keywords.insert("union");
    keywords.insert("const");
    // 可以根据需要添加更多关键字
}

// 分类词法单元：普通标识符还是关键字
string DFA::classifyToken(const string& type, const string& token) const {
    string result;
    
    // 如果是ID类型，检查是否为关键字
    if (type == "ID" && keywords.count(token) > 0) {
        result = token; // 关键字本身作为类型
    } else {
        // 其他情况返回原始类型
        result = type;
    }
    
    // 将结果转换为大写
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

int main() {
    DFA dfa;
    // 初始化关键字表
    dfa.initKeywords();
    
    if (!dfa.loadFromFile("dfa.txt")) {
        cout << "无法打开 DFA 配置文件。\n";
        return 1;
    }

    if (!dfa.validate()) {
        return 1;
    }

    int mode;
    cout << "请选择运行模式 (1: 批量分析符号串, 2: 词法分析): ";
    cin >> mode;

    if (mode == 1) {
        int n;
        cout << "请输入符号串个数: ";
        cin >> n;
        
        vector<pair<string, string>> results; // 存储类型和原字符串对
        string token;
        cout << "请输入" << n << "个用空格分隔的符号串: ";
        
        for (int i = 0; i < n; i++) {
            cin >> token;
            string endState = dfa.getEndState(token);
            if (endState != "ERROR" && dfa.getAcceptStates().count(endState) > 0) {
                string type = dfa.getStateType(endState);
                // 使用两阶段处理：先识别词法单元形态，再判断是否为关键字
                type = dfa.classifyToken(type, token);
                results.push_back({type, token});
            } else {
                results.push_back({"ERROR", token});
            }
        }
        
        // 输出所有符号串的类型和原字符串
        for (const auto& result : results) {
            cout << " (" << result.first << ", " << result.second << ") ";
            cout << endl;
        }
        cout << endl;
    } else if (mode == 2) {
        string line;
        cout << "请输入一行语句进行词法分析: ";
        cin.ignore(); // 清除输入缓冲区
        getline(cin, line);
        
        istringstream iss(line);
        string token;
        vector<pair<string, string>> results; // 存储类型和原字符串对
        
        while (iss >> token) {
            string endState = dfa.getEndState(token);
            if (endState != "ERROR" && dfa.getAcceptStates().count(endState) > 0) {
                string type = dfa.getStateType(endState);
                // 使用两阶段处理：先识别词法单元形态，再判断是否为关键字
                type = dfa.classifyToken(type, token);
                results.push_back({type, token});
            } else {
                results.push_back({"ERROR", token});
            }
        }
        
        // 输出所有符号串的类型和原字符串
        for (const auto& result : results) {
            cout << " (" << result.first << ", " << result.second << ") ";
            cout << endl;
        }
        cout << endl;
    } else {
        cout << "无效的运行模式，请选择1或2。\n";
    }

    return 0;
}
