#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include "lexer.h"
#define LINE_LENGTH 200
using namespace std;

vector<AqlToken> lexer(const char *aqlPathname) throw(lexException) {
  vector<AqlToken> aqlTokens;
  ifstream fin(aqlPathname);
  char line[LINE_LENGTH];
  int row = 0;
  while(fin.getline(line, LINE_LENGTH)) {
    row++;
    for(int col = 0; col < strlen(line); col++) {
      AqlToken token;
      token.row = row;
      token.col = col;
      while(line[col] != ' ' && line[col] != '\0' && line[col] != '\t' && line[col] != '.' &&
            line[col] != '(' && line[col] != ')' && line[col] != '<' &&
            line[col] != '>' && line[col] != '{' && line[col] != '}' &&
            line[col] != '.' && line[col] != ';' && line[col] != ',' &&
            line[col] != '/') {
        col++;
      }
      if (line[col] == '/') {
        col++;
        while(true) {
          if (line[col] == '/' && line[col - 1] != '\\')
            break;
          col++;
        }
        token.name = "REG";
        string stLine = line;
        token.value = stLine.substr(token.col+1, col-token.col-1);
        aqlTokens.push_back(token);
      } else if(token.col != col) {
        string stLine = line;
        string tokenVal = stLine.substr(token.col, col-token.col);
        if(tokenVal == "create" || tokenVal == "view" || tokenVal == "as" ||
           tokenVal == "output" || tokenVal == "select" || tokenVal == "from" ||
           tokenVal == "extract" || tokenVal == "regex" || tokenVal == "on" ||
           tokenVal == "return" || tokenVal == "group" || tokenVal == "and" ||
           tokenVal == "Token" || tokenVal == "pattern") {
          token.name = token.value = tokenVal;
          aqlTokens.push_back(token);
        } else {
          vector<vector<int> > match = findall("[_A-Za-z][_A-Za-z0-9]*", tokenVal.c_str());
          if (match.size() == 1 && match[0][0] == 0 && match[0][1] == tokenVal.length()) {
            token.name = "ID";
            token.value = tokenVal;
            aqlTokens.push_back(token);
          } else {
            match = findall("[0-9]+", tokenVal.c_str());
            if (match.size() == 1 && match[0][0] == 0 && match[0][1] == tokenVal.length()) {
              token.name = "NUM";
              token.value = tokenVal;
              aqlTokens.push_back(token);
            } else {
              token.value = tokenVal;
              throw lexException(token);
            }
          }
        }
      }
      if(line[col] != ' ' && line[col] != '\0' && line[col] != '/' && line[col] != '\t') {
        AqlToken singleChar;
        singleChar.row = row;
        singleChar.col = col;
        singleChar.name = singleChar.value = line[col];
        aqlTokens.push_back(singleChar);
      }
    }
    
  }
  return aqlTokens;
}