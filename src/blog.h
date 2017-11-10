#pragma once

#include "server_http.h"

#include <sstream>
#include <map>
#include <random>
#include <codecvt>
#include <fstream> 
#include <iomanip>

#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/spirit/include/qi.hpp>

#include "scrypt/libscrypt.h"
#include "scrypt/b64.h"

#define BOOST_SPIRIT_THREADSAFE 
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/random/uniform_int_distribution.hpp>

typedef HTTPExServer::Server<HTTPExServer::HTTP> HttpServer; 

class BlogSystem
{
public: 
	enum codons : int
	{
		id,
		title,
		content,
		author,
		postdate
	};

	typedef std::map<std::string, std::string> PostData;
	 
	BlogSystem(std::string dbfilename);

	std::string get_user_ip(std::shared_ptr<HttpServer::Request> request);

	bool is_logged_in(std::shared_ptr<HttpServer::Request> request);

	void loggout(std::string username);

	std::string create_session(std::string username, std::shared_ptr<HttpServer::Request> request);

	void send_page(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss);
	void send_page_encoded(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss);
	void send_page404(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss);

	void process_logout_get(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_login_get(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);

	std::string generate_salt(int length);
	
	void process_login_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_post_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_change_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	std::map<std::string, std::string> get_post_data(std::shared_ptr<HttpServer::Request> request);
	void process_edit_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_delete_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);

	std::stringstream get_posts(std::shared_ptr<HttpServer::Request> rq, int page = 0);
	std::string get_post_information_by_id(std::string post_id, int info);
	void add_controls_general(std::shared_ptr<HttpServer::Request> request, std::stringstream & ss);
	void add_controls_view(std::shared_ptr<HttpServer::Request> request, std::stringstream & ss, std::string post_id);
	static std::string compress(const std::string & data);
	std::string truncate(std::string str, size_t width, bool show_ellipsis);
	std::stringstream get_this_post(std::shared_ptr<HttpServer::Request> request, std::string post_id);
	std::stringstream find_post(std::shared_ptr<HttpServer::Request> request, std::string searchparam);

	std::string get_session_cookie(std::shared_ptr<HttpServer::Request> request);
	int create_post(std::string title, std::string content, std::string author);
	int update_post(std::string post_id, std::string title, std::string content, std::string author);
	int delete_post(std::string post_id); 

	int hash_password(char * dst, const char * passphrase, uint32_t N, uint8_t r, uint8_t p);
	int change_user_details(std::string user_input, std::string pwd_input, std::shared_ptr<HttpServer::Request> request);
	int process_login(std::string user, std::string pwd);

	int get_user_id(std::string user_input);

	std::stringstream parse_blob(std::istream* blob);
	 
	std::string uri_decode(const std::string& in)
	{
		std::string out = ""; 

		out.clear();
		out.reserve(in.size());
		for (std::size_t i = 0; i < in.size(); ++i)
		{
			if (in[i] == '%')
			{
				if (i + 3 <= in.size())
				{
					int value = 0;
					std::istringstream is(in.substr(i + 1, 2));
					if (is >> std::hex >> value)
					{
						out += static_cast<char>(value);
						i += 2;
					}
					else
					{
						return out;
					}
				}
				else
				{
					return out;
				}
			}
			else if (in[i] == '+')
			{
				out += ' ';
			}
			else
			{
				out += in[i];
			}
		} 

		return out;
	}

	const std::string CACHEHOME = "home";
	const std::string REGEXNUMBER = "([0-9]{1,9})";
	const std::string REGEXSEARCH = "([a-z,A-Z,0-9,%,-,_,.,!,~,*,',(,)]+)";
	const int SESSIONEXPIRETIME = 1800;

	std::map<std::string, std::string> cache;
	std::map<std::string, std::string> sessions;
	std::time_t past = 0; 
protected:
	
	std::string filename = "sql.db";
}; 