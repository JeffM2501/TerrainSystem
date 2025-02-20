#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace EditorFramework
{
	namespace ArgumentUtils
	{
		inline bool HasArgument(std::string_view argument, const std::vector<std::string>& args)
		{
			for (const auto& arg : args)
			{
				if (arg == argument)
					return true;
			}

			return false;
		}

		inline std::vector<std::string_view> GetArgumentParams(std::string_view argument, size_t pramCount, const std::vector<std::string>& args)
		{
			std::vector<std::string_view> params;

			int count = 0;
			bool foundArg = false;

			for (const auto& arg : args)
			{
				if (arg == argument)
				{
					foundArg = true;
				}
				else if (foundArg && count < pramCount)
				{
					params.push_back(std::string_view(arg));
					pramCount++;
				}
			}

			return params;
		}
	}
}