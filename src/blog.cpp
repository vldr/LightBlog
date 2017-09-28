#include "blog.hpp"
#include "sqlite_modern_cpp.h"
#include "bbcode/bbcode_parser.h"

#include <sstream>
#include <iterator>
#include <string>
#include <ctime>

#ifdef _WIN32
#include <Wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#endif

BlogSystem::BlogSystem(std::string filename)
{
	try {
		this->filename = filename;
		sqlite::database db(filename);

		db << "create table if not exists users ("
			"   id integer primary key autoincrement not null,"
			"   username text,"
			"   pass text"
			");";

		db << "create table if not exists posts ("
			"   id integer primary key autoincrement not null,"
			"   title text,"
			"   content BLOB,"
			"   postdate text,"
			"   author text"
			");";

		std::cout << "[ Connected to database successfully! ]" << std::endl;

	}
	catch (std::exception &e) {
		std::cout << "[ Failed to connect to database! Error: " << e.what() << "]" << std::endl;
	}
}

std::string BlogSystem::getUserIP(std::shared_ptr<HttpServer::Request> request) {
	std::string iisIP = "";
	for (auto& header : request->header) {
		if (header.first == "X-Forwarded-For") {
			iisIP = header.second;
		}
	}
	
	if (iisIP != "") {
		std::string::size_type loc = iisIP.find(":", 0);
		if (loc != std::string::npos) {
			iisIP.erase(loc, iisIP.length() - loc);
		}

		return iisIP;
	}
	else
		return request->remote_endpoint_address;
}

bool BlogSystem::isLoggedIn(std::shared_ptr<HttpServer::Request> request) {
	if (sessions.find(getSessionCookie(request)) != sessions.end()) {
		auto val = sessions[getSessionCookie(request)];

		if (getUserID(std::get<0>(val)) == 0 || std::get<1>(val) != getUserIP(request)) {
			loggout(std::get<0>(val));
			return false;
		}
		else {
			std::time_t current = std::time(0);
			std::time_t past = std::stoi(std::get<2>(val));

			if (current - past >= SESSIONEXPIRETIME) {
				loggout(std::get<0>(val));
				return false;
			}
		}
	}
	else {
		return false;
	}
	 
	return true;
}

void BlogSystem::loggout(std::string username) {
	for (auto& item : sessions) {
		if (std::get<0>(item.second) == username) {
			sessions.erase(item.first);
		}
	}
}

std::string BlogSystem::createSession(std::string username, std::shared_ptr<HttpServer::Request> request) {
	std::string sessionName = generateSalt(32);
	std::time_t t = std::time(0);

	loggout(username);
	sessions[sessionName] = std::make_tuple(username, getUserIP(request), std::to_string(t));

	return sessionName;
}

void BlogSystem::sendPage(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss)
{
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss.c_str();
}

void BlogSystem::sendPage404(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response, std::string ss)
{
	*response << "HTTP/1.1 404 Not found\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss;
}

void BlogSystem::processLogoutGET(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response)
{
	if (isLoggedIn(request)) {
		auto val = sessions[getSessionCookie(request)];
		loggout(std::get<0>(val));

		sendPage(request, response, "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>You've logged out successfully...</html>");
		return;
	}
	else {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
}


void BlogSystem::processLoginGET(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response)
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
	try {
#ifdef _WIN32
	HCRYPTPROV hProvider = 0;

	if (!::CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
	{
		std::cerr << "[Error at CryptAcquireContextW...]" << std::endl;
		throw std::runtime_error("Error at CryptAcquireContextW...");
	}

	const DWORD dwLength = 8;
	BYTE pbBuffer[dwLength] = {};

	if (!::CryptGenRandom(hProvider, dwLength, pbBuffer))
	{
		::CryptReleaseContext(hProvider, 0);
		std::cerr << "[Error at CryptGenRandom...]" << std::endl;
		throw std::runtime_error("Error at CryptGenRandom...");
	}

	uint32_t random_value;
	memcpy(&random_value, pbBuffer, dwLength);

	if (!::CryptReleaseContext(hProvider, 0)) {
		std::cerr << "[Error at CryptReleaseContext...]" << std::endl;
		throw std::runtime_error("Error at CryptReleaseContext...");
	}
#else
	unsigned long long int random_value = 0;
	size_t size = sizeof(random_value);
	std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
	if (urandom)
	{
		urandom.read(reinterpret_cast<char*>(&random_value), size);
		if (!urandom)
		{
			std::cerr << "[Failed to read from /dev/urandom]" << std::endl;
			urandom.close();
			throw std::runtime_error("Failed to read from /dev/urandom");
		}
		urandom.close();
	}
	else
	{
		std::cerr << "[Failed to open /dev/urandom]" << std::endl;
		throw std::runtime_error("Failed to open /dev/urandom");
	}
#endif

	static auto& chrs = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	std::mersenne_twister_engine<unsigned int, 32, 624, 397,
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
	catch (std::exception &e)
	{
		std::cout << "# ERR: RandomException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return "";
	}
}

void BlogSystem::processLoginPOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response)
{
	std::stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string username;
	std::string password;

	for (std::string& query : words) {
		std::string key;
		std::string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != std::string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "username") {
			std::replace(value.begin(), value.end(), '+', ' ');

			username = value;
		}

		if (key == "password") {
			std::replace(value.begin(), value.end(), '+', ' ');

			password = value;
		}
	}

	if (processLogin(username, password) == 1) {
		std::string random_str = createSession(username, request);

		content << "<html><head><meta http-equiv=\"refresh\" content=\"0; url=../\" /></head>You've logged in successfully...</html>";
		*response << "HTTP/1.1 200 OK\r\nSet-Cookie: vldr_session=" << random_str << ";\r\nSet-Cookie: vldr_scp=" << username
			<< ";\r\nContent-Length: " << content.str().length() << "\r\n\r\n" << content.str().c_str();
	}
	else {
		sendPage(request, response, "wrong password or username...");
	}
}

void BlogSystem::processPostPOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response)
{
	std::stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string title;
	std::string con;

	for (std::string& query : words) {
		std::string key;
		std::string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != std::string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "content") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			con = value;
		}

		if (key == "title") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			title = value;
		}
	}

	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
	else {
		auto val = sessions[getSessionCookie(request)];
		if (createPost(title, con, std::get<0>(val))) 
			sendPage(request, response, "1");
		else
			sendPage(request, response, "0");
	}
}


void BlogSystem::processChangePOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response)
{
	std::stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string newPassword = "";
	std::string newUsername = "";

	for (std::string& query : words) {
		std::string key;
		std::string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != std::string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "pass") {
			std::replace(value.begin(), value.end(), '+', ' ');

			newPassword = value;
		}

		if (key == "user") {
			std::replace(value.begin(), value.end(), '+', ' ');

			newUsername = value;
		}
	}

	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
	else {
		if (newPassword.empty() || newUsername.empty()) {
			sendPage(request, response, "Username or password cannot be empty...");
			return;
		}
		else
			if (changeUserDetails(newUsername, newPassword, request))
				sendPage(request, response, "1");
			else
				sendPage(request, response, "0");
	}
}

void BlogSystem::processEditPOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response)
{
	std::stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string title;
	std::string con;
	std::string post_id;

	for (std::string& query : words) {
		std::string key;
		std::string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != std::string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "post_id") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			post_id = value;
		}

		if (key == "content") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			con = value;
		}

		if (key == "title") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			title = value;
		}
	}

	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
	else {
		auto val = sessions[getSessionCookie(request)];

		if (updatePost(post_id, title, con, std::get<0>(val)))
			sendPage(request, response, "1");
		else
			sendPage(request, response, "0");
	}
}


void BlogSystem::processDeletePOST(std::shared_ptr<HttpServer::Request> request, std::shared_ptr<HttpServer::Response> response)
{
	std::stringstream content;

	std::vector<std::string> words;
	std::string s = request->content.string();
	boost::split(words, s, boost::is_any_of("&"), boost::token_compress_on);

	std::string title;
	std::string con;
	std::string post_id;

	for (std::string& query : words) {
		std::string key;
		std::string value;

		size_t positionOfEquals = query.find("=");
		key = query.substr(0, positionOfEquals);
		if (positionOfEquals != std::string::npos)
			value = query.substr(positionOfEquals + 1);

		if (key == "post_id") {
			std::replace(value.begin(), value.end(), '+', ' ');
			value = UriDecode(value);

			post_id = value;
		}
	}

	if (!isLoggedIn(request)) {
		sendPage(request, response, "You must be logged in to perform this action...");
		return;
	}
	else {
		if (deletePost(post_id))
			sendPage(request, response, "1");
		else
			sendPage(request, response, "0");
	}
}


std::stringstream BlogSystem::parseBlob(std::istream* blob) {
	std::istream* content_parsed = blob;

	std::stringstream s;
	if (content_parsed) {
		s << content_parsed->rdbuf();
	}

	free(content_parsed);

	return s;
}

std::stringstream BlogSystem::getPosts(std::shared_ptr<HttpServer::Request> request, int page)
{
	std::stringstream ss;

	try {
		sqlite::database db(this->filename);

		int count = 0;
		db << "select count(*) from posts;" >> count;

		if (count <= 0) {
			addControlsGeneral(request, ss);
			ss << "<br><br><center>No posts...<center>";
			cache.erase(CACHEHOME + std::to_string(page));
			
			return ss;
		}

		db << "select id,title,content,postdate,author from posts ORDER by id DESC LIMIT 10 OFFSET ? ;"
			<< (page * 10)
			>> [&](int id, std::string title, std::string content, std::string postdate, std::string author)
		{
			std::stringstream s(content);

			bbcode::parser bbparser;
			bbparser.source_stream(s);
			bbparser.parse();

			ss << R"V0G0N(
					<div class="postBox">
						<div class="postHeader">
							<a class="postTitle" href="view/)V0G0N" << id << "\">" << title << R"V0G0N(</a>
							<span class="postDate">)V0G0N" << postdate << R"V0G0N(</span>
						</div>
						<div class="postContent"><p style="white-space:pre-wrap;">)V0G0N" << bbparser.content() << R"V0G0N(</p></div>
						<div class="postFooter">
							<span class="postAuthor">)V0G0N" << author << R"V0G0N(</span>
						</div>
					</div>
				)V0G0N";
		};

		db << "select count(*) from posts ORDER by id DESC;" >> count;

		if (count > 10) {
			if ((count - (page * 10)) == count)
				ss << "<a href=\"/" << (page + 1) << "\"><div class=\"nextButton\">Next</div></a>";
			else if ((count - (page * 10)) > 10)
				ss << "<a href=\"/" << (page - 1) << "\"><div class=\"backButton\">Back</div></a>"
				<< "<a href=\"/" << (page + 1) << "\"><div class=\"nextButton\">Next</div></a>";
			else
				ss << "<a href=\"/" << (page - 1) << "\"><div class=\"backButton\">Back</div></a>";
		}

		cache[CACHEHOME + std::to_string(page)] = ss.str();
		addControlsGeneral(request, ss);

	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		ss << "database server failure";
	}

	return ss;
}

std::string BlogSystem::getPostInformationById(std::string post_id, int info)
{
	std::stringstream ss;
	try {
		sqlite::database db(this->filename);

		int count = 0;
		db << "select count(*) from posts where id=?;"
			<< post_id
			>> count;

		if (count != 1) {
			return "error - post not found...";
		}
		switch (info)
		{
		case codons::id:
			db << "select id from posts where id=? ;" << post_id >> [&](std::string content) { ss << content; };
			break;
		case codons::title:
			db << "select title from posts where id=? ;" << post_id >> [&](std::string content) { ss << content; };
			break;
		case codons::author:
			db << "select author from posts where id=? ;" << post_id >> [&](std::string content) { ss << content; };
			break;
		case codons::content:
			db << "select content from posts where id=? ;" << post_id >> [&](std::string content) { ss << content; };
			break;
		case codons::postdate:
			db << "select postdate from posts where id=? ;" << post_id >> [&](std::string content) { ss << content; };
			break;
		default:
			ss << "unknown codon";
			break;
		}

		return ss.str();
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return "database server failure";
	}
}

void BlogSystem::addControlsGeneral(std::shared_ptr<HttpServer::Request> request, std::stringstream &ss) {
	if (isLoggedIn(request)) {
		auto val = sessions[getSessionCookie(request)];
		ss << R"V0G0N(
					<span class="settings-modal-open" onclick="document.getElementById('settings-modal').style.display = 'block';">
						<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 18 18" class="icon-3rMtHF">
							<path d="M7.15546853,6.47630098e-17 L5.84453147,6.47630098e-17 C5.36185778,-6.47630098e-17 4.97057344,0.391750844 4.97057344,0.875 L4.97057344,1.9775 C4.20662236,2.21136254 3.50613953,2.61688993 2.92259845,3.163125 L1.96707099,2.61041667 C1.76621819,2.49425295 1.52747992,2.46279536 1.30344655,2.52297353 C1.07941319,2.58315171 0.88846383,2.73002878 0.77266168,2.93125 L0.117193154,4.06875 C0.00116776262,4.26984227 -0.0302523619,4.50886517 0.0298541504,4.73316564 C0.0899606628,4.9574661 0.236662834,5.14864312 0.437644433,5.26458333 L1.39171529,5.81583333 C1.21064614,6.59536289 1.21064614,7.40609544 1.39171529,8.185625 L0.437644433,8.736875 C0.236662834,8.85281521 0.0899606628,9.04399223 0.0298541504,9.2682927 C-0.0302523619,9.49259316 0.00116776262,9.73161606 0.117193154,9.93270833 L0.77266168,11.06875 C0.88846383,11.2699712 1.07941319,11.4168483 1.30344655,11.4770265 C1.52747992,11.5372046 1.76621819,11.5057471 1.96707099,11.3895833 L2.92259845,10.836875 C3.50613953,11.3831101 4.20662236,11.7886375 4.97057344,12.0225 L4.97057344,13.125 C4.97057344,13.6082492 5.36185778,14 5.84453147,14 L7.15546853,14 C7.63814222,14 8.02942656,13.6082492 8.02942656,13.125 L8.02942656,12.0225 C8.79337764,11.7886375 9.49386047,11.3831101 10.0774016,10.836875 L11.032929,11.3895833 C11.2337818,11.5057471 11.4725201,11.5372046 11.6965534,11.4770265 C11.9205868,11.4168483 12.1115362,11.2699712 12.2273383,11.06875 L12.8828068,9.93270833 C12.9988322,9.73161606 13.0302524,9.49259316 12.9701458,9.2682927 C12.9100393,9.04399223 12.7633372,8.85281521 12.5623556,8.736875 L11.6082847,8.185625 C11.7893539,7.40609544 11.7893539,6.59536289 11.6082847,5.81583333 L12.5623556,5.26458333 C12.7633372,5.14864312 12.9100393,4.9574661 12.9701458,4.73316564 C13.0302524,4.50886517 12.9988322,4.26984227 12.8828068,4.06875 L12.2273383,2.93270833 C12.1115362,2.73148712 11.9205868,2.58461004 11.6965534,2.52443187 C11.4725201,2.46425369 11.2337818,2.49571128 11.032929,2.611875 L10.0774016,3.16458333 C9.49400565,2.61782234 8.79351153,2.2117896 8.02942656,1.9775 L8.02942656,0.875 C8.02942656,0.391750844 7.63814222,6.47630098e-17 7.15546853,6.47630098e-17 Z M8.5,7 C8.5,8.1045695 7.6045695,9 6.5,9 C5.3954305,9 4.5,8.1045695 4.5,7 C4.5,5.8954305 5.3954305,5 6.5,5 C7.03043298,5 7.53914081,5.21071368 7.91421356,5.58578644 C8.28928632,5.96085919 8.5,6.46956702 8.5,7 Z" transform="translate(2.5 2)"></path>
						</svg>
					</span>
					<div class="settings-modal" id="settings-modal" style="display: none;">
						<div class="settings-modal-inner">
							<span class="settings-modal-close" onclick="document.getElementById('settings-modal').style.display = 'none';">&#215;</span>
							<div class="settings-modal-left">
							   <ul>
									<li><a onclick="openTab(event, 'create')">Create</a></li>
									<li><a onclick="openTab(event, 'settings')">Settings</a></li>
									<li><a class="separate"></a></li>
									<li><a href="/logout">Log Out</a></li>
								</ul>
							</div>

							<div class="settings-modal-right">
								<div id="settings-tab-content">
									<div id="tab-create" style="display:block;" class="settings-tab-content">
										<form id="create-post" action="/api/post" method="post">
											<h1>Title:</h1>
								
											<input class="textbox-other" type="text" name="title">
											<h1>Content:</h1>
								
											<textarea class="textbox-other" rows="10" name="content" style="resize:vertical;" cols="50"></textarea><br><br>
											<input type="submit" value="Post">
										</form>
									</div>
						
									<div id="tab-settings" class="settings-tab-content">
										<form id="change-post" action="/change" method="post">
											<h1>Username:</h1>
											<input class="textbox-other" type="text" name="user" value=")V0G0N" << std::get<0>(val) << R"V0G0N(">
								
											<h1>Password:</h1>
								
											<input class="textbox-other" type="password" name="pass"><br><br>
											<input type="submit" value="Update">
										</form>
									</div>
								</div>
							</div>
						</div>
					</div>
				)V0G0N";
	}
}

void BlogSystem::addControlsView(std::shared_ptr<HttpServer::Request> request, std::stringstream &ss, std::string post_id) {
	if (isLoggedIn(request)) {
		auto val = sessions[getSessionCookie(request)];
		
		ss << R"V0G0N(
					<span class="settings-modal-open" onclick="document.getElementById('settings-modal').style.display = 'block';">
						<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 18 18" class="icon-3rMtHF">
							<path d="M7.15546853,6.47630098e-17 L5.84453147,6.47630098e-17 C5.36185778,-6.47630098e-17 4.97057344,0.391750844 4.97057344,0.875 L4.97057344,1.9775 C4.20662236,2.21136254 3.50613953,2.61688993 2.92259845,3.163125 L1.96707099,2.61041667 C1.76621819,2.49425295 1.52747992,2.46279536 1.30344655,2.52297353 C1.07941319,2.58315171 0.88846383,2.73002878 0.77266168,2.93125 L0.117193154,4.06875 C0.00116776262,4.26984227 -0.0302523619,4.50886517 0.0298541504,4.73316564 C0.0899606628,4.9574661 0.236662834,5.14864312 0.437644433,5.26458333 L1.39171529,5.81583333 C1.21064614,6.59536289 1.21064614,7.40609544 1.39171529,8.185625 L0.437644433,8.736875 C0.236662834,8.85281521 0.0899606628,9.04399223 0.0298541504,9.2682927 C-0.0302523619,9.49259316 0.00116776262,9.73161606 0.117193154,9.93270833 L0.77266168,11.06875 C0.88846383,11.2699712 1.07941319,11.4168483 1.30344655,11.4770265 C1.52747992,11.5372046 1.76621819,11.5057471 1.96707099,11.3895833 L2.92259845,10.836875 C3.50613953,11.3831101 4.20662236,11.7886375 4.97057344,12.0225 L4.97057344,13.125 C4.97057344,13.6082492 5.36185778,14 5.84453147,14 L7.15546853,14 C7.63814222,14 8.02942656,13.6082492 8.02942656,13.125 L8.02942656,12.0225 C8.79337764,11.7886375 9.49386047,11.3831101 10.0774016,10.836875 L11.032929,11.3895833 C11.2337818,11.5057471 11.4725201,11.5372046 11.6965534,11.4770265 C11.9205868,11.4168483 12.1115362,11.2699712 12.2273383,11.06875 L12.8828068,9.93270833 C12.9988322,9.73161606 13.0302524,9.49259316 12.9701458,9.2682927 C12.9100393,9.04399223 12.7633372,8.85281521 12.5623556,8.736875 L11.6082847,8.185625 C11.7893539,7.40609544 11.7893539,6.59536289 11.6082847,5.81583333 L12.5623556,5.26458333 C12.7633372,5.14864312 12.9100393,4.9574661 12.9701458,4.73316564 C13.0302524,4.50886517 12.9988322,4.26984227 12.8828068,4.06875 L12.2273383,2.93270833 C12.1115362,2.73148712 11.9205868,2.58461004 11.6965534,2.52443187 C11.4725201,2.46425369 11.2337818,2.49571128 11.032929,2.611875 L10.0774016,3.16458333 C9.49400565,2.61782234 8.79351153,2.2117896 8.02942656,1.9775 L8.02942656,0.875 C8.02942656,0.391750844 7.63814222,6.47630098e-17 7.15546853,6.47630098e-17 Z M8.5,7 C8.5,8.1045695 7.6045695,9 6.5,9 C5.3954305,9 4.5,8.1045695 4.5,7 C4.5,5.8954305 5.3954305,5 6.5,5 C7.03043298,5 7.53914081,5.21071368 7.91421356,5.58578644 C8.28928632,5.96085919 8.5,6.46956702 8.5,7 Z" transform="translate(2.5 2)"></path>
						</svg>
					</span>
					<div class="settings-modal" id="settings-modal" style="display: none;">
						<div class="settings-modal-inner">
							<span class="settings-modal-close" onclick="document.getElementById('settings-modal').style.display = 'none';">&#215;</span>
							<div class="settings-modal-left">
							   <ul>
									<li><a onclick="openTab(event, 'create')">Create</a></li>
									<li><a onclick="openTab(event, 'settings')">Settings</a></li>
									<li><a onclick="openTab(event, 'edit')">Edit</a></li>
									<li><a onclick="openTab(event, 'delete')">Delete</a></li>
									<li><a class="separate"></a></li>
									<li><a href="/logout">Log Out</a></li>
								</ul>
							</div>

							<div class="settings-modal-right">
								<div id="settings-tab-content">
									<div id="tab-create" style="display:block;" class="settings-tab-content">
										<form id="create-post" action="/api/post" method="post">
											<h1>Title:</h1>
								
											<input class="textbox-other" type="text" name="title">
											<h1>Content:</h1>
								
											<textarea class="textbox-other" rows="10" name="content" style="resize:vertical;" cols="60"></textarea><br><br>
											<input type="submit" value="Post">
										</form>
									</div>
						
									<div id="tab-settings" class="settings-tab-content">
										<form id="change-post" action="/change" method="post">
											<h1>Username:</h1>
											<input class="textbox-other" type="text" name="user" value=")V0G0N" << std::get<0>(val) << R"V0G0N(">
								
											<h1>Password:</h1>
								
											<input class="textbox-other" type="password" name="pass"><br><br>
											<input type="submit" value="Update">
										</form>
									</div>

									<div id="tab-edit" class="settings-tab-content">
										<form id="edit-post" action="/edit" method="post">
											<input type="hidden" name="post_id" value=")V0G0N" << getPostInformationById(post_id, codons::id) << R"V0G0N(" />
											<h1>Title:</h1>
											<input class="textbox-other" type="text" name="title" value=")V0G0N" << getPostInformationById(post_id, codons::title) << R"V0G0N("><br><br>
											<h1>Content:</h1>
											<textarea class="textbox-other" rows="10" name="content" cols="60">)V0G0N" << getPostInformationById(post_id, codons::content) << R"V0G0N(</textarea>

											<br><br>
											<input type="submit" value="Update">
										</form>
									</div>

									<div id="tab-delete" class="settings-tab-content">
										<form id="delete-post" action="/delete" method="post">
											<input type="hidden" name="post_id" value=")V0G0N" << getPostInformationById(post_id, codons::id) << R"V0G0N(" />

											<b><h2>You are about to delete post ")V0G0N" << getPostInformationById(post_id, codons::title) << R"V0G0N("<br> 
											Are you sure?</h2></b><br>
											<input type="submit" value="Delete">
										</form>
									</div>
								</div>
							</div>
						</div>
					</div>
				)V0G0N";
	}

}

std::stringstream BlogSystem::getThisPost(std::shared_ptr<HttpServer::Request> request, std::string post_id)
{
	std::stringstream ss;

	try {
		sqlite::database db(this->filename);

		int count = 0;
		db << "select count(*) from posts where id=?;"
		<< post_id
		>> count;

		if (count != 1) {
			addControlsGeneral(request, ss);

			ss << "<br><br><center> Post was not found or duplicate posts... </center>";

			cache.erase(post_id);

			return ss;
		}

		db << "select id,title,content,postdate,author from posts where id=? ;"
			<< post_id
			>> [&](int id, std::string title, std::string content, std::string postdate, std::string author)
		{
			std::stringstream s(content);

			bbcode::parser bbparser;
			bbparser.source_stream(s);
			bbparser.parse();

			ss << R"V0G0N(
					<div class="postBox">
						<div class="postHeader">
							<span class="postTitleOnPage">)V0G0N" << title << R"V0G0N(</span>
							<span class="postDate">)V0G0N" << postdate << R"V0G0N(</span>
						</div>
						<div class="postContent"><p style="white-space:pre-wrap;">)V0G0N" << bbparser.content() << R"V0G0N(</p></div>
						<div class="postFooter">
							<span class="postAuthor">)V0G0N" << author << R"V0G0N(</span>
						</div>
					</div>
				)V0G0N";
		};

		cache[post_id] = ss.str();
		addControlsView(request, ss, post_id);

	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		ss << "database server failure";
	}
	
	return ss;
}

std::stringstream BlogSystem::findPost(std::shared_ptr<HttpServer::Request> request, std::string value)
{
	std::stringstream ss;

	try {
		value = UriDecode(value);

		sqlite::database db(this->filename);

		db << "select id,title,content,postdate,author from posts where title LIKE ? OR content LIKE ? OR postdate LIKE ? ;"
			<< "%" + value + "%"
			<< "%" + value + "%"
			<< "%" + value + "%"
			>> [&](int id, std::string title, std::string content, std::string postdate, std::string author)
		{
			std::stringstream s(content);

			bbcode::parser bbparser;
			bbparser.source_stream(s);
			bbparser.parse();

			ss << R"V0G0N(
					<div class="postBox">
						<div class="postHeader">
							<a class="postTitle" href="view/)V0G0N" << id << "\">" << title << R"V0G0N(</a>
							<span class="postDate">)V0G0N" << postdate << R"V0G0N(</span>
						</div>
						<div class="postContent"><p style="white-space:pre-wrap;">)V0G0N" << bbparser.content() << R"V0G0N(</p></div>
						<div class="postFooter">
							<span class="postAuthor">)V0G0N" << author << R"V0G0N(</span>
						</div>
					</div>
				)V0G0N";
		};

		addControlsGeneral(request, ss);
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		ss << "database server failure";
	}

	return ss;
}

std::string BlogSystem::getSessionCookie(std::shared_ptr<HttpServer::Request> request)
{
	for (auto& header : request->header) {
		if (header.first == "Cookie") {
			if (header.second.find("vldr_session=") != std::string::npos) {
				unsigned first = header.second.find("vldr_session=");
				std::string strNew = header.second.substr(first, 45);

				std::string key;
				std::string value;
				size_t positionOfEquals = strNew.find("=");
				key = strNew.substr(0, positionOfEquals);
				if (positionOfEquals != std::string::npos)
					value = strNew.substr(positionOfEquals + 1);

				return value.substr(0, 32);
			}
		}
	}
	return "";
}

int BlogSystem::createPost(std::string title, std::string content, std::string author)
{
	std::stringstream ss;

	try {
		sqlite::database db(this->filename);

		std::time_t t = time(0);
		struct tm * now = localtime(&t);
		std::stringstream stime;
		stime << now->tm_mday << '/'
			<< (now->tm_mon + 1) << '/'
			<< (now->tm_year + 1900)
			<< std::endl;

		db << "insert into posts (title, content, postdate, author) values (?,?,?,?);"
			<< title
			<< content
			<< stime.str()
			<< author;

		cache.clear();

		return 1;
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return 0;
	}
}

int BlogSystem::updatePost(std::string post_id, std::string title, std::string content, std::string author)
{
	try {
		sqlite::database db(this->filename);

		time_t t = time(0);
		struct tm * now = localtime(&t);
		std::stringstream stime;
		stime << now->tm_mday << '/'
			<< (now->tm_mon + 1) << '/'
			<< (now->tm_year + 1900)
			<< std::endl;

		int count = 0;

		db << "select count(*) from posts where id=? ;"
			<< post_id
			>> count;

		if (count == 1)
		{
			db << "update posts set title=?, content=?, postdate=?, author=? WHERE id=? ;"
				<< title
				<< content
				<< stime.str()
				<< author
				<< post_id;

			cache.clear();

			return 1;
		}

		return 0;
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return 0;
	}
}

int BlogSystem::deletePost(std::string post_id)
{
	try {
		sqlite::database db(this->filename);

		int count = 0;

		db << "select count(*) from posts where id=? ;"
			<< post_id
			>> count;

		if (count == 1)
		{
			db << "delete from posts where id=? ;"
				<< post_id;

			cache.clear();

			return 1;
		}



		return 0;
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return 0;
	}
}


int BlogSystem::hashPassword(char *dst, const char *passphrase, uint32_t N, uint8_t r, uint8_t p) {
	try {
		uint8_t salt[SCRYPT_SALT_LEN] = {};
		uint8_t	hashbuf[SCRYPT_HASH_LEN];
		char outbuf[256];
		char saltbuf[256];

		std::string generatedSalt = generateSalt(SCRYPT_SALT_LEN);
		if (generatedSalt == "")
		{
			throw std::runtime_error("generateSalt error...");
			return 0;
		}

		copy(generatedSalt.begin(), generatedSalt.end(), salt);
		salt[generatedSalt.length() - 1] = 0;

		auto retval = libscrypt_scrypt(reinterpret_cast<const uint8_t*>(passphrase), strlen(passphrase),
			static_cast<uint8_t*>(salt), SCRYPT_SALT_LEN, N, r, p, hashbuf, sizeof(hashbuf));
		if (retval == -1)
			throw std::runtime_error("libscrypt_scrypt error...");

		retval = libscrypt_b64_encode(static_cast<unsigned char*>(hashbuf), sizeof(hashbuf),
			outbuf, sizeof(outbuf));
		if (retval == -1)
			throw std::runtime_error("libscrypt_b64_encode error...");

		retval = libscrypt_b64_encode(static_cast<unsigned char *>(salt), sizeof(salt),
			saltbuf, sizeof(saltbuf));

		if (retval == -1)
			throw std::runtime_error("libscrypt_b64_encode #2 error...");

		retval = libscrypt_mcf(N, r, p, saltbuf, outbuf, dst);
		if (retval != 1)
			throw std::runtime_error("libscrypt_mcf error...");

		return 1;
	}
	catch (std::runtime_error &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return 0;
	}
}

int BlogSystem::changeUserDetails(std::string user_input, std::string pwd_input, std::shared_ptr<HttpServer::Request> request)
{
	if (user_input.length() == 0 || pwd_input.length() == 0)
		return 0;

	try {
		sqlite::database db(this->filename);

		auto val = sessions[getSessionCookie(request)];
		int count = 0;

		db << "select count(*) from users where username=? ;"
			<< std::get<0>(val)
			>> count;

		if (count == 1)
		{
			char finalPassword[1024];
			auto passphrase = pwd_input.c_str();
			
			hashPassword(finalPassword, passphrase, SCRYPT_N, SCRYPT_r, SCRYPT_p);

			db << "update users set username=?, pass=? where username=? ;"
				<< user_input
				<< finalPassword
				<< std::get<0>(val);

			loggout(std::get<0>(val)); 

			return 1;
		}

		return 0;
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return 0;
	}
}


int BlogSystem::processLogin(std::string user_input, std::string pwd_input)
{
	try {
		sqlite::database db(this->filename);

		int result = 0;
		int count = 0;
		db << "select count(*) from users where username=? ;"
			<< user_input
			>> count;

		if (count == 1)
		{
			db << "select username,pass from users where username=? ;"
				<< user_input
				>> [&](std::string username, std::string pwd)
			{
				char dst[1024];

				copy(pwd.begin(), pwd.end(), dst);
				dst[pwd.length()] = 0;

				if (libscrypt_check(dst, pwd_input.c_str()) > 0)
					result = 1;
				else 
					result = 0;
			};
		}

		return result;
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return 0;
	}
}

int BlogSystem::getUserID(std::string user_input)
{
	try {
		sqlite::database db(this->filename);

		int count = 0;
		db << "select count(*) from users where username=? ;"
			<< user_input
			>> count;

		if (count == 1)
		{
			return 1;
		}

		return 0;
	}
	catch (std::exception &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what() << std::endl;

		return 0;
	}
}
