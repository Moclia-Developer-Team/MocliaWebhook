
#include <iostream>
#include <string>
#include <mirai/mirai.h>
#include <thread>
#include <httplib.h>
#include <nlohmann/json.hpp>

// fu*k windows.h
#ifdef max
#undef max
#endif
#ifdef SendMessage
#undef SendMessage
#endif
#ifdef CreateEvent
#undef CreateEvent
#endif

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

MiraiBot bot;
httplib::Server ser;
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

	
	SessionOptions opts = SessionOptions::FromJsonFile("./config.json");

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

		string finalResult;

		try
		{
			if (whjson.find("pusher") != whjson.end())
			{
				repo newRepo;

				newRepo.FullName = whjson.at("repository").at("full_name");
				newRepo.ref = whjson.at("ref");
				newRepo.lastCommitID = to_string(whjson.at("before")).erase(8);
				newRepo.thisCommitID = to_string(whjson.at("after")).erase(8);
				newRepo.commitNum = whjson.at("commits").size();

				finalResult = "[" + newRepo.FullName + ":" + newRepo.ref + "]\n"
					+ to_string(newRepo.commitNum) + " new commit(s) "
					+ newRepo.lastCommitID + " -> " + newRepo.thisCommitID
					+ "\n---status---\n"; 

				commit commits;

				for (int64_t pointCommit = 0; pointCommit < newRepo.commitNum; pointCommit++)
				{
					commits.ID = to_string(whjson.at("commits").at(pointCommit).at("id")).erase(8);
					commits.Time = whjson.at("commits").at(pointCommit).at("timestamp");
					commits.author = whjson.at("commits").at(pointCommit).at("author").at("name");
					commits.authorEmail = whjson.at("commits").at(pointCommit).at("author").at("email");
					commits.committer = whjson.at("commits").at(pointCommit).at("committer").at("name");
					commits.committerEmail = whjson.at("commits").at(pointCommit).at("author").at("email");
					commits.Message = whjson.at("commits").at(pointCommit).at("message");

					finalResult += "[" + commits.ID + "] " + commits.Time + "\n"
						+ "Author: " + commits.author + "<" + commits.authorEmail + ">\n"
						+ "Committer: " + commits.committer + "<" + commits.committerEmail + ">\n"
						+ "Message: " + commits.Message + "\n";
				}
				MessageChain msg = MessageChain().Plain(finalResult);
				ifstream conf("./config.json");
				json config;
				conf >> config;
				int64_t group = config.at("sendGroup");
				GID_t sendGroup = GID_t(group);
				bot.SendMessage(sendGroup, msg);
			}
			else
			{
				cout << "不支持该json" << endl;
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
	return 0;
}