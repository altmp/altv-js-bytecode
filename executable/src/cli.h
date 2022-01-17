#pragma once

#include <unordered_map>
#include <string>

namespace CLI
{
    class Parser
    {
        std::unordered_map<std::string, std::string> args;

    public:
        Parser(int argc, char* argv[])
        {
            // Parse args
            for(size_t i = 0; i < argc; i++)
            {
                std::string arg = argv[i];
                if(arg.find("--") != 0) continue;

                std::string name = arg.substr(2);
                if(i + 1 < argc) args[name] = argv[i + 1];
                else
                    args[name] = "";
                i++;
            }
        }

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
