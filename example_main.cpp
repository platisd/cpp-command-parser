#include <iostream>

#include "CommandParser.h"
#include <optional>
#include <string>
#include <tuple>

int main(int argc, char* argv[])
{
    const auto help = UnparsedCommand::create("help", "Print this help message");
    const auto schema = UnparsedCommand::create("schema", "Print JTD");
    const auto defaults = UnparsedCommand::create("defaults", "Print default JSON");
    const auto all = UnparsedCommand::create("all", "Print current config JSON");
    const auto list = UnparsedCommand::create("list", "List all available configuration keys", "[subkey]")
                          .withArgs<std::optional<std::string>>();
    const auto get = UnparsedCommand::create("get", "Get configuration key", " <key> [-xyz]")
                         .withArgs<std::string, std::optional<std::string>>();
    const auto clear = UnparsedCommand::create("clear", "Clear configuration key", "<key>").withArgs<std::string>();
    const auto put
        = UnparsedCommand::create("put", "Store key value", "<key> <value>").withArgs<std::string, std::string>();
    const auto subscribe
        = UnparsedCommand::create("subscribe", "Subscribe to configuration key(s) and receive updates", "<key>")
              .withArgs<std::string>();
    const auto verify
        = UnparsedCommand::create("verify", "Verify secret", "<key> <secret>").withArgs<std::string, std::string>();
    const std::tuple commands { help, schema, defaults, all, list, get, clear, put, subscribe, verify };
    const auto parsedCommand = UnparsedCommand::parse(argc, argv, commands);

    if (parsedCommand.is(schema)) {
        std::cout << "schema" << std::endl;
        // auto schema = parsedCommand.getArgs(schema); // Does not compile because schema has no args
    } else if (parsedCommand.is(put)) {
        const auto [key, value] = parsedCommand.getArgs(put);
        std::cout << "put " << key << " " << value << std::endl;
    } else if (parsedCommand.is(list)) {
        const auto [subkey] = parsedCommand.getArgs(list);
        if (subkey) {
            std::cout << "list " << subkey.value() << std::endl;
        } else {
            std::cout << "list" << std::endl;
        }
    } else if (parsedCommand.is(get)) {
        const auto [key, flags] = parsedCommand.getArgs(get);
        if (flags) {
            std::cout << "get " << key << " " << flags.value() << std::endl;
        } else {
            std::cout << "get " << key << std::endl;
        }
    } else if (parsedCommand.is(help)) {
        parsedCommand.help();
    } else {
        parsedCommand.help();
    }

    return 0;
}
