#pragma once

#include <CustomServer/Connection.hpp>
#include <CustomServer/HttpRequestConnection.hpp>
#include <CustomServer/ServerState.hpp>

#include <boost/asio.hpp>

#include <cstdint>
#include <thread>
#include <string>
#include <functional>
#include <vector>


namespace Http::Server
{

/**
 * \brief Http server.
 */
class Server
{
public:
	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	Server(Server&&) = delete;
	Server& operator=(Server&&) = delete;

	//TODO may improve server speed if one thread (shed strand and other)
	explicit Server(
		size_t thread_count,
		const std::string& address,
		const std::string& port,
		std::function<void(HttpRequestConnectionUPtr)> request_handler);

	/**
	* \brief Http server.
	*/
  void Run();

private:
	/**
	* \brief Start accept new connection.
	*/
	void StartAccept();

private:
	//! Thread count.
	const size_t thread_count_ = 1;
	//! Boost asio context.
	boost::asio::io_context io_context_;
	//! Sync get signal and accept new connection.
	boost::asio::io_service::strand strand_;
	//! Server state.
	State state_;
	//! Thread pool.
	std::vector<std::thread> work_threads_;
	//! Signals handler.
	boost::asio::signal_set signals_;
	//! Connections acceptor.
	boost::asio::ip::tcp::acceptor acceptor_;
	//! Requests handler callback.
	std::function<void(HttpRequestConnectionUPtr)> request_handler_;
};

} // namespace Http::Server
