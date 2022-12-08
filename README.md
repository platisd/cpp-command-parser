# Command Parser

[![codecov](https://codecov.io/gh/platisd/cpp-command-parser/branch/main/graph/badge.svg?token=MNGCSVLIUM)](https://codecov.io/gh/platisd/cpp-command-parser) [![Build, unit tests and coverage CI](https://github.com/platisd/cpp-command-parser/actions/workflows/build-ut-coverage.yml/badge.svg)](https://github.com/platisd/cpp-command-parser/actions/workflows/build-ut-coverage.yml) [![clang-format CI](https://github.com/platisd/cpp-command-parser/actions/workflows/clang-format.yml/badge.svg)](https://github.com/platisd/cpp-command-parser/actions/workflows/clang-format.yml) [![clang-tidy CI](https://github.com/platisd/cpp-command-parser/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/platisd/cpp-command-parser/actions/workflows/clang-tidy.yml) [![Commit messages](https://github.com/platisd/cpp-command-parser/actions/workflows/commit-messages.yml/badge.svg)](https://github.com/platisd/cpp-command-parser/actions/workflows/commit-messages.yml)

Command Parser is a C++17 header-only utility library for parsing command line "commands".

We define a _command_ as the _first_ CLI argument passed to a binary in the form of:

```bash
./your_binary command_name <arg1> <arg2> [arg3]
# `command_name` is the command
```

A command may optionally take some (boolean) options

```bash
./your_binary command_name --option1 --option2 <arg1> <arg2> [arg3]
```

As long as your CLI application requires the _first_ argument to be a "command", this library will help you parse the
command, as well as any subcommands and options.

## Usage

See [example_main.cpp](example_main.cpp) for a simple example of how to use the library. In a nutshell:

```cpp
const auto all = UnparsedCommand::create("all", "Print current configuration");
const auto get = UnparsedCommand::create("get", "Get configuration key", "[-xyz] <key> [default]")
                         .withOptions({ "x", "y", "z" }) // Can be --x or -x etc
                         .withAliases({ "g", "get-key" }); // Alternative IDs for the command instead of "get"
                         .withArgs<std::string, std::optional<std::string>>();
const auto encrypt = UnparsedCommand::create(
                         "encrypt"
                         "Encrypt the given files with the specified policy",
                         "<policy> [file...]")
                         .withArgs<std::string, std::vector<std::string>>();
const std::tuple commands { all, get, encrypt };
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
} else if (parsedCommand.is(encrypt)) {
    const auto [policy, files] = parsedCommand.getArgs(encrypt);
    std::cout << "encrypt " << policy << std::endl;
    for (const auto& file : files) {
        std::cout << "file " << file << std::endl;
    }
} else {
        std::cout << "Available commands:" << std::endl;
        std::cout << parsedCommand.help() << std::endl;
}
```

### Allowed types

The following types are permitted as arguments. They are mandatory unless otherwise specified and their usage rules are
enforced during compilation:

* `std::string`
* `bool`
* `int`
* `long`
* `long long`
* `unsigned long`
* `unsigned long long`
* `float`
* `double`
* `long double`
* `std::optional<T>` where `T` is any of the above types
    * Optional argument: May not be provided but must be at the end of the argument list
* `std::vector<T>` where `T` is any of the above types
    * Zero or more optional arguments: May be of any number or not provided, but cannot precede a mandatory argument and
      cannot be combined with a `std::optional` argument

In addition to the above, any user-defined type that is default constructible and constructible from a `std::string` is
also allowed.

## Why not `insert your favorite CLI parsing library here`?

In all fairness, this library was created under the misconception that [cxxopts](https://github.com/jarro2783/cxxopts)
was not able to satisfy our use-case out of the box, which is **not** true. Some of its advantages are:

* Compile-time check of argument count and types (i.e. you cannot "forget" to parse an argument or parse one that was
  not expected)
* Header-only
* No dependencies
* No macros
* No exceptions

## Acknowledgements

This library was developed internally at [neat.no](https://neat.no) and is now open-sourced.
