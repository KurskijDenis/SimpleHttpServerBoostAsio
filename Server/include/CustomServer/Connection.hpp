#pragma once

#include <CustomServer/RequestParser.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <array>
#include <optional>
#include <functional>
#include <memory>
#include <string>


namespace Http::Server
{

class State;

class HttpRequestConnection;
using HttpRequestConnectionUPtr = std::unique_ptr<HttpRequestConnection>;

/**
 * \brief Http connection.
 */
class Connection final : public std::enable_shared_from_this<Connection>
{
public:
	[[nodiscard]] static std::shared_ptr<Connection> CreateHttpConnection(
		State& server_state,
		boost::asio::io_context& io_context,
		std::function<void(HttpRequestConnectionUPtr)> request_handler,
		std::chrono::seconds timeout = std::chrono::seconds{60});

public:
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;

	Connection(Connection&&) = delete;
	Connection& operator=(Connection&&) = delete;

	/**
	 * \brief Return socket.
	 */
	[[nodiscard]] boost::asio::ip::tcp::socket& GetSocket();

	/**
	 * \brief Start http connection.
	 */
	void Start();

	/**
	 * \brief Check if connection is alive.
	 */
	[[nodiscard]] bool ConnectionIsAvailable() const;

	/**
	 * \brief Send data into buffer.
	 *
	 * \param[in] buffer Buffer to send.
	 * \param[in] keep_alive Close connection or not after all data is sended.
	 *
	 * \return True if can send data, false otherwise.
	 */
	[[nodiscard]] bool Write(std::string buffer, const bool keep_alive = false);

	~Connection();

private:

	explicit Connection(
		State& server_state,
		boost::asio::io_context& io_context,
		std::function<void(HttpRequestConnectionUPtr)> request_handler,
		std::chrono::seconds timeout = std::chrono::seconds{60});

	/**
	 * \brief Set connection timeout.
	 */
	void SetTimeoutTimer();
	/**
	 * \brief Cancel timeout timer.
	 */
	void CancelTimeoutTimer();
	/**
	 * \brief Continue connect after life circle.
	 */
	void DoContinueSession();
	/**
	 * \brief Set timer handler.
	 */
	void DoSetTimerHandler();
	/**
	 * \brief Start read from socket.
	 */
	void DoRead();
	/**
	 * \brief Write data into socket.
	 */
	void DoWrite(std::string response, const bool keep_alive = false);

private:
	//! Server state.
	State& server_state_;
	//! Asio context.
	boost::asio::io_context& io_context_;
	//! Request handler function.
	std::function<void(HttpRequestConnectionUPtr)> request_handler_;
	//! Connection socket.
	boost::asio::ip::tcp::socket socket_;
	//! Helps to call write, read and timer wake up consequentially (boost asio socket peculiarities).
	boost::asio::io_service::strand strand_;
	//! Connection timeout.
	std::chrono::seconds timeout_{0};
	//! Buffer to receive bytes from socket.
	std::array<char, 8192> buffer_;
	//! Request parser.
	HttpRequestParser request_parser_;
	//! Hold client response.
	std::string response_;
	//! Time point when connection started.
	std::chrono::steady_clock::time_point connection_started_;
	//! Can send new data into socket or not.
	std::atomic_bool can_write_data_ = false;
	//! Connection id.
	uint64_t connection_id_ = 0;
	//! Timer evokes timeout signal.
	std::optional<boost::asio::steady_timer> timeout_timer_;
	//! Member indicates that all operation in socket was stopped.
	std::atomic_bool canceled_ = false;
};

using ConnectionPtr = std::shared_ptr<Connection>;

} // namespace Http::Server
