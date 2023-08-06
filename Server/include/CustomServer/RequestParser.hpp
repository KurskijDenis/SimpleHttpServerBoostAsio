#pragma once

#include <Http/Types.hpp>
#include <Http/HttpRequest.hpp>

#include <array>
#include <tuple>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>


namespace Http::Server
{

/**
 * \brief Parsing http msg result.
 */
enum class ParsingResult
{
	//! Parsing finish.
	Ok,
	//! Parsing in progress.
	InProgress,
	//! Unknown method.
	UnknownMethodType,
	//! Incorrect uri.
	IncorrectURI,
	//! Incorrect uri size (very big).
	IncorrectURISize,
	//! Unknown http verison.
	UnknowHttpVersion,
	//! Can't find first line.
	NewLine1Error,
	//! Unknown parsing state.
	UnknownState,
	//! Http section has been already parsed.
	AlreadyParsed,
	//! Http key error.
	HttpHeaderKeyError,
	//! Http value error.
	HttpHeaderValueError,
	//! Can't find second new line error.
	NewLine2Error,
	//! Http section is too big.
	HttpHeadersSectionSizeIsBig,
};

/**
 * \brief Get parsing status string.
 */
[[nodiscard]] std::string_view GetParsingResultMsg(const ParsingResult parsing_result) noexcept;

/**
 * \brief String on stack.
 */
template <size_t N>
class CustomStringOnStack final
{
public:
	/**
	 * \brief Add char into string.
	 */
	[[nodiscard]] bool AddChar(const char ch) noexcept
	{
		if (actual_str_size_ >= str_.size())
		{
			return false;
		}

		str_[actual_str_size_] = ch;
		++actual_str_size_;
		return true;
	}

	/**
	 * \brief Get string view representation.
	 */
	[[nodiscard]] std::string_view GetStdStringView() const noexcept
	{
		return std::string_view{str_.data(), actual_str_size_};
	}

	/**
	 * \brief Reset string.
	 */
	void Reset()
	{
		actual_str_size_ = 0;
	}

private:
	static_assert(N > 0);

	std::array<char, N> str_;
	size_t actual_str_size_ = 0;
};

/**
 * \brief Http first line parser.
 */
class HttpFirstLineParser final
{
private:
	/**
	 * \brief Parser state.
	 */
	enum class State
	{
		MethodStart,
		Method,
		Uri,
		HttpVersionH,
		HttpVersionT1,
		HttpVersionT2,
		HttpVersionP,
		HttpVersionSlash,
		HttpVersionMajorStart,
		HttpVersionMajor,
		HttpVersionMinorStart,
		HttpVersionMinor,
		ExpectingNewline1,
		Parsed
	};

public:
	/**
	 * \brief Add char and parser.
	 */
	[[nodiscard]] ParsingResult Parse(const char ch) noexcept;
	/**
	 * \brief Return http method.
	 */
	[[nodiscard]] HttpMethodType GetMethodType() const noexcept;
	/**
	 * \brief Return http uri.
	 */
	[[nodiscard]] std::string GetUri() const noexcept;
	/**
	 * \brief Return http version.
	 */
	[[nodiscard]] HttpVersion GetVersion() const noexcept;

private:
	//! Http parser state.
	State state_ = State::MethodStart;
	//! Http minor version size.
	size_t minor_version_size_ = 0;
	//! Method string.
	CustomStringOnStack<20> method_string_;
	//! Http uri.
	CustomStringOnStack<2048> uri_;
	//! Http method.
	HttpMethodType method_ = HttpMethodType::Unknown;
	//! Http version.
	HttpVersion version_;
};

/**
 * \brief Http headers parser.
 */
class HeadersParser final
{
private:
	/**
	 * \brief Parser state.
	 */
	enum class State
	{
		HeaderLineStart,
		HeaderLws,
		HeaderName,
		HeaderValue,
		ExpectingNewLine2,
		ExpectingNewLine3,
		Parsed
	};
public:
	/**
	 * \brief Add char and parser.
	 */
	[[nodiscard]] ParsingResult Parse(const char ch) noexcept;

	/**
	 * \brief Return all http headers.
	 */
	[[nodiscard]] HeadersMap PopHeaders() noexcept;

	/**
	 * \brief Return content length.
	 */
	[[nodiscard]] std::optional<size_t> GetContentLength() const noexcept;

private:
	/**
	 * \brief String on stack.
	 */
	template<size_t Size>
	class RawString final
	{
	public:
		/**
		 * \brief Add char to string (be careful, doesn't have any checks).
		 */
		void AddChar(const char value) noexcept
		{
			value_[actual_size_] = value;
			++actual_size_;
		}

		/**
		 * \brief Add lower char to string (be careful, doesn't have any checks).
		 */
		void AddLowerChar(const char value) noexcept
		{
			AddChar(std::tolower(value));
		}

		/**
		 * \brief Check if string is empty.
		 */
		[[nodiscard]] bool Empty() const noexcept { return actual_size_ == 0; }
		/**
		 * \brief Return std string and reset current string.
		 */
		[[nodiscard]] std::string PopString() noexcept
		{
			const auto string_size = actual_size_;
			actual_size_ = 0;
			return {value_.data(), value_.data() + string_size};
		}

	private:
		static_assert(Size > 0);
		std::array<char, Size> value_;
		size_t actual_size_ = 0;
	};

private:
	//! Max parsing block size.
	static constexpr size_t max_headers_block_size_ = 8192;

	//! Parser status.
	State state_ = State::HeaderLineStart;
	//! Header hey.
	RawString<max_headers_block_size_> key_;
	//! Header value.
	RawString<max_headers_block_size_> value_;
	//! Already readed symbols count.
	size_t readed_char_count_ = 0;
	//! Headers.
	HeadersMap headers_;
};

/**
 * \brief Http parser.
 */
class HttpRequestParser final
{
private:
	/**
	 * \brief Parser state.
	 */
	enum class State
	{
		HttpStart,
		Headers,
		Body,
		Parsed
	};

public:
	/**
	 * \brief Add char to parser.
	 */
	[[nodiscard]] ParsingResult Parse(char ch) noexcept;
	/**
	 * \brief Add chars to parser.
	 */
	[[nodiscard]] ParsingResult Parse(const char* begin, const char* end) noexcept;
	/**
	 * \brief Pop http request from parser, if possible.
	 */
	[[nodiscard]] std::optional<HttpRequest> PopHttpRequest() noexcept;

private:
	//! Parser status.
	State state_ = State::HttpStart;
	//! First line parser.
	HttpFirstLineParser first_line_parser_;
	//! Headers block parser.
	HeadersParser headers_parser_;
	//! Http body size.
	size_t body_size_ = 0;
	//! Http body.
	std::string body_;
};

} // namespace Http::Server
