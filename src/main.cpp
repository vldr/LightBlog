#include "blog.hpp"
#include "uri.hpp"

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

std::string DB_HOST = "127.0.0.1";
std::string DB_USER = "root";
std::string DB_PASS = "";
std::string DB_DB = "lolyou";

using namespace std; 
using namespace boost::property_tree;

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
	const shared_ptr<ifstream> &ifs); 

int main(int argc, char* argv[]) {
	cout << "[ vldr web app - " << __DATE__ << " ]" << std::endl;

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

		thread work_thread([response, request, &blog] {
			stringstream ss = blog.getThisPost(request->path_match[1]);
			blog.sendPage(request, response, ss.str());
		});
		work_thread.detach();
	};

	server.resource["^/api/home$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			stringstream ss = blog.getPosts();
			blog.sendPage(request, response, ss.str());
		});
		work_thread.detach();
	};

	server.resource["^/post$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			std::stringstream jj;

			jj << R"V0G0N(
					<html>
						<center>
						<form action="post" method="post">
							Username:<br>
							<input type="text" name="username" placeholder=""><br>
							Password:<br>
							<input type="password" name="password" placeholder=""><br>
							<hr>
							Title:<br>
							<input type="text" name="title"><br>
							Content:<br>
							<textarea rows="4" name="content" cols="50"></textarea><br><br>
							<input type="submit" value="Post">
						</form>
					</html>
					)V0G0N";

			blog.sendPage(request, response, jj.str());
		});
		work_thread.detach();
	};

	server.resource["^/post$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			stringstream content;

			std::vector<std::string> words;
			std::string s = request->content.string();
			boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

			std::string title;
			std::string con;
			std::string username;
			std::string password;

			for (string& query : words) {
				string key;
				string value;

				int positionOfEquals = query.find("=");
				key = query.substr(0, positionOfEquals);
				if (positionOfEquals != string::npos)
					value = query.substr(positionOfEquals + 1);

				if (key == "username") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					username = value;
				}

				if (key == "password") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					password = value;
				}

				if (key == "content") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);
					
					blog.encode(value);
					con = value;
				}

				if (key == "title") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					blog.encode(value);
					title = value;
				}
			}

			if (blog.processLogin(username, password) == 1) {
				blog.createPost(title, con, username);

				content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>OK posted...</html>";

				blog.sendPage(request, response, content.str());
			}
			else {
				blog.sendPage(request, response, "unable to login... go back to edit information");
			}
		});
		work_thread.detach();
	};

	server.resource["^/edit/([a-z,A-Z,0-9]+)$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			std::stringstream jj;
			int reply;

			jj << R"V0G0N(
					<html>
						<center>
						<form action="../edit" method="post">
							Username:<br>
							<input type="text" name="username" placeholder=""><br>
							Password:<br>
							<input type="password" name="password" placeholder=""><br>

							<input type="hidden" name="post_id" value=")V0G0N" << blog.getPostInformationById(reply, request->path_match[1], "id").str() << R"V0G0N(" />
							<hr>
							Title:<br>
							<input type="text" name="title" value=")V0G0N" << blog.getPostInformationById(reply, request->path_match[1], "title").str() << R"V0G0N("><br>
							Content:<br>
							<textarea rows="20" name="content" cols="100">)V0G0N" << blog.getPostInformationById(reply, request->path_match[1], "content").str() << R"V0G0N(</textarea><br><br>
							<input type="submit" value="Update">
						</form>
					</html>
					)V0G0N";

			if (reply != 1) {
				blog.sendPage(request, response, "Post was not found...");
				return;
			}

			blog.sendPage(request, response, jj.str());
		});
		work_thread.detach();
	};

	server.resource["^/edit$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			stringstream content;

			std::vector<std::string> words;
			std::string s = request->content.string();
			boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

			std::string title;
			std::string con;
			std::string username;
			std::string post_id;
			std::string password;

			for (string& query : words) {
				string key;
				string value;

				int positionOfEquals = query.find("=");
				key = query.substr(0, positionOfEquals);
				if (positionOfEquals != string::npos)
					value = query.substr(positionOfEquals + 1);

				if (key == "post_id") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					post_id = value;
				}

				if (key == "username") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					username = value;
				}

				if (key == "password") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					password = value;
				}

				if (key == "content") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					blog.encode(value);
					con = value;
				}

				if (key == "title") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					blog.encode(value);
					title = value;
				}
			}

			if (blog.processLogin(username, password) == 1) {
				blog.updatePost(post_id, title, con, username);

				content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../view/" << post_id << "\" /></head>OK posted...</html>";

				blog.sendPage(request, response, content.str());
			}
			else {
				blog.sendPage(request, response, "unable to login... go back to edit information");
			}
		});
		work_thread.detach();
	};

	server.resource["^/delete/([a-z,A-Z,0-9]+)$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {

		thread work_thread([response, request, &blog] {
			std::stringstream jj;
			int reply = 0;

			jj << R"V0G0N(
					<html>
						<center>
						<form action="../delete" method="post">
							Username:<br>
							<input type="text" name="username" placeholder=""><br>
							Password:<br>
							<input type="password" name="password" placeholder=""><br>

							<input type="hidden" name="post_id" value=")V0G0N" << blog.getPostInformationById(reply, request->path_match[1], "id").str() << R"V0G0N(" />

							<hr>

							<b><h1>You are about to delete post ")V0G0N" << blog.getPostInformationById(reply, request->path_match[1], "title").str() << R"V0G0N("<br> 
							Are you sure?</h1></b><br>
							<input type="submit" value="Delete">
						</form>
					</html>
					)V0G0N";

			if (reply != 1) {
				blog.sendPage(request, response, "Post was not found...");
				return;
			}

			blog.sendPage(request, response, jj.str());
		});
		work_thread.detach();
	};

	server.resource["^/delete$"]["POST"] = [&blog](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		thread work_thread([response, request, &blog] {
			stringstream content;

			std::vector<std::string> words;
			std::string s = request->content.string();
			boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

			std::string title;
			std::string con;
			std::string username;
			std::string post_id;
			std::string password;

			for (string& query : words) {
				string key;
				string value;

				int positionOfEquals = query.find("=");
				key = query.substr(0, positionOfEquals);
				if (positionOfEquals != string::npos)
					value = query.substr(positionOfEquals + 1);

				if (key == "post_id") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					post_id = value;
				}

				if (key == "username") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					username = value;
				}

				if (key == "password") {
					std::replace(value.begin(), value.end(), '+', ' ');
					value = UriDecode(value);

					password = value;
				}
			}

			if (blog.processLogin(username, password) == 1) {
				blog.deletePost(post_id);

				content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>OK posted...</html>";

				blog.sendPage(request, response, content.str());
			}
			else {
				blog.sendPage(request, response, "unable to login... go back to edit information");
			}
		});
		work_thread.detach();
	};

	server.resource["^/view/([a-z,A-Z,0-9]+)$"]["GET"] = [&server, &blog](shared_ptr<HttpServer::Response> response,
		shared_ptr<HttpServer::Request> request) {
		
		try {
			auto web_root_path = boost::filesystem::canonical("web");
			auto path = boost::filesystem::canonical(web_root_path / "view");

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
	static vector<char> buffer(131072);
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