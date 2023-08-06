#pragma once

#include <Http/Types.hpp>
#include <Http/HttpResponse.hpp>

#include <array>
#include <list>
#include <tuple>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>


namespace Http::Client
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
	UnknownStatusCode,
	//! Incorrect uri.
	IncorrectStatusText,
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
	//! Body chunk error.
	BodyChunkError,
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
 * \brief Http status line parser.
 */
class HttpStatusLineParser final
{
private:
	/**
	 * \brief Parser state.
	 */
	enum class State
	{
		HttpVersionH,
		HttpVersionT1,
		HttpVersionT2,
		HttpVersionP,
		HttpVersionSlash,
		HttpVersionMajorStart,
		HttpVersionMajor,
		HttpVersionMinorStart,
		HttpVersionMinor,
		CodeStart,
		Code,
		StatusText,
		Descrition,
		ExpectingNewline1,
		Parsed
	};

public:
	/**
	 * \brief Add char and parser.
	 */
	[[nodiscard]] ParsingResult Parse(const char ch) noexcept;
	/**
	 * \brief Return http status code.
	 */
	[[nodiscard]] StatusCode GetStatusCode() const noexcept;
	/**
	 * \brief Return http status text.
	 */
	[[nodiscard]] std::string GetStatusText() const noexcept;
	/**
	 * \brief Return http version.
	 */
	[[nodiscard]] HttpVersion GetVersion() const noexcept;

private:
	//! Http state.
	State state_ = State::HttpVersionH;
	//! Http minor version size.
	size_t minor_version_size_ = 0;
	//! Method string.
	CustomStringOnStack<100> status_text_;
	//! Http version.
	HttpVersion version_;
	//! Http status code (unsigned).
	unsigned code_ = 0;
	//! Http status code.
	StatusCode status_code_ = StatusCode::Unknown;
	//! Http status code size.
	size_t http_status_code_size_ = 0;
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

	/**
	 * \brief Return true if body contain several chunks.
	 */
	[[nodiscard]] bool IsBodyContainChunks() const noexcept;

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
 * \brief Http body parser.
 */
class BodyChunksParser final
{
private:
	/**
	 * \brief Parser state.
	 */
	enum class State
	{
		ChunkSizeStart,
		ChunkSize,
		ChunkNewLine1,
		ChunkBody,
		ChunkBodyExpectNewLine,
		ChunkNewLine2,
		Parsed
	};

public:
	/**
	 * \brief Add char to parser.
	 */
	[[nodiscard]] ParsingResult Parse(char ch) noexcept;
	/**
	 * \brief Pop body.
	 */
	[[nodiscard]] std::string PopBody() noexcept;

private:
	//! Parser status.
	State state_ = State::ChunkSizeStart;
	//! Current chunk size.
	size_t chunk_size_ = 0;
	//! Current chunk.
	std::string current_chunk_;
	//! Readed chunks.
	std::list<std::string> chunks_;
	//! Body size
	size_t body_size_ = 0;
};

/**
 * \brief Http response parser.
 */
class HttpResponseParser final
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
		BodyChunk,
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
	 * \brief Pop http response from parser, if possible.
	 */
	[[nodiscard]] std::optional<HttpResponse> PopHttpResponse() noexcept;

private:
	//! Parser status.
	State state_ = State::HttpStart;
	//! Status line parser.
	HttpStatusLineParser status_line_parser_;
	//! Headers block parser.
	HeadersParser headers_parser_;
	//! Body chunks parser.
	BodyChunksParser body_chunks_parser_;
	//! Http body size.
	size_t body_size_ = 0;
	//! Http body.
	std::string body_;
};

} // namespace Http::Client
