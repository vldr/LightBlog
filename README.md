<img src="http://cdn.vldr.org/C9Jd9H.png" />

# LightBlogExpress
Robust, Fast, MySQL bbcode http blog system written in C++ (tested on MSVC2015). The project is based off of (https://github.com/vldr/LightBlog) and (https://github.com/eidheim/Simple-Web-Server).

It's also incredibly easy to create themes for this blog system, and adding things to it isn't too hard.
I haven't tested with GCC or any other compiler other than MSVC2015 but it should work since the whole project isn't dependent on any windows headers.

# Dependencies
* Boost C++ libraries
* OpenSSL libraries
* MySQL Connector (https://dev.mysql.com/downloads/connector/cpp/)

# Binaries
- Requires <a href="https://www.microsoft.com/en-ca/download/details.aspx?id=48145">Visual C++ Redist. 2015</a><br>
Usage: `LightBlog.exe DB_HOST DB_USER DB_PASS DB_NAME`

# Database
LightBlogExpress uses <b>MySQL</b>, and uses <b>SHA256</b> hashing for passwords.

Default username and password is:

`vlad` and `test`

# Demo 
http://blog.vldr.org/


