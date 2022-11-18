# Command Parser

Command Parser is a header-only utility library for parsing command line "commands".

We define a _command_ as a string passed to a binary as a command line argument in the form of:

```bash
./your_binary command_name <arg1> <arg2> [arg3]
```

## Usage

See [example_main.cpp](example_main.cpp) for a simple example of how to use the library. In a nutshell:

```cpp
const auto all = UnparsedCommand::create("all", "Print current configuration");
const auto get = UnparsedCommand::create("get", "Get configuration key", " <key> [-xyz]")
                         .withArgs<std::string, std::optional<std::string>>();
const std::tuple commands { all, get };
const auto parsedCommand = UnparsedCommand::parse(argc, argv, commands);

if (parsedCommand.is(all)) {
    std::cout << "all" << std::endl;
    // auto config = parsedCommand.getArgs(all); // Does not compile because all has no args
} else if (parsedCommand.is(get)) {
    const auto [key, flags] = parsedCommand.getArgs(get);
    if (flags) {
        std::cout << "get " << key << " " << flags.value() << std::endl;
    } else {
        std::cout << "get " << key << std::endl;
    }
} else {
        parsedCommand.help();
}
```

