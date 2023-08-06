#pragma once

#include <CustomClient/ResponseParser.hpp>

#include <Http/HttpResponse.hpp>
#include <Http/HttpRequest.hpp>

#include <boost/asio.hpp>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <functional>
#include <vector>


namespace Http::Client
{

class Request;
using RequestPtr = std::shared_ptr<Request>;

/**
 * \brief Http request.
 */
class Request final : public std::enable_shared_from_this<Request>
{
public:
	Request(const Request&) = delete;
	Request& operator=(const Request&) = delete;

	Request(Request&&) = delete;
	Request& operator=(Request&&) = delete;

	static RequestPtr Create(
		boost::asio::io_context& io_context,
		const HttpRequest& http_request,
		std::function<void(std::optional<HttpResponse>)> response_handler,
		const std::string& address = "127.0.0.1",
		const uint16_t port = 80,
		const std::chrono::seconds& timeout = std::chrono::seconds{60});

	/**
	 * \brief Send http request.
	 */
	void Send();

	~Request();

private:
	//TODO may improve speed if use several threads, but in this way we should use strand.
	explicit Request(
		boost::asio::io_context& io_context,
		const HttpRequest& http_request,
		std::function<void(std::optional<HttpResponse>)> response_handler,
		const std::string& address,
		const uint16_t port,
		const std::chrono::seconds& timeout);

	/**
	 * \brief Set timeout timer if timeout != 0.
	 */
	void SetTimeoutTimer();
	/**
	 * \brief Set handler, which will handle timer timeout.
	 */
	void DoSetTimerHandler();
	/**
	 * \brief Cancel timer timeout.
	 */
	void CancelTimeoutTimer();
	/**
	 * \brief Connect to endpoints.
	 */
	void DoConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints);
	/**
	 * \brief Write request into socket.
	 */
	void DoWrite();
	/**
	 * \brief Receive response from server.
	 */
	void DoRead();
private:
	//! Boost asio context.
	boost::asio::io_context& io_context_;
	//! Server address.
	const std::string address_;
	//! Server port.
	const uint16_t port_;
	//! Connection timeout.
	const std::chrono::seconds timeout_{0};
	//! Http request string.
	const std::string request_str_;
	//! Response handler callback.
	std::optional<std::function<void(std::optional<HttpResponse>)>> response_handler_;
	//! Response parser.
	HttpResponseParser response_parser_;
	//! Uri resolver.
	boost::asio::ip::tcp::resolver resolver_;
	//! Socket.
	boost::asio::ip::tcp::socket socket_;
	//! Buffer to receive bytes from socket.
	std::array<char, 8192> buffer_;
	//! Time point when connection started.
	std::chrono::steady_clock::time_point connection_started_;
	//! Request id.
	uint64_t request_id_ = 0;
	//! Timer evokes timeout signal.
	std::optional<boost::asio::steady_timer> timeout_timer_;
};

} // namespace Http::Client
