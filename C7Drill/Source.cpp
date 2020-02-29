/*
	calculator08buggy.cpp

	Helpful comments removed.

	We have inserted 3 bugs that the compiler will catch and 3 that it won't.  TEST

	2/23/20 - Have got this working properly and will use this as the base moving forward.

	2/28/20 - Modify the calculator program from Chapter 7 to make the input stream 
	an explicit parameter (as shown in §8.5.8), rather than simply using cin. Also 
	give the Token_stream constructor (§7.8.2) an istream& parameter so that when we 
	figure out how to make our own istreams (e.g., attached to files), we can use the 
	calculator for those. Hint: Don’t try to copy an istream. 
	    2/29/20 - done though I don't quite get the syntax for t the initializaers for Token_Stream

*/

#include "C:\Users\mark.alexieff\source\repos\std_lib_facilities.h"

struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) :kind(ch), value(0) { }					/* Single kind initializer*/
	Token(char ch, double val) :kind(ch), value(val) { }	/* initializer for values*/
	Token(char ch, string n) :kind(ch), name(n) { }			/* inititalizer for variables*/
};

class Token_stream {
	istream& input_stream;
	bool full;
	Token buffer;
public:
	Token_stream() :input_stream(cin),full(0), buffer(0) { }					// Initilizer
	Token_stream(istream& i_s) :full(0), buffer(0), input_stream(i_s) {}		//Initializaer with stream

	Token get();											//function that returns a token
	void unget(Token t) { buffer = t; full = true; }        // puts a token in the buffer

	void ignore(char);										//not sure
};

const char let = 'L';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';

Token Token_stream::get()
{
	if (full) { full = false; return buffer; }      //if there is sometthing in the buffer return it
	char ch;
	cin >> ch;
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':      //Modulo not implemented
	case ';':
	case '=':
	case quit:     // "Q"
		return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	cin.unget();			// not to be confused with Token.unget - this is a cin method 
	double val;							// that puts the last char read back onto the input buffer
	cin >> val;
	return Token(number, val);
	}
	default:
		if (isalpha(ch)) {			// Captures strings for var names with checks for LET and QUIT
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch))) s += ch;
			cin.unget();
			if (s == "let") return Token(let);
			if (s == "quit") return Token(quit);
			return Token(name, s);
		}
		error("Bad token");
	}
}

void Token_stream::ignore(char c)   /*have no idea what this is for*/
{
	if (full && c == buffer.kind) {   //clears the buffer if token of the same kind in buffer
		full = false;
		return;
	}
	full = false;

	char ch;
	while (cin >> ch)				// reads the buffer until ch does not = the character parameter
		if (ch == c) return;
}

struct Variable {
	string name;
	double value;
	Variable(string n, double v) :name(n), value(v) { }
};

vector<Variable> names;

double get_value(string s)			/* finds value of a named variable*/
{
	for (int i = 0; i <= names.size(); ++i)
		if (names[i].name == s) return names[i].value;
	error("get: undefined name ", s);
}

void set_value(string s, double d)     /* sets the value of a variable*/
{
	for (int i = 0; i <= names.size(); ++i)
		if (names[i].name == s) {
			names[i].value = d;
			return;
		}
	error("set: undefined name ", s);
}

bool is_declared(string s)				/*cehcks to see if a name has already been declared for a variable*/
{
	for (int i = 0; i < names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}

Token_stream ts;

double expression(Token_stream& ts);

double primary(Token_stream& ts)
{
	Token t = ts.get();
	double d{ 0 };
	switch (t.kind) {
	case '(':									//returns value of paranthetical expression
		{ d = expression(ts);
		t = ts.get();
		if (t.kind != ')') error("')' expected");  //works much better - key was understadning that
		return d;									// term and expression unget the last thing they read
	}
	case '-':									//returns negative of the following 
		return -primary(ts);
	case '+':
		return primary(ts);
	case number:
		return t.value;							//return the value if a number
	case name:
		return get_value(t.name);				//returns the value of a variable
	default:
		error("primary expected");
	}
}

double term(Token_stream& ts)								//handles mutliplication and division of primaries
{
	double left = primary(ts);
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary(ts);
			break;					
		case '/':
		{	double d = primary(ts);
			if (d == 0) error("divide by zero");
			left /= d;
			break;
		}
		case '%':
		{	double d = primary(ts);
			if (d == 0) error("%:divide by zero");
			left = fmod(left, d);
			break;
		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

double expression(Token_stream& ts)				//handles addition and subtraction
{
	double left = term(ts);
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term(ts);
			break;
		case '-':
			left -= term(ts);
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}

double declaration(Token_stream& ts)
{
	Token t = ts.get();
	if (t.kind != 'a') error("name expected in declaration");
	string name = t.name;
	if (is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name);
	double d = expression(ts);
	names.push_back(Variable(name, d));
	return d;
}

double statement(Token_stream& ts)
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
		return declaration(ts);
	default:
		ts.unget(t);
		return expression(ts);
	}
}

void clean_up_mess(Token_stream& ts)
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate(Token_stream& ts)
{
	while (true) try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get();   //burns multiple ';'s
		if (t.kind == quit) {
			keep_window_open("~~");
			return;
		}
		ts.unget(t);
		cout << result << statement(ts) << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess(ts);
	}
}

int main()

try {
	calculate(ts);
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');   //burns the rest of cin until ;
	keep_window_open("~~");
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	keep_window_open("~~");
	return 2;
}


