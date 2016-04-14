#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;


// The set of terminal symbols in the WLPP grammar.
const char *terminals[] = {
  "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID",
  "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM",
  "PCT", "PLUS", "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI",
  "SLASH", "STAR", "WAIN", "WHILE", "AMP", "LBRACK", "RBRACK", "NEW",
  "DELETE", "NULL"
};


int isTerminal(const string &sym) {
  int idx;
  for(unsigned idx=0; idx<sizeof(terminals)/sizeof(char*); idx++)
    if(terminals[idx] == sym) return 1;
  return 0;
}

// Data structure for storing the parse tree.
class tree {
  public:
    string rule;
    string type;
    vector<string> tokens;
    vector<tree*> children;
    ~tree() { for(unsigned int i=0; i<children.size(); i++) delete children[i]; }
};


vector <tree*> ID_storage;
vector <string> declared_ID;
vector <string> declared_type;
vector <string> ID_used;
vector <tree*> expr_storage;
vector <tree*> tests_storage;
string errormsg = "";
tree* seconddcl;
tree* procedure;
bool errorcheck = true;

// Call this to display an error message and exit the program.
void bail(const string &msg) {
  // You can also simply throw a string instead of using this function.
  throw string(msg);
}

// Read and return wlppi parse tree.
tree *readParse(const string &lhs) {
  // Read a line from standard input.
  string line;
  getline(cin, line);
  if(cin.fail())
    bail("ERROR: Unexpected end of file.");
  tree *ret = new tree();
  // Tokenize the line.
  stringstream ss;
  ss << line;
  while(!ss.eof()) {
    string token;
    ss >> token;
    if(token == "") continue;
    ret->tokens.push_back(token);
  }
  // Ensure that the rule is separated by single spaces.
  for(unsigned int idx=0; idx<ret->tokens.size(); idx++) {
    if(idx>0) ret->rule += " ";
    ret->rule += ret->tokens[idx];
  }

  if ( ret->tokens[0] == "dcl"){
	  ID_storage.push_back(ret);
  }

  if ( ret->tokens[0] == "factor"){
	  ID_storage.push_back(ret);
  }

  if ( ret->tokens[0] == "lvalue"){
	  ID_storage.push_back(ret);
	  expr_storage.push_back(ret);
  }

  if ( ret->tokens[0] == "expr"){
	  expr_storage.push_back(ret);
  }

  if ( ret->tokens[0] == "procedure"){
	  procedure = ret;
  }

  if ( ret->tokens[0] == "statement"){
	  if (ret->tokens[1] != "IF" && ret->tokens[1] != "WHILE")
	  tests_storage.push_back(ret);
  }

  if ( ret->tokens[0] == "test" ){
	  tests_storage.push_back(ret);
  }

  if ( ret->tokens[0] == "dcls"){
	  tests_storage.push_back(ret);
  }

  // Recurse if lhs is a nonterminal.
  if(!isTerminal(lhs)) {
    for(unsigned int idx=1/*skip the lhs*/; idx<ret->tokens.size(); idx++) {
    	ret->children.push_back(readParse(ret->tokens[idx]));
    }
  }
  return ret;
}

void findseconddcl(){
	unsigned int n = 0, i =0 ;
	while (n != procedure->children.size()){
		if( procedure->children.at(n)->tokens.at(0) == "dcl" ){
			i++;
			if( i == 2 ){
				seconddcl = procedure->children.at(n);
			}
		}
		n++;
	}
}


tree *parseTree;

// Compute symbols defined in t.
void genSymbols(tree *t) {
}

// Generate the code for the parse tree t.
void genCode(tree *t) {
}

bool notdeclaredbefore (string a, vector<string> b){
	int i = b.size();
	for (int n = 0; n < i; n++){
		if ( a == b.at(n)){
			return false;
		}
	}
	return true;
}

void get_IDs (tree* a){
	if (a->tokens[0] == "ID"){
		ID_used.push_back(a->tokens[1]);
	}
	else if(!isTerminal(a->tokens[0])){
		for(unsigned int i = 0; i < a->children.size(); i++){
			get_IDs(a->children.at(i));
		}
	}
}

void sortentries (){
	int i = 0, n;
	int length, count = 0;
	string temp;
	n = ID_storage.size();
	while ( i != n ){
		if ( ID_storage.at(i)->tokens.at(0) == "dcl" ){
			temp = ID_storage.at(i)->children.at(1)->tokens.at(1);
			if ( notdeclaredbefore (temp, declared_ID)){
				declared_ID.push_back(temp);
				if ( ID_storage.at(i)->children.at(0)->tokens.size() == 2){
					declared_type.push_back("int");
				}
				else declared_type.push_back("int*");
			}
			else{
				errorcheck = false;
				if (errormsg == "") errormsg = "ERROR: duplicate declaration";
			}
			i++;
		}
		else{
			get_IDs(ID_storage.at(i));
			length = ID_used.size();
			while (count != length){
				if (notdeclaredbefore(ID_used.at(count), declared_ID)){
					errorcheck = false;
					if (errormsg == "" ) errormsg = "ERROR: need declaration before use";
				}
				count++;
			}
			count = 0;
			ID_used.resize(0);
			i++;
		}
	}
}

string findtype (string a){
	for (int unsigned i = 0; i < declared_ID.size(); i++){
		if(a == declared_ID.at(i)){
			return declared_type.at(i);
		}
	}
	return "";
}

void exprcheck (tree* &a){
	if (a->tokens.at(0) == "ID"){
		a->type = findtype (a->tokens.at(1));
		return;
	}
	else if (a->tokens.at(0) == "NUM"){
		a->type = "int";
		return;
	}
	else if (a->tokens.at(0) == "NULL"){
		a->type = "int*";
		return;
	}
	else if (a->tokens.at(0) == "factor"){
		if (a->tokens.at(1) == "NUM"){
			a->type = "int";
			return;
		}
		else if (a->tokens.at(1) == "NULL"){
			a->type = "int*";
			return;
		}
		else if (a->tokens.at(1) == "ID"){
			if (a->children.at(0)->type == ""){
				exprcheck (a->children.at(0));
				return;
			}
			else {
				a->type = a->children.at(0)->type;
				return;
			}
		}
		else if (a->tokens.at(1) == "LPAREN"){
			if (a->children.at(1)->type == ""){
				exprcheck (a->children.at(1));
				return;
			}
			else {
				a->type = a->children.at(1)->type;
				return;
			}
		}
		else if (a->tokens.at(1) == "AMP"){
			if (a->children.at(1)->type == ""){
				exprcheck (a->children.at(1));
				return;
			}
			else if (a->children.at(1)->type == "int"){
				a->type = "int*";
				return;
			}
			else {
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in expressions";
					errorcheck = false;
				}
			}
		}
		else if (a->tokens.at(1) == "STAR"){
			if (a->children.at(1)->type == ""){
				exprcheck (a->children.at(1));
				return;
			}
			else if (a->children.at(1)->type == "int*"){
				a->type = "int";
				return;
			}
			else {
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in expressions";
					errorcheck = false;
				}
			}
		}
		else if (a->tokens.at(1) == "NEW"){
			if (a->children.at(3)->type == ""){
				exprcheck (a->children.at(3));
				return;
			}
			else if (a->children.at(3)->type == "int"){
				a->type = "int*";
				return;
			}
			else {
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in expressions";
					errorcheck = false;
				}
			}
		}
		else{
			if (errormsg == ""){
				errormsg = "ERROR: semantic error in expressions";
				errorcheck = false;
			}
		}
	}
	else if ( a->tokens.at(0) == "lvalue"){
		if (a->tokens.at(1) == "ID"){
			if (a->children.at(0)->type == ""){
				exprcheck (a->children.at(0));
				return;
			}
			else {
				a->type = a->children.at(0)->type;
				return;
			}
		}
		else if (a->tokens.at(1) == "LPAREN"){
			if (a->tokens.at(1) == ""){
				exprcheck (a->children.at(0));
				return;
			}
			else {
				a->type = a->children.at(0)->type;
				return;
			}
		}
		else if (a->tokens.at(1) == "STAR"){
			if (a->children.at(1)->type == ""){
				exprcheck (a->children.at(1));
				return;
			}
			else if (a->tokens.at(1) == "int*"){
				a->type = "int";
				return;
			}
			else{
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in expressions";
					errorcheck = false;
				}
			}
		}
	}
	else if (a->tokens.at(0) == "term"){
		if (a->tokens.at(1) == "factor"){
 			if (a->children.at(0)->type == ""){
				exprcheck(a->children.at(0));
				return;
			}
 			else{
 				a->type = a->children.at(0)->type;
 				return;
 			}
		}

		else if (a->children.at(0)->type == "int" && a->children.at(2)->type == "int"){
			a->type = "int";
			return;
		}

		else if (a->children.at(0)->type == "" || a->children.at(2)->type == ""){
			exprcheck (a->children.at(0));
			exprcheck (a->children.at(2));
			return;
		}

		else{
			if (errormsg == ""){
				errormsg = "ERROR: semantic error in expressions";
				errorcheck = false;
			}
		}
	}
	else if ( a->rule == "expr expr PLUS term"){
		if (a->children.at(0)->type == "int" && a->children.at(2)->type == "int"){
			a->type = "int";
			return;
		}
		else if (a->children.at(0)->type == "int*" && a->children.at(2)->type == "int"){
			a->type = "int*";
			return;
		}
		else if (a->children.at(0)->type == "int" && a->children.at(2)->type == "int*"){
			a->type = "int*";
			return;
		}
		else if (a->children.at(0)->type == "" || a->children.at(2)->type == ""){
			exprcheck(a->children.at(0));
			exprcheck(a->children.at(2));
			return;
		}
		else {
			if (errormsg == ""){
				errormsg = "ERROR: semantic error in expressions";
				errorcheck = false;
			}
		}
	}
	else if ( a->rule == "expr expr MINUS term"){
		if (a->children.at(0)->type == "int" && a->children.at(2)->type == "int"){
			a->type = "int";
			return;
		}
		else if (a->children.at(0)->type == "int*" && a->children.at(2)->type == "int"){
			a->type = "int*";
			return;
		}
		else if (a->children.at(0)->type == "int*" && a->children.at(2)->type == "int*"){
			a->type = "int";
			return;
		}
		else if (a->children.at(0)->type == "" || a->children.at(2)->type == ""){
			exprcheck(a->children.at(0));
			exprcheck(a->children.at(2));
			return;
		}
		else{
			if (errormsg == ""){
				errormsg = "ERROR: semantic error in expressions";
				errorcheck = false;
			}
		}
	}
	else if ( a->rule == "expr term"){
		if (a->children.at(0)->type == ""){
			exprcheck (a->children.at(0));
			return;
		}
		else {
			a->type = a->children.at(0)->type;
			return;
		}
	}
	else if (isTerminal(a->tokens.at(0))){
		a->type = "notype";
		return;
	}
	return;
}


void testerror (tree* &a){
	if (a->tokens.at(0) == "statement"){
		if (a->tokens.at(1) == "lvalue"){
			if (a->children.at(0)->type != a->children.at(2)->type){
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in tests";
					errorcheck = false;
				}
			}
			else return;
		}
		else if (a->tokens.at(1) == "PRINTLN"){
			if (a->children.at(2)->type != "int"){
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in tests";
					errorcheck = false;
				}
			}
			else return;
		}
		else if (a->tokens.at(1) == "DELETE"){
			if (a->children.at(3)->type != "int*"){
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in tests";
					errorcheck = false;
				}
			}
			else return;
		}
	}
	else if (a->tokens.at(0) == "test"){
		if (a->children.at(0)->type != a->children.at(2)->type){
			if (errormsg == ""){
				errormsg = "ERROR: semantic error in tests";
				errorcheck = false;
			}
		}
		else return;
	}
	else if (a->tokens.at(0) == "dcls"){
		if (a->children.size() == 0){
			return;
		}
		else if (a->children.at(3)->tokens.at(0) == "NUM"){
			if (a->children.at(1)->type != "int"){
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in tests";
					errorcheck = false;
				}
			}
			else return;
		}
		else if (a->children.at(3)->tokens.at(0) == "NULL"){
			if (a->children.at(1)->type != "int*"){
				if (errormsg == ""){
					errormsg = "ERROR: semantic error in tests";
					errorcheck = false;
				}
			}
			else return;
		}
	}
}

int main() {
	tree* temp;
  // Main program.
  try {
    parseTree = readParse("S");
    genSymbols(parseTree);
    genCode(parseTree);
  } catch(string msg) {
    cerr << msg << endl;
  }

  sortentries();

  if (errorcheck == false){
	  cerr << errormsg << endl;
  }
/*  else{
	  for (unsigned int i = 0; i < declared_ID.size(); i++){
		  cerr << declared_ID.at(i) << " " << declared_type.at(i) << endl;
  	  }
  }*/

/*  for (unsigned int i = 0; i < expr_storage.size(); i++){
	  cout << expr_storage.at(i)->rule << endl;
  }*/

  findseconddcl();

  if (!(findtype(seconddcl->children.at(1)->tokens.at(1)) == "int")){
	  cerr << "ERROR: second dcl not int" << endl;
	  return 0;
  }

  for (unsigned int i = 0; i < expr_storage.size(); i++){
	  temp = expr_storage.at(i);
	  while (temp->type == ""){
		  exprcheck(temp);
		  if (errorcheck == false){
			  cerr << errormsg << endl;
			  return 0;
		  }
	  }
  }

  for (unsigned int i = 0; i < tests_storage.size(); i++){
	  temp = tests_storage.at(i);
	  testerror(temp);
	  if (errorcheck == false){
		  cerr << errormsg << endl;
		  return 0;
	  }
  }





  if (parseTree) delete parseTree;
  return 0;
}
