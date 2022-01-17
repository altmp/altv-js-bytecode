#include "cli.h"
#include "logger.h"

#include <vector>

using namespace CLI;

Parser::Parser(int argc, char* argv[])
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
