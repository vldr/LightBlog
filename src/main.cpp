#include "blog.h"

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

using namespace boost::property_tree;

void default_resource_send(const HttpServer &server, const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs);

int main(int argc, char* argv[]) 
{
#ifdef _WIN32
	system("cls");
#endif // _WIN32

	std::cout << "[ vldr web app - " << __DATE__ << " ]" << std::endl;

	HttpServer server(8080, 1); 

	std::string filename = "sql.db";

	if (argc == 3) {
		filename = argv[1];
		server.config.port = atoi(argv[2]);

		std::cout << "[ Using parameters to connect... ]" << std::endl;
	}

	BlogSystem blog(filename);

	server.resource["^/api/find/" + blog.REGEXSEARCH + "$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request, &blog]
		{
			blog.send_page(request, response, blog.find_post(request, request->path_match[1]).str());
		});
		work_thread.detach();
	};

	server.resource["^/api/view/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {
		if (blog.cache.find(request->path_match[1]) == blog.cache.end() || blog.is_logged_in(request)) {
			std::thread work_thread([response, request, &blog]
			{
				//std::cout << "Sending from db..." << std::endl;
				blog.send_page(request, response, blog.get_this_post(request, request->path_match[1]).str());
			});
			work_thread.detach();
			
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog.send_page(request, response, blog.cache[request->path_match[1]]);
		}
	};

	server.resource["^/api/home$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		if (blog.cache.find(blog.CACHEHOME + "0") == blog.cache.end() || blog.is_logged_in(request)) {
			std::thread work_thread([response, request, &blog]
			{
				//std::cout << "Sending from db..." << std::endl;
				blog.send_page(request, response, blog.get_posts(request).str());
			});
			work_thread.detach();
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog.send_page(request, response, blog.cache[blog.CACHEHOME + "0"]);
		}
	};

	server.resource["^/api/home/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {
		if (blog.cache.find(blog.CACHEHOME + std::string(request->path_match[1])) == blog.cache.end() || blog.is_logged_in(request)) {
			std::thread work_thread([response, request, &blog]
			{
				//std::cout << "Sending from db..." << std::endl;
				blog.send_page(request, response, blog.get_posts(request, std::stoi(request->path_match[1])).str());
			});
			work_thread.detach();
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog.send_page(request, response, blog.cache[blog.CACHEHOME + std::string(request->path_match[1])]);

		}
	};

	server.resource["^/api/post$"]["POST"] = [&blog](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request, &blog] {
			blog.process_post_post(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/logout$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		std::thread work_thread([response, request, &blog] {
			blog.process_logout_get(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/login$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		std::thread work_thread([response, request, &blog] {
			blog.process_login_get(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/login$"]["POST"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		std::thread work_thread([response, request, &blog] {
			blog.process_login_post(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/reload$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		std::thread work_thread([response, request, &blog] {
			if (blog.is_logged_in(request)) {
				blog.cache.clear();
				blog.send_page(request, response, "Cache cleared...");
			}
			else
				blog.send_page(request, response, "You need to be logged in to perform this action...");
		});
		work_thread.detach();
	};

	server.resource["^/change$"]["POST"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		std::thread work_thread([response, request, &blog] {
			blog.process_change_post(request, response);
		});
		work_thread.detach();
	};


	server.resource["^/edit$"]["POST"] = [&blog](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request, &blog] {
			blog.process_edit_post(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/delete$"]["POST"] = [&blog](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request, &blog] {
			blog.process_delete_post(request, response);
		});
		work_thread.detach();
	};

	server.resource["^/view/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / "view");

			auto cykablyat = std::make_shared<std::istringstream>();
			auto ifs = std::make_shared<std::ifstream>();
			ifs->open(path.string(), std::ifstream::out | std::ios::binary);

			if (*ifs) {
				ifs->seekg(0, std::ios::end);
				auto length = ifs->tellg();
				ifs->seekg(0, std::ios::beg);

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=60\r\nContent-Length: " << length << "\r\n\r\n";
				default_resource_send(server, response, ifs);
			}
			else
				throw std::invalid_argument("could not read file");
		}
		catch (const std::exception &e) {
			std::string content = "404: Not Found";
			blog.send_page404(request, response, content);
		}
	};

	server.resource["^/" + blog.REGEXNUMBER + "$"]["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response,
		std::shared_ptr<HttpServer::Request> request) {

		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / "index");

			auto cykablyat = std::make_shared<std::istringstream>();
			auto ifs = std::make_shared<std::ifstream>();
			ifs->open(path.string(), std::ifstream::out | std::ios::binary);

			if (*ifs) {
				ifs->seekg(0, std::ios::end);
				auto length = ifs->tellg();
				ifs->seekg(0, std::ios::beg);

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=60\r\nContent-Length: " << length << "\r\n\r\n";
				default_resource_send(server, response, ifs);
			}
			else
				throw std::invalid_argument("could not read file");
		}
		catch (const std::exception &e) {
			std::string content = "404: Not Found";
			blog.send_page404(request, response, content);
		}
	};

	server.default_resource["GET"] = [&server, &blog](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / request->path);

			if (std::distance(web_root_path.begin(), web_root_path.end())>std::distance(path.begin(), path.end()) ||
				!std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
				throw std::invalid_argument("path must be within root path");
			if (boost::filesystem::is_directory(path))
				path /= "index";
			if (!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
				throw std::invalid_argument("file does not exist");

			auto ifs = std::make_shared<std::ifstream>();
			ifs->open(path.string(), std::ifstream::in | std::ios::binary);

			if (*ifs) {
				ifs->seekg(0, std::ios::end);
				auto length = ifs->tellg();

				ifs->seekg(0, std::ios::beg);

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=60\r\nContent-Length: " << length << "\r\n\r\n";
				default_resource_send(server, response, ifs);
			}
			else
				throw std::invalid_argument("could not read file");
		}
		catch (const std::exception &e) {
			std::string content = "404: Not Found";
			blog.send_page404(request, response, content);
		}
	};

	std::thread server_thread([&server]() {
		server.start();
	});
	server_thread.join();

	 
	return 0;
}

void default_resource_send(const HttpServer &server, const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs) {
	int KB128 = 131072;
	std::vector<char> buffer(KB128 * 32);
	std::streamsize read_length;
	if ( ( read_length = ifs->read(&buffer[0], buffer.size()).gcount() ) > 0) {
		response->write(&buffer[0], read_length);

		if (read_length == static_cast<std::streamsize>(buffer.size())) {
			server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
				if (!ec)
					default_resource_send(server, response, ifs);
				else
					std::cerr << "Connection interrupted" << std::endl;
			});
		}
	}
}