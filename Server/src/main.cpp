#include <CustomServer/Server.hpp>
#include <CustomServer/RequestHandler.hpp>

#include <Http/HttpRequest.hpp>
#include <Http/HttpResponse.hpp>

#include <iostream>
#include <memory>
#include <exception>


int main(int argc, char* argv[])
{
	try
	{
		// Check command line arguments.
		if (argc != 5)
		{
			std::cerr << "Usage: http_server <address> <port> <threads> <doc_root>\n";
			return 1;
		}
		Http::Server::RequestHandler request_handler(argv[4]);

		// Initialise the server.
		Http::Server::Server s(
			std::stoul(argv[3]),
			argv[1],
			argv[2],
			[&request_handler]
			(Http::Server::HttpRequestConnectionUPtr http_request)
			{
				if (!http_request)
				{
					return;
				}
				const auto request = http_request->GetRequest();
				http_request->Send(request_handler.HandleRequest(request));
			});

		// Run the server until stopped.
		s.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
		return 2;
	}

	return 0;
}
