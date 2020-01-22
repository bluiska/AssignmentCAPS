// Assignment1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdarg>
#include <thread>
#include <chrono>
#include <numeric>
#include <vector>
#include <stdlib.h> 
#include <time.h>   
#include <future>
#include <unordered_map>

#include "tcp_client.h"

//#define ALLOW_PRINT  //Uncomment to enable printing of messages to console.

#ifdef ALLOW_PRINT
#define PRINT(text) std::cout << text;
#define PRINTF(x, args) printf(x, args);
#else
#define PRINT(x)
#define PRINTF(x, args)
#endif

#define DEFAULT_PORT 12345

using namespace std;

enum class MessageType {
	POST = 0,
	READ,
	COUNT,
	LIST
};

unordered_map<MessageType, string> threadTypeMap = {
	{MessageType::POST, "Poster"},
	{MessageType::READ, "Reader"},
	{MessageType::COUNT, "Counter"},
	{MessageType::LIST, "Listener"}
};

struct ThroughputResult {

	ThroughputResult() {
		this->id = -1;
		this->ThreadType = "Unknown";
		this->throughput = -1;
	}

	ThroughputResult(int id, string ThreadType, double throughput) {
		this->id = id;
		this->ThreadType = ThreadType;
		this->throughput = throughput;
	}

	int id;
	string ThreadType;
	double throughput;
};

string nextRequest(int, MessageType);
void clientCommunication(const string&, int, int, promise<ThroughputResult>&&, MessageType);

vector<string> postedMessages;

vector<string> topics({ "University", "Work", "Life", "Hobby", "Cooking", "Garden"});

unordered_map<string, vector<string>> messages =
{ {"University", vector<string>({
	"U: I study Computer Science.",
	"U: I have an assignment about concurrency and parallelism.",
	"U: The program will be tested in room 9341.",
	"U: Sheffield Hallam University",
	"U: Wish the coffee at Cantor was still just £1."})},
{"Work", vector<string>({
	"W: The work at the office starts at 9am.",
	"W: My collegues are great!",
	"W: Wish we had a break at work more often.",
	"W: Payroll number: 123456789",
	"W: Salary: £98,920 (before tax)"})},
{"Life", vector<string>({
	"L: I need to sort out my bills.",
	"L: That sales man is knocking on the door again!",
	"L: Wish I could get a new car.",
	"L: My mobile number is: 01234567812",
	"L: Don't forget to put the bins out!"})},
{"Hobby", vector<string>({
	"H: I can finally play the song on my guitar!",
	"H: That drumset is amazing!",
	"H: Why won't this game run?",
	"H: Gosh I need a new graphics card!",
	"H: Player health: 10%, Coins: 150"})},
{"Cooking", vector<string>({
	"C: Add 200g of minced meat",
	"C: Turn up the heat to 190C.",
	"C: I need a new slow cooker. The one I have is too small for our family!",
	"C: This recepie is rubbish.",
	"C: Reminder: Buy more eggs."})},
{"Garden", vector<string>({
	"G: This rose is beautiful.",
	"G: I need a new rake.",
	"G: That tree has been there for so long. It is rotting away :(",
	"G: Why aren't my tomatoes growing?",
	"G: This green house is unnecessary."})}
};

mutex print;

int main(int argc, char** argv)
{
	srand(time(NULL));

	if (argc < 2) {
		printf("usage: %s server-name|IP-address\n", argv[0]);
		return 1;
	}

	cout << "Client connected to server on " << argv[1] << endl;

	int posterNumber;

	cout << "Number of poster threads: ";
	cin >> posterNumber;

	int readerNumber;

	cout << "Number of reader threads: ";
	cin >> readerNumber;

	int counterNumber;

	cout << "Number of counter threads: ";
	cin >> counterNumber;

	int listerNumber;

	cout << "Number of lister threads: ";
	cin >> listerNumber;

	int timeLimit;

	cout << "Time to run for (seconds): ";
	cin >> timeLimit;

	vector<future<ThroughputResult>> resultFutures;

	vector<thread> posterThreads;
	vector<thread> readerThreads;
	vector<thread> counterThreads;
	vector<thread> listerThreads;

	for (int i(0); i < posterNumber; ++i) {
		promise<ThroughputResult> p;
		resultFutures.push_back(p.get_future());
		posterThreads.push_back(thread(clientCommunication, argv[1], i, timeLimit, move(p), MessageType::POST));
	}

	for (int i(0); i < readerNumber; ++i) {
		promise<ThroughputResult> p;
		resultFutures.push_back(p.get_future());
		readerThreads.push_back(thread(clientCommunication, argv[1], i, timeLimit, move(p), MessageType::READ));
	}

	for (int i(0); i < counterNumber; ++i) {
		promise<ThroughputResult> p;
		resultFutures.push_back(p.get_future());
		counterThreads.push_back(thread(clientCommunication, argv[1], i, timeLimit, move(p), MessageType::COUNT));
	}

	for (int i(0); i < listerNumber; ++i) {
		promise<ThroughputResult> p;
		resultFutures.push_back(p.get_future());
		listerThreads.push_back(thread(clientCommunication, argv[1], i, timeLimit, move(p), MessageType::LIST));
	}

	double totalThroughput(0);

	std::for_each(resultFutures.begin(), resultFutures.end(), [&totalThroughput](future<ThroughputResult>& r) {
		auto result = r.get();
		totalThroughput += result.throughput;
		cout << "Throughput for " << result.ThreadType << " thread #" << result.id << " is " << result.throughput << endl;
		});


	cout << "Average throughput = " << totalThroughput / resultFutures.size() << endl;

	std::for_each(posterThreads.begin(), posterThreads.end(), [](thread& t) { t.join(); });
	std::for_each(readerThreads.begin(), readerThreads.end(), [](thread& t) { t.join(); });
	std::for_each(counterThreads.begin(), counterThreads.end(), [](thread& t) { t.join(); });
	std::for_each(listerThreads.begin(), listerThreads.end(), [](thread& t) { t.join(); });


	system("pause");

	return 0;
}

void clientCommunication(const string& ip, int id, int time_limit, promise<ThroughputResult>&& result, MessageType type) {
	TCPClient client(ip, DEFAULT_PORT);

	client.OpenConnection();

	int count(0);
	std::string request;
	auto start(chrono::steady_clock::now());
	do {
		const auto& req(nextRequest(count, type));
		const auto& res(client.send(req));

#ifdef ALLOW_PRINT
		print.lock();
		PRINT(endl << "Request: " << req << " | Response: " << res << endl);
		print.unlock();
#endif // ALLOW_PRINT

		++count;
	} while (chrono::steady_clock::now() < start + chrono::seconds(time_limit));

	double throughput = count / (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() / 1000.0);
	result.set_value(ThroughputResult(id, threadTypeMap[type], throughput));

	client.CloseConnection();
	PRINT("Done sending");
}

string nextRequest(int count, MessageType type) {
	switch (type) {
	case MessageType::POST:
	{
		int r(count % 6);
		int m(count % 5);
		return "POST@" + topics[r] + "#" + messages[topics[r]][m];
	}
	case MessageType::READ:
	{
		int r(count % 6);
		int approxRequests(count / 6);
		int n(approxRequests > 0 ? rand() % approxRequests : 1);
		return "READ@" + topics[r] + "#" + to_string(n);
	}
	case MessageType::COUNT:
	{
		int r = count % 6;
		return "COUNT@" + topics[r];
	}
	case MessageType::LIST:
	{
		return "LIST";
	}
	default:
		return "";

	}
}