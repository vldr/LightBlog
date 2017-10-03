<img src="http://i.imgur.com/1HMmEwX.png" />

# LightBlog
LightBlog is a fast and light weight blog written in C++. This blog system is portable by design and is cross-platform... Successfully compiled on macOS Sierra 10.12.6, Ubuntu 16.04, Ubuntu 16.04 ARM (aarch64), and Windows 10.

# Minimum Requirements
<b>OS</b>: Windows, Linux based distros., and macOS<br> 
<b>Processor</b>: 1.0 GHZ single-core or better<br> 
<b>Memory</b>: 1 MB - 50 MB<br>
<b>Storage</b>: 10 MB - 100 MB available space<br>

# Media
<img src="http://i.imgur.com/O2mPMCJ.gif" />

# Demo 
http://blog.vldr.org/

# Dependencies
* Boost C++ libraries
* OpenSSL libraries (if you wish to use HTTPS, you will need to implement it yourself).

# Binaries
### Windows
Setup:
1. Download and install <a href="https://go.microsoft.com/fwlink/?LinkId=746572">Visual C++ Redist. 2017</a>
2. Download <a href="https://raw.githubusercontent.com/vldr/LightBlog/master/bin/lightblog.exe">lightblog.exe</a>.
3. Download <a href="https://raw.githubusercontent.com/vldr/LightBlog/master/sql/sql.db">sql.db</a>.
4. Run `lightblog.exe DBFILENAME PORT`

### macOS Sierra 10.12.6 x86_64
Setup:
1. `svn checkout https://github.com/vldr/LightBlog/trunk/web`
2. `curl -o sql.db https://raw.githubusercontent.com/vldr/LightBlog/master/sql/sql.db`
3. `curl -o lightblog.macos_sierra_10_12_6_x64 https://raw.githubusercontent.com/vldr/LightBlog/master/bin/lightblog.macos_sierra_10_12_6_x64`
4. `chmod +x lightblog.macos_sierra_10_12_6_x64`
5. `./lightblog.macos_sierra_10_12_6_x64 DBFILENAME PORT`

### Ubuntu 16.04 x86_64
Setup:
1. `svn checkout https://github.com/vldr/LightBlog/trunk/web`
2. `wget https://raw.githubusercontent.com/vldr/LightBlog/master/sql/sql.db`
3. `wget https://raw.githubusercontent.com/vldr/LightBlog/master/bin/lightblog.ubuntu_16_04_x64`
4. `chmod +x lightblog.ubuntu_16_04_x64`
5. `./lightblog.ubuntu_16_04_x64 DBFILENAME PORT`

### Ubuntu 16.04 aarch64
Setup:
1. `apt-get update`
2. `apt-get install libboost-all-dev`
3.  `svn checkout https://github.com/vldr/LightBlog/trunk/web`
4. `wget https://raw.githubusercontent.com/vldr/LightBlog/master/sql/sql.db`
5. `wget https://raw.githubusercontent.com/vldr/LightBlog/master/bin/lightblog.ubuntu_16_04_arm64`
6. `chmod +x lightblog.ubuntu_16_04_arm64`
7. `./lightblog.ubuntu_16_04_arm64 DBFILENAME PORT`

# Benchmark
8000 hits per second sustained (higher would require better network)
<img src="https://i.imgur.com/VTvNtLk.png" />

# Database
LightBlog uses <b>SQLite 3</b>.

# Password Hashing
SCrypt is used for hashing passwords...

Go to `/login` and logon to change username and pasword; 
default username and password is:
`admin` and `admin`
