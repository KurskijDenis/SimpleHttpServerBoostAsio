#include <CustomServer/Connection.hpp>

#include <CustomServer/ServerState.hpp>
#include <CustomServer/HttpRequestConnection.hpp>
#include <CustomServer/RequestHandler.hpp>

#include <Http/HttpResponse.hpp>

#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>


namespace Http::Server
{

std::shared_ptr<Connection> Connection::CreateHttpConnection(
	State& server_state,
	boost::asio::io_context& io_context,
	std::function<void(HttpRequestConnectionUPtr)> request_handler,
	const std::chrono::seconds timeout)
{
	return std::shared_ptr<Connection>{
		new Connection{server_state, io_context, std::move(request_handler), timeout}};
}

boost::asio::ip::tcp::socket& Connection::GetSocket()
{
	return socket_;
}

void Connection::Start()
{
	// start connection only once
	if (connection_id_ != 0)
	{
		return;
	}

	connection_started_ = std::chrono::steady_clock::now();
	static uint64_t connection_count = 0;
	connection_id_ = ++connection_count;
	SetTimeoutTimer();
	DoRead();
}

bool Connection::ConnectionIsAvailable() const
{
	return !server_state_.IsStopped() && !canceled_;
}

bool Connection::Write(std::string buffer, const bool keep_alive)
{
	if (!can_write_data_ || !ConnectionIsAvailable())
	{
		return false;
	}

	strand_.post(
		[this, self = shared_from_this(), response = std::move(buffer), keep_alive]() mutable
		{
			DoWrite(std::move(response), keep_alive);
		});
	return true;
}

Connection::~Connection()
{
	server_state_.RemoveConnection();
}
Connection::Connection(
	State& server_state,
	boost::asio::io_context& io_context,
	std::function<void(HttpRequestConnectionUPtr)> request_handler,
	const std::chrono::seconds timeout)
	: server_state_(server_state)
	, io_context_(io_context)
	, request_handler_(std::move(request_handler))
	, socket_(io_context_)
	, strand_(io_context_)
	, timeout_(timeout)
{
	if (!request_handler_)
	{
		throw std::runtime_error("Request handler isn't set");
	}

	server_state_.AddConnection();
}

void Connection::SetTimeoutTimer()
{
	if (timeout_ == std::chrono::seconds{0})
	{
		return;
	}

	timeout_timer_.emplace(io_context_);
	timeout_timer_->expires_after(timeout_);
	DoSetTimerHandler();
}

void Connection::CancelTimeoutTimer()
{
	if (timeout_timer_)
	{
		timeout_timer_->cancel();
	}
}

void Connection::DoContinueSession()
{
	strand_.post(
		[this, self = shared_from_this()]()
		{
			request_parser_ = HttpRequestParser{};
			response_.clear();
			DoRead();
		});
}

void Connection::DoSetTimerHandler()
{
	if (!timeout_timer_)
	{
		return;
	}

	timeout_timer_->async_wait(
		boost::asio::bind_executor(strand_,
		[this, self = shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec && ec == boost::asio::error::operation_aborted)
			{
				return;

			}
			canceled_ = true;

			std::cout << "Stop connection " << connection_id_ << " because of timeout \n";

			// The deadline has passed. The socket is closed so that any outstanding
			// asynchronous operations are cancelled.
			socket_.close();
		}));
}

void Connection::DoRead()
{
	socket_.async_read_some(
		boost::asio::buffer(buffer_),
		boost::asio::bind_executor(strand_,
		[this, self = shared_from_this()](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (ec)
			{
				CancelTimeoutTimer();
				std::cerr << "Can't read data for connection " << connection_id_ << ":" << ec.message() << std::endl;
				return;
			}

			const auto result = request_parser_.Parse(buffer_.data(), buffer_.data() + bytes_transferred);
			if (result == ParsingResult::InProgress)
			{
				DoRead();
				return;
			}

			can_write_data_ = true;

			if (result != ParsingResult::Ok)
			{
				DoWrite(StockResponse(StatusCode::BadRequest).PackToString());
				return;
			}

			auto http_request = request_parser_.PopHttpRequest();
			if (!http_request)
			{
				DoWrite(StockResponse(StatusCode::BadRequest).PackToString());
				return;
			}

			request_handler_(
				std::make_unique<HttpRequestConnection>(
					std::move(*http_request),
					shared_from_this()));
		}));
}

void Connection::DoWrite(std::string response, const bool keep_alive)
{
	bool writing_data = true;

	if (!can_write_data_.compare_exchange_strong(writing_data, false) || server_state_.IsStopped())
	{
		return;
	}

	std::swap(response, response_);

	socket_.async_send(
		boost::asio::buffer(response_),
		boost::asio::bind_executor(strand_,
		[this, self = shared_from_this(), keep_alive]
		(boost::system::error_code ec, size_t)
		{
			std::cout << "Finish request " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - connection_started_).count() << "micros" << std::endl;
			if (ec)
			{
				std::cerr << "Can't send data for connection " << connection_id_ << ":" << ec.message() << std::endl;
				CancelTimeoutTimer();
				return;
			}

			if (keep_alive)
			{
				DoContinueSession();
				return;
			}

			CancelTimeoutTimer();
      }));
}

} // namespace Http::Server
