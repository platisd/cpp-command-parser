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
struct isAllowedCustomType : std::false_type {
};

template <typename T>
struct isAllowedCustomType<
    T,
    std::enable_if_t<std::is_default_constructible<T>::value && std::is_constructible<T, std::string>::value>>
    : std::true_type {
};

template <typename T>
struct isAllowedType : isAllowedCustomType<T> {
};

template <>
struct isAllowedType<std::string> : std::true_type {
};

template <>
struct isAllowedType<void> : std::true_type {
};

template <>
struct isAllowedType<int> : std::true_type {
};

template <>
struct isAllowedType<long> : std::true_type {
};

template <>
struct isAllowedType<long long> : std::true_type {
};

template <>
struct isAllowedType<unsigned long> : std::true_type {
};

template <>
struct isAllowedType<unsigned long long> : std::true_type {
};

template <>
struct isAllowedType<float> : std::true_type {
};

template <>
struct isAllowedType<double> : std::true_type {
};

template <>
struct isAllowedType<long double> : std::true_type {
};

template <>
struct isAllowedType<bool> : std::true_type {
};

template <typename T>
struct isAllowedType<std::vector<T>> : isAllowedType<T> {
};

template <typename T>
struct isAllowedType<std::optional<T>> : isAllowedType<T> {
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

// constexpr std::count since it's not available in C++17
// Adapted from: https://en.cppreference.com/w/cpp/algorithm/count
template <class InputIt, class T>
constexpr auto count(InputIt first, InputIt last, const T& value)
{
    typename std::iterator_traits<InputIt>::difference_type n = 0;
    for (; first != last; ++first) {
        if (*first == value) {
            ++n;
        }
    }
    return n;
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
        : data_ { ts... }
    {
    }

    using value_type = std::tuple_element_t<0, std::tuple<Ts...>>;

    [[nodiscard]] constexpr const value_type* begin() const { return static_cast<const value_type*>(data_); }

    [[nodiscard]] constexpr const value_type* end() const
    {
        return static_cast<const value_type*>(data_) + sizeof...(Ts);
    }

private:
    value_type data_[sizeof...(Ts)];
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
    constexpr ArrayWrapper r { isAllowedType<Types>::value... };
    return allOf(r.begin(), r.end(), [](auto v) { return v; });
}

template <class... Types>
constexpr bool doesNotContainBothOptionalAndVector()
{
    constexpr ArrayWrapper optionals { isOptional<Types>::value... };
    constexpr auto optionalArguments = count(optionals.begin(), optionals.end(), true);
    constexpr ArrayWrapper vectors { isVector<Types>::value... };
    constexpr auto vectorArguments = count(vectors.begin(), vectors.end(), true);

    return optionalArguments == 0 || vectorArguments == 0;
}

template <class... Types>
constexpr bool containsAtMostOneVector()
{
    constexpr ArrayWrapper vectors { isVector<Types>::value... };

    return count(vectors.begin(), vectors.end(), true) <= 1;
}

constexpr void removeAllLeadingDashes(std::string_view& view)
{
    view.remove_prefix(std::min(view.find_first_not_of('-'), view.size()));
}

constexpr bool isAnOption(std::string_view argument)
{
    // Too small to be an option
    if (argument.size() < 2) {
        return false;
    }
    // It's a negative number
    if (argument[0] == '-' && std::isdigit(argument[1], std::locale::classic())) {
        return false;
    }
    // Does not start with one or two dashes
    const auto startsWithSingleDash = argument[0] == '-';
    const auto startsWithDoubleDash = argument[0] == '-' && argument[1] == '-';
    if (!startsWithSingleDash && !startsWithDoubleDash) {
        return false;
    }
    // Contains spaces (we will eventually have to rethink this when doing #7)
    if (argument.find(' ') != std::string_view::npos) {
        return false;
    }
    // If we start with two dashes, then there should be at least one more character
    if (startsWithDoubleDash && argument.size() < 3) {
        return false;
    }
    // If we start with two dashes, then the following character should be alphanumeric
    if (startsWithDoubleDash && !std::isalnum(argument[2], std::locale::classic())) {
        return false;
    }

    return true;
}

template <typename T, typename Tuple>
struct typeInTuple;

template <typename T, typename... Types>
struct typeInTuple<T, std::tuple<Types...>> : std::disjunction<std::is_same<T, Types>...> {
};

template <typename... Args>
class UnparsedCommandImpl
{
public:
    static_assert(
        hasNoPrecedingOptionalArguments<Args...>(),
        "All optional arguments must be placed in the end of the argument list");
    static_assert(
        hasAllowedTypes<Args...>(),
        "All arguments must be one of the following: "
        "bool, int, long, long long, unsigned long, unsigned long long, float, double, long double, std::string "
        "or any type that is "
        "default constructible and constructible from a std::string"
        "along with their std::optional and std::vector variants");
    static_assert(
        hasNoPrecedingVector<Args...>(),
        "All vector arguments must be placed in the end of the argument list");
    static_assert(containsAtMostOneVector<Args...>(), "At most one vector argument is allowed");
    static_assert(
        doesNotContainBothOptionalAndVector<Args...>(),
        "Cannot expect both std::optional and std::vector arguments");

    /**
     * @brief Default constructor to be used internally, users should use the normal constructor
     * @warning This constructor is only to be used internally
     */
    UnparsedCommandImpl() = default;

    /**
     * @brief Construct an unparsed command
     * @param id The command "ID" (e.g. "help")
     * @param aliases The command aliases (e.g. "h")
     * @param description
     * @param usage How to use the command (e.g. "help [command]")
     * @warning Users should probably use the helper function `UnparsedCommand::create`
     */
    UnparsedCommandImpl(
        std::string id,
        std::unordered_set<std::string> aliases,
        std::string description,
        std::string usage,
        std::unordered_set<std::string> options)
        : id_ { std::move(id) }
        , aliases_ { std::move(aliases) }
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
        constexpr ArrayWrapper r { isVector<Args>::value... };
        if (anyOf(r.begin(), r.end(), [](auto v) { return v; })) {
            return std::numeric_limits<std::size_t>::max();
        }
        return sizeof...(Args);
    }

    /**
     * @brief Get the minimum expected number of arguments for this command
     * @return the number of arguments
     * @warning This function is mostly to be used internally
     */
    constexpr std::size_t getRequiredArgCount() const
    {
        constexpr ArrayWrapper r { !isOptional<Args>::value && !isVector<Args>::value
                                   && !std::is_same_v<void, Args>... };
        return count(r.begin(), r.end(), true);
    }

    using ArgumentsType = std::tuple<Args...>;

    /**
     * @brief Get the command ID
     * @return The command ID
     */
    std::string id() const { return id_; };

    /**
     * @brief Check if the supplied command ID matches the ID or the aliases of this command
     * @param command
     * @return true if the command ID matches the ID or the aliases, false otherwise
     */
    bool matches(const std::string& command) const { return command == id_ || aliases_.count(command) > 0; }

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
     * @return A new command with the specified argument types added to any existing ones
     */
    template <typename... T>
    auto withArgs() const
    {
        // We need to filter out void, mostly from the existing arguments, or it will cause issues when instantiating it
        using TupleOfExistingArgsWithoutVoid = decltype(std::tuple_cat(
            std::conditional_t<std::is_same_v<void, Args>, std::tuple<>, std::tuple<Args>> {}...));
        using TupleOfNewArgsWithoutVoid
            = decltype(std::tuple_cat(std::conditional_t<std::is_same_v<void, T>, std::tuple<>, std::tuple<T>> {}...));
        using InstanceWithConcatinatedArgs
            = decltype(createFromTuples(TupleOfExistingArgsWithoutVoid {}, TupleOfNewArgsWithoutVoid {}));

        return InstanceWithConcatinatedArgs { id_, aliases_, description_, usage_, options_ };
    }

    /**
     * @brief Construct a new command with the specified options
     * @return A new command with the additional options
     */
    UnparsedCommandImpl<Args...> withOptions(std::unordered_set<std::string> options) const
    {
        // Be forgiving to the user misunderstanding the API usage and strip any leading dashes
        std::unordered_set<std::string> sanitizedOptions(options.size());
        for (const auto& option : options) {
            auto view = std::string_view { option };
            removeAllLeadingDashes(view);
            sanitizedOptions.emplace(view);
        }
        sanitizedOptions.insert(options_.begin(), options_.end());
        return UnparsedCommandImpl<Args...> { id_, aliases_, description_, usage_, std::move(sanitizedOptions) };
    }

    /**
     * @brief Construct a new command with the specified ID aliases
     * @return A new command with the additional IDs
     */
    UnparsedCommandImpl<Args...> withAliases(std::unordered_set<std::string> aliases) const
    {
        aliases.insert(aliases_.begin(), aliases_.end());
        return UnparsedCommandImpl<Args...> { id_, std::move(aliases), description_, usage_, options_ };
    }

private:
    std::string id_ {};
    std::unordered_set<std::string> aliases_ {};
    std::string description_ {};
    std::string usage_ {};
    std::unordered_set<std::string> options_ {};
    std::unordered_set<std::string> shortOptions_ {};

    template <typename... T>
    static constexpr bool hasNoArguments()
    {
        return std::is_same_v<std::tuple<T...>, std::tuple<void>>;
    }

    template <typename... ExistingTypes, typename... NewTypes>
    static auto createFromTuples(std::tuple<ExistingTypes...>, std::tuple<NewTypes...>)
    {
        return UnparsedCommandImpl<ExistingTypes..., NewTypes...> {};
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
     * @param argc The argument count
     * @param argv The argument values
     * @param commands A tuple with the unparsed commands
     * @warning Users should probably use the helper function `UnparsedCommand::parse`
     */
    ParsedCommandImpl(int argc, char* argv[], const T& commands)
        : helpPrompt_ { createHelpPrompt(commands) }
    {
        if (argc < 2) {
            std::cerr << "No command passed" << std::endl;
            return;
        }
        const std::string commandId { argv[1] };
        std::vector<std::string> unparsedArgs {};
        std::vector<std::string> unparsedOptions {};
        for (int i = 2; i < argc; ++i) {
            std::string_view argument { argv[i] };
            if (details::isAnOption(argument)) {
                details::removeAllLeadingDashes(argument);
                unparsedOptions.emplace_back(argument.data());
            } else {
                unparsedArgs.emplace_back(argument.data());
            }
        }

        // Find if the commandId exists in the supplied commands
        bool commandFound {};
        bool commandHasCorrectArguments {};
        details::visitTuple(
            commands,
            [&commandId, &commandFound, &unparsedArgs, &commandHasCorrectArguments, &unparsedOptions, this, index = 0U](
                auto&& command) mutable {
                if (command.matches(commandId)) {
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
                        commandId_ = command.id();
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
    }

    /**
     * @brief Check if the parsed command is the supplied (unparsed) command
     * @tparam CommandType The unparsed command
     * @param command
     * @return Whether the parsed command matches the unparsed command
     */
    template <typename CommandType>
    [[nodiscard]] bool is(const CommandType& command) const
    {
        static_assert(
            details::typeInTuple<CommandType, T>::value,
            "The specified command was not included in the tuple of commands passed when calling "
            "UnparsedCommand::parse");
        return command.id() == commandId_;
    }

    /**
     * @brief Get the parsed arguments for the supplied command
     * @tparam CommandType The unparsed command
     * @param command
     * @return  A command-specific tuple with the parsed arguments
     */
    template <typename CommandType>
    [[nodiscard]] auto getArgs(const CommandType& command) const
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
    [[nodiscard]] bool hasOption(const std::string& option) const
    {
        // Let's be forgiving if someone looks for "--option" instead of "option"
        std::string_view view { option };
        details::removeAllLeadingDashes(view);
        return parsedOptions_.find(std::string { view }) != parsedOptions_.end();
    }

    /**
     * @brief Get any unknown options encountered during parsing
     * @return A set of unknown options
     */
    [[nodiscard]] std::unordered_set<std::string> getUnknownOptions() const { return unknownOptions_; }

    /**
     * @brief Get the help prompt
     * @return The help prompt
     */
    [[nodiscard]] std::string help() const { return helpPrompt_; }

private:
    std::optional<std::size_t> commandIndex_ {};
    using ParsedArgumentsType = decltype(details::transformUnparsedArgumentsType(T {}));
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

    void parseArgument(int& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stoi(unparsedArgs[index]);
    }

    void parseArgument(long& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stol(unparsedArgs[index]);
    }

    void parseArgument(long long& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stoll(unparsedArgs[index]);
    }

    void parseArgument(unsigned long& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stoul(unparsedArgs[index]);
    }

    void
    parseArgument(unsigned long long& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stoull(unparsedArgs[index]);
    }

    void parseArgument(float& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stof(unparsedArgs[index]);
    }

    void parseArgument(double& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stod(unparsedArgs[index]);
    }

    void parseArgument(long double& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        argToSet = std::stold(unparsedArgs[index]);
    }

    void parseArgument(bool& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        for (const auto& trueValue : { "true", "yes", "1", "on" }) {
            const auto argumentIsTrueIgnoringCase
                = std::equal(unparsedArgs[index].begin(), unparsedArgs[index].end(), trueValue, [](auto a, auto b) {
                      return std::tolower(a) == std::tolower(b);
                  });
            if (argumentIsTrueIgnoringCase) {
                argToSet = true;
                return;
            }
        }
    }

    template <typename E>
    void parseArgument(std::vector<E>& argToSet, const std::vector<std::string>& unparsedArgs, unsigned int& index)
    {
        while (index < unparsedArgs.size()) {
            E element {};
            parseArgument(element, unparsedArgs, index);
            argToSet.emplace_back(element);
            ++index;
        }
    }
    template <typename E>
    void
    parseArgument(std::optional<E>& argToSet, const std::vector<std::string>& unparsedArgs, const unsigned int& index)
    {
        E element {};
        parseArgument(element, unparsedArgs, index);
        argToSet = element;
    }
};

namespace UnparsedCommand {
/**
 * @brief Helper function to create an unparsed command
 * @param id The command ID (e.g. "add"), i.e. the first argument passed to the binary
 * @param description The command description (e.g. "Add a new item")
 * @param usage The command usage (e.g. "add <item>") including arguments and options
 * @return An unparsed command
 */
[[nodiscard]] inline details::UnparsedCommandImpl<void>
create(const std::string& id, const std::string& description, const std::string& usage = "")
{
    return details::UnparsedCommandImpl<void> { id, {}, description, usage, {} };
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
[[nodiscard]] ParsedCommandImpl<T> parse(int argc, char* argv[], const T& unparsedCommands)
{
    return ParsedCommandImpl<T> { argc, argv, unparsedCommands };
}
} // namespace UnparsedCommand
