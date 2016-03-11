#ifndef PARSER_H
#define PARSER_H

#include<algorithm>
#include<iostream>
#include<map>
#include<vector>
#include<exception>
#include<stack>
#include "AQLPattern.h"
#include "lexer.h"
#include "tokenizer.h"
using namespace std;
static string Create = "create";
static string View = "view";
static string As = "as";
static string Output = "output";
static string Select = "select";
static string From = "from";
static string Extract = "extract";
static string Regex = "regex";
static string On = "on";
static string Return = "return";
static string Group = "group";
static string And = "and";
static string Token = "Token";
static string Pattern = "pattern";
static string REG = "REG";
static string ID = "ID";
static string NUM = "NUM";

struct selectItem {
	string selectID1, selectID2;
	selectItem(string a, string b) {
		selectID1 = a;
		selectID2 = b;
	}
};

struct from {
	string fromID1, fromID2;
	from(string a, string b) {
		fromID1 = a;
		fromID2 = b;
	}
};

struct group {
	string GroupNum, GroupID;
	group(string a, string b) {
		GroupNum = a;
		GroupID = b;
	}
};

struct Col {
	string colName, regex;
	vector< vector<int> > element;
	vector< ContentToken>  ContentTokens;
	vector<vector< ContentToken>> groupTokens;
	Col(){	}

	Col(const Col& other)
	{
		colName = other.colName;
		regex = other.regex;
		for (int i = 0; i < other.element.size(); i++)
		{
			element.push_back(other.element.at(i));
		}

		for (int i = 0; i < other.ContentTokens.size(); i++)
		{
			ContentTokens.push_back(other.ContentTokens.at(i));
		}
		for (int i = 0; i < other.groupTokens.size(); i++)
		{
			groupTokens.push_back(other.groupTokens.at(i));
		}
	}

	bool operator==(const Col& other) const
	{
		return colName == other.colName;
	}

	void AddGroup(int groupnum) {
		for (auto iter = ContentTokens.begin(); iter != ContentTokens.end(); iter++) {
			iter->InGroup.insert(groupnum);
		}
	}

	void SetGroup(set<int> gr) {
		for (auto ittt = gr.begin(); ittt != gr.end(); ittt++)
		for (auto iter = ContentTokens.begin(); iter != ContentTokens.end(); iter++) {
			iter->InGroup.insert(*ittt);
		}
	}

	void addGroups(int groupnum) {
		for (int i = 0; i < groupTokens.size(); i++) {
			for (int j = 0; j < groupTokens[i].size(); j++) {
				groupTokens[i][j].InGroup.insert(groupnum);
			}
		}
	}

	void setGroups(set<int> gr) {
		for (int i = 0; i < groupTokens.size(); i++) {
			for (int j = 0; j < groupTokens[i].size(); j++) {
				groupTokens[i][j].InGroup = gr;
			}
		}
	}

};

struct view {
	string viewName;
	vector<string> viewNames;
	vector<Col> cols;
};

class ParseException : public exception {
public:
	ParseException(AqlToken token) {
		cout << "There is an parse Error near Row " << token.row << ", col " << token.col << endl;
	}
};

class parser : public exception {
public:
	parser(vector<AqlToken> AQL, string Article_path) throw (ParseException);
	~parser() throw() {	}
private:
	vector<AqlToken> aql;
	int current;

	vector<view> views;
	view tempV;
	Col tempC;

	string article;
	string article_path;
	string OutputView;

	vector<string> AliasIDs;
	vector<group> Groups;
	vector<from> FromLists;
	vector<selectItem> SelectLists;
	string ColumnID1, ColumnID2;

	bool isPattern;
	string pattern_str;
	map<int, string> pattern_groups;
	map<int, vector<int>> childgroups;

	void makePatternGroup(const string& str);
	map<int, vector<AQLPattern>> patterns;

	int getTokenDistance(ContentToken t1, ContentToken t2);

	vector<ContentToken> TokenList;
	map<int, int> getTokenByBegin;
	map<int, ContentToken> getContentTokenByBegin;
	map<int, ContentToken> getContentTokenByBegin_Left;
	
	Col getViewCol(const AQLPattern& pattern);
	Col getViewSingularCols(const AQLPattern& pattern);
	Col getViewColByRegex(const AQLPattern& pattern);
	Col getViewSingularColByRegex(const AQLPattern& pattern);
	Col PatternCol(Col&col, const AQLPattern& pattern);
	view group0_view;

	int findPattern(vector<AQLPattern> a, vector<AQLPattern> b, int startpos);
	void make_pattern(int pos, set<int> groups);

	vector<vector<ContentToken>> produce_col(int i);      //找出所有可能的token拼接
	map<int, bool> usagePattern;   //pattern是否已经填充过.

	map<int, vector<vector<ContentToken>>> patternGroupCols;  

	void Alias();

	void Select_item();
	void Select_list();
	void Select_stmt();

	void From_item();
	void From_list();

	void Column();

	void Single_group();
	void Group_spec();

	void Name_spec();
	void Regex_spec();

	void Atom();
	void Atom_after();

	void Pattern_group();
	void Pattern_pkg();
	void Pattern_expr();
	void Pattern_spec();

	void Extract_spec();
	void Extract_stmt();

	void View_stmt();
	void Create_stmt();
	void Output_stmt();
};
#endif
