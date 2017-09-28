#pragma once
#include "server_http.hpp"
#include "client_http.hpp"

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

typedef std::stringstream(*functionGetPost)(std::string);
typedef std::stringstream(*functionGetHome)();

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

	std::string getUserIP(std::shared_ptr<HttpServer::Request> request);

	bool isLoggedIn(std::shared_ptr<HttpServer::Request> request);

	void loggout(std::string username);

	std::string createSession(std::string username, std::shared_ptr<HttpServer::Request> request);

	void sendPage(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss);
	void sendPage404(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss);

	void processLogoutGET(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void processLoginGET(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);

	std::string generateSalt(int length);
	
	void processLoginPOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void processPostPOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void processChangePOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void processEditPOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);
	void processDeletePOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response);

	std::stringstream getPosts(std::shared_ptr<HttpServer::Request> rq, int page = 0);
	std::string getPostInformationById(std::string post_id, int info);
	void addControlsGeneral(std::shared_ptr<HttpServer::Request> request, std::stringstream & ss);
	void addControlsView(std::shared_ptr<HttpServer::Request> request, std::stringstream & ss, std::string post_id);
	std::stringstream getThisPost(std::shared_ptr<HttpServer::Request> request, std::string post_id);
	std::stringstream findPost(std::shared_ptr<HttpServer::Request> request, std::string searchparam);

	std::string getSessionCookie(std::shared_ptr<HttpServer::Request> request);
	int createPost(std::string title, std::string content, std::string author);
	int updatePost(std::string post_id, std::string title, std::string content, std::string author);
	int deletePost(std::string post_id);

	int hashPassword(char * dst, const char * passphrase, uint32_t N, uint8_t r, uint8_t p);
	int changeUserDetails(std::string user_input, std::string pwd_input, std::shared_ptr<HttpServer::Request> request);
	int processLogin(std::string user, std::string pwd);

	int getUserID(std::string user_input);

	std::stringstream parseBlob(std::istream* blob);

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

	std::string UriDecode(const std::string & sSrc)
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

	std::wstring WideUriDecode(const std::string & sSrc)
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
	const int SESSIONEXPIRETIME = 360;

	std::map<std::string, std::string> cache;
	std::map<std::string, std::tuple<std::string, std::string, std::string>> sessions;
protected:
	
	std::string filename = "sql.db";
	std::string user_g = "";
	std::string pwd_g = "";
	std::string db_g = "";
	std::string ip_g = "";
};