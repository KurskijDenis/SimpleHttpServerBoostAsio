#include <CustomClient/RequestSender.hpp>

#include <Http/HttpRequest.hpp>
#include <Http/HttpResponse.hpp>

#include <iostream>
#include <memory>
#include <exception>


void ResponsePrinter(std::optional<Http::HttpResponse> response)
{
	if (response)
	{
		std::cout << response->PackToString();
	}
	else
	{
		std::cout << "\\\\\\\\\\\\\\\\\\\\\\\\\\\\ERROR//////////////////////////////";
	}
}

int main(int argc, char* argv[])
{
	try
	{
		// Check command line arguments.
		if (argc != 4)
		{
			std::cerr << "Usage: http_client <address> <port> <url> string\n";
			return 1;
		}

		Http::Client::RequestSender sender(argv[1], static_cast<uint16_t>(std::stoul(argv[2])));
		Http::HttpRequest request(Http::HttpMethodType::Get, argv[3], Http::HttpVersion{}, {}, {});
		std::cout << request.PackToString() << std::endl;
		sender.SendRequest(request, &ResponsePrinter);
		std::this_thread::sleep_for(std::chrono::seconds{2});
		// Http::Server::RequestHandler request_handler(argv[4]);

		// // Initialise the server.
		// Http::Server::Server s(
		// 	std::stoul(argv[3]),
		// 	argv[1],
		// 	argv[2],
		// 	[&request_handler]
		// 	(Http::Server::HttpRequestConnectionUPtr http_request)
		// 	{
		// 		if (!http_request)
		// 		{
		// 			return;
		// 		}
		// 		const auto request = http_request->GetRequest();
		// 		http_request->Send(request_handler.HandleRequest(request));
		// 	});

		// // Run the server until stopped.
		// s.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
		return 2;
	}

	return 0;
}
