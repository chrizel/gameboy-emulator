#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>

#include <boost/algorithm/string.hpp>

using namespace std;

static map<string,string> byteArguments;
static map<string,string> wordArguments;
static map<string,string> condArguments;
static vector<string> condOps;

string extractInstructionsetName(string fileName)
{
    int start = 0;
    int end = fileName.length()-1;

    // Find end...
    for (int i = end; i >= 0; --i) {
        char c = fileName[i];
        if (c == '/' || c == '\\')
            break;
        if (c == '.') {
            end = i-1;
            break;
        }
    }

    // Find start...
    for (int i = end; i >= 0; --i) {
        char c = fileName[i];
        if (c == '/' || c == '\\') {
            start = i+1;
            break;
        }
    }

    if (start >= end) {
        cerr << "Cannot extract instructionset name from string '" << fileName << "'" << endl;
        exit(1);
    }

    return fileName.substr(start, end-start+1);
}

void split(const string &str, char sep, vector<string> &v)
{
    v.clear();

    int i = 0;
    int len = str.size();
    while (i < len) {
        while (i < len && (str[i] == sep))
            ++i;

        string buf;
        while (i < len && !(str[i] == sep)) {
            buf += str[i];
            ++i;
        }

        v.push_back(buf);
    }
}

string codeForArgument(const string &argument)
{
    if (byteArguments.find(argument) != byteArguments.end())
        return byteArguments[argument];
    if (wordArguments.find(argument) != wordArguments.end())
        return wordArguments[argument];

    cerr << "Cannot find code for argument '" << argument << "'" << endl;
    exit(1);
}

string generateInstructionCodeForLine(const string &line)
{
    if (line.length() == 0 || line[0] == '#')
        return "";

    stringstream output;

    vector<string> items;
    split(line, ' ', items);

    if (items.size() < 4) {
        cerr << "Error in syntax of '" << line << "'" << endl;
        exit(1);
    }

    vector<string> args;
    if (items.size() > 4)
        split(items[4], ',', args);

    string mnemonic = items[3];
    string assembly = items[3] + (items.size() > 4 ? " " + items[4] : "");
    string code = items[0];
    string length = items[1];
    string condition;

    vector<string> cycles;
    split(items[2], '/', cycles);
    string cycles0 = cycles.size() > 0 ? cycles[0] : "0";
    string cycles1 = cycles.size() > 1 ? cycles[1] : "0";

    if (args.size() > 0 &&
        find(condOps.begin(), condOps.end(), mnemonic) != condOps.end() &&
        condArguments.find(args[0]) != condArguments.end()) 
    {
        condition = condArguments[args[0]];
        args.erase(args.begin());
    }

    string arg;
    if (args.size() > 0 && (args[0][0] >= '0' && args[0][0] <= '9')) {
        arg = args[0];
        args.erase(args.begin());
        if (arg[arg.size()-1] == 'H')
            arg = "0x" + arg.substr(0, arg.size()-1);
    }

    string op;
    string templateArgument;
    if (args.size() > 0) {
        // TODO
        if (byteArguments.find(args[0]) != byteArguments.end()) {
            templateArgument = "<byte>";
            op = "op1";
        } else {
            templateArgument = "<word>";
            op = "op2";
        }
    } else {
        op = "op0";
    }

    output << "    // " << line << endl;
    output << "    " << op << " = new " << mnemonic << "_Instruction" << templateArgument << "();" << endl
           << "    " << op << "->cpu = cpu;" << endl
           << "    " << op << "->code = 0x" << code << ";" << endl
           << "    " << op << "->length = " << length << ";" << endl
           << "    " << op << "->cycles0 = " << cycles0 << ";" << endl
           << "    " << op << "->cycles1 = " << cycles1 << ";" << endl
           << "    " << op << "->mnemonic = \"" << assembly << "\";" << endl;

    if (condition != "") {
        output << "    " << op << "->condition = " << condition << ";" << endl;
    }

    if (arg != "") {
        output << "    " << op << "->arg = " << arg << ";" << endl;
    }

    int i = 0;
    for (vector<string>::iterator it = args.begin(); it != args.end(); ++it) {
        output << "    " << op << "->ref" << i << " = " << codeForArgument(*it) << ";" << endl;
        ++i;
    }
    output << "    instructionSet->add(" << op << ");" << endl;

    return output.str();
}

static void init()
{
    byteArguments["(BC)"]  = "new MemoryReference<byte>(cpu, cpu->bc)";
    byteArguments["(C)"]   = "new Memory_SingleRegister_Reference<byte>(cpu, cpu->c)";
    byteArguments["(DE)"]  = "new MemoryReference<byte>(cpu, cpu->de)";
    byteArguments["(HL)"]  = "new MemoryReference<byte>(cpu, cpu->hl)";
    byteArguments["(HL+)"] = "new MemoryReference<byte>(cpu, cpu->hl, 1)";
    byteArguments["(HL-)"] = "new MemoryReference<byte>(cpu, cpu->hl, -1)";
    byteArguments["(a8)"]  = "new Memory_a8_Reference<byte>(cpu, cpu->pc)";
    byteArguments["(a16)"] = "new Memory_a16_Reference<byte>(cpu, cpu->pc)";
    byteArguments["A"]     = "new RegisterReference<byte>(cpu->a)";
    byteArguments["B"]     = "new RegisterReference<byte>(cpu->b)";
    byteArguments["C"]     = "new RegisterReference<byte>(cpu->c)";
    byteArguments["D"]     = "new RegisterReference<byte>(cpu->d)";
    byteArguments["E"]     = "new RegisterReference<byte>(cpu->e)";
    byteArguments["F"]     = "new RegisterReference<byte>(cpu->f)";
    byteArguments["H"]     = "new RegisterReference<byte>(cpu->h)";
    byteArguments["L"]     = "new RegisterReference<byte>(cpu->l)";
    byteArguments["d8"]    = "new MemoryReference<byte>(cpu, cpu->pc)";
    byteArguments["r8"]    = "new MemoryReference<byte>(cpu, cpu->pc)";

    wordArguments["SP"]    = "new RegisterReference<word>(cpu->sp)";
    wordArguments["AF"]    = "new RegisterReference<word>(cpu->af)";
    wordArguments["BC"]    = "new RegisterReference<word>(cpu->bc)";
    wordArguments["DE"]    = "new RegisterReference<word>(cpu->de)";
    wordArguments["HL"]    = "new RegisterReference<word>(cpu->hl)";
    wordArguments["a16"]   = "new MemoryReference<word>(cpu, cpu->pc)";
    wordArguments["d16"]   = "new MemoryReference<word>(cpu, cpu->pc)";

    condArguments["NZ"]    = "new NZ_Condition()";
    condArguments["NC"]    = "new NC_Condition()";
    condArguments["Z"]     = "new Z_Condition()";
    condArguments["C"]     = "new C_Condition()";

    condOps.push_back("JR");
    condOps.push_back("JP");
    condOps.push_back("RET");
    condOps.push_back("CALL");
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input file> <output file>" << endl;
        return 1;
    }

    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cerr << "Cannot open file " << argv[1] << endl;
        return 1;
    }

    ofstream outfile(argv[2]);
    if (!outfile.is_open()) {
        cerr << "Cannot open file " << argv[2] << endl;
        return 1;
    }

    init();

    string instructionsetName = extractInstructionsetName(argv[2]);
    string upperInstructionsetName = boost::to_upper_copy(instructionsetName);

    outfile << "#ifndef " << upperInstructionsetName << "_H" << endl
            << "#define " << upperInstructionsetName << "_H" << endl
            << endl
            << "#include \"cpu.h\"" << endl
            << "#include \"instructions.h\"" << endl
            << "#include \"instructionset.h\"" << endl
            << "#include \"references.h\"" << endl
            << endl
            << "void initialize_" << instructionsetName << "(InstructionSet *instructionSet, CPU *cpu) {" << endl
            << "    Instruction *op0;" << endl
            << "    ReferenceInstruction<byte> *op1;" << endl
            << "    ReferenceInstruction<word> *op2;" << endl
            << endl;

    string line;
    while (infile.good()) {
        getline(infile, line);
        outfile << generateInstructionCodeForLine(line) << endl;
    }

    outfile << "}" << endl
            << endl
            << "#endif" << endl;

    return 0;
}
