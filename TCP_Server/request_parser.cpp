#include "request_parser.h"

PostRequest::PostRequest() : valid(0)
{ }

PostRequest::~PostRequest()
{ }

PostRequest PostRequest::parse(std::smatch match, std::string request)
{
	PostRequest post;

	post.topicId = match[1];
	post.message = match[2];
	post.valid = 1;

	return post;
}

const std::string& PostRequest::getTopicId()
{
	return topicId;
}

const std::string& PostRequest::getMessage()
{
	return message;
}
const std::string& PostRequest::toString()
{
	return std::string("POST") + topicId + std::string("#") + message;
}

ReadRequest::ReadRequest() : valid(0)
{ }

ReadRequest::~ReadRequest()
{ }

ReadRequest ReadRequest::parse(std::smatch match, std::string request)
{
	ReadRequest read;

	read.topicId = match[1];
	read.postId = std::stoi(match[2]);
	read.valid = 1;

	return read;
}

const std::string& ReadRequest::getTopicId()
{
	return topicId;
}

int ReadRequest::getPostId()
{
	return postId;
}

const std::string& ReadRequest::toString()
{
	return std::string("READ") + topicId + std::string("#") + std::to_string(postId);
}

CountRequest::CountRequest() : valid(0)
{ }

CountRequest::~CountRequest()
{ }

CountRequest CountRequest::parse(std::smatch match, std::string request)
{
	CountRequest count;

	count.topicId = match[1];
	count.valid = 1;

	return count;
}

const std::string& CountRequest::getTopicId()
{
	return topicId;
}

const std::string& CountRequest::toString()
{
	return std::string("COUNT") + topicId;
}

ListRequest::ListRequest() : valid(0)
{ }

ListRequest::~ListRequest()
{ }

ListRequest ListRequest::parse(std::string request)
{
	ListRequest list;

	list.valid = 1;

	return list;
}

const std::string& ListRequest::toString()
{
	return std::string("LIST");
}
