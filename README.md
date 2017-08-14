<img src="http://i.imgur.com/1HMmEwX.png" />

# LightBlogExpress
LightBlog Express is the fastest and most robust blog written in C++ (tested on MSVC2015). The project is based off of (https://github.com/vldr/LightBlog) and (https://github.com/eidheim/Simple-Web-Server).

LightBlogExpress is insanely fast and was able to withstand load of up to ~19953 clients per second using the caching system...
It is effortless to implement custom themes and modules for the blog system. Successfully compiled on macOS Sierra, Ubuntu 14.04, and Windows 10.

<img src="http://i.imgur.com/PSCLLbr.png" />

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

### Ubuntu 14.04 x64
Setup:
1. `apt-get update`
2. `sudo apt-get install libboost-all-dev`
3. `sudo apt-get update && \
sudo apt-get install build-essential software-properties-common -y && \
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
sudo apt-get update && \
sudo apt-get install gcc-snapshot -y && \
sudo apt-get update && \
sudo apt-get install gcc-6 g++-6 -y && \
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-6 && \
sudo apt-get install gcc-4.8 g++-4.8 -y && \
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8;`
4. `wget https://github.com/vldr/LightBlogExpress/raw/master/bin/ubuntu1404x64mysql/libmysqlcppconn.so`
5. `wget https://github.com/vldr/LightBlogExpress/raw/master/bin/lightblog.ubuntu1404x64`
6. `chmod +x lightblog.ubuntu1404x64`
7. `./lightblog.ubuntu1404x64 DB_HOST DB_USER DB_PASS DB_NAME PORT`

# Database
LightBlogExpress uses <b>MySQL</b>, and uses <b>SHA256</b> hashing for passwords.

Default username and password is:

`vlad` and `test`

# Images
<img src="http://i.imgur.com/FmlGIFC.png" />

# Demo 
http://blog.vldr.org/


