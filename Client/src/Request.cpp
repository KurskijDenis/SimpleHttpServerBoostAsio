#include <CustomClient/Request.hpp>

#include <Http/HttpResponse.hpp>
#include <Http/HttpRequest.hpp>

#include <cstdint>
#include <optional>
#include <functional>
#include <iostream>
#include <string>
#include <utility>


namespace Http::Client
{

RequestPtr Request::Create(
	boost::asio::io_context& io_context,
	const HttpRequest& http_request,
	std::function<void(std::optional<HttpResponse>)> response_handler,
	const std::string& address,
	const uint16_t port,
	const std::chrono::seconds& timeout)
{
	return std::shared_ptr<Request>{
		new Request{
			io_context,
			http_request,
			std::move(response_handler),
			address,
			port,
			timeout
		}};
}

void Request::Send()
{
	connection_started_ = std::chrono::steady_clock::now();
	static uint64_t request_count = 0;
	request_id_ = ++request_count;

	SetTimeoutTimer();
	resolver_.async_resolve(
		address_,
		std::to_string(port_),
		[this, self = shared_from_this()](
			const boost::system::error_code& err,
			const boost::asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (err)
			{
				std::cerr << "Error: " << err.message() << "\n";
				CancelTimeoutTimer();
				return;
			}
			DoConnect(endpoints);
		});
}

Request::~Request()
{
	// Request may die after RequestSender has died.
	// Don't use members and reference from RequestSender.
	if (response_handler_)
	{
		(*response_handler_)(std::nullopt);
		response_handler_.reset();
	}
}

Request::Request(
	boost::asio::io_context& io_context,
	const HttpRequest& http_request,
	std::function<void(std::optional<HttpResponse>)> response_handler,
	const std::string& address,
	const uint16_t port,
	const std::chrono::seconds& timeout)
	: io_context_(io_context)
	, address_(address)
	, port_(port)
	, timeout_(std::move(timeout))
	, request_str_(http_request.PackToString())
	, response_handler_(std::move(response_handler))
	, resolver_(io_context_)
	, socket_(io_context_)
{
}

void Request::SetTimeoutTimer()
{
	if (timeout_ == std::chrono::seconds{0})
	{
		return;
	}

	timeout_timer_.emplace(io_context_);
	timeout_timer_->expires_after(timeout_);
	DoSetTimerHandler();
}

void Request::DoSetTimerHandler()
{
	if (!timeout_timer_)
	{
		return;
	}

	timeout_timer_->async_wait(
		[this, self = shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec && ec == boost::asio::error::operation_aborted)
			{
				return;

			}

			std::cout << "Stop request " << request_id_ << " because of timeout \n";
			socket_.close();
		});
}

void Request::CancelTimeoutTimer()
{
	if (timeout_timer_)
	{
		timeout_timer_->cancel();
	}
}

void Request::DoConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints)
{
	boost::asio::async_connect(
		socket_,
		endpoints,
		[this, self = shared_from_this()](const boost::system::error_code& err, const boost::asio::ip::tcp::endpoint&)
		{
			if (err)
			{
				std::cerr << "Error: " << err.message() << "\n";
				CancelTimeoutTimer();
				return;
			}
			DoWrite();
		});
}

void Request::DoWrite()
{
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(request_str_),
		[this, self = shared_from_this()](const boost::system::error_code& err, size_t)
		{
			if (err)
			{
				std::cerr << "Error: " << err.message() << "\n";
				CancelTimeoutTimer();
				return;
			}
			DoRead();
		});
}

void Request::DoRead()
{
	socket_.async_read_some(
		boost::asio::buffer(buffer_),
		[this, self = shared_from_this()](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (ec)
			{
				CancelTimeoutTimer();
				std::cerr << "Error: " << ec.message() << "\n";
				return;
			}

			const auto result = response_parser_.Parse(buffer_.data(), buffer_.data() + bytes_transferred);
			if (result == ParsingResult::InProgress)
			{
				DoRead();
				return;
			}

			CancelTimeoutTimer();
			if (result != ParsingResult::Ok)
			{
				std::cerr << "Error bad response\n";
				return;
			}

			(*response_handler_)(response_parser_.PopHttpResponse());
			response_handler_.reset();
			std::cout << "Finish request " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - connection_started_).count() << "micros" << std::endl;
			socket_.close();
		});
}

} // namespace Http::Client
