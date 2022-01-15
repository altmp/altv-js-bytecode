#include "cli.h"
#include "logger.h"

int main(int argc, char* argv[])
{
    CLI::Parser parser(argc, argv);
    if(parser.DidFail()) return 1;

    if(parser.IsEmpty())
    {
        Logger::Instance().LogError("Failed to parse commandline arguments\n" + (std::string)parser);
        return 1;
    }

    if(parser.HasArgument("help"))
    {
        Logger::Instance().Log("\n" + (std::string)parser);
        return 0;
    }

    return 0;
}
