#include<iostream>
#include<vector>
#include<io.h>
#include "parser.h"
#include "lexer.h"
using namespace std;
void getFiles(string path, vector<string>& files)
{
	//文件句柄
	long   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
int main(int argc, char* argv[]) {
	try {
		string AQL_path, Article_path;
		if (argc >= 2) {
			AQL_path = argv[1];
			Article_path = argv[2];
		}
		else {
			//cin >> AQL_path >> Article_path;
			AQL_path = "Revenue.aql";
			Article_path = "Revenue.input";
		}
		

		vector<AqlToken> AqlTokens = lexer(AQL_path.c_str());

		size_t found = Article_path.find(".");
		if (found == string::npos) {
			vector<string> Article_paths;
			getFiles(Article_path, Article_paths);
			for (int i = 0; i < Article_paths.size(); i++) {
				parser Parser(AqlTokens, Article_paths[i]);
			}
		}
		else
		{
			parser Parser(AqlTokens, Article_path);
		}
	} catch (lexException e){
		cout << e.what();
	}
		return 0;
}
