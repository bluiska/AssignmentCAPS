#ifndef __REQUESTPARSER_H
#define __REQUESTPARSER_H

#include <string>
#include <regex>

class PostRequest
{
public:
	PostRequest();
	~PostRequest();
	static PostRequest parse(std::smatch match, std::string request);
	const std::string& getTopicId();
	const std::string& getMessage();
	const std::string& toString();

	std::string topicId;
	std::string message;
	bool valid;
};

class ReadRequest
{
public:
	ReadRequest();
	~ReadRequest();
	static ReadRequest parse(std::smatch match, std::string request);
	const std::string& getTopicId();
	int getPostId();
	const std::string& toString();

	std::string topicId;
	int postId;
	bool valid;
};

class CountRequest
{
public:
	CountRequest();
	~CountRequest();
	static CountRequest parse(std::smatch match, std::string request);
	const std::string& getTopicId();
	const std::string& toString();

	std::string topicId;
	bool valid;
};

class ListRequest
{
public:
	ListRequest();
	~ListRequest();
	static ListRequest parse(std::string request);
	const std::string& toString();
	bool valid;
};

#endif //__REQUESTPARSER_H

