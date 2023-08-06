#include <CustomClient/ResponseParser.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <cassert>
#include <iostream>


namespace Http::Client
{

namespace
{

constexpr size_t max_minor_version_size = 5;
constexpr size_t max_status_code_size = 6;
constexpr size_t max_status_code = 10000;
constexpr size_t max_body_size = 1 << 22;

bool IsChar(const char c) noexcept
{
	return c >= 0 && c <= 127;
}

uint8_t GetHex(const char c) noexcept
{
	if (std::isdigit(c))
	{
		return c - '0';
	}
	const auto lc = std::tolower(c);
	if (lc >= 'a' && lc <= 'f')
	{
		return 10 + (lc - 'a');
	}
	return std::numeric_limits<uint8_t>::max();
}

bool IsCtl(const char c) noexcept
{
	return (c >= 0 && c <= 31) || (c == 127);
}

bool IsSpecial(const char c) noexcept
{
	static const std::unordered_set<char> special_chars =
	{
		'(', ')', '<', '>', '@',
		',', ';', ':', '\\', '"',
		'/', '[', ']', '?', '=',
		'{', '}', ' ', '\t'
	};
	return special_chars.count(c) != 0;
}

} // namespace

std::string_view GetParsingResultMsg(const ParsingResult parsing_result) noexcept
{
	using namespace std::literals;
	static const std::unordered_map<ParsingResult, std::string_view> msgs = {
		{ParsingResult::Ok, "Ok"sv},
		{ParsingResult::InProgress, "Parsing in progress"sv},
		{ParsingResult::UnknownStatusCode, "Unknown status code"sv},
		{ParsingResult::IncorrectStatusText, "Incorrect status text"sv},
		{ParsingResult::UnknowHttpVersion, "Unknown http version"sv},
		{ParsingResult::NewLine1Error, "Can't find new line after first http line"sv},
		{ParsingResult::UnknownState, "Internal error unknown parsing states"sv},
		{ParsingResult::AlreadyParsed, "Request was already parsed"sv},
		{ParsingResult::HttpHeaderKeyError, "Http key error"sv},
		{ParsingResult::HttpHeaderValueError, "Http value error"sv},
		{ParsingResult::NewLine2Error, "Can't find new line after header section"sv},
		{ParsingResult::HttpHeadersSectionSizeIsBig, "Headers section is too big"sv},
		{ParsingResult::BodyChunkError, "Can't read body chunks"sv},
	};
	const auto it = msgs.find(parsing_result);
	return it != msgs.cend() ? it->second : "Internal error unknown parsing state"sv;
}

ParsingResult HttpStatusLineParser::Parse(const char ch) noexcept
{
	switch(state_)
	{
	case State::HttpVersionH:
	{
		if (std::tolower(ch) == 'h')
		{
			state_ = State::HttpVersionT1;
			return ParsingResult::InProgress;
		}
		return ParsingResult::UnknowHttpVersion;
	}
	case State::HttpVersionT1:
	{
		if (std::towlower(ch) == 't')
		{
			state_ = State::HttpVersionT2;
			return ParsingResult::InProgress;
		}
		return ParsingResult::UnknowHttpVersion;
	}
	case State::HttpVersionT2:
	{
		if (std::towlower(ch) == 't')
		{
			state_ = State::HttpVersionP;
			return ParsingResult::InProgress;
		}
		return ParsingResult::UnknowHttpVersion;
	}
	case State::HttpVersionP:
	{
		if (std::towlower(ch) == 'p')
		{
			state_ = State::HttpVersionSlash;
			return ParsingResult::InProgress;
		}
		return ParsingResult::UnknowHttpVersion;
	}
	case State::HttpVersionSlash:
	{
		if (ch == '/')
		{
			state_ = State::HttpVersionMajorStart;
			return ParsingResult::InProgress;
		}
		return ParsingResult::UnknowHttpVersion;
	}
	case State::HttpVersionMajorStart:
	{
		if (ch != '0' && ch != '1')
		{
			return ParsingResult::UnknowHttpVersion;
		}
		version_.major = ch - '0';
		state_ = State::HttpVersionMajor;
		return ParsingResult::InProgress;
	}
	case State::HttpVersionMajor:
	{
		if (ch != '.')
		{
			return ParsingResult::UnknowHttpVersion;
		}
		state_ = State::HttpVersionMinorStart;
		return ParsingResult::InProgress;
	}
	case State::HttpVersionMinorStart:
	{
		if (!std::isdigit(ch))
		{
			return ParsingResult::UnknowHttpVersion;
		}
		++minor_version_size_;
		version_.minor = ch - '0';
		state_  = State::HttpVersionMinor;
		return ParsingResult::InProgress;
	}
	case State::HttpVersionMinor:
	{
		if (ch == ' ')
		{
			state_ = State::CodeStart;
			return ParsingResult::InProgress;
		}
		if (!std::isdigit(ch))
		{
			return ParsingResult::UnknowHttpVersion;
		}
		version_.minor = (version_.minor * 10) + (ch - '0');
		++minor_version_size_;
		if (minor_version_size_ >= max_minor_version_size)
		{
			return ParsingResult::UnknowHttpVersion;
		}
		return ParsingResult::InProgress;
	}
	case State::CodeStart:
	{
		if (!std::isdigit(ch))
		{
			return ParsingResult::UnknownStatusCode;
		}
		code_ = ch - '0';
		state_ = State::Code;
		return ParsingResult::InProgress;
	}
	case State::Code:
	{
		if (ch == ' ' || ch == '\t' || ch == '\r')
		{
			status_code_ = GetStatusCodeFromUnsigned(code_);
			if (status_code_ == StatusCode::Unknown)
			{
				return ParsingResult::UnknownStatusCode;
			}
			state_ = ch == '\r' ? State::ExpectingNewline1 : State::StatusText;
			return ParsingResult::InProgress;
		}
		if (!std::isdigit(ch))
		{
			return ParsingResult::UnknownStatusCode;
		}
		++http_status_code_size_;
		if (http_status_code_size_ > max_status_code_size)
		{
			return ParsingResult::UnknownStatusCode;
		}
		code_ = code_ * 10 + (ch - '0');
		if (code_ > max_status_code)
		{
			return ParsingResult::UnknownStatusCode;
		}
		return ParsingResult::InProgress;
	}
	case State::StatusText:
	{
		if (ch == '\r')
		{
			state_ = State::ExpectingNewline1;
			return ParsingResult::InProgress;
		}
		if (IsCtl(ch))
		{
			return ParsingResult::IncorrectStatusText;
		}
		if (!status_text_.AddChar(ch))
		{
			return ParsingResult::IncorrectStatusText;
		}
		return ParsingResult::InProgress;
	}
	case State::ExpectingNewline1:
	{
		if (ch != '\n')
		{
			return ParsingResult::NewLine1Error;
		}
		state_ = State::Parsed;
		return ParsingResult::Ok;
	}
	case State::Parsed: return ParsingResult::AlreadyParsed;
	default: return ParsingResult::UnknownState;
	}
	return ParsingResult::UnknownState;
}

StatusCode HttpStatusLineParser::GetStatusCode() const noexcept
{
	return status_code_;
}

std::string HttpStatusLineParser::GetStatusText() const noexcept
{
	return std::string{status_text_.GetStdStringView()};
}

HttpVersion HttpStatusLineParser::GetVersion() const noexcept
{
	return version_;
}

ParsingResult HeadersParser::Parse(const char ch) noexcept
{
	++readed_char_count_;

	if (readed_char_count_ > max_headers_block_size_)
	{
		return ParsingResult::HttpHeadersSectionSizeIsBig;
	}

	switch (state_)
	{
	case State::HeaderLineStart:
	{
		if (ch == '\r')
		{
			state_ = State::ExpectingNewLine3;
			return ParsingResult::InProgress;
		}
		if (!key_.Empty() && (ch == ' ' || ch == '\t'))
		{
			state_ = State::HeaderLws;
			return ParsingResult::InProgress;
		}
		if (!IsChar(ch) || IsCtl(ch) || IsSpecial(ch))
		{
			return ParsingResult::HttpHeaderKeyError;
		}
		key_.AddLowerChar(ch);
		state_ = State::HeaderName;
		return ParsingResult::InProgress;
	}
	case State::HeaderLws:
	{
		if (ch == '\r')
		{
			state_ = State::ExpectingNewLine2;
			return ParsingResult::InProgress;
		}
		if (ch == ' ' || ch == '\t')
		{
			return ParsingResult::InProgress;
		}
		else if (IsCtl(ch))
		{
			return ParsingResult::HttpHeaderValueError;
		}
		value_.AddChar(ch);
		state_ = State::HeaderValue;
		return ParsingResult::InProgress;
	}
	case State::HeaderName:
	{
		if (ch == ':')
		{
			state_ = State::HeaderLws;
			return ParsingResult::InProgress;
		}
		if (!IsChar(ch) || IsCtl(ch) || IsSpecial(ch))
		{
			return ParsingResult::HttpHeaderKeyError;
		}
		key_.AddChar(ch);
		return ParsingResult::InProgress;
	}
	case State::HeaderValue:
	{
		if (ch == '\r')
		{
			state_ = State::ExpectingNewLine2;
			return ParsingResult::InProgress;
		}
		if (IsCtl(ch))
		{
			return ParsingResult::HttpHeaderValueError;
		}
		value_.AddChar(ch);
		return ParsingResult::InProgress;
	}
	case State::ExpectingNewLine2:
	{
		if (ch != '\n')
		{
			return ParsingResult::HttpHeaderValueError;
		}
		if (!key_.Empty())
		{
			headers_.emplace(key_.PopString(), value_.PopString());
		}
		state_ = State::HeaderLineStart;
		return ParsingResult::InProgress;
	}
	case State::ExpectingNewLine3:
	{
		if (ch != '\n')
		{
			return ParsingResult::NewLine2Error;
		}
		state_ = State::Parsed;
		return ParsingResult::Ok;
	}
	case State::Parsed: return ParsingResult::AlreadyParsed;
	default: return ParsingResult::UnknownState;
	}
	return ParsingResult::UnknownState;
}

HeadersMap HeadersParser::PopHeaders() noexcept
{
	assert(key_.Empty() && value_.Empty());
	return std::move(headers_);
}

std::optional<size_t> HeadersParser::GetContentLength() const noexcept
{
	const auto it = headers_.find("Content-Length");
	if (it == headers_.cend())
	{
		return 0;
	}

	size_t content_length = 0;
	const auto& value = it->second;
	for (const auto ch : value)
	{
		if (ch == ' ')
		{
			continue;
		}

		if (!std::isdigit(ch))
		{
			return std::nullopt;
		}

		if (content_length > max_body_size / 10)
		{
			return std::nullopt;
		}

		content_length *= 10;
		const auto nv = ch - '0';
		if (content_length > max_body_size - nv)
		{
			return std::nullopt;
		}
		content_length += nv;
	}
	return content_length;
}

bool HeadersParser::IsBodyContainChunks() const noexcept
{
	const auto it = headers_.find("Transfer-Encoding");
	if (it == headers_.cend())
	{
		return false;
	}
	return it->second == "chunked";
}

ParsingResult BodyChunksParser::Parse(char ch) noexcept
{
	switch(state_)
	{
	case State::ChunkSizeStart:
	{
		chunk_size_ = 0;
		if (ch == '\r')
		{
			state_ = State::ChunkNewLine1;
			return ParsingResult::InProgress;
		}
		const auto iv = GetHex(ch);
		if (iv >= 16)
		{
			return ParsingResult::BodyChunkError;
		}
		chunk_size_ = iv;
		state_ = State::ChunkSize;
		return ParsingResult::InProgress;
	}
	case State::ChunkSize:
	{
		if (ch == '\r')
		{
			state_ = State::ChunkNewLine1;
			return ParsingResult::InProgress;
		}
		const auto iv = GetHex(ch);
		if (iv >= 16)
		{
			return ParsingResult::BodyChunkError;
		}
		chunk_size_ = (chunk_size_ * 16) + iv;
		return ParsingResult::InProgress;
	}
	case State::ChunkNewLine1:
	{
		if (ch != '\n')
		{
			return ParsingResult::BodyChunkError;
		}
		if (chunk_size_ == 0)
		{
			state_ = State::Parsed;
			return ParsingResult::Ok;
		}
		current_chunk_.reserve(chunk_size_);
		state_ = State::ChunkBody;
		return ParsingResult::InProgress;
	}
	case State::ChunkBody:
	{
		current_chunk_.push_back(ch);
		if (current_chunk_.size() == chunk_size_)
		{
			body_size_ += current_chunk_.size();
			chunks_.emplace_back(std::move(current_chunk_));
			current_chunk_.clear();
			chunk_size_ = 0;
			state_ = State::ChunkBodyExpectNewLine;
		}
		return ParsingResult::InProgress;
	}
	case State::ChunkBodyExpectNewLine:
	{
		if (ch != '\r')
		{
			return ParsingResult::BodyChunkError;
		}
		state_ = State::ChunkNewLine2;
		return ParsingResult::InProgress;
	}
	case State::ChunkNewLine2:
	{
		if (ch != '\n')
		{
			return ParsingResult::BodyChunkError;
		}
		state_ = State::ChunkSizeStart;
		return ParsingResult::InProgress;
	}
	case State::Parsed: return ParsingResult::AlreadyParsed;
	default: return ParsingResult::UnknownState;
	}
	return ParsingResult::UnknownState;
}

std::string BodyChunksParser::PopBody() noexcept
{
	std::string result;
	result.reserve(body_size_);
	for (auto& str : chunks_)
	{
		result += str;
		str.clear();
	}
	chunks_.clear();
	return result;
}

ParsingResult HttpResponseParser::Parse(const char ch) noexcept
{
	switch(state_)
	{
	case State::HttpStart:
	{
		const auto result = status_line_parser_.Parse(ch);
		if (result != ParsingResult::Ok)
		{
			return result;
		}
		state_ = State::Headers;
		return ParsingResult::InProgress;
	}
	case State::Headers:
	{
		const auto header_parser_result = headers_parser_.Parse(ch);
		if (header_parser_result != ParsingResult::Ok)
		{
			return header_parser_result;
		}
		if (headers_parser_.IsBodyContainChunks())
		{
			state_ = State::BodyChunk;
			return ParsingResult::InProgress;
		}
		if (const auto bs = headers_parser_.GetContentLength(); bs)
		{
			body_size_ = *bs;
		}
		if (body_size_ == 0)
		{
			state_ = State::Parsed;
			return ParsingResult::Ok;
		}
		state_ = State::Body;
		body_.reserve(body_size_);
		return ParsingResult::InProgress;
	}
	case State::Body:
	{
		body_.push_back(ch);
		if (body_.size() != body_size_)
		{
			return ParsingResult::InProgress;
		}
		state_ = State::Parsed;
		return ParsingResult::Ok;
	}
	case State::BodyChunk:
	{
		const auto result = body_chunks_parser_.Parse(ch);
		if (result != ParsingResult::Ok)
		{
			return result;
		}
		body_ = body_chunks_parser_.PopBody();
		body_size_ = body_.size();
		state_ = State::Parsed;
		return ParsingResult::Ok;
	}
	case State::Parsed: return ParsingResult::AlreadyParsed;
	default: return ParsingResult::UnknownState;
	}
	return ParsingResult::UnknownState;
}

ParsingResult HttpResponseParser::Parse(const char* begin, const char* end) noexcept
{
	if (begin == end)
	{
		return ParsingResult::InProgress;
	}
	for (; begin != end; ++begin)
	{
		const auto parse_result = Parse(*begin);
		if (parse_result != ParsingResult::InProgress)
		{
			return parse_result;
		}
	}
	return ParsingResult::InProgress;
}

std::optional<HttpResponse> HttpResponseParser::PopHttpResponse() noexcept
{
	if (state_ != State::Parsed)
	{
		return std::nullopt;
	}

	return HttpResponse{
		status_line_parser_.GetStatusCode(),
		headers_parser_.PopHeaders(),
		std::move(body_),
		status_line_parser_.GetStatusText(),
		status_line_parser_.GetVersion()
	};
}

} // namespace Http::Client
