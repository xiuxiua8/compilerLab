#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <queue>

using namespace std;

class DFA {
    set<string> alphabet;
    set<string> states;
    string startState;
    set<string> acceptStates;
    map<pair<string, string>, string> transitions;

public:
    bool loadFromFile(const string& filename);
    bool validate();
    bool simulate(const string& input);
    void generateLanguage(int maxLength);

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
            string state;
            while (iss >> state) acceptStates.insert(state);
        } else if (line.find("transition:") == 0) {
            while (getline(file, line) && !line.empty()) {
                istringstream iss(line);
                string from, symbol, to;
                if (iss >> from >> symbol >> to) {
                    transitions[{from, symbol}] = to;
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

int main() {
    DFA dfa;
    if (!dfa.loadFromFile("dfa.txt")) {
        cout << "无法打开 DFA 配置文件。\n";
        return 1;
    }

    if (!dfa.validate()) {
        return 1;
    }

    int N;
    cout << "请输入最大字符串长度 N：";
    cin >> N;
    dfa.generateLanguage(N);

    string testStr;
    cout << "请输入要测试的字符串：";
    cin >> testStr;
    if (dfa.simulate(testStr)) {
        cout << "该字符串被接受。\n";
    } else {
        cout << "该字符串被拒绝。\n";
    }

    return 0;
}
