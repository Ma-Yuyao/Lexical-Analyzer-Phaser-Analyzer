// lexical-analysis.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#define KEYWORD_FILE_PATH "./KEYWORD.txt"
#define GRAMMAR_FILE_PATH "./GRAMMAR.txt"
#define SOURCES_FILE_PATH "./SOURCES.txt"
#define MATCHING_FILE_PATH "./MATCHING.txt"
#define OUTPUT_FILE_PATH "./OUTPUT.txt"

char str_file[1000];  //用于保存读取文件的内容

string ch = "";       //用于保存输出LOG的内容

vector<string> keyword;  //用于保存KEYWORD.txt中的keyword的内容

struct NFA_sturct     //定义NFA三元组 "当前状态|接收字符|下一个状态"
{
	string now;
	string input;
	string next;
};
struct DFA            //定义DFA————开始状态、结束状态
{
	string startState;
	vector<string> endState;
	map<string, string> f;
};
struct NFA            //定义NFA————开始状态、结束状态
{
	string startState;
	vector<string> endState;
	NFA_sturct f[200];
};
struct closure        //定义闭包closure
{
	string name;
	vector<string> t;
};
bool comp(vector<string> v1, vector<string> v2)
{
	for (int i = 0; i<v2.size(); i++)
	{
		if (find(v1.begin(), v1.end(), v2[i]) == v1.end())
			return false;
	}
	return true;
}
bool operator == (const closure& c1, const closure& c2)
{
	if (comp(c1.t, c2.t) && comp(c2.t, c1.t))
		return true;
	else
		return false;
}
						
vector<string> split(const string &s, const string &seperator) //自定义字符串分割函数split 
{
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		//找到字符串中首个不等于分隔符的字母；
		int flag = 0;
		while (i != s.size() && flag == 0) {
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]) {
					++i;
					flag = 0;
					break;
				}
		}

		//找到又一个分隔符，将两个分隔符之间的字符串取出；
		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0) {
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]) {
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j) {
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

void read_keywords()         //从KEYWORD文件中读取keywords
{
	fstream file;
	file.open(KEYWORD_FILE_PATH, ios::in);
	if (!file.is_open())
	{
		cout << "Can not find the file--'KEYWORD.txt',please check!";
	}
	else
	{
		//不要跳过间隔符
		file.unsetf(ios::skipws);
		//按行读取
		while (file.getline(str_file, 1000))
		{
			keyword.push_back(str_file);
		}
		//用于将每行数据以" "分隔开
		for (int i = 0;i < keyword.size();i++)
		{
			//以" "相分隔
			vector<string> v = split(keyword[i], " ");
			keyword[i] = v[2];
		}
		file.close();
	}
}

DFA trans(NFA nfa, vector<string> inchar, int line)    //子集法，将NFA化成DFA 
{
	fstream file;
	DFA dfa;
	dfa.endState.assign(nfa.endState.begin(), nfa.endState.end());  //将nfa.endState赋给dfa.endState
	dfa.startState = nfa.startState;
	vector<closure> C;
	vector<int> flags;     //flags作为标志位，0即未被标记
	closure clo;
	clo.t.push_back(nfa.startState);
	clo.name = nfa.startState;
	C.push_back(clo);
	flags.push_back(0);
	clo.t.clear();
	clo.name = "0";
	vector<int> ::iterator it = find(flags.begin(), flags.end(), 0);  //在flags中寻找0，即若还有没有标记的子集
	while (it != flags.end())  //当找到的时候
	{
		int nPosition = distance(flags.begin(), it);  //得到它是第几个子集
		flags[nPosition] = 1;		//该子集的标志位设为"1"

		for (int i = 0; i<inchar.size(); i++)
		{
			string ch = inchar[i];
			for (int j = 0; j<C[nPosition].t.size(); j++)
			{
				for (int k = 0; k<line; k++)
				{
					if (ch == nfa.f[k].input&&C[nPosition].t[j] == nfa.f[k].now)//输入符等于NFA的现在状态
					{
						clo.t.push_back(nfa.f[k].next); //将NFA的next加入clo.t
					}
				}
			}
			if (clo.t.size() != 0)      //求出的closure不为空 
			{
				vector<closure>::iterator cit = find(C.begin(), C.end(), clo);
				if (cit == C.end())		// C不包含新的clo 
				{
					clo.name[0]++;
					string t = clo.name;
					for (int i = 0; i<nfa.endState.size(); i++)
					{
						vector<string>::iterator it = find(clo.t.begin(), clo.t.end(), nfa.endState[i]); //在clo.t中找nfa的终态
						if (it != clo.t.end()) //如果找到了
							clo.name = *it;  //clo的类型就是终态
					}
					//改名 
					C.push_back(clo);
					flags.push_back(0);
					string key = C[nPosition].name;
					key += ch;
					dfa.f.insert(pair<string, string>(key, clo.name));
					clo.name = t;
				}
				else  			//C包含clo 
				{
					int p1 = distance(C.begin(), cit);
					string key = C[nPosition].name;
					key += ch;
					dfa.f.insert(pair<string, string>(key, C[p1].name));
				}
				clo.t.clear();
			}

		}
		it = find(flags.begin(), flags.end(), 0);
	}

	return dfa;
}

void GetTime()     //获得系统时间
{
	string Time = "";
	time_t rawtime;
	struct tm *ptminfo;
	time(&rawtime);
	ptminfo = localtime(&rawtime);
	int year = ptminfo->tm_year + 1900;   //取得年份
	int month = ptminfo->tm_mon + 1;   //取得月份
	int day = ptminfo->tm_mday;  //取得日期
	int hour = ptminfo->tm_hour;  //取得小时
	int mintue = ptminfo->tm_min;  //取得分钟
	int second = ptminfo->tm_sec;  //取得秒钟
	Time += to_string(year) + "年";
	Time += to_string(month) + "月";
	Time += to_string(day) + "日";
	Time += to_string(hour) + "时";
	Time += to_string(mintue) + "分";
	Time += to_string(second) + "秒";
	Time += "\n";
	ch += Time;
}
int main()
{
	read_keywords();             //读取KEYWORD.txt
	fstream file;					
	DFA dfa;
	NFA nfa;
	vector<string> inchar;
	vector<string> type;
	vector<string> token;
	string keywordState = "K";   //定义关键字的符号
	string symbolState = "I";    //定义标识符的符号
	nfa.startState = "S";        //定义nfa初态
	dfa.startState = "S";		 //定义dfa初态
	int line = 0;

	file.open("INCHAR.txt", ios::in);
	if (!file.is_open())
	{
		cout << "Can not find the file--'INCHAR.txt',please check!";
	}
	else
	{
		while (file >> str_file)
			inchar.push_back(str_file);
	}
	file.close();

	file.open(GRAMMAR_FILE_PATH, ios::in);
	if (!file.is_open())
	{
		cout << "Can not find the file--'GTAMMAR.txt',please check!";
	}
	else
	{
		while (file.getline(str_file, 10))//读取文件中每一行的文法，并将它们输入到NFA中
		{
			char* s = str_file;
			nfa.f[line].now = *s++;
			s++;
			s++;
			if (*s == '$')
			{
				nfa.endState.push_back(nfa.f[line].now);
				dfa.endState.push_back(nfa.f[line].now);
			}
			else
			{
				nfa.f[line].input = *s++;
				nfa.f[line].next = *s;
			}
			line++;
		}
	}
	file.close();
	dfa = trans(nfa, inchar, line);   //将NFA转换为DFA

	file.open(SOURCES_FILE_PATH, ios::in);
	if (!file.is_open())
	{
		cout << "Can not find the file--'sources.txt',please check!";
	}
	else
	{
		cout<<"读取到的SOURCES内容如下：\n";
		while (file >> str_file)	//按照分隔符,逐个读入到str_file,逐个处理 
		{
			string state = dfa.startState;
			char* s = str_file;
			cout << std::left << setw(10) << str_file << endl;
			string temp;			//存储token 
			while (*s)
			{
				char input = *s;
				string key;   		//构造dfa转换f的key 
				key += state;
				key += input;
				map<string, string>::iterator it;
				it = dfa.f.find(key);    //查找能否转换到下一状态 
				if (it != dfa.f.end())   //可以则转换状态,将输入加入temp 
				{
					state = it->second;
					temp += input;
				}
				else                   	 //不可以则判断状态是否为终态 
				{
					vector<string>::iterator it;
					it = find(dfa.endState.begin(), dfa.endState.end(), state);
					if (it == dfa.endState.end())     //不是终态则出错 
					{
						cout << "error";
						system("pause");
						return 0;
					}
					else                //是终态则存入token和type 
					{
						if (state == symbolState)    //如果是标识符则判断是否为关键字 
						{
							vector<string>::iterator it = find(keyword.begin(), keyword.end(), temp);
							if (it != keyword.end())
								state = keywordState;
						}
						token.push_back(temp);
						type.push_back(state);
						temp = "";
						state = dfa.startState;
						continue;
					}
				}
				s++;
				if (!*s)			//如果str_file遍历完了 
				{
					vector<string>::iterator it;
					it = find(dfa.endState.begin(), dfa.endState.end(), state);
					if (it == dfa.endState.end())     //不是终态则出错 
					{
						cout << "error";
						system("pause");
						return 0;
					}
					else                //是终态则存入token和type 
					{
						if (state == symbolState)
						{
							vector<string>::iterator it = find(keyword.begin(), keyword.end(), temp);
							if (it != keyword.end())
								state = keywordState;
						}
						token.push_back(temp);
						type.push_back(state);
						temp = "";
						state = dfa.startState;
					}
				}

			}
		}
		file.close();
	}

	file.open(MATCHING_FILE_PATH, ios::in);
	vector<string> MATCHING_type;
	vector<string> MATCHING_to_type;
	if (!file.is_open())
	{
		cout << "Can not find the file--'MATCHING.txt',please check!";
	}
	else
	{
		//不要跳过间隔符
		file.unsetf(ios::skipws);
		while (file.getline(str_file, 1000))
		{
			MATCHING_type.push_back(str_file);
		}
		//用于将每行数据以" "分隔开
		for (int i = 0;i < MATCHING_type.size();i++)
		{
			//以" "相分隔
			vector<string> v = split(MATCHING_type[i], " ");
			MATCHING_type[i] = v[0];
			MATCHING_to_type.push_back(v[1]);
		}
		file.close();
	}
	
	for(int i = 0; i < token.size(); i++)
	{
		for(int j = 0; j < MATCHING_type.size(); j++)
		{
			if( type[i].compare(MATCHING_type[j]) == 0)
			{
				type[i].assign(MATCHING_to_type[j]);
				break;
			}
		}
	}
	cout << "\n-------------------------------------------\n";
	cout << "          分析出的词法结果如下：           \n";
	for (int i = 0; i < token.size(); i++)
		cout << std::left << setw(10) << token[i] << "     --->     " << std::right << setw(15) << type[i] << endl;

	GetTime();

	file.open(OUTPUT_FILE_PATH, ios::out);

	for (int i = 0; i < token.size(); i++)
	{
		ch += token[i] + " ---> " + type[i] + "\n";
		file << ch ;
		ch = "";
	}
	file.close();
	
	cout << "\n生成的词法结果已保存到'./OUTPUT.txt'\n\n";
	system("pause");
	return 0;
}
