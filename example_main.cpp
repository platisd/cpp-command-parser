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
    const auto get = UnparsedCommand::create("get", "Get configuration key", "[-xyz] <key> [default]")
                         .withOptions({ "x", "y", "z" })
                         .withArgs<std::string, std::optional<std::string>>();
    const auto clear = UnparsedCommand::create("clear", "Clear configuration key", "<key>").withArgs<std::string>();
    const auto put
        = UnparsedCommand::create("put", "Store key value", "<key> <value>").withArgs<std::string, std::string>();
    const auto subscribe
        = UnparsedCommand::create("subscribe", "Subscribe to configuration key(s) and receive updates", "<key>")
              .withArgs<std::string>();
    const auto verify
        = UnparsedCommand::create("verify", "Verify secret", "<key> <secret>").withArgs<std::string, std::string>();
    const auto encrypt = UnparsedCommand::create(
                             "encrypt"
                             "Encrypt the given files with the specified policy",
                             "<policy> [file...]")
                             .withArgs<std::string, std::vector<std::string>>();
    const std::tuple commands { help, schema, defaults, all, list, get, clear, put, subscribe, verify, encrypt };
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
        const auto [key, defaultValue] = parsedCommand.getArgs(get);
        const auto x = parsedCommand.hasOption("x");
        const auto y = parsedCommand.hasOption("y");
        const auto z = parsedCommand.hasOption("z");
        std::cout << "get " << key << " " << x << " " << y << " " << z << std::endl;
        if (defaultValue) {
            std::cout << "default " << defaultValue.value() << std::endl;
        }
    } else if (parsedCommand.is(encrypt)) {
        const auto [policy, files] = parsedCommand.getArgs(encrypt);
        std::cout << "encrypt " << policy << std::endl;
        for (const auto& file : files) {
            std::cout << "file " << file << std::endl;
        }
    } else {
        auto helpPrompt = parsedCommand.help();
        std::cerr << "Available commands:" << std::endl;
        std::cerr << helpPrompt << std::endl;
    }

    using namespace CommandParser::literals;

    const auto o = "someIntKey"_i;
    (void) o;

    return 0;
}
