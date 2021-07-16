#include <iostream>
#include <string>
#include <thread>
#include <mirai/mirai.h>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "myheader.h"

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

MiraiBot bot;
string input;
int CommandNum;

void CommandSys()
{
	map<string, int> CommandType = {
		{"stop",1}
	};
	while (true)
	{
		try
		{
			cin >> input;
			CommandNum = CommandType[input];
			switch (CommandNum)
			{
				// 结束运行程序
			case 1:
				bot.Disconnect();
				exit(0);
				break;
				// 指令错误回复
			default:
				cout << "指令错误" << endl;
				break;
			}
		}
		catch (const std::exception& ex)
		{
			cout << ex.what() << endl;
		}

	}
}

int main()
{
#if defined(WIN32) || defined(_WIN32)
	// 切换代码页，让 CMD 可以显示 UTF-8 字符
	system("chcp 65001");
#endif

	
	SessionOptions opts;
	opts.BotQQ = 2556410868_qq;				// 请修改为你的机器人QQ
	opts.HttpHostname = "localhost";		// 请修改为和 mirai-api-http 配置文件一致
	opts.WebSocketHostname = "localhost";	// 同上
	opts.HttpPort = 5036;					// 同上
	opts.WebSocketPort = 5036;				// 同上
	opts.EnableVerify = false;
	opts.SingleMode = true;
	opts.VerifyKey = "VerifyKey";			// 同上

	while (true)
	{
		try
		{
			cout << "尝试与 mirai-api-http 建立连接..." << endl;
			bot.Connect(opts);
			break;
		}
		catch (const std::exception& ex)
		{
			cout << ex.what() << endl;
		}
		MiraiBot::SleepSeconds(1);
	}
	cout << "Bot Working..." << endl;

	thread CommandSystem(CommandSys);
	CommandSystem.detach();
	
	httplib::Server ser;

	


	ser.Post("/webhook", [](const httplib::Request& req, httplib::Response& res) {
		cout << req.body << endl << endl;
		json whjson = json::parse(req.body);


		struct repo
		{
			string FullName; // 完整仓库名 
			string ref; // 完整分支名 
			int64_t commitNum; // 提交数量 
			string lastCommitID; // 上次提交短ID 
			string thisCommitID; // 当次提交短ID 
		};
		

		struct commit
		{
			string ID;
			string Time;
			string Message;
			string author; // 提交作者 
			string authorEmail;
			string committer; // 提交者 
			string committerEmail;
		};

		struct commitPath
		{
			string id;
			string time;
			string msg;
			string author;
			string authorEmail;
			string committer;
			string committerEmail;	
		};

		string finalResult;

		try
		{
			if (whjson.find("/pusher"_json_pointer) != whjson.end())
			{
				repo newRepo;

				newRepo.FullName = whjson.at("/repository/full_name"_json_pointer);
				newRepo.ref = whjson.at("/ref"_json_pointer);
				newRepo.lastCommitID = to_string(whjson.at("/before"_json_pointer)).erase(8);
				newRepo.thisCommitID = to_string(whjson.at("/after"_json_pointer)).erase(8);
				newRepo.commitNum = whjson.at("/commits").size();

				finalResult = "[" + newRepo.FullName + ":" + newRepo.ref + "]\n"
							+ to_string(newRepo.commitNum) + " new commit(s) "
							+ newRepo.lastCommitID + " -> " + newRepo.thisCommitID
							+ "\n---status---\n"; 

				commit commits;
				commitPath path;

				for (int64_t pointCommit = 0; pointCommit < newRepo.commitNum; pointCommit++)
				{
					path.id = "/commits/" + to_string(pointCommit) + "/id";
					commits.ID = to_string(whjson.at("commits").at(pointCommit).at("id")).erase(8);
				}
				
			}
		}
		catch(const exception& e)
		{
			std::cerr << e.what() << '\n';
		}
		
		
		
		res.set_content("received!", "text/plain");
		});

	
	std::cout << "listening 0.0.0.0:2256" << std::endl;

	// 在失去与mah的连接后重连
	bot.On<LostConnection>([&](LostConnection e)
		{
			cout << e.ErrorMessage << " (" << e.Code << ")" << endl;
			while (true)
			{
				try
				{
					cout << "尝试连接 mirai-api-http..." << endl;
					bot.Reconnect();
					cout << "与 mirai-api-http 重新建立连接!" << endl;
					break;
				}
				catch (const std::exception& ex)
				{
					cout << ex.what() << endl;
				}
				MiraiBot::SleepSeconds(1);
			}
		});

	ser.listen("0.0.0.0", 2256);

	string cmd;
	while (cin >> cmd)
	{
		if (cmd == "exit")
		{
			// 程序结束前必须调用 Disconnect，否则 mirai-api-http 会内存泄漏。
			bot.Disconnect();
			exit(0);
		}
	}
	
	return 0;
}