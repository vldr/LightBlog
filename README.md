<img src="http://i.imgur.com/1HMmEwX.png" />

# LightBlog
LightBlog Express is a very fast, and light weight blog written in C++. The project is based off of (https://github.com/vldr/LightBlog) and (https://github.com/eidheim/Simple-Web-Server). Successfully compiled on macOS Sierra, Ubuntu 16.04, and Windows 10.

I'm gladly looking for contributions for any bugs, features, et al.

# Images
<img src="http://i.imgur.com/O2mPMCJ.gif" />

# Demo 
http://blog.vldr.org/

# Dependencies
* Boost C++ libraries
* OpenSSL libraries (if you wish to use HTTPS, you will need to implement it yourself).

# Binaries
### Windows
- Requires <a href="https://go.microsoft.com/fwlink/?LinkId=746572">Visual C++ Redist. 2017</a><br>
- Requires https://github.com/vldr/LightBlog/raw/master/sql/sql.db

Usage: `LightBlog.exe DBFILENAME PORT`

### macOS Sierra (xcode developer tools required) (boost)
Setup: 
1. `ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
2. `brew install boost`
3. `chmod +x lightblog.macos`
4. `./lightblog.macos DB_HOST DB_USER DB_PASS DB_NAME PORT`

### Ubuntu 16.04 x86_64
Setup:
1. `apt-get update`
2. `apt-get install libboost-all-dev`
3. `wget https://github.com/vldr/LightBlog/raw/master/sql/sql.db`
4. `wget https://github.com/vldr/LightBlog/raw/master/bin/lightblog.ubuntu_16_04_x64`
5. `chmod +x lightblog.ubuntu_16_04_x64`
6. `./lightblog.ubuntu_16_04_x64 DBFILENAME PORT`

### Ubuntu 16.04 aarch64 (ARM 64)
Setup:
1. `apt-get update`
2. `apt-get install libboost-all-dev`
3. `wget https://github.com/vldr/LightBlog/raw/master/sql/sql.db`
4. `wget https://github.com/vldr/LightBlog/raw/master/bin/lightblog.ubuntu_16_04_arm64`
5. `chmod +x lightblog.ubuntu_16_04_arm64`
6. `./lightblog.ubuntu_16_04_arm64 DBFILENAME PORT`

# Database
LightBlogExpress uses <b>SQLite 3</b>.

# Password Hashing
SCrypt is used for hashing passwords...

Go to `/login` and logon to change username and pasword; 
default username and password is:
`admin` and `admin`


