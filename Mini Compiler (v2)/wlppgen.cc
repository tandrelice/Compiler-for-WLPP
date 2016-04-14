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
  for(idx=0; idx<sizeof(terminals)/sizeof(char*); idx++)
    if(terminals[idx] == sym) return 1;
  return 0;
}

// Data structure for storing the parse tree.
class tree {
  public:
    string rule;
    vector<string> tokens;
    vector<tree*> children;
    bool assigned;
    int i;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};


vector <tree*> ID_storage;
vector <tree*> operation_storage;
vector <string> declared_ID;
vector <string> declared_type;
vector <string> ID_used;
tree* rootnode;
tree* thefactor;
string errormsg = "";
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
  ret->assigned = false;
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
  for(int idx=0; idx<ret->tokens.size(); idx++) {
    if(idx>0) ret->rule += " ";
    ret->rule += ret->tokens[idx];
  }

  if ( ret->tokens[0] == "dcl"){
	  ID_storage.push_back(ret);
  }

  else if ( ret->tokens[0] == "factor"){
	  ID_storage.push_back(ret);
	  operation_storage.push_back(ret);
  }

  else if ( ret->tokens[0] == "lvalue"){
	  ID_storage.push_back(ret);
  }

  else if ( ret->tokens[0] == "expr"){
	  operation_storage.push_back(ret);
  }

  else if ( ret->tokens[0] == "term"){
	  operation_storage.push_back(ret);
  }

  else if ( ret->tokens[0] == "procedure"){
	  rootnode = ret;
  }
  // Recurse if lhs is a nonterminal.
  if(!isTerminal(lhs)) {
    for(int idx=1/*skip the lhs*/; idx<ret->tokens.size(); idx++) {
      ret->children.push_back(readParse(ret->tokens[idx]));
    }
  }
  return ret;
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
		for(int i = 0; i < a->children.size(); i++){
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

int main() {
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
  else{
	  for (unsigned int i = 0; i < declared_ID.size(); i++){
		  cerr << declared_ID.at(i) << " " << declared_type.at(i) << endl;
	  }
  }

  int i = 0;
  if (thefactor->children.at(0)->tokens[1] == declared_ID.at(i) && i == 0){
	  cout << "add $3, $0, $1" << endl;
	  cout << "jr $31" << endl;
  }
  else{
	  cout << "add $3, $0, $2" << endl;
	  cout << "jr $31" << endl;
  }
  if (parseTree) delete parseTree;
  return 0;
}
