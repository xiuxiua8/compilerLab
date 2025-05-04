# 定义图中的状态转换为有限自动机描述格式
alphabet = set()
states = set()
transitions = []

# 图中的状态与状态间的转移（手动从图提取）
# 形式为 (current_state, input_symbol, next_state)
raw_transitions = [
    ("0", ";", "2"),
    ("0", "-", "A"),
    ("0", "+", "4A"),
    ("0", ".", "C"),
    ("A", ".", "C"),
    ("AB", ".", "CD"),
    ("4A", ".", "C"),
    ("0", *list("0123456789"), "AB"),
    ("0", *list("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), "8"),
    ("AB", *list("0123456789"), "AB"),
    ("AB", "e", "EF"),
    ("AB", "E", "EF"),
    ("A", *list("0123456789"), "AB"),
    ("C", *list("0123456789"), "CD"),
    ("CD", *list("0123456789"), "CD"),
    ("CD", "e", "EF"),
    ("CD", "E", "EF"),
    ("EF", *list("0123456789"), "FG"),
    ("EF", "+", "F"),
    ("EF", "-", "F"),
    ("F", *list("0123456789"), "FG"),
    ("FG", *list("0123456789"), "FG"),
    ("4A", "=", "5"),
    ("4A", "+", "6"),
    ("8", *list("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"), "8"),
]

# 扩展 transitions
for tran in raw_transitions:
    from_state = tran[0]
    symbols = tran[1:-1]
    to_state = tran[-1]
    for symbol in symbols:
        alphabet.add(symbol)
        states.add(from_state)
        states.add(to_state)
        transitions.append((from_state, symbol, to_state))

# 添加接受状态（从图判断）
accept_states = {"2", "5", "6", "8", "AB", "CD", "FG"}

# 转换为文本格式
lines = []
lines.append("alphabet: " + " ".join(sorted(alphabet)))
lines.append("states: " + " ".join(sorted(states)))
lines.append("start: 0")
lines.append("accept: " + " ".join(sorted(accept_states)))
lines.append("transition:")

for from_state, symbol, to_state in transitions:
    lines.append(f"{from_state} {symbol} {to_state}")

# 合并成一个文本块输出
output = "\n".join(lines)
#output[:3000]  # Show partial output if too long

print(output)

# 将文本保存到文件
with open("dfa.txt", "w") as file:
    file.write(output)

print("DFA 已保存到 dfa.txt")

