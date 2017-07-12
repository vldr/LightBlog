#include "blog.hpp"

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/range.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <string.h>

std::string DB_HOST = "127.0.0.1";
std::string DB_USER = "root";
std::string DB_PASS = "";
std::string DB_DB = "blog";

#define DBG

using namespace std;
using namespace boost::property_tree;

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
	const shared_ptr<ifstream> &ifs); 

int main(int argc, char* argv[]) {
#ifdef DBG
	cout << "[ vldr web app - " << __DATE__ << " ]" << std::endl;
#endif // DBG

	HttpServer server(8080, 1);
	
	if (argc == 5) {
		DB_HOST = argv[1];
		DB_USER = argv[2];

		if (std::string(argv[3]) == "\"\"")
			DB_PASS = "";
		else
			DB_PASS = argv[3];
		
		DB_DB = argv[4];
		cout << "[ Using parameters to connect... ]" << std::endl;
	}

	BlogSystem blog(DB_HOST, DB_USER, DB_PASS, DB_DB);

	server.resource["^/api/view/([a-z,A-Z,0-9]+)$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		if (blog.reload) {
			if (blog.ss_articles.find(request->path_match[1]) == blog.ss_articles.end()) {
				//cout << "[ Using hdd not found item... ]" << std::endl;

				thread work_thread([response, request, &blog] {
					stringstream ss = blog.getThisPost(request->path_match[1]);
					blog.sendPage(request, response, ss.str());
					blog.reload = true;
				});
				work_thread.detach();
			}
			else {
				//cout << "[ Using cache... ]" << std::endl;

				blog.sendPage(request, response, blog.ss_articles[request->path_match[1]]);
			}
		}
		else {
			thread work_thread([response, request, &blog] {
				stringstream ss = blog.getThisPost(request->path_match[1]);
				blog.sendPage(request, response, ss.str()); 
				blog.reload = true;
			});
			work_thread.detach();
		}
	};

	server.resource["^/api/home$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		if (!blog.reload || blog.ss_posts.length() == 0) {
			thread work_thread([response, request, &blog] 
			{
				stringstream ss = blog.getPosts();
				blog.sendPage(request, response, ss.str());
				//cout << "[ Using hdd... ]" << std::endl;
			});
			work_thread.detach();
		}
		else {
			blog.sendPage(request, response, blog.ss_posts);
			//cout << "[ Using cache... ]" << std::endl;
		}
	};

	server.resource["^/post$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			blog.processPostGET(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/post$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			blog.processPostPOST(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/logout$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			blog.processLogoutGET(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/login$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			blog.processLoginGET(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/login$"]["POST"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			blog.processLoginPOST(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/reload$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {


		thread work_thread([response, request, &blog] {
			if (blog.sessions.find(blog.getSessionCookie(request)) != blog.sessions.end()) {
				std::pair<string, string> val = blog.sessions[blog.getSessionCookie(request)];
				if (blog.processLogin(val.first, val.second) == 1) {
					blog.reload = false;
					blog.ss_articles.clear();
					blog.ss_posts.clear();
					blog.sendPage(request, response, "Cache cleared...");
					return;
				}
				else {
					blog.sendPage(request, response, "You need to be logged in to perform this action...");
					return;
				}
			}
			else {
				blog.sendPage(request, response, "You need to be logged in to perform this action...");
				return;
			}
		});
		work_thread.detach();
	};

	server.resource["^/edit/([a-z,A-Z,0-9]+)$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			blog.processEditGET(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/edit$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			blog.processEditPOST(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/delete/([a-z,A-Z,0-9]+)$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			blog.processDeleteGET(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/delete$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			blog.processDeletePOST(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/view/([a-z,A-Z,0-9]+)$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / "view");

			auto cykablyat = make_shared<istringstream>();
			auto ifs = make_shared<ifstream>();
			ifs->open(path.string(), ifstream::out | ios::binary);

			if (*ifs) {
				ifs->seekg(0, ios::end);
				auto length = ifs->tellg();
				ifs->seekg(0, ios::beg);

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=60\r\nContent-Length: " << length << "\r\n\r\n";
				default_resource_send(server, response, ifs);
			}
			else
				throw invalid_argument("could not read file");
		}
		catch (const exception &e) {
			string content = "404: Not Found";
			blog.sendPage404(request, response, content);
		}
	};

	server.default_resource["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / request->path);

			if (distance(web_root_path.begin(), web_root_path.end())>distance(path.begin(), path.end()) ||
				!equal(web_root_path.begin(), web_root_path.end(), path.begin()))
				throw invalid_argument("path must be within root path");
			if (boost::filesystem::is_directory(path))
				path /= "index";
			if (!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
				throw invalid_argument("file does not exist");

			auto ifs = make_shared<ifstream>();
			ifs->open(path.string(), ifstream::in | ios::binary);

			if (*ifs) {
				ifs->seekg(0, ios::end);
				auto length = ifs->tellg();

				ifs->seekg(0, ios::beg);

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=60\r\nContent-Length: " << length << "\r\n\r\n";
				default_resource_send(server, response, ifs);
			}
			else
				throw invalid_argument("could not read file");
		}
		catch (const exception &e) {
			string content = "404: Not Found";
			blog.sendPage404(request, response, content);
		}
	};

	thread server_thread([&server]() {
		server.start();
	});

	this_thread::sleep_for(chrono::seconds(1));
	server_thread.join();
	return 0;
}

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs) {
	vector<char> buffer(131072);
	streamsize read_length;
	if ( ( read_length = ifs->read(&buffer[0], buffer.size()).gcount() ) > 0) {
		response->write(&buffer[0], read_length);

		if (read_length == static_cast<streamsize>(buffer.size())) {
			server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
				if (!ec)
					default_resource_send(server, response, ifs);
				else
					cerr << "Connection interrupted" << endl;
			});
		}
	}
}