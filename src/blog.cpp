#include "blog.hpp"

#include <sstream>
#include "bbcode/bbcode_parser.h"
#include <iterator>
#include <string>

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
		cout << "[ Failed to connect to mysql server! ]" << endl;
	}
}

void BlogSystem::sendPage(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, std::string ss)
{
	cout << ">> " << request->remote_endpoint_address << ":" <<
		request->remote_endpoint_port << " has requested (" << request->path << ")...\n";

	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss.c_str();
}

void BlogSystem::sendPage404(shared_ptr<HttpServer::Request> request, shared_ptr<HttpServer::Response> response, string ss)
{
	cout << ">> " << request->remote_endpoint_address << ":" <<
		request->remote_endpoint_port << " has requested (" << request->path << ")...\n";

	*response << "HTTP/1.1 404 Not found\r\nContent-Length: " << ss.length() << "\r\n\r\n" << ss;
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

		int count = res->rowsCount();

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

std::stringstream BlogSystem::getPosts()
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

		pstmt.reset(con->prepareStatement("SELECT * FROM posts ORDER by id DESC"));
		res.reset(pstmt->executeQuery());

		int count = res->rowsCount();

		if (count <= 0) {
			ss << "<br><br><center>No posts...<center>";

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
							<a href="post"><img class="editPostButton" src="http://www.famfamfam.com/lab/icons/silk/icons/application_add.png" /></a>
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

		int count = res->rowsCount();

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

		int count = res->rowsCount();

		if (count != 1) {
			ss << "<br><br><center> Post was not found or duplicate posts... </center>";

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
							<span class="postTitleOnPage">)V0G0N" << sk.str() << R"V0G0N(</span>
							<span class="postDate">)V0G0N" << res->getString("postdate") << R"V0G0N(</span>
						</div>
						<div class="postContent"><p style="white-space:pre-wrap;">)V0G0N" << bbCodeParsed << R"V0G0N(</p></div>
						<div class="postFooter">
							<a href="../edit/)V0G0N" << res->getString("id") << R"V0G0N("><img class="editPostButton" src="http://www.famfamfam.com/lab/icons/silk/icons/application_edit.png" /></a>
							<a href="../delete/)V0G0N" << res->getString("id") << R"V0G0N("><img class="editPostButton" src="http://www.famfamfam.com/lab/icons/silk/icons/application_delete.png" /></a>
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
		if (header.first == "Cookie" && boost::starts_with(header.second, "vldr_session")) {
			string query = header.second;
			string key;
			string value;
			int positionOfEquals = query.find("=");
			key = query.substr(0, positionOfEquals);
			if (positionOfEquals != string::npos)
				value = query.substr(positionOfEquals + 1);
			
			return value;
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

		int count = res->rowsCount();

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

		int count = res->rowsCount();

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

		int count = res->rowsCount();

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

		pwd_input = sha256(pwd_input);
		pstmt->setString(2, pwd_input);

		res.reset(pstmt->executeQuery());

		int count = res->rowsCount();

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

		int count = res->rowsCount();

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
