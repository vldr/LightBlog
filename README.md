<img src="http://i.imgur.com/1HMmEwX.png" />

# LightBlogExpress
LightBlog Express is the fastest and most robust blog written in C++ (tested on MSVC2015). The project is based off of (https://github.com/vldr/LightBlog) and (https://github.com/eidheim/Simple-Web-Server).


LightBlogExpress is insanely fast, and was able to withstand load of up to ~695 clients per second on a Intel Core i3 machine using the  smart caching system...

<img src="http://i.imgur.com/o1AVmjp.png" />

It's also incredibly easy to create themes for this blog system, and adding things to it isn't too hard.

I haven't tested with GCC or any other compiler other than MSVC2015 but it should work since the whole project isn't dependent on any windows headers.

# Dependencies
* Boost C++ libraries
* OpenSSL libraries
* MySQL Connector (https://dev.mysql.com/downloads/connector/cpp/)

# Binaries
### Windows
- Requires <a href="https://www.microsoft.com/en-ca/download/details.aspx?id=48145">Visual C++ Redist. 2015</a><br>
Usage: `LightBlog.exe DB_HOST DB_USER DB_PASS DB_NAME PORT`

### macOS Sierra (xcode developer tools required)
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


