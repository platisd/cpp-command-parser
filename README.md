# Command Parser

Command Parser is a header-only utility library for parsing command line "commands".

We define a _command_ as a string passed to a binary as a command line argument in the form of:

```bash
./your_binary command_name <arg1> <arg2> [arg3]
```

A command may optionally take some (boolean) options

```bash
./your_binary command_name --option1 --option2 <arg1> <arg2> [arg3]
```

## Usage

See [example_main.cpp](example_main.cpp) for a simple example of how to use the library. In a nutshell:

```cpp
const auto all = UnparsedCommand::create("all", "Print current configuration");
const auto get = UnparsedCommand::create("get", "Get configuration key", "[-xyz] <key> [default]")
                         .withArgs<std::string, std::optional<std::string>>();
const std::tuple commands { all, get };
const auto parsedCommand = UnparsedCommand::parse(argc, argv, commands);

if (parsedCommand.is(all)) {
    std::cout << "all" << std::endl;
    // auto config = parsedCommand.getArgs(all); // Does not compile because all has no args
} else if (parsedCommand.is(get)) {
    const auto [key, defaultValue] = parsedCommand.getArgs(get);
    const auto x = parsedCommand.hasOption("x");
    const auto y = parsedCommand.hasOption("y");
    const auto z = parsedCommand.hasOption("z");
    std::cout << "get " << key << " " << x << " " << y << " " << z << std::endl;
    if (defaultValue) {
        std::cout << "default " << defaultValue.value() << std::endl;
    }
} else {
        parsedCommand.help();
}
```

