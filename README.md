<img src="http://i.imgur.com/1HMmEwX.png" />

# LightBlogExpress
LightBlog Express is the fastest and most robust blog written in C++ (tested on MSVC2015). The project is based off of (https://github.com/vldr/LightBlog) and (https://github.com/eidheim/Simple-Web-Server).

LightBlogExpress is insanely fast and was able to withstand load of up to ~19953 clients per second using the caching system...
It is effortless to implement custom themes and modules for the blog system. Successfully compiled on macOS Sierra, Ubuntu 14.04, and Windows 10.

<img src="http://i.imgur.com/l2YqPM4.png" />

# Dependencies
* Boost C++ libraries
* OpenSSL libraries (if you wish to use HTTPS, you will need to implement it yourself).
* MySQL Connector (https://dev.mysql.com/downloads/connector/cpp/)

# Binaries
### Windows
- Requires <a href="https://www.microsoft.com/en-ca/download/details.aspx?id=48145">Visual C++ Redist. 2015</a><br>
Usage: `LightBlog.exe DB_HOST DB_USER DB_PASS DB_NAME PORT`

### macOS Sierra (xcode developer tools required) (boost)
Setup: 
1. `ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
2. `brew install boost`
3. `chmod +x lightblog.macos`
4. `./lightblog.macos DB_HOST DB_USER DB_PASS DB_NAME PORT`

# Database
LightBlogExpress uses <b>MySQL</b>, and uses <b>SHA256</b> hashing for passwords.

Default username and password is:

`vlad` and `test`

# Images
<img src="http://i.imgur.com/FmlGIFC.png" />

# Demo 
http://blog.vldr.org/


