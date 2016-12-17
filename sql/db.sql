-- phpMyAdmin SQL Dump
-- version 4.5.1
-- http://www.phpmyadmin.net
--
-- Host: 127.0.0.1
-- Generation Time: Dec 17, 2016 at 08:17 PM
-- Server version: 10.1.19-MariaDB
-- PHP Version: 5.6.28

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `lolyou`
--

-- --------------------------------------------------------

--
-- Table structure for table `posts`
--

CREATE TABLE `posts` (
  `id` int(11) NOT NULL,
  `title` text CHARACTER SET utf8 COLLATE utf8_general_mysql500_ci NOT NULL,
  `content` longtext CHARACTER SET utf8 COLLATE utf8_general_mysql500_ci NOT NULL,
  `postdate` text CHARACTER SET utf8 COLLATE utf8_general_mysql500_ci NOT NULL,
  `author` text CHARACTER SET ucs2 COLLATE ucs2_general_mysql500_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `posts`
--

INSERT INTO `posts` (`id`, `title`, `content`, `postdate`, `author`) VALUES
(44, 'Welcome to your blog!', 'LightBlog automatically resizes your images even if they [b]are[/b] over 4K in resolution. \n\n\nLightBlog also is dynamic and will &lt;b&gt;resize&lt;/b&gt; according to what device it&apos;s being used on.', '17/12/2016\n', 'vlad'),
(57, 'My day at the zoo!', '[img]http://foundersracing.com/wp-content/uploads/2013/07/490831542.jpg[/img]\n[quote=Vlad Reinis]cyka[/quote]\n\n[b][color=#42c8f4]hello my &#1594;[/color][/b]\n\n[pre]\n&lt;!DOCTYPE html&gt;\n&lt;html&gt;\n&lt;body&gt;\n\n&lt;h1&gt;My First Heading&lt;/h1&gt;\n\n&lt;p&gt;My first paragraph.&lt;/p&gt;\n\n&lt;/body&gt;\n&lt;/html&gt;\n\n[/pre]', '17/12/2016\n', 'vlad');

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` int(11) NOT NULL,
  `username` text,
  `pass` text
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`id`, `username`, `pass`) VALUES
(1, 'vlad', '9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `posts`
--
ALTER TABLE `posts`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `posts`
--
ALTER TABLE `posts`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=58;
--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
