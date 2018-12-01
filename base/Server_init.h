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

namespace CONFIG
{
std::map<std::string, std::string> Type;
std::map<int, std::string> Status;

int MAX_FD = 0;
int MAX_EPOLL_EVENTS = 0;
int MAX_THREADS = 0;
int READ_BUFFER = 0;
int WRITE_BUFFER = 0;
int MAX_FILE_LEN = 0;
std::string doc_root;

} // namespace CONFIG

using namespace rapidjson;
class BaseFp
{
  public:
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

	ServerInit() : http_fp("../config/Http_init.json"), server_fp("../config/Server_init.json")
	{
		InitServer();
		InitHttp();
	}

  private:
	void InitServer()
	{
		FileReadStream stream(server_fp.GetFp(), read_buffer, sizeof(read_buffer));
		Document bank;
		bank.ParseStream(stream);
		assert(bank.IsObject());

		CONFIG::MAX_FD = bank["server-config"]["max-fd"].GetInt();
		CONFIG::MAX_EPOLL_EVENTS = bank["server-config"]["max-epoll-events"].GetInt();
		CONFIG::MAX_THREADS = bank["server-config"]["max-threads"].GetInt();
		CONFIG::READ_BUFFER = bank["server-config"]["read-buffer"].GetInt();
		CONFIG::WRITE_BUFFER = bank["server-config"]["write-buffer"].GetInt();
		CONFIG::MAX_FILE_LEN = bank["server-config"]["filename-len"].GetInt();
		doc_root = bank["default-config"]["www-root"].GetString();
	}
	void InitHttp()
	{
		FileReadStream stream(http_fp.GetFp(), read_buffer, sizeof(read_buffer));

		Document bank;
		bank.ParseStream(stream);

		assert(bank.IsObject());

		const Value &content_type = bank["content-types"];
		assert(content_type.IsArray());
		/*初始化　content-type */
		for (auto &v : content_type.GetArray())
		{
			CONFIG::Type.insert({v["ext"].GetString(), v["type"].GetString()});
		}

		const Value &status = bank["http-status-string"];
		assert(status.IsArray());
		/*初始化　http-status */
		for (auto &v : status.GetArray())
		{
			CONFIG::Status.insert({v["status"].GetInt(), v["description"].GetString()});
		}
	}

	char read_buffer[65535];
	BaseFp http_fp;
	BaseFp server_fp;
};

#endif
