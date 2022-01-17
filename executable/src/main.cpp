#include "cli.h"
#include "logger.h"

int main(int argc, char* argv[])
{
    CLI::Parser parser(argc, argv);
    if(parser.IsEmpty())
    {
        Logger::Instance().LogError("Failed to parse commandline arguments");
        return 1;
    }

    if(parser.HasArgument("help"))
    {
        // todo: add help
        Logger::Instance().Log("");
        return 0;
    }

    return 0;
}
