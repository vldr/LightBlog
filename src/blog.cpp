#include "blog.hpp"

#include <sstream>
#include "bbcode/bbcode_parser.h"
#include <iterator>
#include <string>

/**
	TODO: 
	- remove namespace...
	- remove bbcode namespace...
	- remove all the dodgy code.
**/
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

void BlogSystem::sendPage(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, std::string ss)
{
	/*std::string iisIP = "";
	for (auto& header : request->header) {
		if (header.first == "X-Forwarded-For") {
			iisIP = header.second;
		}
	}
	 
	if (iisIP != "")
		cout << ">> " << iisIP << ":" <<
		request->remote_endpoint_port << " has requested (" << request->path << ")...\n";
	else
		cout << ">> " << request->remote_endpoint_address << ":" <<
		request->remote_endpoint_port << " has requested (" << request->path << ")...\n";*/

	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) == 1) {
			// Bodgenation
			ss.append("<span style='text-align: left;font-size:10px; position:fixed; top:10; left:10; padding:10px; color:black; background-color:lightgray; border-radius:5px;'><a href='/logout' style='color:black;text-decoration:underline;'>Welcome, " + val.first + ".</a>"
				+ (request->path_match[0] == std::string("/api/view/") + std::string(request->path_match[1]) ? "<br><hr><a style='color:black;' href='/edit/" + std::string(request->path_match[1]) + "'>Edit</a><br>"
				+ " <a style='color:black;' href='/delete/" + std::string(request->path_match[1]) + "'>Delete</a><br>" + "<a style='color:black;' href='/post'>New</a>"  
				: "<br><hr><a style='color:black;' href='/post'>New</a>") + "</span>");
		}
	}
	

	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss.c_str();
}

void BlogSystem::sendPage404(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, string ss)
{
	/*std::string iisIP = "";
	for (auto& header : request->header) {
		if (header.first == "X-Forwarded-For") {
			iisIP = header.second;
		}  
	}

	if (iisIP != "")
		cout << ">> " << iisIP << ":" <<
		request->remote_endpoint_port << " has requested (" << request->path << ")...\n";
	else
		cout << ">> " << request->remote_endpoint_address << ":" <<
		request->remote_endpoint_port << " has requested (" << request->path << ")...\n";*/

	*response << "HTTP/1.1 404 Not found\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss;
}

void BlogSystem::processLogoutGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) == 1) {
			sessions[getSessionCookie(request)] = std::make_pair("", "");
			sendPage(request, response, "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>You've logged out successfully...</html>");
			return;
		} else {
			sendPage(request, response, "You must be logged in to perform this action...");
			return;
		}
	}
	else {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
}

void BlogSystem::processPostGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) != 1) {
			sendPage(request, response, "You must be logged in to perform this action...");
			return;
		}
	}
	else {
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
					<input type="text" name="title"><br>
					Content:<br>
					<textarea rows="4" name="content" cols="50"></textarea><br><br>
					<input type="submit" value="Post">
				</form>
			</html>
			)V0G0N";

	sendPage(request, response, jj.str());
}

void BlogSystem::processLoginGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	std::stringstream jj;

	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) == 1) {
			sendPage(request, response, "Logged in already...");
			return;
		}
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

			username = value;
		}

		if (key == "password") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			password = value;
		}
	}

	if (processLogin(username, password) == 1) {
		//createPost(title, con, username);

		content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>You've logged in successfully...</html>";

		/*cout << ">> " << request->remote_endpoint_address << ":" <<
		request->remote_endpoint_port << " has requested (" << request->path << ")...\n";*/

		/*if (password.length() != 64)
			password = sha256(password);*/

		string random_str = RandomString(32);
		sessions[random_str] = std::make_pair(username, password);

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

			encode(value);
			con = value;
		}

		if (key == "title") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			encode(value);
			title = value;
		}
	}

	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) != 1) {
			sendPage(request, response, "You must be logged in to perform this action...");
			return;
		}
		else {
			createPost(title, con, val.first);

			content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>OK posted...</html>";

			*response << "HTTP/1.1 200 OK\r\nSet-Cookie: peorp=" << "jioj98nnui" << ";\r\nSet-Cookie: kojij=" << "gyg7hggyug"
				<< ";\r\nContent-Length: " << content.str().length() << "\r\n\r\n" << content.str().c_str();
		}
	}
	else {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
}

void BlogSystem::processEditGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) != 1) {
			sendPage(request, response, "You must be logged in to perform this action...");
			return;
		}
	}
	else {
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
			<input type="text" name="title" value=")V0G0N" << getPostInformationById(reply, request->path_match[1], "title").str() << R"V0G0N("><br>
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

			post_id = value;
		}

		if (key == "content") {
			std::replace(value.begin(), value.end(), '+', ' ');

			value = UriDecode(value);
			encode(value);
			con = value;
		}

		if (key == "title") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			encode(value);
			title = value;
		}
	}

	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) != 1) {
			sendPage(request, response, "You must be logged in to perform this action...");
			return;
		}
		else {
			updatePost(post_id, title, con, val.first);

			content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../view/" << post_id << "\" /></head>OK posted...</html>";

			*response << "HTTP/1.1 200 OK\r\nSet-Cookie: user=" << "ftyftyf" << ";\r\nSet-Cookie: pass=" << "juhiuh87"
				<< ";\r\nContent-Length: " << content.str().length() << "\r\n\r\n" << content.str().c_str();
		}
	}
	else {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
}

void BlogSystem::processDeleteGET(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response)
{
	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) != 1) {
			sendPage(request, response, "You must be logged in to perform this action...");
			return;
		}
	}
	else {
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

			post_id = value;
		}
	}

	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		std::pair<string, string> val = sessions[getSessionCookie(request)];
		if (processLogin(val.first, val.second) != 1) {
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
	else {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
}

std::stringstream BlogSystem::getInfo(std::string input)
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

		pstmt.reset(con->prepareStatement("SELECT * FROM users WHERE id=?"));
		pstmt->setString(1, input);

		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count <= 0) {
			ss << "User not found!!";

			pstmt->close();
			con->close();
			
			return ss;
		}

		for (;;)
		{
			while (res->next()) {
				ss << "<html>";
				ss << "<pre>Username: " << res->getString("username") << "\nPassword: " << res->getString("pass")
					<< "\nID: " << res->getInt("id") << "</pre>" << endl;
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

		if ((count - (page * 10)) == count)
			ss << "<a href=\"/" << (page + 1) << "\"><div class=\"nextButton\">Next</div></a>";
		else if ((count - (page * 10)) > 10) 
			ss << "<a href=\"/" << (page - 1) << "\"><div class=\"backButton\">Back</div></a>"
			   << "<a href=\"/" << (page + 1) << "\"><div class=\"nextButton\">Next</div></a>";
		else 
			ss << "<a href=\"/" << (page - 1) << "\"><div class=\"backButton\">Back</div></a>";

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
				//cout << value.substr(0, 32) << endl;

				//cout << strNew << endl;

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

int BlogSystem::processLogin(std::string user_input, std::string pwd_input)
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

		if (pwd_input.length() != 64)
			pwd_input = sha256(pwd_input);

		pstmt->setString(2, pwd_input);

		res.reset(pstmt->executeQuery());

		size_t count = res->rowsCount();

		if (count <= 0) {
			ss << "User information is incorrect...";

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

		pwd_input = sha256(pwd_input);
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
