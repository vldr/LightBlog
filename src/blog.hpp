#pragma once
#include "server_http.hpp"
#include "client_http.hpp"

#include <iostream>
#include <sstream>
#include <map>
#include <codecvt>
#include <fstream>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/spirit/include/qi.hpp>

#include <vector>
#include "scrypt/libscrypt.h"
#include "scrypt/b64.h"

typedef std::stringstream(*functionGetPost)(std::string);
typedef std::stringstream(*functionGetHome)();

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

	std::string getUserIP(shared_ptr<HttpServer::Request> request);

	bool isLoggedIn(shared_ptr<HttpServer::Request> request);

	void sendPage(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, std::string ss);
	void sendPage404(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, std::string ss);

	void processLogoutGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);

	void processPostGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);
	void processLoginGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);

	std::string generateSalt(int length);
	
	void processLoginPOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);
	void processPostPOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);

	void processChangeGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);
	void processChangePOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);

	void processEditGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);
	void processEditPOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);

	void processDeleteGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);
	void processDeletePOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response);

	std::stringstream getPosts(int page = 0);
	std::stringstream getPostInformationById(int &reply, std::string id, std::string info);
	std::stringstream getThisPost(std::string post_id);
	std::stringstream findPost(std::string searchparam);

	std::string getSessionCookie(shared_ptr<HttpServer::Request> request);
	int createPost(std::string title, std::string content, std::string author);
	int updatePost(std::string post_id, std::string title, std::string content, std::string author);
	int deletePost(std::string post_id);
	std::string getUserID(std::string user, std::string pwd);

	int hashPassword(char * dst, const char * passphrase, uint32_t N, uint8_t r, uint8_t p);
	void changeUserDetails(std::string user_input, std::string pwd_input, shared_ptr<HttpServer::Request> request);
	int processLogin(std::string user, std::string pwd);

	std::stringstream parseBlob(istream* blob);

	void Encode(std::string& data) {
#ifdef _WIN32
		wstring_convert<codecvt_utf8<unsigned int>, unsigned int> cv;
#else
		wstring_convert<codecvt_utf8<char32_t>, char32_t> cv;
#endif // _WIN32
		
		auto str32 = cv.from_bytes(data);
		std::string buffer;

		for (auto c : str32)
			if (uint_least32_t(c) > 127)
				buffer.append("&#" + std::to_string(uint_least32_t(c)) + ";");
			else 
				switch ((char)c) {
					//case '&':  buffer.append("&amp;");       break;
					case '\"': buffer.append("&quot;");      break;
					case '\'': buffer.append("&apos;");      break;
					case '<':  buffer.append("&lt;");        break;
					case '>':  buffer.append("&gt;");        break;
					default: buffer.push_back((char)c);      break;
				}
			
		data.swap(buffer);
	}

	const char HEX2DEC[256] =
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

	std::map<string, string> cache;
	std::map<string, std::tuple<string, string, string>> sessions;
protected:
	
	std::string user_g;
	std::string pwd_g;
	std::string db_g;
	std::string ip_g;
};