#ifndef AQLPATTERN
#define AQLPATTERN

#include <string>
using std::string;

enum PatternType
{
	TOKEN, REGEX, VIEWCOLUMN
};

struct AQLPattern
{
	PatternType type;

	int mincount, maxcount;
	string regex;
	string ViewName, ViewCol;

	AQLPattern()
	{
		Clear();
	}

	AQLPattern(PatternType t, int min, int max, string r, string vn, string vc)
		: type(t), mincount(min), maxcount(max), regex(r), ViewName(vn), ViewCol(vc) {}

	void Clear()
	{
		mincount = maxcount = 1;
		regex = ViewName = ViewCol = "";
	}

	bool operator== (const AQLPattern& other) const {
		if (type == other.type) {
			switch (type)
			{
				case 0:
					return (mincount == other.mincount && maxcount == other.maxcount);
					break;
				case 1:
					return regex == other.regex;
					break;

				default:
					return (ViewName == other.ViewName && ViewCol == other.ViewCol);
					break;
			}
		} 
		return false;
	}
};

#endif