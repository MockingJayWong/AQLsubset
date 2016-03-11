#include<iostream>
#include<vector>
#include<stdexcept>
#include <sstream>
#include <fstream> 
#include "AQLPattern.h"
#include "parser.h"
using namespace std;

int stringToInt(string str);

parser::parser(vector<AqlToken> AQL, string Article_path) throw (ParseException) {
	try {
		article_path = Article_path;
		OutputView = "Processing " + article_path + '\n';
		aql = AQL;

		for (current = 0; current < aql.size();) {
			string str = aql.at(current++).name;
			if (str == Create) {
				isPattern = false;
				Create_stmt();
			}
			else if (str == Output) {
				Output_stmt();
			}
			else {
				throw ParseException(aql.at(current - 1));
			}

			//对于每条语句，清空临时变量
			tempV.cols.clear();
			tempV.viewName = "";
			tempC.colName = "";
			tempC.regex = "";
			tempC.element.clear();
			tempC.ContentTokens.clear();

			FromLists.clear();
			AliasIDs.clear();
			Groups.clear();
			SelectLists.clear();
			for (int i = 0; i < views.size(); i++) {
				views.at(i).viewNames.clear();
			}
		}
	}
	catch (ParseException e) {
		cout << e.what();
	}
}
void parser::Alias() {
	try {
		if (aql.at(current++).name == As) {
			if (aql.at(current++).name == ID) {
				AliasIDs.push_back(aql.at(current - 1).value);
				//该语句结束 
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			//该语句结束
			current--;
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Select_item() {
	try {
		if (aql.at(current++).name == ID) {
			if (aql.at(current++).name == ".") {
				if (aql.at(current++).name == ID) {
					SelectLists.push_back(selectItem(aql.at(current - 3).value, aql.at(current - 1).value));
					Alias();
					if (AliasIDs.size() != SelectLists.size()) {
						AliasIDs.push_back(SelectLists.at(SelectLists.size() - 1).selectID2);
					}
				}
				else {
					throw ParseException(aql.at(current - 2));
				}
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Select_list() {
	try {
		Select_item();
		while (aql.at(current++).name == ",") {
			Select_item();
		}
		current--;
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::From_item() {
	try {
		if (aql.at(current++).name == ID) {
			if (aql.at(current++).name == ID) {
				FromLists.push_back(from(aql.at(current - 2).value, aql.at(current - 1).value));
				//from_item判断结束，合法 
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::From_list() {
	try {
		From_item();
		if (current >= aql.size()) {
			//该语句结束
			return;
		}
		while (aql.at(current++).name == ",") {
			From_item();
			if (current >= aql.size()) {
				//该语句结束
				return;
			}
		}
		current--;
		//该语句结束 
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Select_stmt() {
	try {
		Select_list();
		if (aql.at(current++).name == From) {
			From_list();
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Column() {
	try {
		if (aql.at(current++).name == ID) {
			ColumnID1 = aql.at(current - 1).value;
			if (aql.at(current++).name == ".") {
				if (aql.at(current++).name == ID) {
					ColumnID2 = aql.at(current - 1).value;
					//colunm判断结束，合法 
				}
				else {
					throw ParseException(aql.at(current - 2));
				}
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Single_group() {
	try {
		if (aql.at(current++).name == Group) {
			if (aql.at(current++).name == NUM) {
				if (aql.at(current++).name == As) {
					if (aql.at(current++).name == ID) {
						Groups.push_back(group(aql.at(current - 3).value, aql.at(current - 1).value));
						//该语句结束 
					}
					else {
						throw ParseException(aql.at(current - 2));
					}
				}
				else {
					throw ParseException(aql.at(current - 2));
				}
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Group_spec() {
	try {
		Single_group();
		while (aql.at(current++).name == And) {
			Single_group();
		}
		current--;
		//该语句结束 
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Name_spec() {
	try {
		string str = aql.at(current++).name;
		if (str == As) {
			if (aql.at(current++).name == ID) {
				tempC.colName = aql.at(current - 1).value;
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else if (str == Return) {
			Group_spec();
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Regex_spec() {
	try {
		if (aql.at(current++).name == REG) {
			tempC.regex = aql.at(current - 1).value;
			if (aql.at(current++).name == On) {
				Column();
				Name_spec();
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Atom() {
	try {
		string str = aql.at(current++).name;
		pattern_str += aql.at(current - 1).value;

		if (str == ID) {
			if (aql.at(current++).name == ".") {
				pattern_str += aql.at(current - 1).value;

				if (aql.at(current++).name == ID) {
					pattern_str += aql.at(current - 1).value;

					if (aql.at(current++).name == ">") {
						pattern_str += aql.at(current - 1).value;
						//atom判断结束，合法 
					}
					else {
						throw ParseException(aql.at(current - 2));
					}
				}
				else {
					throw ParseException(aql.at(current - 2));
				}
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else if (str == Token) {
			if (aql.at(current++).name == ">") {
				pattern_str += aql.at(current - 1).value;

				//atom判断结束，合法 
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Atom_after() {
	try {
		if (aql.at(current++).name == "{") {
			pattern_str += aql.at(current - 1).value;

			if (aql.at(current++).name == NUM) {
				pattern_str += aql.at(current - 1).value;

				if (aql.at(current++).name == ",") {
					pattern_str += aql.at(current - 1).value;

					if (aql.at(current++).name == NUM) {
						pattern_str += aql.at(current - 1).value;

						if (aql.at(current++).name == "}") {
							pattern_str += aql.at(current - 1).value;

							//判断结束，合法 
						}
						else {
							throw ParseException(aql.at(current - 2));
						}
					}
					else {
						throw ParseException(aql.at(current - 2));
					}
				}
				else {
					throw ParseException(aql.at(current - 2));
				}
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			current--;
			return;
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Pattern_group() {
	try {
		Pattern_expr();
		if (aql.at(current++).name == ")") {
			pattern_str += aql.at(current - 1).value;

			//判断结束 
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Pattern_pkg() {
	try {
		string str = aql.at(current++).name;

		if (str == "<") {
			pattern_str += aql.at(current - 1).value;
			Atom();
			Atom_after();
		}
		else if (str == REG) {
			//atom判断结束，合法 
			pattern_str += "/" + aql.at(current - 1).value + "/";
			Atom_after();
		}
		else if (str == "(") {
			pattern_str += aql.at(current - 1).value;
			Pattern_group();
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Pattern_expr() {
	try {
		Pattern_pkg();
		while (aql[current].name == "<" || aql[current].name == REG || aql[current].name == "(") {
			Pattern_pkg();
		}
		//该语句结束 
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Pattern_spec() {
	isPattern = true;
	pattern_str = "";

	Pattern_expr();

	makePatternGroup(pattern_str);

	Name_spec();
}

void parser::makePatternGroup(const string& str)
{
	int branket_count = 0;
	vector<pair<int, string>> st;

	string tmp = "";
	pattern_groups.clear();
	childgroups.clear();

	vector<int> g0_v;
	childgroups.insert(make_pair(0, g0_v));
	for (int i = 0; i < str.size(); i++)
	{
		bool isBranketHandled = false;
		if (i == 0)
		{
			if (str[i] == '(')
			{
				isBranketHandled = true;
				if (tmp != "")
				{
					st.push_back(make_pair(-1, tmp));
					tmp = "";
				}

				st.push_back(make_pair(++branket_count, "("));

				vector<int> vi;
				childgroups.insert(make_pair(branket_count, vi));
			}
		}
		else if (str[i] == '(' && str[i - 1] != '\\')
		{
			isBranketHandled = true;
			if (tmp != "")
			{
				st.push_back(make_pair(-1, tmp));
				tmp = "";
			}

			st.push_back(make_pair(++branket_count, "("));

			vector<int> vi;
			childgroups.insert(make_pair(branket_count, vi));
		}
		else if (str[i] == ')' && str[i - 1] != '\\')
		{
			isBranketHandled = true;
			string to_push = tmp;
			while (st.back().first < 0)
			{
				tmp = st.back().second + tmp;
				st.pop_back();
			}
			int sbf = st.back().first;
			pattern_groups.insert(make_pair(sbf, tmp));
			st.pop_back();

			if (!st.empty())
			{
				for (auto it = st.rbegin(); it != st.rend(); it++)
				{
					pair<int, string> p = *it;
					if (p.first >= 0)
					{
						childgroups.at(p.first).push_back(sbf);
						break;
					}
				}
				st.push_back(make_pair(-1, tmp));
			}
			else
			{
				childgroups.at(0).push_back(sbf);
			}

			tmp = "";
		}
		if (!isBranketHandled && !st.empty())
		{
			tmp += str[i];
		}
	}

	string totalstr = "";
	for (int i = 0; i < pattern_str.size(); i++)
	{
		bool isAdd = true;
		if (i == 0)
		{
			if (pattern_str[i] == '(')
			{
				isAdd = false;
			}
		}
		else if ((pattern_str[i] == '(' && pattern_str[i - 1] != '\\') || (pattern_str[i] == ')' && pattern_str[i - 1] != '\\'))
		{
			isAdd = false;
		}
		if (isAdd)
		{
			totalstr += pattern_str[i];
		}
	}
	pattern_groups.insert(make_pair(0, totalstr));
}

void parser::Extract_spec() {
	try {
		string str = aql.at(current++).name;
		if (str == Regex) {
			Regex_spec();
		}
		else if (str == Pattern) {
			Pattern_spec();
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::Extract_stmt() {
	try {
		Extract_spec();
		if (aql.at(current++).name == From) {
			From_list();
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
		//该语句结束 
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
void parser::View_stmt() {
	try {
		string str = aql.at(current++).name;
		if (str == Select) {
			Select_stmt();
		}
		else if (str == Extract) {
			Extract_stmt();
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}

//提取某个特定正则的列，拼接N次后的列
Col parser::getViewColByRegex(const AQLPattern& pattern) {
	Col col = getViewSingularColByRegex(pattern);
	col = PatternCol(col, pattern);

	return col;
}
//提取某个特定正则的列
Col parser::getViewSingularColByRegex(const AQLPattern& pattern) {
	Col newtempC;
	newtempC.element = findall(pattern.regex.c_str(), article.c_str());

	for (int i = 0; i < newtempC.element.size(); i++) {
		string ContentTokenName = "";
		for (int j = newtempC.element[i][0]; j < newtempC.element[i][1]; j++) {
			ContentTokenName += article[j];
		}
		struct ContentToken token = ContentToken(ContentTokenName, newtempC.element[i][0], newtempC.element[i][1]);


		newtempC.ContentTokens.push_back(token);
	}
	return newtempC;
}

//提取某个特定名称的列，拼接N次后的列
Col parser::getViewCol(const AQLPattern& pattern)
{
	Col col = getViewSingularCols(pattern);
	col = PatternCol(col, pattern);

	return col;
}
//提取某个特定名称的列，未拼接
Col parser::getViewSingularCols(const AQLPattern& pattern)
{

	for (int v = 0; v < views.size(); v++)
	{
		vector<string> vns = views.at(v).viewNames;
		if (find(vns.begin(), vns.end(), pattern.ViewName) != vns.end())
		{
			for (int c = 0; c < views.at(v).cols.size(); c++)
			{
				if (views.at(v).cols.at(c).colName == pattern.ViewCol)
				{
					//找到对应的列
					return views.at(v).cols.at(c);
				}
			}

		}
	}

	return Col();
}
//将列col拼接pattern.mincount-pattern.maxcount次满足的新列
Col parser::PatternCol(Col&col, const AQLPattern& pattern) {
	Col tempCol = col;
	Col returnCol = col;

	for (int i = 1; i < pattern.maxcount; i++) {
		Col newCol;
		for (int j = 0; j < col.ContentTokens.size(); j++) {
			for (int k = tempCol.ContentTokens.size() - 1; k >= 0; k--) {
				int tmpDistance = getTokenByBegin.at(tempCol.ContentTokens[k].begin) - getTokenByBegin.at(col.ContentTokens[j].end);
				if (tmpDistance < 0 || tempCol.ContentTokens[k].end <= col.ContentTokens[j].begin)
					continue;
				if (tmpDistance == 0) {
					ContentToken tempToken;
					tempToken.begin = col.ContentTokens[j].begin;
					tempToken.end = tempCol.ContentTokens[k].end;
					newCol.ContentTokens.push_back(tempToken);
				}
			}
		}

		col = newCol;
		if (i >= pattern.mincount || (i == pattern.mincount && pattern.mincount == 1)) {
			for (int j = 0; j < col.ContentTokens.size(); j++) {
				returnCol.ContentTokens.push_back(col.ContentTokens[j]);
			}
		}
		else if (i == pattern.mincount - 1) {
			returnCol = col;
		}

	}

	return returnCol;
}
//计算两个Token之间的距离(间隔Token数)
int parser::getTokenDistance(ContentToken tright, ContentToken tleft)
{
	/*return tright.begin_token - tleft.end_token;*/
	int res = getTokenByBegin.at(tright.begin) - getTokenByBegin.at(tleft.end);
	return res;
}

//先产生所有可能的情况
vector<vector<ContentToken>> parser::produce_col(int i) {
	//组中不止一个元素
	//注意遇到token的时候更新minDistance,maxDistance
	vector<ContentToken> OneTokenVector;
	int minDistance = 0, maxDistance = 0;
	vector<vector<ContentToken>> preList, currentList;
	bool isFirstToken = false;
	for (int j = 0; j < patterns.at(i).size(); j++)
	{
		//不允许第一个为<token>格式。


		AQLPattern iter = patterns.at(i).at(j);
		if (j == 0)
		{

			if (iter.type == VIEWCOLUMN)
			{
				Col col = getViewCol(iter);
				for (int k = 0; k < col.ContentTokens.size(); k++) {
					OneTokenVector.push_back(col.ContentTokens[k]);
					preList.push_back(OneTokenVector);
					OneTokenVector.clear();
				}
			}
			else if (iter.type == REGEX) {
				Col col = getViewColByRegex(iter);
				for (int k = 0; k < col.ContentTokens.size(); k++) {
					OneTokenVector.push_back(col.ContentTokens[k]);
					preList.push_back(OneTokenVector);
					OneTokenVector.clear();
				}
			}
			else if (iter.type == TOKEN) {
				minDistance = iter.mincount;
				maxDistance = iter.maxcount;
				isFirstToken = true;
			}
		}
		else {
			currentList.clear();

			if (iter.type == VIEWCOLUMN || iter.type == REGEX)
			{

				Col col;
				if (iter.type == VIEWCOLUMN) {
					col = getViewCol(iter);
				}
				else {
					col = getViewColByRegex(iter);
				}

				if (isFirstToken) {
					for (int k = 0; k < col.ContentTokens.size(); k++) {
						ContentToken tmpcol = col.ContentTokens[k];
						ContentToken tmpToken = getContentTokenByBegin[tmpcol.begin];
						for (int p = 0; p <= maxDistance; p++) {
							if (tmpToken.begin == 0)
								break;
							if (p >= minDistance) {
								tmpcol.begin = tmpToken.begin;
								OneTokenVector.push_back(tmpcol);
								preList.push_back(OneTokenVector);
								OneTokenVector.clear();
							}
							tmpToken = getContentTokenByBegin_Left[tmpToken.begin - 1];
						}

					}
					isFirstToken = false;
					minDistance = maxDistance = 0;
				}
				else {
					for (int q = 0; q < preList.size(); q++) {
						for (int p = col.ContentTokens.size() - 1; p >= 0; p--) {
							int tmpDistance = getTokenDistance(col.ContentTokens[p], preList[q][preList[q].size() - 1]);
							if (
								tmpDistance >= minDistance &&
								tmpDistance <= maxDistance && col.ContentTokens[p].end > preList[q][preList[q].size() - 1].begin) {
								currentList.insert(currentList.begin(), preList[q]);
								currentList[0].push_back(col.ContentTokens[p]);
							}

						}
					}
					minDistance = maxDistance = 0;
					preList = currentList;
				}


			}
			else {
				minDistance += iter.mincount;
				maxDistance += iter.maxcount;
			}
		}
	}

	//处理以token结尾的情况
	currentList.clear();
	if (minDistance != 0 || maxDistance != 0) {
		int next;
		for (int k = 0; k < preList.size(); k++) {
			ContentToken tmpcol = preList[k][preList[k].size() - 1];
			ContentToken tmpToken = getContentTokenByBegin[tmpcol.end - 1];
			for (int p = 0; p <= maxDistance; p++) {
				if (tmpToken.end == article.size())
					break;
				if (p >= minDistance) {
					tmpcol.end = tmpToken.end;
					OneTokenVector = preList[k];
					OneTokenVector.push_back(tmpcol);
					currentList.push_back(OneTokenVector);
					OneTokenVector.clear();
				}
				tmpToken = getContentTokenByBegin[tmpToken.end];
			}
		}
		preList = currentList;
	}

	currentList.clear();
	if (isFirstToken) {
		Col col;

		ContentToken tmpToken = getContentTokenByBegin[0];
		while (tmpToken.end != getContentTokenByBegin[getContentTokenByBegin.size() - 1].end)
		{
			col.ContentTokens.push_back(tmpToken);
			tmpToken = getContentTokenByBegin[tmpToken.end];
		}

		AQLPattern pattern;
		pattern.mincount = minDistance;
		pattern.maxcount = maxDistance;
		col = PatternCol(col, pattern);

		for (int i = 0; i < col.ContentTokens.size(); i++) {
			OneTokenVector.push_back(col.ContentTokens[i]);
			currentList.push_back(OneTokenVector);
			OneTokenVector.clear();
		}

		preList = currentList;
	}

	return preList;
}

//找到b在a中的首位置
int parser::findPattern(vector<AQLPattern> a, vector<AQLPattern> b, int startpos) {
	for (int i = startpos; i < a.size(); i++) {
		if (b[0] == a[i]) {
			int k = 0;
			for (k = 0; k < b.size();)
			{
				if (a[i + k] == b[k]) {
					k++;
				}
				else {
					break;
				}
			}
			if (k == b.size()) {
				return i;
			}
		}
	}
	return -1;
}

//递归调用pattern
void parser::make_pattern(int position, set<int> groups) {
	if (childgroups.at(position).size() == 0 && !usagePattern[position]) {
		//爬取单个子组情况,调用produce_col();
		groups.insert(position);
		Col c;
		c.groupTokens = produce_col(position);
		c.setGroups(groups);
		//将已有分组信息的token放入map中，当作第position组.
		patternGroupCols.insert(make_pair(position, c.groupTokens));
		usagePattern[position] = true;
	}
	else {
		for (int i = 0; i < childgroups.at(position).size(); i++) {
			if (usagePattern[childgroups[position][i]] == false) {
				make_pattern(childgroups[position][i], groups);
			}
		}

		//整合组内所有子组的信息.
		groups.insert(position);

		int pattern_step = 0, tmp_step, pr = 0;
		int minDistance = 0, maxDistance = 0;
		vector<vector<ContentToken>> preGroup, currentGroup;
		vector<ContentToken> OneTokenVector;
		bool isFirstToken = false;
		//遍历该组的所有pattern类型。
		for (; pattern_step < patterns[position].size();) {
			//判断是否已经走完所有的子组.
			if (pr < childgroups[position].size()) {
				tmp_step = findPattern(patterns[position], patterns[childgroups[position][pr]], pattern_step);
			}
			else {
				tmp_step = patterns[position].size();
			}
			//是子组，就匹配子组与已有匹配结果
			if (tmp_step == pattern_step) {
				AQLPattern iter = patterns.at(position).at(pattern_step);
				if (tmp_step == 0)
				{
					preGroup = patternGroupCols[childgroups[position][pr]];
					//这里不可能遇到token，因为(<Token>{1,2})这样的表达式是无意义的，我们不允许)
				}
				else {
					currentGroup.clear();
					//从patternGroupCols中搜寻已有的分组信息
					vector<vector<ContentToken>> gps = patternGroupCols[childgroups[position][pr]];
					//处理之前全是token类型后接分组的pattern格式，例如<Token>(XX)，用获取getContentTokenByBegin这个map找到对应上个token
					if (isFirstToken) {
						for (int k = 0; k < gps.size(); k++) {
							ContentToken tmpcol = gps[k][0];
							ContentToken tmpToken = getContentTokenByBegin[tmpcol.begin];
							for (int p = 0; p <= maxDistance; p++) {
								if (tmpToken.begin == 0)
									break;
								if (p >= minDistance) {
									tmpcol.begin = tmpToken.begin;
									OneTokenVector.push_back(tmpcol);
									preGroup.push_back(OneTokenVector);
									OneTokenVector.clear();
								}
								tmpToken = getContentTokenByBegin_Left[tmpToken.begin - 1];
							}

						}
						minDistance = maxDistance = 0;
						isFirstToken = false;
					}
					else {
						for (int q = 0; q < preGroup.size(); q++) {
							for (int p = gps.size() - 1; p >= 0; p--) {
								int tmpDistance = getTokenByBegin.at(gps[p][0].begin) - getTokenByBegin.at(preGroup[q][preGroup[q].size() - 1].end);
								if (
									tmpDistance >= minDistance &&
									tmpDistance <= maxDistance && gps[p][0].end > preGroup[q][preGroup[q].size() - 1].begin) {
									currentGroup.insert(currentGroup.begin(), preGroup[q]);
									for (int k = 0; k < gps[p].size(); k++) {
										currentGroup[0].push_back(gps[p][k]);
									}

								}

							}
						}
						minDistance = maxDistance = 0;
						preGroup = currentGroup;
					}
				}
				//pattern_step记录遍历过该组里哪些AQLPattern类型，pr为遍历哪些分组。
				pattern_step += patterns[childgroups[position][pr]].size();
				pr++;
			}
			else {
				currentGroup.clear();
				//组内非子组的pattern，例如（（A）BC（D）），即BC需要用下面来遍历。
				for (int kr = pattern_step; kr < tmp_step; kr++) {
					AQLPattern iter = patterns.at(position).at(kr);
					if (pattern_step == 0)
					{
						if (iter.type == VIEWCOLUMN)
						{
							Col col = getViewCol(iter);
							for (int k = 0; k < col.ContentTokens.size(); k++) {
								OneTokenVector.push_back(col.ContentTokens[k]);
								preGroup.push_back(OneTokenVector);
								OneTokenVector.clear();
							}
						}
						else if (iter.type == REGEX) {
							Col col = getViewColByRegex(iter);
							for (int k = 0; k < col.ContentTokens.size(); k++) {
								OneTokenVector.push_back(col.ContentTokens[k]);
								preGroup.push_back(OneTokenVector);
								OneTokenVector.clear();
							}
						}
						else if (iter.type == TOKEN) {
							minDistance = iter.mincount;
							maxDistance = iter.maxcount;
							isFirstToken = true;
						}

					}
					else {
						if (iter.type == VIEWCOLUMN || iter.type == REGEX)
						{

							Col col;
							if (iter.type == VIEWCOLUMN) {
								col = getViewCol(iter);
							}
							else {
								col = getViewColByRegex(iter);
							}
							if (isFirstToken) {
								for (int k = 0; k < col.ContentTokens.size(); k++) {
									ContentToken tmpcol = col.ContentTokens[k];
									ContentToken tmpToken = getContentTokenByBegin[tmpcol.begin];
									for (int p = 0; p <= maxDistance; p++) {
										if (tmpToken.begin == 0)
											break;
										if (p >= minDistance) {
											tmpcol.begin = tmpToken.begin;
											OneTokenVector.push_back(tmpcol);
											preGroup.push_back(OneTokenVector);
											OneTokenVector.clear();
										}
										tmpToken = getContentTokenByBegin_Left[tmpToken.begin - 1];
									}
								}
								minDistance = maxDistance = 0;
								isFirstToken = false;
							}
							else {
								for (int q = 0; q < preGroup.size(); q++) {
									for (int p = col.ContentTokens.size() - 1; p >= 0; p--) {
										int tmpDistance = getTokenByBegin.at(col.ContentTokens[p].begin) - getTokenByBegin.at(preGroup[q][preGroup[q].size() - 1].end);
										if (tmpDistance >= minDistance &&tmpDistance <= maxDistance && col.ContentTokens[p].end > preGroup[q][preGroup[q].size() - 1].begin) {
											currentGroup.insert(currentGroup.begin(), preGroup[q]);
											currentGroup[0].push_back(col.ContentTokens[p]);
										}
									}
								}
								minDistance = maxDistance = 0;
								preGroup = currentGroup;
							}
						}
						else {
							minDistance += iter.mincount;
							maxDistance += iter.maxcount;
						}
					}
					pattern_step++;
				}
			}
		}
		//处理组内最后出现token情况，((A)BC),这种情况用下面方法处理，获取getContentTokenByBegin这个map找到对应下个token
		currentGroup.clear();
		if (minDistance != 0 || maxDistance != 0) {
			for (int k = 0; k < preGroup.size(); k++) {
				ContentToken tmpcol = preGroup[k][preGroup[k].size() - 1];
				ContentToken tmpToken = getContentTokenByBegin[tmpcol.end - 1];
				for (int p = 0; p <= maxDistance; p++) {
					if (tmpToken.end == article.size())
						break;
					if (p >= minDistance) {
						tmpcol.end = tmpToken.end;
						OneTokenVector = preGroup[k];
						OneTokenVector.push_back(tmpcol);
						currentGroup.push_back(OneTokenVector);
						OneTokenVector.clear();
					}
					tmpToken = getContentTokenByBegin[tmpToken.end];
				}
			}
			preGroup = currentGroup;
		}
		//为该组的每个token添加分组信息，并添加该组信息到patternGroupCols
		Col tempCol;
		int mi, ma;
		currentGroup = preGroup;
		for (int p = 0; p < currentGroup.size(); p++) {
			for (int q = 0; q < currentGroup[p].size(); q++) {
				currentGroup[p][q].InGroup.insert(position);
			}
		}
		patternGroupCols.insert(make_pair(position, currentGroup));
		usagePattern[position] = true;

	}
}

void parser::Create_stmt() {
	try {
		if (aql.at(current++).name == View) {
			if (aql.at(current++).name == ID) {
				tempV.viewName = aql.at(current - 1).value;

				if (aql.at(current++).name == As) {
					View_stmt();
				}
				else {
					throw ParseException(aql.at(current - 2));
				}
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
		if (aql.at(current++).name == ";") {
			//Create语句结束，创建相应的view
			if (FromLists.at(0).fromID1 == "Document") {

				if (TokenList.size() == 0)
				{
					fstream fs(article_path.c_str());
					stringstream ss;
					ss << fs.rdbuf();
					article = ss.str();

					TokenList = tokenize(article_path.c_str(), getTokenByBegin, getContentTokenByBegin, getContentTokenByBegin_Left);

				}

				tempC.element = findall(tempC.regex.c_str(), article.c_str());

				for (int i = 0; i < tempC.element.size(); i++) {
					string ContentTokenName = "";
					for (int j = tempC.element[i][0]; j < tempC.element[i][1]; j++) {
						ContentTokenName += article[j];
					}
					struct ContentToken token = ContentToken(ContentTokenName, tempC.element[i][0], tempC.element[i][1]);
					tempC.ContentTokens.push_back(token);
				}

				if (tempC.colName.size() == 0 && Groups.size() == 1)
					tempC.colName = Groups.at(0).GroupID;
				tempV.cols.push_back(tempC);
				views.push_back(tempV);
			}
			else {
				//将FromLists中对所有view的别名加入到对应view的viewNames中 
				for (int i = 0; i < FromLists.size(); i++) {
					for (int j = 0; j < views.size(); j++) {
						if (views.at(j).viewName == FromLists.at(i).fromID1) {
							views.at(j).viewNames.push_back(FromLists.at(i).fromID2);
						}
					}
				}

				//将view的原名也放入到viewNames中 
				if (isPattern)
				{
					//创建 pattern对应的view
					patterns.clear();
					//处理分组内的字符串为AQLPattern类型的数据
					for (int i = 0; i < pattern_groups.size(); i++)
					{
						int group_num = i;

						//万一Group数字跟括号不匹配呢
						string str = pattern_groups.at(group_num);

						string _left = "", _right = "", _regex = "";
						int _min = 1, _max = 1;
						vector<AQLPattern> tempVP;
						AQLPattern tempAP;
						bool isToken = false;
						for (int j = 0; j < str.length();)
						{
							switch (str[j])
							{
							case '<':
								//It is a ViewColumn or a Token
								tempAP.Clear();
								j++;
								while (str[j] != '.') {
									tempAP.ViewName += str[j++];
									if (str[j] == '>' && tempAP.ViewName == "Token") {
										tempAP.type = TOKEN;
										isToken = true;
										j++;
										break;
									}
								}
								if (!isToken) {
									j++;
									while (str[j] != '>') {
										tempAP.ViewCol += str[j++];
									}
									j++; //消除'>'
									tempAP.type = VIEWCOLUMN;
								}
								if (str[j] == '{') {
									_max = _min = 0;
									j++; //消除'>{'
									while (str[j] != ',') {
										_min = 10 * _min + str[j++] - '0';
									}
									j++;
									while (str[j] != '}') {
										_max = 10 * _max + str[j++] - '0';
									}
									j++; //消除'}'
									//	tempAP.Clear();
									tempAP.maxcount = _max, tempAP.mincount = _min;
									_max = _min = 1;
								}
								tempVP.push_back(tempAP);
								tempAP.Clear();
								isToken = false;
								break;

							case '/':
								tempAP.Clear();
								j++; //Eliminate '/'
								while (j < str.length()) {
									tempAP.regex += str[j++];

									if (j >= str.length())
										break;
									if (str[j] == '/' && str[j - 1] != '\\')
										break;
								}
								tempAP.type = REGEX;

								if (str[++j] == '{')  //Eliminate '/'
								{
									_max = _min = 0;
									j++; //消除'{'
									while (str[j] != ',') {
										_min = 10 * _min + str[j++] - '0';
									}
									j++;
									while (str[j] != '}') {
										_max = 10 * _max + str[j++] - '0';
									}
									j++; //消除'}'
									tempAP.maxcount = _max, tempAP.mincount = _min;
									_max = _min = 1;
								}

								tempVP.push_back(tempAP);
								tempAP.Clear();
								break;

							}
						}

						patterns.insert(make_pair(group_num, tempVP));
					}

					usagePattern.clear();
					patternGroupCols.clear();
					//开始处理相应分组的信息
					set<int> gr;
					make_pattern(0, gr);

					// First create the cols
					for (int gn = 0; gn < Groups.size(); gn++)
					{
						Col gcol;
						gcol.colName = Groups.at(gn).GroupID;
						tempV.cols.push_back(gcol);
					}

					// Deal with Group0
					vector<vector<ContentToken>> vv = patternGroupCols.at(0);
					for (vector<vector<ContentToken>>::iterator g0it = vv.begin(); g0it != vv.end(); g0it++)
					{
						int s = g0it->at(0).begin, e = g0it->at(g0it->size() - 1).end;
						string str = article.substr(s, e - s);
						ContentToken tk = ContentToken(str, s, e);
						tempV.cols.at(0).ContentTokens.push_back(tk);
					}

					// Deal with other Groups
					if (Groups.size() > 1)
					{
						for (vector<vector<ContentToken>>::iterator g0it = vv.begin(); g0it != vv.end(); g0it++)
						{
							bool zero = true;

							// Deal with group gn
							for (vector<group>::iterator git = Groups.begin(); git != Groups.end(); git++)
							{
								if (zero)
								{
									zero = false;
									continue;
								}

								int gn = stringToInt(git->GroupNum);
								string gname = git->GroupID;

								int s = 0, e = 0;
								// Search the correspinding row of G0, to find any possible element of gn
								for (vector<ContentToken>::iterator tk = g0it->begin(); tk != g0it->end();)
								{
									if (find(tk->InGroup.begin(), tk->InGroup.end(), gn) != tk->InGroup.end())
									{
										s = tk->begin;
										e = tk->end;
										tk++;
										if (tk != g0it->end())
										{
											while (find(tk->InGroup.begin(), tk->InGroup.end(), gn) != tk->InGroup.end())
											{
												e = tk->end;
												tk++;
												if (tk == g0it->end())
													break;
											}
										}

										ContentToken newToken = ContentToken(article.substr(s, e - s), s, e);
										// Insert that token to the corresponding Col
										for (vector<Col>::iterator c = tempV.cols.begin(); c != tempV.cols.end(); c++)
										{
											if (c->colName == gname)
											{
												c->ContentTokens.push_back(newToken);
												break;
											}
										}
										break;
									}
									else
									{
										tk++;
									}
								}
							}
						}
					}
				}
				else
				{
					//创建 select对应的view
					for (int i = 0; i < SelectLists.size(); i++) {
						for (int j = 0; j < views.size(); j++) {
							for (int k = 0; k < views.at(j).viewNames.size(); k++) {
								if (SelectLists.at(i).selectID1 == views.at(j).viewNames.at(k)) {
									tempC.colName = SelectLists.at(i).selectID2;
									for (int a = 0; a < views.at(j).cols.size(); a++) {
										if (tempC.colName == views.at(j).cols.at(a).colName) {
											tempC.ContentTokens = views.at(j).cols.at(a).ContentTokens;
										}
									}
									tempC.colName = AliasIDs.at(i);

								}
							}

						}
						tempV.cols.push_back(tempC);
						tempC.colName = "";
						tempC.ContentTokens.clear();
					}
				}

				views.push_back(tempV);
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}
//返回一个int的长度 
int intSize(int num) {
	if (num == 0) return 1;
	int size = 0;
	while (num) {
		num /= 10;
		size++;
	}
	return size;
}
//返回字符串num个‘-’ 
string repeatStr(int num, string temp) {
	string str = temp;
	for (int i = 0; i < num; i++)
		str += temp;
	return str;
}
//int转string
string intToString(int num) {
	stringstream ss;
	string str;
	ss << num;
	ss >> str;
	return str;
}
//string转int
int stringToInt(string str)
{
	int res = 0;
	for (int i = 0; i < str.size(); i++)
	{
		res *= 10;
		res += str[i] - '0';
	}
	return res;
}
void parser::Output_stmt() {
	try {
		if (aql.at(current++).name == View) {
			if (aql.at(current++).name == ID) {
				tempV.viewName = aql.at(current - 1).value;
				Alias();
			}
			else {
				throw ParseException(aql.at(current - 2));
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
		if (aql.at(current++).name == ";") {
			//Output语句结束，输出对应的View
			for (int i = 0; i < views.size(); i++) {
				if (views.at(i).viewName != tempV.viewName)
					continue;
				OutputView += "View: ";
				if (AliasIDs.empty()) {
					OutputView += views.at(i).viewName;
				}
				else {
					OutputView += AliasIDs.at(0);
				}

				vector<int> ColSizes;
				int MaxRowSize = 0;
				//获取每列中最大长度，并处于ColSizes中. 记录每列的最大行数，存于MaxRowSize中 
				for (int j = 0; j < views.at(i).cols.size(); j++) {
					MaxRowSize = views.at(i).cols.at(j).ContentTokens.size() > MaxRowSize ? views.at(i).cols.at(j).ContentTokens.size() : MaxRowSize;

					int MaxSize = 4;
					for (int k = 0; k < views.at(i).cols.at(j).ContentTokens.size(); k++) {
						int ColSize = 4;
						ColSize += views.at(i).cols.at(j).ContentTokens[k].token.size() + intSize(views.at(i).cols.at(j).ContentTokens[k].begin) + intSize(views.at(i).cols.at(j).ContentTokens[k].end);
						MaxSize = ColSize > MaxSize ? ColSize : MaxSize;
					}
					ColSizes.push_back(MaxSize);
				}

				for (int j = 0; j < views.at(i).cols.size(); j++) {
					if (views.at(i).cols.at(j).ContentTokens.size() == 0 || MaxRowSize == 0) {
						ColSizes.at(j) = views.at(i).cols.at(j).colName.size() > 9 ? views.at(i).cols.at(j).colName.size() : 9;
					}
				}

				//记录view的第一行，即边框 
				OutputView += "\n+";
				for (int j = 0; j < ColSizes.size(); j++) {
					OutputView += repeatStr(ColSizes.at(j), "-") + "+";
				}
				//记录view的第二行，即各列名称
				OutputView += "\n|";
				for (int j = 0; j < views.at(i).cols.size(); j++) {
					OutputView += views.at(i).cols.at(j).colName + repeatStr(ColSizes.at(j) - views.at(i).cols.at(j).colName.size(), " ") + "|";
				}
				//记录view的第三行，即边框
				OutputView += "\n+";
				for (int j = 0; j < ColSizes.size(); j++) {
					OutputView += repeatStr(ColSizes.at(j), "-") + "+";
				}
				OutputView += "\n";
				//记录view的各列各行
				if (MaxRowSize == 0) {
					OutputView += "|";
					for (int j = 0; j < views.at(i).cols.size(); j++) {
						OutputView += "Empty set" + repeatStr(ColSizes.at(j) - 9, " ") + "|";
					}
					OutputView += "\n";
				}
				for (int j = 0; j < MaxRowSize; j++) {
					OutputView += "|";
					for (int k = 0; k < views.at(i).cols.size(); k++) {
						if (j < views.at(i).cols.at(k).ContentTokens.size()) {
							string str = views.at(i).cols.at(k).ContentTokens.at(j).token + ":(";
							str += intToString(views.at(i).cols.at(k).ContentTokens.at(j).begin) + "," + intToString(views.at(i).cols.at(k).ContentTokens.at(j).end) + ")";
							OutputView += str + repeatStr(ColSizes.at(k) - str.size(), " ") + "|";
						}
						else if (views.at(i).cols.at(k).ContentTokens.size() == 0) {
							OutputView += "Empty set |";
						}
						else {
							OutputView += "NULL" + repeatStr(ColSizes.at(k) - 4, " ") + "|";
						}
					}
					OutputView += "\n";
				}
				//记录view的最后一行，即边框
				OutputView += "+";
				for (int j = 0; j < ColSizes.size(); j++) {
					OutputView += repeatStr(ColSizes.at(j), "-") + "+";
				}
				OutputView += "\n" + intToString(MaxRowSize) + " rows in set\n";
				cout << OutputView << endl;
				OutputView = "";
			}
		}
		else {
			throw ParseException(aql.at(current - 2));
		}
	}
	catch (out_of_range) {
		throw ParseException(aql.at(current - 2));
	}
}