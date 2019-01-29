/*************************************************************************
	> File Name: Server_init.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月01日 星期六 15时22分55秒
 ************************************************************************/

#ifndef _SERVER_INIT_H
#define _SERVER_INIT_H

#include "../rapidjson/document.h"
#include "../rapidjson/filereadstream.h"

#include "./base.hpp"
#include <map>
#include <iostream>

namespace ConfigValue
{
std::map<std::string, std::string> Type;
std::map<int, std::string> Status;

} // namespace ConfigValue

namespace ConfigServer
{
class BaseFp
{
  public:
	BaseFp() {}
	BaseFp(const BaseFp &) = delete;
	BaseFp &operator=(const BaseFp &) = delete;

	explicit BaseFp(const char *path, const char *mode = "r")
	{
		http_fp = fopen(path, mode);
		if (!http_fp)
			throw CallFailed("Server_init.h 文件：fopen function failed !!! at line  ", __LINE__);
	}
	~BaseFp()
	{
		fclose(http_fp);
	}
	inline FILE *GetFp()
	{
		return http_fp;
	}

  protected:
	FILE *http_fp;
};

class ServerInit
{
  public:
	ServerInit(const ServerInit &) = delete;
	ServerInit &operator=(const ServerInit &) = delete;
	~ServerInit() {}

	ServerInit() : http_fp("../config/Http_init.json") /* , server_fp("../config/Server_init.json") */
	{
		//InitServer();
		InitHttp();
	}

  private:
	// void InitServer()
	// {
	// 	rapidjson::FileReadStream stream(server_fp.GetFp(), read_buffer, sizeof(read_buffer));
	// 	rapidjson::Document bank;
	// 	bank.ParseStream(stream);
	// 	assert(bank.IsObject());

	// 	static const int MAX_FD = bank["server-config"]["max-fd"].GetInt();
	// 	static const int MAX_EPOLL_EVENTS = bank["server-config"]["max-epoll-events"].GetInt();
	// 	static const int MAX_THREADS = bank["server-config"]["max-threads"].GetInt();
	// 	static const int READ_BUFFER = bank["server-config"]["read-buffer"].GetInt();
	// 	static const int WRITE_BUFFER = bank["server-config"]["write-buffer"].GetInt();
	// 	static const int MAX_FILE_LEN = bank["server-config"]["filename-len"].GetInt();
	// 	static const std::string doc_root = bank["default-config"]["www-root"].GetString();
	// }
	void InitHttp()
	{
		//printf("进入InitHttp 函数\n");

		rapidjson::FileReadStream stream(http_fp.GetFp(), read_buffer, sizeof(read_buffer));

		rapidjson::Document bank;
		bank.ParseStream(stream);

		assert(bank.IsObject());

		const rapidjson::Value &content_type = bank["content-types"];
		assert(content_type.IsArray());
		/*初始化　content-type */
		for (auto &v : content_type.GetArray())
		{
			ConfigValue::Type.insert({v["ext"].GetString(), v["type"].GetString()});
		}
		//for (auto &t : ConfigValue::Type)
		//	std::cout << t.first << " , " << t.second << std::endl;

		const rapidjson::Value &status = bank["http-status-string"];
		assert(status.IsArray());
		/*初始化　http-status */
		for (auto &v : status.GetArray())
		{
			ConfigValue::Status.insert({v["status"].GetInt(), v["description"].GetString()});
		}
	}

	char read_buffer[65535];
	BaseFp http_fp;
	//BaseFp server_fp;
};
} // namespace ConfigServer

#endif
