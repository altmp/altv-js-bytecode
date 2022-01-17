#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace CLI
{
    class Parser
    {
        std::unordered_map<std::string, std::string> args;

    public:
        Parser(int argc, char* argv[]);

        bool IsEmpty() const
        {
            return args.size() == 0;
        }
        bool HasArgument(const std::string& name) const
        {
            return args.count(name) != 0;
        }
        std::string GetArgument(const std::string& name) const
        {
            return args.at(name);
        }
    };
}  // namespace CLI
