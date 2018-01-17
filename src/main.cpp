#include "blog.h"

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

namespace BlogPages { 
	HttpServer * server;
	BlogSystem * blog;

	inline void default_resource_send(const HttpServer* server, const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs) {		int KB128 = 131072;
		std::vector<char> buffer(KB128);
		std::streamsize read_length;
		if ((read_length = ifs->read(&buffer[0], buffer.size()).gcount()) > 0) {
			response->write(&buffer[0], read_length);

			if (read_length == static_cast<std::streamsize>(buffer.size())) {
				server->send(response, [&server, response, ifs](const boost::system::error_code &ec) {
					if (!ec)
						BlogPages::default_resource_send(server, response, ifs);
				});
			}
		}
	}

	inline void view_post_get(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		try {
			auto web_root_path = boost::filesystem::canonical("web");

			boost::filesystem::path path = boost::filesystem::canonical(web_root_path / "view");

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

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=86400\r\nContent-Length: " << length << "\r\n\r\n";
				BlogPages::default_resource_send(server, response, ifs);
			}
			else
				throw std::invalid_argument("could not read file");
		}
		catch (const std::exception &e) {
			std::string content = "<h1>Not Found</h1><p>The requested URL was not found on this server.</p>";
			blog->send_page404(request, response, content);
		}
	}

	inline void view_all_posts_get(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		try {
			auto web_root_path = boost::filesystem::canonical("web");

			boost::filesystem::path path = boost::filesystem::canonical(web_root_path / "index");

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

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=86400\r\nContent-Length: " << length << "\r\n\r\n";
				BlogPages::default_resource_send(server, response, ifs);
			}
			else
				throw std::invalid_argument("could not read file");
		}
		catch (const std::exception &e) {
			std::string content = "<h1>Not Found</h1><p>The requested URL was not found on this server.</p>";
			blog->send_page404(request, response, content);
		}
	}

	inline void get_file(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		try {
			auto web_root_path = boost::filesystem::canonical("web");

			boost::filesystem::path path;

			path = boost::filesystem::canonical(web_root_path / request->path);

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

				*response << "HTTP/1.1 200 OK\r\nCache-Control:max-age=86400\r\nContent-Length: " << length << "\r\n\r\n";
				BlogPages::default_resource_send(BlogPages::server, response, ifs);
			}
			else 
				throw std::invalid_argument("could not read file");
		}
		catch (const std::exception &e) {  
			std::string content = "<h1>Not Found</h1><p>The requested URL was not found on this server.</p>";
			blog->send_page404(request, response, content);
		}
	}

	inline void find_post(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		//std::thread work_thread([response, request]
		//{
			blog->send_page(request, response, blog->find_post(request, request->path_match[1]).str());
		//});
		//work_thread.detach();
	}

	inline void change_post(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request] {
			blog->process_change_post(request, response);
		});
		work_thread.detach();
	}

	inline void edit_post(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		//std::thread work_thread([response, request] {
			BlogPages::blog->process_edit_post(request, response);
		//});
		//work_thread.detach();
	}

	inline void delete_post(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		//std::thread work_thread([response, request] {
			BlogPages::blog->process_delete_post(request, response);
		//});
		//work_thread.detach(); 
	}

	inline void login_post(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request] {
			BlogPages::blog->process_login_post(request, response);
		}); 
		work_thread.detach();
	}

	inline void login_get(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request] {
			BlogPages::blog->process_login_get(request, response);
		});
		work_thread.detach();
	}

	inline void view_post(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		if (BlogPages::blog->cache.find(request->path_match[1]) == blog->cache.end() || blog->is_logged_in(request)) {
			//std::thread work_thread([response, request] {
				//std::cout << "Sending from db..." << std::endl;
				blog->send_page(request, response, blog->get_this_post(request, request->path_match[1]).str());
			//});
			//work_thread.detach();

		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog->send_page_encoded(request, response, blog->cache[request->path_match[1]]);
		}
	}

	inline void view_home(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		if (BlogPages::blog->cache.find(blog->CACHEHOME + "0") == blog->cache.end() || blog->is_logged_in(request)) {
			//std::thread work_thread([response, request] {
			//std::cout << "Sending from db..." << std::endl;
			blog->send_page(request, response, blog->get_posts(request).str());
			//});
			//work_thread.detach();
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog->send_page_encoded(request, response, blog->cache[blog->CACHEHOME + "0"]);
		}
	}

	inline void view_specific_home(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		if (BlogPages::blog->cache.find(blog->CACHEHOME + std::string(request->path_match[1])) == blog->cache.end() || blog->is_logged_in(request)) {
			//std::thread work_thread([response, request]
			//{
				//std::cout << "Sending from db..." << std::endl;
				blog->send_page(request, response, blog->get_posts(request, std::stoi(request->path_match[1])).str());
			//});
			//work_thread.detach();
		}
		else {
			//std::cout << "Sending from cache..." << std::endl;
			blog->send_page_encoded(request, response, blog->cache[blog->CACHEHOME + std::string(request->path_match[1])]);

		}
	}

	inline void post_post(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		//std::thread work_thread([response, request] {
			BlogPages::blog->process_post_post(request, response);
		//});
		//work_thread.detach();
	}

	inline void logout_get(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		//std::thread work_thread([response, request] {
			BlogPages::blog->process_logout_get(request, response);
		//});
		//work_thread.detach();
	}

	inline void reload_get(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
		std::thread work_thread([response, request] {
			if (BlogPages::blog->is_logged_in(request)) {
				BlogPages::blog->cache.clear();
				BlogPages::blog->send_page(request, response, "Cache cleared...");
			}
			else
				blog->send_page(request, response, "You need to be logged in to perform this action...");
		});
		work_thread.detach();
	}
};


int main(int argc, char* argv[]) 
{
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
	
	std::cout << "[ vldr web app - " << __DATE__ << " ]" << std::endl;

	std::string filename = "sql.db";
	int threads = 1; 

	if (argc > 1) {
		std::cout << "[ Using " << argv[1] << " threads ]" << std::endl;
		threads = atoi(argv[1]);
	} 
	  
	if (argc == 4) {
		std::cout << "[ Using " << argv[3] << " as sql filename ]" << std::endl;
		filename = argv[3];
	} 

	BlogPages::server = new HttpServer(8080, threads);

	if (argc > 2) {
		std::cout << "[ Using " << argv[2] << " as port ]" << std::endl;
		BlogPages::server->config.port = atoi(argv[2]);
	}

	BlogPages::blog = new BlogSystem(filename); 

	BlogPages::server->resource["^/login$"]["GET"] = BlogPages::login_get;
	BlogPages::server->resource["^/login$"]["POST"] = BlogPages::login_post;
	BlogPages::server->resource["^/delete$"]["POST"] = BlogPages::delete_post;
	BlogPages::server->resource["^/edit$"]["POST"] = BlogPages::edit_post;
	BlogPages::server->resource["^/change$"]["POST"] = BlogPages::change_post;
	BlogPages::server->resource["^/api/post$"]["POST"] = BlogPages::post_post;

	BlogPages::server->resource["^/reload$"]["GET"] = BlogPages::reload_get;
	BlogPages::server->resource["^/logout$"]["GET"] = BlogPages::logout_get;
	BlogPages::server->resource["^/api/home/" + BlogPages::blog->REGEXNUMBER + "$"]["GET"] = BlogPages::view_specific_home;
	BlogPages::server->resource["^/api/home$"]["GET"] = BlogPages::view_home;
	BlogPages::server->resource["^/api/view/" + BlogPages::blog->REGEXSEARCH + "$"]["GET"] = BlogPages::view_post;
	BlogPages::server->resource["^/api/find/" + BlogPages::blog->REGEXSEARCH + "$"]["GET"] = BlogPages::find_post;
	      
	BlogPages::server->resource["^/index/" + BlogPages::blog->REGEXNUMBER + "$"]["GET"] = BlogPages::view_all_posts_get;
	BlogPages::server->resource["^/index$"]["GET"] = BlogPages::view_all_posts_get;
	BlogPages::server->resource["^/" + BlogPages::blog->REGEXSEARCH + "$"]["GET"] = BlogPages::view_post_get;
	BlogPages::server->default_resource["GET"] = BlogPages::get_file; 
	 
	BlogPages::server->start();

	delete[] BlogPages::server;
	delete[] BlogPages::blog;
	  
	return 0;
}