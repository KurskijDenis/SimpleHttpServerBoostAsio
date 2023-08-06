#pragma once

#include <atomic>

namespace Http::Server
{

class State final
{
public:
	/**
	 * \brief Add conenction.
	 */
	void AddConnection() noexcept;
	/**
	 * \brief Remove connection.
	 */
	void RemoveConnection() noexcept;
	/**
	 * \brief Stop conneciton if server work.
	 */
	[[nodiscard]] bool Stop() noexcept;
	/**
	 * \brief Return connection count.
	 */
	[[nodiscard]] unsigned ConnectionCount() const noexcept;
	/**
	 * \brief Check if server is working.
	 */
	[[nodiscard]] bool HasConnections() const noexcept;
	/**
	 * \brief Check if server was stopped.
	 */
	[[nodiscard]] bool IsStopped() const noexcept;

private:
	// TODO change into hardware_destructive_interference_size
	//! Server conenction count.
	alignas(128) std::atomic_uint connection_count_ = 0;
	//! Server was stooped.
	alignas(128) std::atomic_bool stopped_ = false;
};

} // namespace Http::Server
