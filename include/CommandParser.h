#pragma once

#include <algorithm>
#include <any>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
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

template <typename T, typename Valid = void>
struct isVector : std::false_type {
};

template <typename T>
struct isVector<std::vector<T>> : std::true_type {
};

template <typename T, typename Valid = void>
struct isVoid : std::false_type {
};

template <typename T>
struct isVoid<T, std::enable_if_t<std::is_same_v<T, void>>> : std::true_type {
};

template <typename T, typename Valid = void>
struct isString : std::false_type {
};

template <typename T>
struct isString<T, std::enable_if_t<std::is_same_v<T, std::string>>> : std::true_type {
};

template <typename T, typename Valid = void>
struct isOptionalString : std::false_type {
};

template <typename T>
struct isOptionalString<T, std::enable_if_t<isOptional<T>::value && isString<typename T::value_type>::value>>
    : std::true_type {
};

template <typename T, typename Valid = void>
struct isVectorOfStrings : std::false_type {
};

template <typename T>
struct isVectorOfStrings<T, std::enable_if_t<std::is_same_v<T, std::vector<std::string>>>> : std::true_type {
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

// constexpr std::all_of because it's not available in C++17
template <class InputIt, class UnaryPredicate>
constexpr bool allOf(InputIt first, InputIt last, UnaryPredicate p)
{
    for (; first != last; ++first) {
        if (!p(*first)) {
            return false;
        }
    }
    return true;
}

template <class InputIt, class UnaryPredicate>
constexpr bool anyOf(InputIt first, InputIt last, UnaryPredicate p)
{
    for (; first != last; ++first) {
        if (p(*first)) {
            return true;
        }
    }
    return false;
}

template <class... Ts>
class ArrayWrapper
{
public:
    explicit constexpr ArrayWrapper(Ts... ts)
        : data { ts... }
    {
    }

    using value_type = std::tuple_element_t<0, std::tuple<Ts...>>;

    [[nodiscard]] constexpr const value_type* begin() const { return static_cast<const value_type*>(data); }

    [[nodiscard]] constexpr const value_type* end() const
    {
        return static_cast<const value_type*>(data) + sizeof...(Ts);
    }

private:
    value_type data[sizeof...(Ts)];
};

/// @brief Check that arguments which are mandatory are always expected before optional ones
template <class... Types>
constexpr bool hasNoPrecedingOptionalArguments()
{
    // TODO(dimitris): Remove ArrayWrapper and replace with constexpr std::array<bool, sizeof...(Types)>
    //  once we can use a newer clang version, since clang-8 does not think std::array is a literal type
    constexpr ArrayWrapper r { !isOptional<Types>::value && !isVector<Types>::value... };
    return isPartitioned(r.begin(), r.end(), [](auto v) { return v; });
}

/// @brief Check that if we have vector arguments they are always the last ones
template <class... Types>
constexpr bool hasNoPrecedingVector()
{
    constexpr ArrayWrapper r { !isVector<Types>::value... };
    return isPartitioned(r.begin(), r.end(), [](auto v) { return v; });
}

template <class... Types>
constexpr bool hasAllowedTypes()
{
    constexpr ArrayWrapper r { isOptionalString<Types>::value || isVoid<Types>::value || isString<Types>::value
                               || isVectorOfStrings<Types>::value... };
    return allOf(r.begin(), r.end(), [](auto v) { return v; });
}

template <class T, class... Types>
constexpr bool containsType()
{
    constexpr ArrayWrapper r { std::is_same_v<T, Types>... };
    return anyOf(r.begin(), r.end(), [](auto v) { return v; });
}

template <class T, class... Types>
constexpr std::size_t countType()
{
    constexpr ArrayWrapper r { std::is_same_v<T, Types>... };
    std::size_t count { 0 };
    for (const auto& v : r) {
        if (v) {
            ++count;
        }
    }
    return count;
}

template <typename... Args>
class UnparsedCommandImpl
{
public:
    static_assert(
        hasNoPrecedingOptionalArguments<Args...>(),
        "All optional arguments must be placed in the end of the "
        "argument list");
    static_assert(
        hasAllowedTypes<Args...>(),
        "All arguments must either be void, std::string, std::optional<std::string>, or std::vector<std::string>");
    static_assert(
        hasNoPrecedingVector<Args...>(),
        "All vector arguments must be placed in the end of the argument list");
    static_assert(countType<std::vector<std::string>, Args...>() <= 1, "At most one vector argument is allowed");
    static_assert(
        !(containsType<std::optional<std::string>, Args...>() && containsType<std::vector<std::string>, Args...>()),
        "Cannot expect both std::optional<std::string> and std::vector<std::string> arguments");

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
    UnparsedCommandImpl(
        std::string id,
        std::string description,
        std::string usage,
        std::unordered_set<std::string> options)
        : id_ { std::move(id) }
        , description_ { std::move(description) }
        , usage_ { std::move(usage) }
        , options_ { std::move(options) }
    {
        for (const auto& option : options_) {
            if (option.size() == 1) {
                shortOptions_.emplace(option);
            }
        }
    }

    /**
     * @brief Get the maximum expected number of arguments for this command
     *        If arguments is just void then return 0 otherwise return the size of arguments
     * @return The number of arguments
     * @warning This function is mostly to be used internally
     */
    constexpr std::size_t getMaxArgCount() const
    {
        if (hasNoArguments<Args...>()) {
            return 0;
        }
        if (containsType<std::vector<std::string>, Args...>()) {
            return std::numeric_limits<std::size_t>::max();
        }
        return sizeof...(Args);
    }

    /**
     * @brief Get the minimum expected number of arguments for this command
     * @return the number of arguments
     * @warning This function is mostly to be used internally
     */
    constexpr std::size_t getRequiredArgCount() const { return countType<std::string, Args...>(); }

    using ArgumentsType = std::tuple<Args...>;

    /**
     * @brief Get the command ID
     * @return The command ID
     */
    std::string id() const { return id_; };

    /**
     * @brief Get the command description
     * @return The command description
     */
    std::string usage() const { return usage_; };

    /**
     * @brief Get the command usage
     * @return The command usage
     */
    std::string description() const { return description_; };

    /**
     * @brief Get all the command options
     * @return The command options
     * @warning This function is to be used mostly internally
     */
    std::unordered_set<std::string> options() const { return options_; };

    /**
     * @brief Get the command short options
     * @return The command short options
     * @warning This function is to be used mostly internally
     */
    std::unordered_set<std::string> shortOptions() const { return shortOptions_; };

    /**
     * @brief Construct a new command with the specified argument types
     * @tparam T The argument types
     * @return A new command with the specified argument types
     */
    template <typename... T>
    UnparsedCommandImpl<T...> withArgs() const
    {
        return UnparsedCommandImpl<T...> { id_, description_, usage_, options_ };
    }

    /**
     * @brief Construct a new command with the specified options
     * @return The command options
     */
    UnparsedCommandImpl<Args...> withOptions(const std::unordered_set<std::string>& options) const
    {
        return UnparsedCommandImpl<Args...> { id_, description_, usage_, options };
    }

private:
    std::string id_ {};
    std::string description_ {};
    std::string usage_ {};
    std::unordered_set<std::string> options_ {};
    std::unordered_set<std::string> shortOptions_ {};

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
} // namespace details

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
        : helpPrompt_(createHelpPrompt(commands))
    {
        if (argc < 2) {
            std::cerr << "No command passed" << std::endl;
            return;
        }
        const std::string commandId { argv[1] };
        std::vector<std::string> unparsedArgs {};
        std::vector<std::string> unparsedOptions {};
        for (int i = 2; i < argc; ++i) {
            if (argv[i][0] == '-') {
                std::string_view option { argv[i] };
                option.remove_prefix(std::min(option.find_first_not_of('-'), option.size())); // Remove all leading '-'
                unparsedOptions.emplace_back(option.data());
            } else {
                unparsedArgs.emplace_back(argv[i]);
            }
        }

        // Find if the commandId exists in the supplied commands
        bool commandFound {};
        bool commandHasCorrectArguments {};
        details::visitTuple(
            commands,
            [&commandId, &commandFound, &unparsedArgs, &commandHasCorrectArguments, &unparsedOptions, this, index = 0U](
                auto&& command) mutable {
                if (command.id() == commandId) {
                    commandFound = true;
                    const auto expectedMaxNumberOfArguments = command.getMaxArgCount();
                    const auto expectedMinNumberOfArguments = command.getRequiredArgCount();
                    if (unparsedArgs.size() < expectedMinNumberOfArguments
                        || unparsedArgs.size() > expectedMaxNumberOfArguments) {
                        std::cerr << "Wrong number of arguments for command: " << commandId << std::endl;
                        std::cerr << command.id() << " " << command.usage() << " " << command.description()
                                  << std::endl;
                        std::cerr << "Expected ";
                        const auto atLeastOrAtMost
                            = unparsedArgs.size() < expectedMinNumberOfArguments ? "at least " : "at most ";
                        std::cerr << atLeastOrAtMost << expectedMaxNumberOfArguments << " arguments, got "
                                  << unparsedArgs.size() << " instead: ";
                        for (const auto& arg : unparsedArgs) {
                            std::cerr << "\"" << arg << "\""
                                      << " ";
                        }
                        std::cerr << std::endl;
                    } else {
                        commandHasCorrectArguments = true;
                        commandIndex_ = index;
                        commandId_ = commandId;
                        // Match options
                        for (const auto& unparsedOption : unparsedOptions) {
                            // Match stand-alone options
                            const auto availableOptions = command.options();
                            const auto search = availableOptions.find(unparsedOption);
                            if (search != availableOptions.end()) {
                                parsedOptions_.emplace(unparsedOption);
                            } else { // Match compound options (e.g. -abc instead of -a -b -c)
                                const auto allCharactersAreShortOptions
                                    = std::all_of(unparsedOption.begin(), unparsedOption.end(), [&command](auto c) {
                                          return command.shortOptions().find(std::string { c })
                                              != command.shortOptions().end();
                                      });
                                if (allCharactersAreShortOptions) {
                                    for (const auto c : unparsedOption) {
                                        parsedOptions_.emplace(std::string { c });
                                    }
                                } else {
                                    unknownOptions_.emplace(unparsedOption);
                                }
                            }
                        }
                    }
                }
                ++index;
            });

        if (!commandFound) {
            std::cerr << "Unrecognized command: " << commandId << std::endl;
            return;
        }

        if (!commandHasCorrectArguments) {
            return;
        }

        // Parse the unparsed arguments
        details::visitTuple(
            parsedArguments_,
            [&unparsedArgs, this, argumentIndex = 0U](auto&& argumentsToParse) mutable {
                if (argumentIndex == commandIndex_.value()) {
                    details::visitTuple(
                        argumentsToParse,
                        [&unparsedArgs, this, parsedArgumentIndex = 0U](auto&& arg) mutable {
                            if (parsedArgumentIndex >= unparsedArgs.size()) {
                                // We end up here when we are out of unparsed arguments. This can happen when:
                                // We expected an optional argument that was not passed by the user in which case we
                                // want to leave the argument in the error state that it is, i.e. nullopt
                                return;
                            }
                            this->parseArgument(arg, unparsedArgs, parsedArgumentIndex);
                            ++parsedArgumentIndex;
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
        assert((is(command)) && "Command not found"); // NOLINT (cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        static_cast<void>(command); // Avoid unused parameter warning in non-debug builds
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
     * @brief Check if the supplied option was encountered for the parsed command
     * @param option
     * @return true if the option was encountered, false otherwise
     */
    bool hasOption(const std::string& option) const { return parsedOptions_.find(option) != parsedOptions_.end(); }

    /**
     * @brief Get any unknown options encountered during parsing
     * @return A set of unknown options
     */
    std::unordered_set<std::string> getUnknownOptions() const { return unknownOptions_; }

    /**
     * @brief Get the help prompt
     * @return The help prompt
     */
    std::string help() const { return helpPrompt_; }

private:
    std::optional<std::size_t> commandIndex_ {};
    using ParsedArgumentsType = decltype(transformUnparsedArgumentsType(T {}));
    ParsedArgumentsType parsedArguments_ {};
    std::string commandId_ {};
    std::string helpPrompt_ {};
    std::unordered_set<std::string> parsedOptions_ {};
    std::unordered_set<std::string> unknownOptions_ {};

    /**
     * @brief Create the help prompt by finding the longest command id and usage so the description is nicely aligned
     * @param commands
     */
    std::string createHelpPrompt(const T& commands) const
    {
        std::size_t longestCommandIdAndUsage { 0 };
        details::visitTuple(commands, [&longestCommandIdAndUsage](auto&& command) {
            constexpr std::size_t separatorSize { 1 };
            const auto commandIdAndUsage = command.id().size() + command.usage().size() + separatorSize;
            if (commandIdAndUsage > longestCommandIdAndUsage) {
                longestCommandIdAndUsage = commandIdAndUsage;
            }
        });

        std::stringstream helpPrompt {};
        details::visitTuple(commands, [&helpPrompt, longestCommandIdAndUsage](auto&& command) {
            helpPrompt << " " << command.id() << " " << command.usage()
                       << std::string(longestCommandIdAndUsage - (command.id().size() + command.usage().size()), ' ')
                       << command.description() << std::endl;
        });

        return helpPrompt.str();
    }

    template <typename ArgumentType>
    void parseArgument(ArgumentType& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = unparsedArgs[index];
    }

    void
    parseArgument(std::vector<std::string>& argToSet, const std::vector<std::string>& unparsedArgs, unsigned int& index)
    {
        while (index < unparsedArgs.size()) {
            argToSet.emplace_back(unparsedArgs[index]);
            ++index;
        }
    }
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
    return details::UnparsedCommandImpl<void> { id, description, usage, {} };
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
