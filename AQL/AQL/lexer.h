#ifndef LEXER_H
#define LEXER_H
#include<iostream>
#include <string>
#include <vector>
#include <exception>
using namespace std;

struct AqlToken {
  string name, value;
  int row, col;
};

class lexException:public exception {
 public:
  lexException(AqlToken token) {
    cout << token.value << "; " << token.row << "行，" << token.col << "列，词法错误\n";
  }
};

vector<AqlToken> lexer(const char *aqlPathname) throw(lexException);
vector< vector<int> > findall(const char *regex, const char *content);

#endif