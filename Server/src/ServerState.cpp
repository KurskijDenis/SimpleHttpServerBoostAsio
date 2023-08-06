#include <CustomServer/ServerState.hpp>


namespace Http::Server
{

void State::AddConnection() noexcept
{
	++connection_count_;
}

void State::RemoveConnection() noexcept
{
	--connection_count_;
}

bool State::Stop() noexcept
{
	auto exp_value = false;
	return stopped_.compare_exchange_strong(exp_value, true);
}

unsigned State::ConnectionCount() const noexcept
{
	return connection_count_;
}

bool State::HasConnections() const noexcept
{
	return !stopped_ || connection_count_ != 0;
}

bool State::IsStopped() const noexcept
{
	return stopped_;
}


} // namespace Http::Server
