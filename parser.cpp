#include "parser.hpp"
#include "lexer.hpp"
#include <string>
#include <vector>
#include <map>

struct Func {
    struct Out {
        bool error;
        std::string out;
    };

    static Out set(std::vector<Lexer> code) {
        if (code.empty()) return {.error = true};
        bool right = false;
        bool name = false;
        std::string number1 = "0";

        std::string data[3] = {"", "", ""};

        for (size_t size = 0; size < code.size(); size++) {
            if (code[size].lexerdata == "=" && (!right)) {
                if (size + 1 < code.size()) {
                    data[2] = "\tmov %0 " + code[size+1].lexerdata + ";";
                    size++;
                }
                right = true;
                continue;
            }

            if (code[size].lexerdata == ":" && (!right && !name)) {
                name = true;
                continue;
            }

            if (!right) {
                if (name) data[1] = code[size].lexerdata;
                else data[0] += code[size].lexerdata;
            }

            if (right) {
                std::string number2 = "__ERR__";
                std::string op = "__ERR__";
                if (size + 1 < code.size()) {
                    op = code[size].lexerdata;
                    number2 = code[size+1].lexerdata;

                    if (op == "+") data[2] += "\n\tadd %" + number1 + " " + number2 + ";";
                    else if (op == "-") data[2] += "\n\tdec %" + number1 + " " + number2 + ";";
                    else if (op == "*") data[2] += "\n\tmul %" + number1 + " " + number2 + ";";
                    else if (op == "/") data[2] += "\n\tdiv %" + number1 + " " + number2 + ";";
                }
            }
        }

        std::string out = "";
        if (name) {
            if (data[0].empty() || data[1].empty()) return {.error = true};
            out += "new " + data[0] + " " + data[1] + ";";

            if (right) {
                if (data[2].empty() || (!right)) return {.error = true};
                out += "\nset " + data[1] + " {\n" + data[2] + "\n};";
            }
        } else {
            if (data[0].empty()) return {.error = true};

            if (data[0].back() == '~') out += "del " + data[0].substr(0, data[0].size() - 1) + ";";
            else {
                if (data[2].empty() || !right) return {.error = true};
                out += "set " + data[0] + " {\n" + data[2] + "\n};";
            }
        }

        return {.error = false, .out = out};
    }

    struct If {
        std::string left, op1, right, op2;
    };

    static Out equal(std::vector<Lexer> code) {
        if (code.empty()) return {.error = true};

        if (code[0].lexerdata != "if") return {.error = true};
        
        code.erase(code.begin());
        std::vector<Func::If> datas;
        Func::If data;
        bool right = false;

        for (size_t size = 0; size < code.size(); size++) {
            if (code[size].lexerdata == "==" ||
                code[size].lexerdata == "!=" ||
                code[size].lexerdata == "&&" ||
                code[size].lexerdata == "||") {

                if (data.op1.empty()) {
                    data.op1 = code[size].lexerdata;
                    right = true;
                } else if (data.op2.empty()) {
                    data.op2 = code[size].lexerdata;
                    right = false;
                    datas.push_back(data);
                    data.op1 = "";
                    data.op2 = "";
                    data.left = "";
                    data.right = "";
                }
                continue;
            }

            if (right) data.right += code[size].lexerdata;
            else data.left += code[size].lexerdata;
        }

        if (!data.left.empty() || !data.right.empty() || !data.op1.empty() || !data.op2.empty()) datas.push_back(data);

        std::string out = "if {\n";
        std::string opp2 = "&&";

        for (auto& [left, op1, right, op2] : datas) {
            if (right.empty()) right = "1";
            out += "\tcmp " + left + " " + right + ";\n";
            if (opp2 == "||") {
                if (op1 == "==") out += "\tje true;\n";
                else if (op1 == "!=") out += "\tjne true;\n";
                else if (op1 == "&&") out += "\tand true;\n";
                else if (op1 == "||") out += "\tor true;\n";

                if (!opp2.empty() && opp2 != op2) out += "\tjmp false;\n";
                else opp2 = op2;
            } else if (opp2 == "&&") {
                if (op1 == "==" || op1 == "&&") out += "\tjne false;\n";
                else if (op1 == "!=" || op1 == "||") out += "\tje false;\n";

                if (!opp2.empty() && opp2 != op2) out += "\tjmp true;\n";
                else opp2 = op2;
            }
            if (!op2.empty()) opp2 = op2;
        }

        out += "};";

        return {.error = false, .out = out};
    }

    static Out loop(std::vector<Lexer> code) {
        if (code.empty()) return {.error = true};
        
        if (code[0].lexerdata != "while") return {.error = true};
        
        code.erase(code.begin());
        std::vector<Func::If> datas;
        Func::If data;
        bool right = false;

        for (size_t size = 0; size < code.size(); size++) {
            if (code[size].lexerdata == "==" ||
                code[size].lexerdata == "!=" ||
                code[size].lexerdata == "&&" ||
                code[size].lexerdata == "||") {

                if (data.op1.empty()) {
                    data.op1 = code[size].lexerdata;
                    right = true;
                } else if (data.op2.empty()) {
                    data.op2 = code[size].lexerdata;
                    right = false;
                    datas.push_back(data);
                    data.op1 = "";
                    data.op2 = "";
                    data.left = "";
                    data.right = "";
                }
                continue;
            }

            if (right) data.right += code[size].lexerdata;
            else data.left += code[size].lexerdata;
        }

        if (!data.left.empty() || !data.right.empty() || !data.op1.empty() || !data.op2.empty()) datas.push_back(data);

        std::string out = "while {\n";
        std::string opp2 = "&&";

        for (auto& [left, op1, right, op2] : datas) {
            if (right.empty()) right = "1";
            out += "\tcmp " + left + " " + right + ";\n";
            if (opp2 == "||") {
                if (op1 == "==") out += "\tje true;\n";
                else if (op1 == "!=") out += "\tjne true;\n";
                else if (op1 == "&&") out += "\tand true;\n";
                else if (op1 == "||") out += "\tor true;\n";

                if (!opp2.empty() && opp2 != op2) out += "\tjmp false;\n";
                else opp2 = op2;
            } else if (opp2 == "&&") {
                if (op1 == "==" || op1 == "&&") out += "\tjne false;\n";
                else if (op1 == "!=" || op1 == "||") out += "\tje false;\n";

                if (!opp2.empty() && opp2 != op2) out += "\tjmp true;\n";
                else opp2 = op2;
            }
            if (!op2.empty()) opp2 = op2;
        }

        out += "};";

        return {.error = false, .out = out};
    }
} static Func;

std::map<int, std::vector<std::string>> parserfunc(std::map<int, std::vector<Lexer>> code) {
    std::map<int, std::vector<std::string>> out;

    for (const auto& [line, parser] : code) {        
        std::vector<std::string> tokens;
        Func::Out token;

        token = Func::set(parser);
        if (!token.error) {
            tokens.push_back(token.out);
            out[line] = tokens;
            continue;
        }

        token = Func::equal(parser);
        if (!token.error) {
            tokens.push_back(token.out);
            out[line] = tokens;
            continue;
        }

        token = Func::loop(parser);
        if (!token.error) {
            tokens.push_back(token.out);
            out[line] = tokens;
            continue;
        }

        if (token.error) tokens.push_back("error(line/expression: " + std::to_string(line+1) + ");");

        out[line] = tokens;
    }

    return out;
}
