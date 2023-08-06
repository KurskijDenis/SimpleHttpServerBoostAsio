#include <CustomServer/Server.hpp>

#include <CustomServer/Connection.hpp>

#include <csignal>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <utility>


namespace Http::Server
{

Server::Server(
	const size_t thread_count,
	const std::string& address,
	const std::string& port,
	std::function<void(HttpRequestConnectionUPtr)> request_handler)
	: thread_count_(thread_count)
	, strand_(io_context_)
	, signals_(io_context_)
	, acceptor_(io_context_)
	, request_handler_(std::move(request_handler))

{
	if (thread_count_ == 0)
	{
		throw std::runtime_error("Thread count should be more than 0 for http server");
	}
	// Register to handle the signals that indicate when the server should exit.
	// It is safe to register for the same signal multiple times in a program,
	// provided all registration for the specified signal is made through Asio.
	signals_.add(SIGINT);
	signals_.add(SIGTERM);
#if defined(SIGQUIT)
	signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
	signals_.async_wait(
		boost::asio::bind_executor(strand_,
		[this](boost::system::error_code ec, int signo)
		{
			if (ec)
			{
				std::cerr << "Got some error while was waiting signal: " << ec.message();
				return;
			}

			std::cout << "Got signal " << signo << std::endl;
			if (state_.Stop())
			{
				acceptor_.close();
			}
		}));

	// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
	boost::asio::ip::tcp::resolver resolver(acceptor_.get_executor());
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();

	StartAccept();
}

void Server::Run()
{
	work_threads_.reserve(thread_count_);

	for (size_t i = 0; i < thread_count_; ++i)
	{
		work_threads_.emplace_back([&io_context = io_context_]() {io_context.run();});
	}

	for (auto& thread : work_threads_)
	{
		thread.join();
	}

	while (state_.HasConnections());
}

void Server::StartAccept()
{
	if (state_.IsStopped())
	{
		return;
	}

	auto new_connection = Connection::CreateHttpConnection(state_, io_context_, request_handler_);
	acceptor_.async_accept(
		new_connection->GetSocket(),
		boost::asio::bind_executor(strand_,
		[connection = new_connection, this](const boost::system::error_code& e)
		{
			if (e)
			{
				if (e == boost::asio::error::operation_aborted)
				{
					std::cerr << "Accpetor was closed: " << e.message() << std::endl;
					return;
				}
				std::cerr << "Accpetor error: " << e.message() << std::endl;
			}
			connection->Start();
			StartAccept();
		}
	));
}

} // namespace Http::Server
