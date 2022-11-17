#pragma once

#include <algorithm>
#include <any>
#include <cassert>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace details {
// Adapted from
// https://github.com/heavyeyelid/virthttp/blob/master/include/virt_wrap/utility.hpp#L42-L46
template <typename T, typename V, size_t... I>
constexpr void tupleVisitImpl(T&& t, V&& v, std::index_sequence<I...>)
{
    (..., v(std::get<I>(t)));
}

template <typename T, typename V>
constexpr void visitTuple(T&& t, V&& v)
{
    tupleVisitImpl(
        std::forward<T>(t),
        std::forward<V>(v),
        std::make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>());
}

template <typename T, typename Valid = void>
struct isOptional : std::false_type {
};

template <typename T>
struct isOptional<std::optional<T>> : std::true_type {
};

// Adapted from cppreference:
// https://en.cppreference.com/w/cpp/algorithm/is_partitioned
template <class InputIt, class UnaryPredicate>
constexpr bool isPartitioned(InputIt first, InputIt last, UnaryPredicate p)
{
    for (; first != last; ++first) {
        if (!p(*first)) {
            break;
        }
    }
    for (; first != last; ++first) {
        if (p(*first)) {
            return false;
        }
    }
    return true;
}

template <class... Types>
constexpr bool hasNoPrecedingOptional()
{
    constexpr std::array<bool, sizeof...(Types)> r { !isOptional<Types>::value... };
    return isPartitioned(r.begin(), r.end(), [](auto v) { return v; });
}

template <typename... Args>
class UnparsedCommandImpl
{
public:
    static_assert(
        hasNoPrecedingOptional<Args...>(),
        "All optional arguments must be placed in the end of the "
        "argument list");

    /**
     * @brief Default constructor to be used internally, users should use the normal constructor
     * @warning This constructor is only to be used internally
     */
    UnparsedCommandImpl() = default;

    /**
     * @brief Construct an unparsed command
     * @param id The command "ID" (e.g. "help")
     * @param description
     * @param usage How to use the command (e.g. "help [command]")
     * @warning Users should probably use the helper function `UnparsedCommand::create`
     */
    UnparsedCommandImpl(std::string id, std::string description, std::string usage)
        : id_ { std::move(id) }
        , description_ { std::move(description) }
        , usage_ { std::move(usage) }
    {
    }

    /**
     * @brief Get the maximum expected number of arguments for this command
     *        If arguments is just void then return 0 otherwise return the size of arguments
     * @return The number of arguments
     * @warning This function is only to be used mostly internally
     */
    constexpr std::size_t getMaxArgCount() const { return hasNoArguments<Args...>() ? 0 : sizeof...(Args); }

    /// Specialize getOptionalArgCount for when a command has no arguments
    template <typename T = std::tuple<Args...>>
    constexpr std::enable_if_t<std::is_same_v<T, std::tuple<void>>, std::size_t> getOptionalArgCount() const
    {
        return 0;
    }

    /**
     * @brief Count the number of std::optional occurrences in the supplied arguments
     * @return The number of optional arguments
     * @warning This function is only to be used mostly internally
     */
    template <typename T = std::tuple<Args...>>
    constexpr std::enable_if_t<!std::is_same_v<T, std::tuple<void>>, std::size_t> getOptionalArgCount() const
    {
        // Count the number of std::optional inside Args
        std::size_t optionalArgCount = 0;
        details::visitTuple(ArgumentsType {}, [&optionalArgCount](auto&& arg) {
            if constexpr (isOptional<std::decay_t<decltype(arg)>>::value) {
                ++optionalArgCount;
            }
        });

        return optionalArgCount;
    }

    using ArgumentsType = std::tuple<Args...>;

    /**
     * @brief Get an explanation with how to use the command
     * @return A string with the usage
     */
    std::string help() const { return "::Usage::  " + id_ + " " + usage_ + " ::Description:: " + description_; };

    /**
     * @brief Get the command ID
     * @return The command ID
     */
    std::string id() const { return id_; };

    /**
     * @brief Construct a new command with the specified argument types
     * @tparam T The argument types
     * @return A new command with the specified argument types
     */
    template <typename... T>
    UnparsedCommandImpl<T...> withArgs() const
    {
        return UnparsedCommandImpl<T...> { id_, description_, usage_ };
    }

private:
    std::string id_ {};
    std::string description_ {};
    std::string usage_ {};

    template <typename... T>
    static constexpr bool hasNoArguments()
    {
        return std::is_same_v<std::tuple<T...>, std::tuple<void>>;
    }
};

/**
 * @brief Given a tuple of UnparsedCommandImpl create a tuple of
 * UnparsedCommandImpl::ParsedArgumentsType but use std::any instead of void to
 * make its usage easier
 * @tparam T
 * @return A tuple of tuples with the argument types
 */
template <typename... T>
constexpr auto transformUnparsedArgumentsType(std::tuple<T...>)
{
    return std::tuple<std::conditional_t<
        std::is_same_v<typename T::ArgumentsType, std::tuple<void>>,
        std::tuple<std::any>,
        typename T::ArgumentsType>...> {};
}
}

template <typename T>
class ParsedCommandImpl
{
public:
    /**
     * @brief Constructs a parsed command
     * @param unparsedCommand A tuple with the unparsed commands
     * @param argc The argument count
     * @param argv The argument values
     * @warning Users should probably use the helper function `UnparsedCommand::parse`
     */
    ParsedCommandImpl(int argc, char* argv[], const T& commands)
    {
        details::visitTuple(commands, [this](auto&& command) {
            helpPrompt_ << " " << command.id() << " - " << command.help() << std::endl;
        });

        if (argc < 2) {
            std::cerr << "No command passed" << std::endl;
            return;
        }
        const std::string commandId { argv[1] };
        std::vector<std::string> unparsedArgs {};
        for (int i = 2; i < argc; ++i) {
            unparsedArgs.emplace_back(argv[i]);
        }

        // Find if the commandId exists in the supplied commands
        bool commandFound {};
        bool commandHasCorrectArguments {};
        details::visitTuple(
            commands,
            [&commandId, &commandFound, &unparsedArgs, &commandHasCorrectArguments, this, index = 0U](
                auto&& command) mutable {
                if (command.id() == commandId) {
                    commandFound = true;
                    const auto expectedMaxNumberOfArguments = command.getMaxArgCount();
                    const auto expectedMinNumberOfArguments
                        = expectedMaxNumberOfArguments - command.getOptionalArgCount();
                    if (unparsedArgs.size() < expectedMinNumberOfArguments
                        || unparsedArgs.size() > expectedMaxNumberOfArguments) {
                        std::cerr << "Wrong number of arguments for command: " << commandId << std::endl;
                        std::cerr << command.help() << std::endl;
                        std::cerr << "Expected ";
                        const auto atLeastOrAtMost
                            = unparsedArgs.size() < expectedMinNumberOfArguments ? "at least " : "at most ";
                        std::cerr << atLeastOrAtMost << expectedMaxNumberOfArguments << " arguments, got "
                                  << unparsedArgs.size() << " instead: ";
                        for (const auto& arg : unparsedArgs) {
                            std::cerr << arg << " ";
                        }
                        std::cerr << std::endl;
                    } else {
                        commandHasCorrectArguments = true;
                        commandIndex_ = index;
                        commandId_ = commandId;
                    }
                }
                ++index;
            });

        if (!commandHasCorrectArguments) {
            return;
        }

        if (!commandFound) {
            std::cerr << "Unrecognized command: " << commandId << std::endl;
            return;
        }

        // Parse the unparsed arguments
        details::visitTuple(
            parsedArguments_,
            [&unparsedArgs, this, argumentIndex = 0U](auto&& argumentsToParse) mutable {
                if (argumentIndex == commandIndex_.value()) {
                    details::visitTuple(argumentsToParse, [&unparsedArgs, argumentIndex = 0U](auto&& arg) mutable {
                        if (argumentIndex >= unparsedArgs.size()) {
                            // We end up here when we have an optional argument that was not passed by the user In this
                            // case we want to leave the argument in the error state that it is, i.e. nullopt
                            return;
                        }
                        arg = unparsedArgs[argumentIndex];
                        ++argumentIndex;
                    });
                }
                ++argumentIndex;
            });

        commandId_ = commandId;
    }

    /**
     * @brief Check if the parsed command is the supplied (unparsed) command
     * @tparam CommandType The unparsed command
     * @param command
     * @return Whether the parsed command matches the unparsed command
     */
    template <typename CommandType>
    bool is(const CommandType& command) const
    {
        return command.id() == commandId_;
    }

    /**
     * @brief Get the parsed arguments for the supplied command
     * @tparam CommandType The unparsed command
     * @param command
     * @return  A command-specific tuple with the parsed arguments
     */
    template <typename CommandType>
    typename CommandType::ArgumentsType getArgs(const CommandType& command) const
    {
        assert((is(command)) && "Command not found");
        typename CommandType::ArgumentsType argsToReturn {};
        details::visitTuple(parsedArguments_, [&argsToReturn, this, index = 0U](auto&& arg) mutable {
            if (index == commandIndex_.value()) {
                argsToReturn = std::any_cast<typename CommandType::ArgumentsType>(arg);
            }
            ++index;
        });

        return argsToReturn;
    }

    /**
     * @brief Get the help prompt
     */
    void help() const
    {
        std::cerr << "Available commands:" << std::endl;
        std::cerr << helpPrompt_.str() << std::endl;
    }

private:
    std::optional<std::size_t> commandIndex_ {};
    using ParsedArgumentsType = decltype(transformUnparsedArgumentsType(T {}));
    ParsedArgumentsType parsedArguments_ {};
    std::string commandId_ {};
    std::stringstream helpPrompt_ {};
};

namespace UnparsedCommand {
/**
 * @brief Helper function to create an unparsed command
 * @param id The command ID (e.g. "add")
 * @param description The command description (e.g. "Add a new item")
 * @param usage The command usage (e.g. "add <item>")
 * @return An unparsed command
 */
inline details::UnparsedCommandImpl<void>
create(const std::string& id, const std::string& description, const std::string& usage = "")
{
    return details::UnparsedCommandImpl<void> { id, description, usage };
}

/**
 * @brief Helper function to parse a command given the supplied CLI arguments and a list of available commands
 * @tparam T A tuple with the available unparsed commands
 * @param argc The number of CLI arguments
 * @param argv The CLI arguments
 * @param unparsedCommands A tuple with the available unparsed commands
 * @return A parsed command
 */
template <typename T>
ParsedCommandImpl<T> parse(int argc, char* argv[], const T& unparsedCommands)
{
    return ParsedCommandImpl<T> { argc, argv, unparsedCommands };
}
} // namespace UnparsedCommand