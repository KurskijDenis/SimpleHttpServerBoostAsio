#include <CustomClient/ClientState.hpp>


namespace Http::Client
{

void State::AddRequest() noexcept
{
	++request_count_;
}

void State::RemoveRequest() noexcept
{
	--request_count_;
}

bool State::Stop() noexcept
{
	auto exp_value = false;
	return stopped_.compare_exchange_strong(exp_value, true);
}

unsigned State::RequestCount() const noexcept
{
	return request_count_;
}

bool State::HasRequests() const noexcept
{
	return !stopped_ || request_count_ != 0;
}

bool State::IsStopped() const noexcept
{
	return stopped_;
}

} // namespace Http::Client
