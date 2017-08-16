#include "blog.hpp"

#include <sstream>
#include "bbcode/bbcode_parser.h"
#include <iterator>
#include <string>
#include <ctime>


#ifdef _WIN32
#include <Wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#endif


using namespace std;
using namespace bbcode;

BlogSystem::BlogSystem(std::string user, std::string pwd, std::string db, std::string ip)
{
	user_g = user;
	pwd_g = pwd;
	db_g = db;
	ip_g = ip;

	try {
		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(user_g, pwd_g, db_g));
		con->setSchema(ip_g);

		con->close();

		cout << "[ Connected to mysql server successfully! ]" << endl;

	}
	catch (sql::SQLException &e) {
		cout << "[ Failed to connect to mysql server! Error: " << e.getErrorCode() << "]" << endl;
	}
}

std::string BlogSystem::getUserIP(shared_ptr<HttpServer::Request> request) {
	std::string iisIP = "";
	for (auto& header : request->header) {
		if (header.first == "X-Forwarded-For") {
			iisIP = header.second;
		}
	}

	if (iisIP != "")
		return iisIP;
	else
		return request->remote_endpoint_address;
}

bool BlogSystem::isLoggedIn(shared_ptr<HttpServer::Request> request) {
	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		auto val = sessions[getSessionCookie(request)];

		if (std::get<1>(val) != getUserIP(request)) {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}

void BlogSystem::sendPage(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, std::string ss)
{
	if (isLoggedIn(request)) {
		auto val = sessions[getSessionCookie(request)];
		ss.append("<span style='text-align: left;font-size:10px; position:fixed; top:10; left:10; padding:10px; color:black; background-color:lightgray; border-radius:5px;'><a href='/logout' style='color:black;text-decoration:underline;'>Welcome, " + std::get<0>(val) + ".</a>"
			+ (request->path_match[0] == std::string("/api/view/") + std::string(request->path_match[1]) ? "<br><hr><a style='color:black;' href='/edit/" + std::string(request->path_match[1]) + "'>Edit</a><br>"
				+ " <a style='color:black;' href='/delete/" + std::string(request->path_match[1]) + "'>Delete</a><br>" + "<a style='color:black;' href='/post'>New</a>"
				: "<br><hr><a style='color:black;' href='/post'>New</a>") + "</span>");
	}

	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss.c_str();
}

void BlogSystem::sendPage404(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, string ss)
{
	*response << "HTTP/1.1 404 Not found\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss;
}

void BlogSystem::processLogoutGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (isLoggedIn(request)) {
		sessions[getSessionCookie(request)] = std::make_tuple("", "", "");
		sendPage(request, response, "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>You've logged out successfully...</html>");
		return;
	}
	else {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
}

void BlogSystem::processPostGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}

	std::stringstream jj;

	jj << R"V0G0N(
			<html>
				<head>
					<script src="//ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>

					<style>
						body {
							font-family: 'Open Sans',sans-serif;
						}
	
						.topbar {
							position: absolute !important;
		
							right: 0px !important;
							top:21px !important;
		
							width: auto !important;
							font-size:18px !important;
							padding-right:40px !important;
						}
						.topbar-li {
							display: inline !important;
							margin-left:14.5px !important;;
						}
						.topbar-li a {
							color:black !important;
							text-decoration:underline !important;
							font-family: 'Open Sans', sans-serif;
						}
					</style>
				</head>
				<center>
				
				<div class="topbar">
					<ul class="topbar-ul">
						<li class="topbar-li"><a href="../">Return home...</a></li>
					</ul>
				</div>

				<form action="post" method="post">
					Title:<br>
					<input type="text" name="title"><br><br>
					Content:<br>
					<textarea rows="20" name="content" cols="100"></textarea><br><br>
					<input type="submit" value="Post">
				</form>
			</html>
			)V0G0N";

	sendPage(request, response, jj.str());
}

void BlogSystem::processLoginGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	std::stringstream jj;

	if (isLoggedIn(request)) {
		sendPage(request, response, "Logged in already...");
		return;
	}

	jj << R"V0G0N(
			<html>
				<head>
					<script src="https://cdn.rawgit.com/Caligatio/jsSHA/master/src/sha256.js"></script>

					<script src="//ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>

					<style>
						body {
							font-family: 'Open Sans',sans-serif;
						}
	
						.topbar {
							position: absolute !important;
		
							right: 0px !important;
							top:21px !important;
		
							width: auto !important;
							font-size:18px !important;
							padding-right:40px !important;
						}
						.topbar-li {
							display: inline !important;
							margin-left:14.5px !important;;
						}
						.topbar-li a {
							color:black !important;
							text-decoration:underline !important;
							font-family: 'Open Sans', sans-serif;
						}
					</style>
				</head>
				<center>
				
				<div class="topbar">
					<ul class="topbar-ul">
						<li class="topbar-li"><a href="../">Return home...</a></li>
					</ul>
				</div>

				<form action="login" method="post">
					Username:<br>
					<input type="text" id="username" name="username" placeholder=""><br><br>
					Password:<br>
					<input type="password" id="password" name="password" placeholder=""><br><br>
					<input type="submit" value="Login">
				</form>
			</html>
			)V0G0N";

	sendPage(request, response, jj.str());
}

std::string BlogSystem::generateSalt(int length) {
#ifdef _WIN32
	HCRYPTPROV hProvider = 0;

	if (!::CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
	{
		std::cerr << "[Error at CryptAcquireContextW...]" << std::endl;
		return "";
	}

	const DWORD dwLength = 8;
	BYTE pbBuffer[dwLength] = {};

	if (!::CryptGenRandom(hProvider, dwLength, pbBuffer))
	{
		::CryptReleaseContext(hProvider, 0);
		std::cerr << "[Error at CryptGenRandom...]" << std::endl;
		return "";
	}

	uint32_t random_value;
	memcpy(&random_value, pbBuffer, dwLength);

	if (!::CryptReleaseContext(hProvider, 0)) {
		std::cerr << "[Error at CryptReleaseContext...]" << std::endl;
		return "";
	}
#else
	unsigned long long int random_value = 0;
	size_t size = sizeof(random_value);
	ifstream urandom("/dev/urandom", ios::in | ios::binary);
	if (urandom)
	{
		urandom.read(reinterpret_cast<char*>(&random_value), size);
		if (!urandom)
		{
			std::cerr << "[Failed to read from /dev/urandom]" << std::endl;
			urandom.close();
			return "";
		}
		urandom.close();
	}
	else
	{
		std::cerr << "[Failed to open /dev/urandom]" << std::endl;
		return "";
	}
#endif
	static auto& chrs = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	mersenne_twister_engine<unsigned int, 32, 624, 397,
		31, 0x9908b0df,
		11, 0xffffffff,
		7, 0x9d2c5680,
		15, 0xefc60000,
		18, 1812433253> generator(random_value);
	std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

	std::string random_str;

	random_str.reserve(length);

	while (length--)
		random_str += chrs[pick(generator)];

	return random_str;

}

void BlogSystem::processLoginPOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string username;
	std::string password;

	for (string& query : words) {
		string key;
		string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "username") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			username = value;
		}

		if (key == "password") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			password = value;
		}
	}

	if (processLogin(username, password) == 1) {
		std::string random_str = generateSalt(32);
		std::time_t t = std::time(0);
		sessions.clear();
		sessions[random_str] = std::make_tuple(username, request->remote_endpoint_address, std::to_string(t));

		content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>You've logged in successfully...</html>";
		*response << "HTTP/1.1 200 OK\r\nSet-Cookie: vldr_session=" << random_str << ";\r\nSet-Cookie: vldr_scp=" << username
			<< ";\r\nContent-Length: " << content.str().length() << "\r\n\r\n" << content.str().c_str();
	}
	else {
		sendPage(request, response, "wrong password or username...");
	}
}

void BlogSystem::processPostPOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string title;
	std::string con;

	for (string& query : words) {
		string key;
		string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "content") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			con = value;
		}

		if (key == "title") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			title = value;
		}
	}

	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
	else {
		auto val = sessions[getSessionCookie(request)];
		createPost(title, con, std::get<0>(val));

		content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>OK posted...</html>";

		*response << "HTTP/1.1 200 OK\r\nSet-Cookie: peorp=" << "jioj98nnui" << ";\r\nSet-Cookie: kojij=" << "gyg7hggyug"
			<< ";\r\nContent-Length: " << content.str().length() << "\r\n\r\n" << content.str().c_str();
	}
}

void BlogSystem::processEditGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}

	std::stringstream jj;
	int reply;

	jj << R"V0G0N(
	<html>
		<head>
			<script src="//ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>

			<style>
				body {
					font-family: 'Open Sans',sans-serif;
				}
	
				.topbar {
					position: absolute !important;
		
					right: 0px !important;
					top:21px !important;
		
					width: auto !important;
					font-size:18px !important;
					padding-right:40px !important;
				}
				.topbar-li {
					display: inline !important;
					margin-left:14.5px !important;;
				}
				.topbar-li a {
					color:black !important;
					text-decoration:underline !important;
					font-family: 'Open Sans', sans-serif;
				}
			</style>
		</head>
		<center>

		<div class="topbar">
			<ul class="topbar-ul">
				<li class="topbar-li"><a href="../">Return home...</a></li>
			</ul>
		</div>

		<form action="../edit" method="post">	
			<input type="hidden" name="post_id" value=")V0G0N" << getPostInformationById(reply, request->path_match[1], "id").str() << R"V0G0N(" />
			Title:<br>
			<input type="text" name="title" value=")V0G0N" << getPostInformationById(reply, request->path_match[1], "title").str() << R"V0G0N("><br><br>
			Content:<br>
			<textarea rows="20" name="content" cols="100">)V0G0N" << getPostInformationById(reply, request->path_match[1], "content").str() << R"V0G0N(</textarea><br><br>
			<input type="submit" value="Update">
		</form>
	</html>
	)V0G0N";

	if (reply != 1) {
		sendPage(request, response, "Post was not found...");
		return;
	}

	sendPage(request, response, jj.str());
}

void BlogSystem::processEditPOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string title;
	std::string con;
	std::string post_id;

	for (string& query : words) {
		string key;
		string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "post_id") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			post_id = value;
		}

		if (key == "content") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			con = value;
		}

		if (key == "title") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			title = value;
		}
	}

	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
	else {
		auto val = sessions[getSessionCookie(request)];
		updatePost(post_id, title, con, std::get<0>(val));

		content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../view/" << post_id << "\" /></head>OK posted...</html>";

		*response << "HTTP/1.1 200 OK\r\nSet-Cookie: user=" << "ftyftyf" << ";\r\nSet-Cookie: pass=" << "juhiuh87"
			<< ";\r\nContent-Length: " << content.str().length() << "\r\n\r\n" << content.str().c_str();
	}
}

void BlogSystem::processDeleteGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}

	std::stringstream jj;
	int reply;

	jj << R"V0G0N(
	<html>
		<head>
			<style>
				body {
					font-family: 'Open Sans',sans-serif;
				}
	
				.topbar {
					position: absolute !important;
		
					right: 0px !important;
					top:21px !important;
		
					width: auto !important;
					font-size:18px !important;
					padding-right:40px !important;
				}
				.topbar-li {
					display: inline !important;
					margin-left:14.5px !important;;
				}
				.topbar-li a {
					color:black !important;
					text-decoration:underline !important;
					font-family: 'Open Sans', sans-serif;
				}
			</style>
		</head>
		<center>

		<div class="topbar">
			<ul class="topbar-ul">
				<li class="topbar-li"><a href="../">Return home...</a></li>
			</ul>
		</div>

		<form action="../delete" method="post">	
			<input type="hidden" name="post_id" value=")V0G0N" << getPostInformationById(reply, request->path_match[1], "id").str() << R"V0G0N(" />

			<b><h1>You are about to delete post ")V0G0N" << getPostInformationById(reply, request->path_match[1], "title").str() << R"V0G0N("<br> 
			Are you sure?</h1></b><br>
			<input type="submit" value="Delete">
		</form>
	</html>
	)V0G0N";

	if (reply != 1) {
		sendPage(request, response, "Post was not found...");
		return;
	}

	sendPage(request, response, jj.str());
}

void BlogSystem::processDeletePOST(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string title;
	std::string con;
	std::string post_id;

	for (string& query : words) {
		string key;
		string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "post_id") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);
			Encode(value);

			post_id = value;
		}
	}

	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
	else {
		deletePost(post_id);

		content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>OK posted...</html>";

		*response << "HTTP/1.1 200 OK\r\nSet-Cookie: user=" << "ftyf67f" << ";\r\nSet-Cookie: pass=" << "7t6t76g76g"
			<< ";\r\nContent-Length: " << content.str().length() << "\r\n\r\n" << content.str().c_str();
	}
}


std::stringstream BlogSystem::parseBlob(istream* blob) {
	istream* content_parsed = blob;

	stringstream s;
	if (content_parsed) {
		s << content_parsed->rdbuf();
	}

	free(content_parsed);

	return s;
}

std::stringstream BlogSystem::getPosts(int page)
{
	std::stringstream ss;

	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		pstmt.reset(con->prepareStatement("SELECT * FROM posts ORDER by id DESC LIMIT 10 OFFSET ?"));
		pstmt->setInt(1, (page * 10));

		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count <= 0) {
			ss << "<br><br><center>No posts...<center>";

			pstmt->close();
			con->close();

			cache.erase(CACHEHOME + std::to_string(page));
			
			return ss;
		}

		for (;;)
		{
			while (res->next()) {
				stringstream s = parseBlob(res->getBlob("content"));
				stringstream sk = parseBlob(res->getBlob("title"));

				parser bbparser;
				bbparser.source_stream(s);
				bbparser.parse();

				std::string bbCodeParsed = bbparser.content();

				ss << R"V0G0N(
					<div class="postBox">
						<div class="postHeader">
							<a class="postTitle" href="view/)V0G0N" << res->getString("id") << "\">" << sk.str() << R"V0G0N(</a>
							<span class="postDate">)V0G0N" << res->getString("postdate") << R"V0G0N(</span>
						</div>
						<div class="postContent"><p style="white-space:pre-wrap;">)V0G0N" << bbCodeParsed << R"V0G0N(</p></div>
						<div class="postFooter">
							<span class="postAuthor">)V0G0N" << res->getString("author") << R"V0G0N(</span>
						</div>
					</div>
				)V0G0N";
			}

			if (pstmt->getMoreResults())
			{
				res.reset(pstmt->getResultSet());
				continue;
			}
			break;
		}

		pstmt.reset(con->prepareStatement("SELECT * FROM posts ORDER by id DESC"));
		res.reset(pstmt->executeQuery());

		count = res->rowsCount();

		if (count > 10) {
			if ((count - (page * 10)) == count)
				ss << "<a href=\"/" << (page + 1) << "\"><div class=\"nextButton\">Next</div></a>";
			else if ((count - (page * 10)) > 10)
				ss << "<a href=\"/" << (page - 1) << "\"><div class=\"backButton\">Back</div></a>"
				<< "<a href=\"/" << (page + 1) << "\"><div class=\"nextButton\">Next</div></a>";
			else
				ss << "<a href=\"/" << (page - 1) << "\"><div class=\"backButton\">Back</div></a>";
		}

		pstmt->close();
		con->close();

	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		ss << "mysql server failure";
	}

	cache[CACHEHOME + std::to_string(page)] = ss.str();

	return ss;
}

std::stringstream BlogSystem::getPostInformationById(int &reply, std::string post_id, std::string info)
{
	std::stringstream ss;

	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		pstmt.reset(con->prepareStatement("SELECT * FROM posts WHERE id=?"));
		pstmt->setString(1, post_id);

		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count <= 0) {
			ss << "";

			pstmt->close();
			con->close();

			reply = 0;
			return ss;
		}

		for (;;)
		{
			while (res->next()) {
				reply = 1;

				return parseBlob(res->getBlob(info));
			}
			if (pstmt->getMoreResults())
			{
				res.reset(pstmt->getResultSet());
				continue;
			}
			break;
		}

		pstmt->close();
		con->close();

	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		reply = -1;
	}

	return ss;
}

std::stringstream BlogSystem::getThisPost(std::string post_id)
{
	std::stringstream ss;

	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		pstmt.reset(con->prepareStatement("SELECT * FROM posts WHERE id=?"));
		pstmt->setString(1, post_id);

		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count != 1) {
			ss << "<br><br><center> Post was not found or duplicate posts... </center>";

			pstmt->close();
			con->close();

			cache.erase(post_id);

			return ss;
		}

		for (;;)
		{
			while (res->next()) {
				stringstream s = parseBlob(res->getBlob("content"));
				stringstream sk = parseBlob(res->getBlob("title"));

				parser bbparser;
				bbparser.source_stream(s);
				bbparser.parse();

				std::string bbCodeParsed = bbparser.content();

				ss << R"V0G0N(
					<div class="postBox">
						<div class="postHeader">
							<span class="postTitleOnPage">)V0G0N" << sk.str() << R"V0G0N(</span>
							<span class="postDate">)V0G0N" << res->getString("postdate") << R"V0G0N(</span>
						</div>
						<div class="postContent"><p style="white-space:pre-wrap;">)V0G0N" << bbCodeParsed << R"V0G0N(</p></div>
						<div class="postFooter">
							<!--<a href="../edit/)V0G0N" << res->getString("id") << R"V0G0N("><img class="editPostButton" src="http://www.famfamfam.com/lab/icons/silk/icons/application_edit.png" /></a>
							<a href="../delete/)V0G0N" << res->getString("id") << R"V0G0N("><img class="editPostButton" src="http://www.famfamfam.com/lab/icons/silk/icons/application_delete.png" /></a>-->
							<span class="postAuthor">)V0G0N" << res->getString("author") << R"V0G0N(</span>
						</div>
					</div>
				)V0G0N";

				//cout << "[ Using hdd saving item... ]" << std::endl;
			}
			if (pstmt->getMoreResults())
			{
				res.reset(pstmt->getResultSet());
				continue;
			}
			break;
		}

		pstmt->close();
		con->close();
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		ss << "mysql server failure";
	}

	cache[post_id] = ss.str();
	return ss;
}

std::stringstream BlogSystem::findPost(std::string value)
{
	std::stringstream ss;

	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {
		value = UriDecode(value);
		Encode(value);

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		pstmt.reset(con->prepareStatement("SELECT * FROM posts WHERE title LIKE ? OR content LIKE ? OR postdate LIKE ?"));
		pstmt->setString(1, "%" + value + "%");
		pstmt->setString(2, "%" + value + "%");
		pstmt->setString(3, "%" + value + "%");

		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count < 1) {
			ss << "<br><br><center>Sadly, no post(s) were found...</center>";

			pstmt->close();
			con->close();

			return ss;
		}

		for (;;)
		{
			while (res->next()) {
				stringstream s = parseBlob(res->getBlob("content"));
				stringstream sk = parseBlob(res->getBlob("title"));

				parser bbparser;
				bbparser.source_stream(s);
				bbparser.parse();

				std::string bbCodeParsed = bbparser.content();

				ss << R"V0G0N(
					<div class="postBox">
						<div class="postHeader">
							<a class="postTitle" href="view/)V0G0N" << res->getString("id") << "\">" << sk.str() << R"V0G0N(</a>
							<span class="postDate">)V0G0N" << res->getString("postdate") << R"V0G0N(</span>
						</div>
						<div class="postContent"><p style="white-space:pre-wrap;">)V0G0N" << bbCodeParsed << R"V0G0N(</p></div>
						<div class="postFooter">
							<span class="postAuthor">)V0G0N" << res->getString("author") << R"V0G0N(</span>
						</div>
					</div>
				)V0G0N";

				//cout << "[ Using hdd saving item... ]" << std::endl;
			}
			if (pstmt->getMoreResults())
			{
				res.reset(pstmt->getResultSet());
				continue;
			}
			break;
		}

		pstmt->close();
		con->close();
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		ss << "mysql server failure";
	}

	return ss;
}

std::string BlogSystem::getSessionCookie(shared_ptr<HttpServer::Request> request)
{
	for (auto& header : request->header) {
		if (header.first == "Cookie") {
			if (header.second.find("vldr_session=") != std::string::npos) {
				unsigned first = header.second.find("vldr_session=");
				string strNew = header.second.substr(first, 45);

				string key;
				string value;
				size_t positionOfEquals = strNew.find("=");
				key = strNew.substr(0, positionOfEquals);
				if (positionOfEquals != string::npos)
					value = strNew.substr(positionOfEquals + 1);

				return value.substr(0, 32);
			}
		}
	}
	return "";
}

int BlogSystem::createPost(std::string title, std::string content, std::string author)
{
	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		time_t t = time(0);
		struct tm * now = localtime(&t);
		stringstream stime;
		stime << now->tm_mday << '/'
			<< (now->tm_mon + 1) << '/'
			<< (now->tm_year + 1900)
			<< endl;

		pstmt.reset(con->prepareStatement("INSERT INTO posts (title, content, postdate, author) VALUES (?,?,?,?)"));
		pstmt->setString(1, title);
		pstmt->setString(2, content);
		pstmt->setString(3, stime.str());
		pstmt->setString(4, author);

		res.reset(pstmt->executeQuery());

		cache.clear();

		size_t count = res->rowsCount();

		if (count <= 0) {
			pstmt->close();
			con->close();

			return 0;
		}
		else {
			pstmt->close();
			con->close();

			return 1;
		}
	}
	catch (sql::SQLException e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return -1;
	}

	return 0;
}

int BlogSystem::updatePost(std::string post_id, std::string title, std::string content, std::string author)
{
	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		time_t t = time(0); 
		struct tm * now = localtime(&t);
		stringstream stime;
		stime << now->tm_mday << '/'
			<< (now->tm_mon + 1) << '/'
			<< (now->tm_year + 1900)
			<< endl;

		pstmt.reset(con->prepareStatement("UPDATE posts SET title=?, content=?, postdate=?, author=? WHERE id=?"));

		pstmt->setString(1, title);
		pstmt->setString(2, content);
		pstmt->setString(3, stime.str());
		pstmt->setString(4, author);
		pstmt->setString(5, post_id);

		

		res.reset(pstmt->executeQuery());

		cache.clear();

		size_t count = res->rowsCount();

		if (count <= 0) {
			pstmt->close();
			con->close();

			return 0;
		}
		else {
			pstmt->close();
			con->close();

			return 1;
		}


	}
	catch (sql::SQLException e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return -1;
	}

	return 0;
}

int BlogSystem::deletePost(std::string post_id)
{
	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		pstmt.reset(con->prepareStatement("DELETE FROM posts WHERE id=?"));
		pstmt->setString(1, post_id);

		res.reset(pstmt->executeQuery());

		cache.clear();

		size_t count = res->rowsCount();

		if (count <= 0) {
			pstmt->close();
			con->close();

			return 0;
		}
		else {



			pstmt->close();
			con->close();

			return 1;
		}
	}
	catch (sql::SQLException e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return -1;
	}

	return 0;
}


int BlogSystem::hashPassword(char *dst, const char *passphrase, uint32_t N, uint8_t r, uint8_t p) {
	int retval;
	uint8_t salt[SCRYPT_SALT_LEN] = {};
	uint8_t	hashbuf[SCRYPT_HASH_LEN];
	char outbuf[256];
	char saltbuf[256];

	std::string generatedSalt = generateSalt(SCRYPT_SALT_LEN);
	if (generatedSalt == "")
	{
		return 0;
	}

	copy(generatedSalt.begin(), generatedSalt.end(), salt);
	salt[generatedSalt.length() - 1] = 0;

	retval = libscrypt_scrypt((const uint8_t*)passphrase, strlen(passphrase),
		(uint8_t*)salt, SCRYPT_SALT_LEN, N, r, p, hashbuf, sizeof(hashbuf));
	if (retval == -1)
		return 0;

	retval = libscrypt_b64_encode((unsigned char*)hashbuf, sizeof(hashbuf),
		outbuf, sizeof(outbuf));
	if (retval == -1)
		return 0;

	retval = libscrypt_b64_encode((unsigned char *)salt, sizeof(salt),
		saltbuf, sizeof(saltbuf));
	if (retval == -1)
		return 0;

	retval = libscrypt_mcf(N, r, p, saltbuf, outbuf, dst);
	if (retval != 1)
		return 0;

	return 1;
}

int BlogSystem::processLogin(std::string user_input, std::string pwd_input)
{
	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		pstmt.reset(con->prepareStatement("SELECT * FROM users WHERE username=?"));
		pstmt->setString(1, user_input);
		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count != 1) {
			pstmt->close();
			con->close();

			return 0;
		}

		for (;;)
		{
			while (res->next()) 
			{
				/*char finalhh[1024];
				const char *passphrase = pwd_input.c_str();
				hashPassword(finalhh, passphrase, SCRYPT_N, SCRYPT_r, SCRYPT_p);

				std::cout << finalhh << std::endl;*/

				char dst[1024];
				auto pwd = res->getString("pass").asStdString();

				copy(pwd.begin(), pwd.end(), dst);
				dst[pwd.length()] = 0;

				if (libscrypt_check(dst, pwd_input.c_str()) > 0)
					return 1;
				else
					return 0;

				pstmt->close();
				con->close();

				return 0;
			}
			if (pstmt->getMoreResults())
			{
				res.reset(pstmt->getResultSet());
				continue;
			}
			break;
		}
	}
	catch (sql::SQLException e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return -1;
	}

	return 0;
}

std::string BlogSystem::getUserID(std::string user_input, std::string pwd_input)
{
	std::stringstream ss;

	string url(user_g);
	const string user(pwd_g);
	const string pass(db_g);
	const string database(ip_g);

	try {

		sql::Driver * driver = get_driver_instance();

		std::auto_ptr< sql::Connection > con(driver->connect(url, user, pass));
		con->setSchema(database);

		std::auto_ptr< sql::PreparedStatement >  pstmt;
		std::auto_ptr< sql::ResultSet > res;

		pstmt.reset(con->prepareStatement("SELECT * FROM users WHERE username=? AND pass=?"));
		pstmt->setString(1, user_input);

		//pwd_input = sha256(pwd_input);
		pstmt->setString(2, pwd_input);

		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count != 1) {
			pstmt->close();
			con->close();

			return "";
		}

		for (;;)
		{
			while (res->next()) {
				stringstream ss;
				ss << res->getString("id");

				pstmt->close();
				con->close();

				return ss.str();
			}
			if (pstmt->getMoreResults())
			{
				res.reset(pstmt->getResultSet());
				continue;
			}
			break;
		}
	}
	catch (sql::SQLException e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return "";
	}

	return "";
}
