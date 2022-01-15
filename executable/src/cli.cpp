#include "cli.h"
#include "logger.h"

using namespace CLI;

Parser::Parser(int argc, char* argv[])
{
    SetupArguments(parser);
    try
    {
        results = parser.parse(argc, argv);
    }
    catch(const std::exception& e)
    {
        Logger::Instance().LogError(e.what());
        failed = true;
    }
}

// *** CLI Args
void Parser::SetupArguments(argagg::parser& parser)
{
    auto& args = parser.definitions;

    args.push_back({ "help", { "-h", "--help" }, "Shows this help message", 0 });
}
