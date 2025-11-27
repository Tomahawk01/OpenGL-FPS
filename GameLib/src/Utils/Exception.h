#pragma once

#include <format>
#include <stacktrace>
#include <stdexcept>
#include <string>

namespace Game {

	class Exception : public std::runtime_error
	{
	public:
		template<class ...Args>
		Exception(std::format_string<Args...> msg, Args&&... args)
			: std::runtime_error{ std::format(msg, std::forward<Args>(args)...) }
			, m_What{ std::format("{}\n{}", std::runtime_error::what(), std::stacktrace::current(1)).c_str() }
		{}

		std::string to_string() const
		{
			return m_What;
		}

		const char* what() const noexcept override
		{
			return m_What.c_str();
		}

	private:
		std::string m_What;
	};

}
