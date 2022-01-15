#pragma once

#include "argagg.h"
#include <vector>

namespace CLI
{
    class Parser
    {
        argagg::parser parser;
        argagg::parser_results results;
        bool failed = false;

        void SetupArguments(argagg::parser& parser);

    public:
        Parser(int argc, char* argv[]);

        bool DidFail() const
        {
            return failed;
        }
        bool IsEmpty() const
        {
            for(auto& arg : results.options)
            {
                if(arg.second.count() != 0) return false;
            }
            return results.count() == 0;
        }
        bool HasArgument(const std::string& name) const
        {
            return results[name];
        }

        operator std::string() const
        {
            return parser.to_string();
        }
    };
}  // namespace CLI
