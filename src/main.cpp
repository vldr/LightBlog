#include "blog.hpp"

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;
using namespace boost::property_tree;

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
	const shared_ptr<ifstream> &ifs); 

int main(int argc, char* argv[]) 
{
#ifdef _WIN32
	system("cls");
#endif // _WIN32

	cout << "[ vldr web app - " << __DATE__ << " ]" << std::endl;

	HttpServer server(8080, 1);

	std::string filename = "sql.db";

	if (argc == 3) {
		filename = argv[1];
		server.config.port = atoi(argv[2]);

		cout << "[ Using parameters to connect... ]" << std::endl;
	}

	BlogSystem blog(filename);

	server.resource["^/api/find/" + blog.REGEXSEARCH + "$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog]
		{
			blog.sendPage(request, response, blog.findPost(request, request->path_match[1]).str());
		});
		work_thread.detach();
	};

	server.resource["^/api/view/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {
		if (blog.cache.find(request->path_match[1]) == blog.cache.end() || blog.isLoggedIn(request)) {
			thread work_thread([response, request, &blog]
			{
				//std::cout << "Sending from db..." << std::endl;
				blog.sendPage(request, response, blog.getThisPost(request, request->path_match[1]).str());
			});
			work_thread.detach();
			
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog.sendPage(request, response, blog.cache[request->path_match[1]]);
		}
	};

	server.resource["^/api/home$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		if (blog.cache.find(blog.CACHEHOME + "0") == blog.cache.end() || blog.isLoggedIn(request)) {
			thread work_thread([response, request, &blog]
			{
				//std::cout << "Sending from db..." << std::endl;
				blog.sendPage(request, response, blog.getPosts(request).str());
			});
			work_thread.detach();
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog.sendPage(request, response, blog.cache[blog.CACHEHOME + "0"]);
		}
	};

	server.resource["^/api/home/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {
		if (blog.cache.find(blog.CACHEHOME + std::string(request->path_match[1])) == blog.cache.end() || blog.isLoggedIn(request)) {
			thread work_thread([response, request, &blog]
			{
				//std::cout << "Sending from db..." << std::endl;
				blog.sendPage(request, response, blog.getPosts(request, std::stoi(request->path_match[1])).str());
			});
			work_thread.detach();
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog.sendPage(request, response, blog.cache[blog.CACHEHOME + std::string(request->path_match[1])]);

		}
	};

	server.resource["^/api/post$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
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
			if (blog.isLoggedIn(request)) {
				blog.cache.clear();
				blog.sendPage(request, response, "Cache cleared...");
			}
			else
				blog.sendPage(request, response, "You need to be logged in to perform this action...");
		});
		work_thread.detach();
	};

	server.resource["^/change$"]["POST"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			blog.processChangePOST(request, response);
		});
		work_thread.detach();
	};


	server.resource["^/edit$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			blog.processEditPOST(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/delete$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			blog.processDeletePOST(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/view/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
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

	server.resource["^/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / "index");

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
	server_thread.join();

	 
	return 0;
}

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs) {
	int KB128 = 131072;
	vector<char> buffer(KB128 * 32);
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