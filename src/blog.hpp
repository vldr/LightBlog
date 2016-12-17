#pragma once
#include "server_http.hpp"
#include "client_http.hpp"

#include <iostream>
#include <sstream>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <boost/functional/hash.hpp>

#include <vector>
#include "sha256.hpp"

using namespace std;

typedef HTTPExServer::Server<HTTPExServer::HTTP> HttpServer;
typedef HTTPExServer::Client<HTTPExServer::HTTP> HttpClient;

class BlogSystem
{
public:
	struct blogSessionToken {
		std::string name;
		std::string data;
		std::string ip;
	};

	BlogSystem(std::string user, std::string pwd, std::string db, std::string ip);

	void sendPage(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, std::string ss);
	void sendPage404(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, std::string ss);

	std::stringstream getInfo(std::string input);
	std::stringstream getPosts();
	std::stringstream getPostInformationById(int &reply, std::string id, std::string info);
	std::stringstream getThisPost(std::string post_id);

	std::string getSessionCookie(shared_ptr<HttpServer::Request> request);
	int createPost(std::string title, std::string content, std::string author);
	int updatePost(std::string post_id, std::string title, std::string content, std::string author);
	int deletePost(std::string post_id);
	std::string getUserID(std::string user, std::string pwd);

	int processLogin(std::string user, std::string pwd);

	std::stringstream parseBlob(istream* blob);

	void encode(std::string& data) {
		std::string buffer;
		buffer.reserve(data.size());
		for (size_t pos = 0; pos != data.size(); ++pos) {
			switch (data[pos]) {
			//case '&':  buffer.append("&amp;");       break;
			case '\"': buffer.append("&quot;");      break;
			case '\'': buffer.append("&apos;");      break;
			case '<':  buffer.append("&lt;");        break;
			case '>':  buffer.append("&gt;");        break;
			default:   buffer.append(&data[pos], 1); break;
			}
		}
		data.swap(buffer);
	}
protected:
	std::string user_g;
	std::string pwd_g;
	std::string db_g;
	std::string ip_g;
};
