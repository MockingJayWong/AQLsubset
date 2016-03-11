#ifndef TOKENIZER_H
#define TOKENIZER_H
#include<iostream>
#include<map>
#include<set>
using namespace std;
struct ContentToken {
	string token;
	int begin;
	int end;

	set<int> InGroup;
	ContentToken() {
		token = "";
		begin = 0;
		end = 0;
	}
	ContentToken(const ContentToken& other) : token(other.token), begin(other.begin), end(other.end)
	{
		for (set<int>::iterator it = other.InGroup.begin(); it != other.InGroup.end(); it++)
		{
			InGroup.insert(*it);
		}
	}
	ContentToken(string t, int b, int e) {
		token = t;
		begin = b;
		end = e;
	}

	bool operator<(const ContentToken& other) const
	{
		return begin < other.begin;
	}
	bool operator==(const ContentToken& other) const
	{
		return token < other.token && begin == other.begin && end == other.end;
	}
};

vector<ContentToken> tokenize(const char *filepath, map<int, int>& getTokenByBegin,
	map<int, ContentToken>& getContentTokenByBegin,
	map<int, ContentToken>& getContentTokenByBegin_Left);

#endif
