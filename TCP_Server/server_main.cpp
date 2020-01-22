// Assignment1-Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <regex>
#include <unordered_map>
#include <deque>

#include "tcp_server.h"
#include "request_parser.h"
#include "ThreadPool.h"

//#define ALLOW_PRINT  //Uncomment to enable printing of messages to console.

#ifdef ALLOW_PRINT
#define PRINT(text) std::cout << text;
#define PRINTF(x, args) printf(x, args);
#else
#define PRINT(x)
#define PRINTF(x, args)
#endif

#define DEFAULT_PORT 12345

BOOL SetThreadPriority(
	HANDLE hThread,
	int    nPriority
);

using namespace std;

TCPServer server(DEFAULT_PORT);

std::deque<ReceivedSocketData> request_queue;
mutex request_mtx;
condition_variable request_cv;

void requestListener(const ReceivedSocketData&);
void requestWorker(unordered_map<string, vector<string>>&, shared_mutex&);

void put(ReceivedSocketData);
ReceivedSocketData take();

#define POST_REGEX "^POST(@[^@#]*)#(.+)$"
#define READ_REGEX "^READ(@[^@#]*)#([0-9]+)$"
#define COUNT_REGEX "^COUNT(@[^@#]*)$"
#define LIST_REGEX "^LIST$"

static regex post_regex(POST_REGEX);
static regex read_regex(READ_REGEX);
static regex count_regex(COUNT_REGEX);
static regex list_regex(LIST_REGEX);

ThreadPool threadPool(10);

int main() {

	unordered_map<string, vector<string>> forumMap;
	shared_mutex forum_mtx;

	cout << "Detected hardware concurrency: " << thread::hardware_concurrency() << endl;
	cout << "Starting server. Send \"exit\" (without quotes) to terminate a client connection." << endl;

	thread worker1(requestWorker, ref(forumMap), ref(forum_mtx));
	//thread worker2(requestWorker, ref(forumMap), ref(forum_mtx));

	SetThreadPriority(worker1.native_handle(), THREAD_PRIORITY_HIGHEST);
	//SetThreadPriority(worker2.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);

	worker1.detach();
	//worker2.detach();

	while (1) {
		ReceivedSocketData receivedData(server.accept());

		//Don't detach, retain pointer to later delete. Maybe?
		//thread listener(requestListener, receivedData);
		//SetThreadPriority(listener.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
		//listener.detach();
		threadPool.enqueue(requestListener, receivedData);
	}

	return 0;
}

void requestListener(const ReceivedSocketData& client) {

	cout << endl << "Connected to client on socket " << client.ClientSocket << endl;

	ReceivedSocketData receivedData(client);

	while (client.ClientSocket != INVALID_SOCKET) {

		server.receiveData(receivedData, 1);

		if (receivedData.request != "")
		{
			put(receivedData);
		}
	}

	server.closeClientSocket(receivedData);

	cout << "Client connection on socket " << client.ClientSocket << " closed." << endl;
}

void requestWorker(unordered_map<string, vector<string>>& forum, shared_mutex& frm_mtx) {

	ReceivedSocketData receivedData;

	smatch match;

	do {
		receivedData = take();

		//POST request
		if (regex_match(receivedData.request, match, post_regex, regex_constants::match_default)) {
			PostRequest post(PostRequest::parse(match, receivedData.request));

			PRINT("Post request: " << post.toString() << endl);
			PRINT("Post topic: " << post.getTopicId() << endl);
			PRINT("Post message: " << post.getMessage() << endl);

			string message(post.getMessage());

			if (message.size() > 140) {
				message.resize(140);
			}

			const auto& topic(post.getTopicId());

			frm_mtx.lock();

			forum[topic].push_back(message);

			int postId = forum[topic].size() - 1;

			frm_mtx.unlock();

			receivedData.reply = to_string(postId);

			server.sendReply(receivedData);
			continue;
		}
		//READ request
		else if (regex_match(receivedData.request, match, read_regex, regex_constants::match_default)) {
			ReadRequest read(ReadRequest::parse(match, receivedData.request));

			PRINT("Read request" << read.toString() << endl);
			PRINT("Read topic: " << read.getTopicId() << endl);
			PRINT("Read post id: " << read.getPostId() << endl);

			const auto& topic(read.getTopicId());
			frm_mtx.lock_shared();

			if (!forum[topic].empty() && read.getPostId() <= forum[topic].size() - 1 && read.getPostId() >= 0) {
				receivedData.reply = forum[topic].at(read.getPostId());
			}
			else {
				receivedData.reply = "";
			}

			frm_mtx.unlock_shared();

			server.sendReply(receivedData);
			continue;
		}
		//COUNT request
		else if (regex_match(receivedData.request, match, count_regex, regex_constants::match_default)) {

			CountRequest count(CountRequest::parse(match, receivedData.request));

			PRINT("Count request: " << count.toString() << endl);
			PRINT("Count topic: " << count.getTopicId() << endl);

			frm_mtx.lock_shared();

			unsigned int messageCount(forum[count.getTopicId()].size());

			frm_mtx.unlock_shared();

			receivedData.reply = to_string(messageCount);
			server.sendReply(receivedData);
			continue;
		}
		//LIST request
		else if (regex_match(receivedData.request, match, list_regex, regex_constants::match_default)) {
			ListRequest list(ListRequest::parse(receivedData.request));

			PRINT("List request: " << list.toString() << endl);
			string topics("");

			frm_mtx.lock_shared();
			auto itr = forum.begin();

			while (itr != forum.end()) {
				topics.append(itr->first);
				if (++itr != forum.end()) {
					topics += "#";
				}
			}
			frm_mtx.unlock_shared();

			receivedData.reply = topics;
			server.sendReply(receivedData);
			continue;
		}

		if (receivedData.request != "exit") {
			receivedData.reply = "";
			server.sendReply(receivedData);
		}
		
	} while (1);
}

void put(ReceivedSocketData request) {
	//No limit on the queue and realistically we won't fill it for this scenario
	unique_lock<mutex> locker(request_mtx);
	request_queue.push_front(request);
	request_cv.notify_all();
	locker.unlock();
}

ReceivedSocketData take() {
	unique_lock<mutex> locker(request_mtx);
	while (request_queue.empty()) {
		request_cv.wait(locker);
	}
	auto val(request_queue.back());
	request_queue.pop_back();
	request_cv.notify_all();
	locker.unlock();
	return val;
}