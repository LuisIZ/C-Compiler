#include <iostream>
#include <string>
#include <cstring>
#include <list>
#include <unordered_map>

using namespace std;

// ----- TOKENS -----

class Token {
public:
    enum Type {
        PLUS, MINUS, MUL, DIV, NUM, ERR, RP, LP, END, SC, ID, PRINT, ASSIGN
    };
    static const char *token_names[13];
    Type type;
    string lexema;

    Token(Type);

    Token(Type, char c);

    Token(Type, const string &source, int first, int last);
};

const char *Token::token_names[13] = {"PLUS", "MINUS", "MUL", "DIV", "NUM", "ERR", "RP", "LP", "END", "SC", "ID",
                                      "PRINT", "ASSIGN"};

Token::Token(Type type) : type(type) { lexema = ""; }

Token::Token(Type type, char c) : type(type) { lexema = c; }

Token::Token(Type type, const string &source, int first, int last) : type(type) { lexema = source.substr(first, last); }

// -----

ostream &operator<<(ostream &outs, const Token &tok) {
    if (tok.lexema.empty()) {
        return outs << Token::token_names[tok.type];
    } else {
        return outs << Token::token_names[tok.type] << "(" << tok.lexema << ")";
    }
}

ostream &operator<<(ostream &outs, const Token *tok) {
    return outs << *tok;
}

// ----- SCANNER -----

class Scanner {
private:
    string input;
    int first, current;
public:
    Scanner(const char *input_string);

    Token *next_token();

    ~Scanner();
};

Scanner::Scanner(const char *input_string) : input(input_string), first(0), current(0) {}

Token *Scanner::next_token() {
    Token *token;

    while (input[current] == ' ') {
        current++;
    }
    if (input[current] == '\0') {
        return new Token(Token::END);
    }

    char c = input[current];
    first = current;

    if (isdigit(c)) {
        current++;
        while (isdigit(input[current])) {
            current++;
        }
        token = new Token(Token::NUM, input, first, current - first);
    } else if (isalpha(c)) {
        current++;
        while (isalnum(input[current])) {
            current++;
        }
        if ("print" == input.substr(first, current - first)) {
            token = new Token(Token::PRINT, input, first, current - first);
        } else {
            token = new Token(Token::ID, input, first, current - first);
        }
    } else if (strchr("+-*/();=", c)) {
        switch (c) {
            case '+':
                token = new Token(Token::PLUS, c);
                break;
            case '-':
                token = new Token(Token::MINUS, c);
                break;
            case '*':
                token = new Token(Token::MUL, c);
                break;
            case '/':
                token = new Token(Token::DIV, c);
                break;
            case '(':
                token = new Token(Token::LP, c);
                break;
            case ')':
                token = new Token(Token::RP, c);
                break;
            case ';':
                token = new Token(Token::SC, c);
                break;
            case '=':
                token = new Token(Token::ASSIGN, c);
                break;
            default:
                cout << "Error: Scanning an invalid character" << endl;
        }
        current++;
    } else {
        token = new Token(Token::ERR, c);
        current++;
    }
    return token;
}

Scanner::~Scanner() {}

// ----- AST -----

// -----

enum BinaryOp {
    PLUS, MINUS, MUL, DIV
};

// -----

class Exp {
public:
    virtual void print() = 0;

    virtual int eval() = 0;

    virtual ~Exp() = 0;

    static char bin_op_to_char(BinaryOp op);
};

char Exp::bin_op_to_char(BinaryOp op) {
    char c = ' ';
    switch (op) {
        case PLUS:
            c = '+';
            break;
        case MINUS:
            c = '-';
            break;
        case MUL:
            c = '*';
            break;
        case DIV:
            c = '/';
            break;
        default:
            c = '$';
    }
    return c;
}

Exp::~Exp() {}

// -----

class BinaryExp : public Exp {
public:
    Exp *left, *right;
    BinaryOp op;

    BinaryExp(Exp *l, Exp *r, BinaryOp op);

    void print();

    int eval();

    ~BinaryExp();
};

BinaryExp::BinaryExp(Exp *l, Exp *r, BinaryOp op) : left(l), right(r), op(op) {}

void BinaryExp::print() {
    left->print();
    char c = bin_op_to_char(this->op);
    cout << ' ' << c << ' ';
    right->print();
}

int BinaryExp::eval() {
    int result;
    int v1 = left->eval();
    int v2 = right->eval();
    switch (this->op) {
        case PLUS:
            result = v1 + v2;
            break;
        case MINUS:
            result = v1 - v2;
            break;
        case MUL:
            result = v1 * v2;
            break;
        case DIV:
            result = v1 / v2;
            break;
        default:
            cout << "Error: unrecognized character." << endl;
            result = 0;
    }
    return result;
}

BinaryExp::~BinaryExp() {
    delete left;
    delete right;
}

// -----

class NumberExp : public Exp {
public:
    int value;

    NumberExp(int v);

    void print();

    int eval();

    ~NumberExp();
};

NumberExp::NumberExp(int v) : value(v) {}

void NumberExp::print() {
    cout << value;
}

int NumberExp::eval() {
    return value;
}

NumberExp::~NumberExp() {}

// -----

class Stmt {
public:
    virtual void execute() = 0;

    virtual ~Stmt() = 0;
};

Stmt::~Stmt() {}

// -----

class Program {
private:
    list<Stmt *> slist;
public:
    Program();

    void add(Stmt *s);

    void interpreter();

    static unordered_map<string, int> memory;

    ~Program();
};

Program::Program() {}

void Program::add(Stmt *s) {
    slist.push_back(s);
}

void Program::interpreter() {
    for (Stmt *stmt: slist) {
        stmt->execute();
    }
}

Program::~Program() {
    for (Stmt *stmt: slist) {
        delete stmt;
    }
}

// -----

class AssignStm : public Stmt {
private:
    string id;
    Exp *right_side;
public:
    AssignStm(string id, Exp *e);

    void execute();

    ~AssignStm();
};

AssignStm::AssignStm(std::string id, Exp *e) : id(id), right_side(e) {};

void AssignStm::execute() {
    int value = right_side->eval();
    Program::memory[id] = value;
}

AssignStm::~AssignStm() {
    delete right_side;
}

// -----

class PrintStm : public Stmt {
private:
    Exp *e;
public:
    PrintStm(Exp *e);

    void execute();

    ~PrintStm();
};

PrintStm::PrintStm(Exp *e) : e(e) {}

void PrintStm::execute() {
    cout << e->eval() << endl;
}

PrintStm::~PrintStm() {
    delete e;
}

// -----

class IdExp : public Exp {
public:
    string id;

    IdExp(string id);

    void print();

    int eval();

    ~IdExp();
};

IdExp::IdExp(std::string id) : id(id) {}

void IdExp::print() {
    cout << id;
}

int IdExp::eval() {
    if (Program::memory.count(id) == 0) {
        cout << "Error: variable '" << id << "' not declared." << endl;
        exit(1);
    }
    return Program::memory[id];
}

IdExp::~IdExp() {}

// ----- PARSER -----

// -----

class Parser {
private:
    Scanner *scanner;
    Token *current, *previous;

    bool match(Token::Type token_type);

    bool check(Token::Type token_type);

    bool advance();

    bool is_at_end();

    Stmt *parse_stmt();

    Stmt *assign_stmt();

    Stmt *print_stmt();

    Exp *parse_expression();

    Exp *parse_term();

    Exp *parse_factor();

    bool token_to_operator(Token *tk, BinaryOp &op);

public:
    Parser(Scanner *scanner);

    Program *parse_program();
};

bool Parser::match(Token::Type token_type) {
    if (check(token_type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type token_type) {
    if (is_at_end()) {
        return false;
    }
    return current->type == token_type;
}

bool Parser::advance() {
    if (!is_at_end()) {
        Token *temp = current;
        if (previous) {
            delete previous;
        }
        current = scanner->next_token();
        previous = temp;
        if (check(Token::ERR)) {
            cout << "Error: Parsing an unrecognized character: " << current->lexema << endl;
            exit(0);
        }
        return true;
    }
    return false;
}

bool Parser::is_at_end() {
    return (current->type == Token::END);
}

Parser::Parser(Scanner *scanner) : scanner(scanner) {
    previous = nullptr;
    current = scanner->next_token();
    return;
}

// ----- PARSER (GRAMMAR)

Program *Parser::parse_program() { // Program ::= StmtList <-> Program ::= Stmt(´;´Stmt)*
    Program *p = new Program();
    p->add(parse_stmt());
    while (match(Token::SC)) {
        p->add(parse_stmt());
    }
    return p;
}

Stmt *Parser::parse_stmt() { // Stmt :: id '=' Exp | 'print' '(' Exp ')'
    Stmt *s = NULL;
    Exp *e;
    if (match(Token::ID)) {
        string lex = previous->lexema;
        if (!match(Token::ASSIGN)) {
            exit(0);
        }
        s = new AssignStm(lex, parse_expression());
    } else if (match(Token::PRINT)) {
        if (!match(Token::LP)) {
            exit(0);
        }
        e = parse_expression();
        if (!match(Token::RP)) {
            exit(0);
        }
        s = new PrintStm(e);
    } else {
        cout << "Error: Statement not found during Parsing" << endl;
        exit(0);
    }
    return s;
}

Exp *Parser::parse_expression() { // Exp ::= Term (('+' | '-') Term)*
    Exp *left = parse_term();
    while (match(Token::PLUS) || match(Token::MINUS)) {
        BinaryOp op;
        if (previous->type == Token::PLUS) {
            op = PLUS;
        } else if (previous->type == Token::MINUS) {
            op = MINUS;
        }
        Exp *right = parse_term();
        left = new BinaryExp(left, right, op);
    }
    return left;
}

Exp *Parser::parse_term() { // Term :: Factor (('*' | '/') Factor)*
    Exp *left = parse_factor();
    while (match(Token::MUL) || match(Token::DIV)) {
        BinaryOp op;
        if (previous->type == Token::MUL) {
            op = MUL;
        } else if (previous->type == Token::DIV) {
            op = DIV;
        }
        Exp *right = parse_factor();
        left = new BinaryExp(left, right, op);
    }
    return left;
}

Exp *Parser::parse_factor() { // Factor ::= id | Num | '(' Exp ')'
    Exp *e;
    if (match(Token::ID)) {
        return new IdExp(previous->lexema);
    } else if (match(Token::NUM)) {
        return new NumberExp(stoi(previous->lexema));
    } else if (match(Token::LP)) {
        e = parse_expression();
        if (!match(Token::RP)) {
            cout << "Error: right parenthesis not found during parsing" << endl;
            exit(0);
        }
        return e;
    }
    cout << "Error: a number was expected during parsing" << endl;
    exit(0);
}

// ----- OTROS -----

void test_scanner(Scanner *scanner) {
    Token *current;
    current = scanner->next_token();
    while (current->type != Token::END) {
        if (current->type == Token::ERR) {
            cout << "Error: Scanning an invalid token" << current->lexema << endl;
            break;
        } else {
            cout << current << endl;
        }
        current = scanner->next_token();
    }
    exit(1);
}

// -----

unordered_map<string, int> Program::memory;

// ----- MAIN -----

int main(int number_of_arguments, const char *lines_of_code[]) {
    if (number_of_arguments != 2) {
        cout << "Error: Incorrect number of arguments in console" << endl;
        exit(1);
    }

    Scanner scanner(lines_of_code[1]);

    // test_scanner(&scanner);

    Parser parser(&scanner);

    Program *prog = parser.parse_program();

    prog->interpreter();

    delete prog;

    return 0;
}