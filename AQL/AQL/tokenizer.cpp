/* tokenizer for txt input */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include "lexer.h"
#include "tokenizer.h"
using namespace std;
// all the blank characters
const char BLANK[3] = { '\t', ' ', '\n' };


bool isBlank(char ch) {
	for (int i = 0; i < sizeof(BLANK); i++) {
		if (ch == BLANK[i]) return true;
	}
	return false;
}
bool isDelimiter(char ch) {
	if (isBlank(ch) || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
		return false;
	return true;
}
// eg:
// input: (const char *) PerLoc.input
// output: vector<ContentToken>
vector<ContentToken> tokenize(const char *filepath, map<int, int>& getTokenByBegin,
	map<int, ContentToken>& getContentTokenByBegin,
	map<int, ContentToken>& getContentTokenByBegin_Left) {
	vector<ContentToken> result;
	ifstream ifs;
	ifs.open(filepath, ifstream::in);

	char c;
	string temp;
	int length = 0;
	int start = 0;
	int end = 0;

	int current = 0, totaltokens = 0;
	vector<int> recorder, recorder_left;
	ContentToken lastToken;

	c = ifs.get();
	while (ifs.good()) {
		if (isBlank(c) || isDelimiter(c)) {
			if (!temp.empty()) {
				end = start + temp.length();

				ContentToken newtoken = ContentToken(temp, start, end);
				result.push_back(newtoken);
				start = end;
				// clear
				temp = "";

				for (vector<int>::iterator it = recorder.begin(); it != recorder.end(); it++)
				{
					getTokenByBegin.insert(make_pair(*it, totaltokens));
					getContentTokenByBegin.insert(make_pair(*it, newtoken));
				}


				lastToken = newtoken;

				totaltokens++;
				recorder.clear();
			}
			if (isBlank(c)) {
				recorder.push_back(current);
				recorder_left.push_back(current);
				start++;
				end++;

				if (!isBlank(ifs.peek()))
				{
					for (vector<int>::iterator it = recorder_left.begin(); it != recorder_left.end(); it++)
					{
						getContentTokenByBegin_Left.insert(make_pair(*it, lastToken));
					}
					recorder_left.clear();
				}
			}
			if (isDelimiter(c)) {
				for (vector<int>::iterator it = recorder_left.begin(); it != recorder_left.end(); it++)
				{
					getContentTokenByBegin_Left.insert(make_pair(*it, lastToken));
				}
				recorder_left.clear();

				recorder_left.push_back(current);
				recorder.push_back(current);
				temp.push_back(c);
				end = start + 1;
				
				ContentToken newtoken = ContentToken(temp, start, end);
				result.push_back(newtoken);
				start = end;
				// clear
				temp = "";

				for (vector<int>::iterator it = recorder.begin(); it != recorder.end(); it++)
				{
					getTokenByBegin.insert(make_pair(*it, totaltokens));
					getContentTokenByBegin.insert(make_pair(*it, newtoken));
				}

				lastToken = newtoken;

				totaltokens++;
				recorder.clear();
			}

			// 考虑特殊字符后紧跟实际字符的情况
			char pk = ifs.peek();
			if (isDelimiter(c)
				&& (isDelimiter(pk) || ((pk >= '0' && pk <= '9') || (pk >= 'a' && pk <= 'z') || (pk >= 'A' && pk <= 'Z'))))
			{
				for (vector<int>::iterator it = recorder_left.begin(); it != recorder_left.end(); it++)
				{
					getContentTokenByBegin_Left.insert(make_pair(*it, lastToken));
				}
				recorder_left.clear();
			}

			c = ifs.get();
			current++;
			continue;
		}
		temp.push_back(c);
		c = ifs.get();
		recorder.push_back(current);
		recorder_left.push_back(current++);
	}

	// 最后一个字符处理
	getTokenByBegin.insert(make_pair(current - 1, totaltokens - 1));
	getContentTokenByBegin.insert(make_pair(current - 1, result.at(result.size() - 1)));
	ifs.close();
	return result;
}
