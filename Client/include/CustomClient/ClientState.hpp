#pragma once

#include <atomic>


namespace Http::Client
{

class State final
{
public:
	/**
	 * \brief Add request.
	 */
	void AddRequest() noexcept;
	/**
	 * \brief Remove request.
	 */
	void RemoveRequest() noexcept;
	/**
	 * \brief Stop conneciton if request sender work.
	 */
	[[nodiscard]] bool Stop() noexcept;
	/**
	 * \brief Return active request count.
	 */
	[[nodiscard]] unsigned RequestCount() const noexcept;
	/**
	 * \brief Check if requests are processing.
	 */
	[[nodiscard]] bool HasRequests() const noexcept;
	/**
	 * \brief Check if client was stopped.
	 */
	[[nodiscard]] bool IsStopped() const noexcept;

private:
	// TODO change into hardware_destructive_interference_size
	//! Server conenction count.
	alignas(128) std::atomic_uint request_count_ = 0;
	//! Server was stooped.
	alignas(128) std::atomic_bool stopped_ = false;
};

} // namespace Http::Client
