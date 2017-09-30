#pragma once
#include "server_http.h"
#include "client_http.h"

#include <sstream>
#include <map>
#include <codecvt>
#include <fstream>

#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/spirit/include/qi.hpp>

#include "scrypt/libscrypt.h"
#include "scrypt/b64.h"

typedef HTTPExServer::Server<HTTPExServer::HTTP> HttpServer;
typedef HTTPExServer::Client<HTTPExServer::HTTP> HttpClient;

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
	 
	BlogSystem(std::string dbfilename);

	std::string get_user_ip(std::shared_ptr<HttpServer::Request> request);

	bool is_logged_in(std::shared_ptr<HttpServer::Request> request);

	void loggout(std::string username);

	std::string create_session(std::string username, std::shared_ptr<HttpServer::Request> request);

	void send_page(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss);
	void send_page404(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss);

	void process_logout_get(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_login_get(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);

	std::string generate_salt(int length);
	
	void process_login_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_post_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_change_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_edit_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void process_delete_post(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);

	std::stringstream get_posts(std::shared_ptr<HttpServer::Request> rq, int page = 0);
	std::string get_post_information_by_id(std::string post_id, int info);
	void add_controls_general(std::shared_ptr<HttpServer::Request> request, std::stringstream & ss);
	void add_controls_view(std::shared_ptr<HttpServer::Request> request, std::stringstream & ss, std::string post_id);
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

	const signed char HEX2DEC[256] =
	{
		/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
		/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

		/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
	};

	std::string uri_decode(const std::string & sSrc)
	{
		const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
		const size_t SRC_LEN = sSrc.length();
		const unsigned char * const SRC_END = pSrc + SRC_LEN;
		const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

		char * const pStart = new char[SRC_LEN];
		char * pEnd = pStart;

		while (pSrc < SRC_LAST_DEC)
		{
			if (*pSrc == '%')
			{
				char dec1, dec2;
				if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
					&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
				{
					*pEnd++ = (dec1 << 4) + dec2;
					pSrc += 3;
					continue;
				}
			}

			*pEnd++ = *pSrc++;
		}

		// the last 2- chars
		while (pSrc < SRC_END)
			*pEnd++ = *pSrc++;

		std::string sResult(pStart, pEnd);
		delete[] pStart;

		return sResult;
	}

	std::wstring wide_uri_decode(const std::string & sSrc)
	{
		const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
		const size_t SRC_LEN = sSrc.length();
		const unsigned char * const SRC_END = pSrc + SRC_LEN;
		const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

		char * const pStart = new char[SRC_LEN];
		char * pEnd = pStart;

		while (pSrc < SRC_LAST_DEC)
		{
			if (*pSrc == '%')
			{
				char dec1, dec2;
				if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
					&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
				{
					*pEnd++ = (dec1 << 4) + dec2;
					pSrc += 3;
					continue;
				}
			}

			*pEnd++ = *pSrc++;
		}

		// the last 2- chars
		while (pSrc < SRC_END)
			*pEnd++ = *pSrc++;

		std::wstring sResult(pStart, pEnd);
		delete[] pStart;
		return sResult;
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