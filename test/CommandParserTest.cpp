#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CommandParser.h"

namespace {
template <std::size_t Size>
std::vector<char*> toArgv(std::array<std::string, Size>& arguments)
{
    std::vector<char*> argv {};
    for (auto& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    argv.push_back(nullptr);

    return argv;
}
}

using std::string_literals::operator""s;

TEST(CommandParserTest, ParsedCommandImpl_WhenNotEnoughArgumentCount_WillNotParse)
{
    auto command = UnparsedCommand::create("dummyCommand"s, "dummyDescription"s);
    int argc = 1;
    std::array arguments { "binary"s, "dummyCommand"s }; // We "lie" about the actual argument count to stress the test
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_FALSE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenTooManyCommandArguments_WillNotParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s);
    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "dummyArgument"s };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_FALSE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenTooFewCommandArguments_WillNotParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::string>();
    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_FALSE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenUnknownCommand_WillNotParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s);
    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, "someUnknownCommand"s };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_FALSE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCorrectNumberOfArguments_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::string>();
    constexpr int argc = 3;
    std::string firstArgument { "firstArgument" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, parsedFirstArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenMultipleArguments_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::string, std::string>();
    constexpr int argc = 4;
    std::string firstArgument { "firstArgument" };
    std::string secondArgument { "secondArgument" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument, secondArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedSecondArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, parsedFirstArgument);
    EXPECT_EQ(secondArgument, parsedSecondArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenMultipleOptionalArguments_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::optional<std::string>, std::optional<std::string>>();
    constexpr int argc = 4;
    std::string firstArgument { "firstArgument" };
    std::string secondArgument { "secondArgument" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument, secondArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedSecondArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, parsedFirstArgument);
    EXPECT_EQ(secondArgument, parsedSecondArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenMandatoryThereButOptionalArgumentsMissing_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::optional<std::string>>();
    constexpr int argc = 3;
    std::string firstArgument { "firstArgument" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedSecondArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, parsedFirstArgument);
    EXPECT_FALSE(parsedSecondArgument.has_value());
}

TEST(CommandParserTest, ParsedCommandImpl_WhenOptionalArgumentsButNoneProvided_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::optional<std::string>, std::optional<std::string>>();
    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedSecondArgument] = parsedCommand.getArgs(command);
    EXPECT_FALSE(parsedFirstArgument.has_value());
    EXPECT_FALSE(parsedSecondArgument.has_value());
}

TEST(CommandParserTest, ParsedCommandImpl_WhenMultipleOptionalArgumentsButSomeProvided_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::optional<std::string>, std::optional<std::string>>();
    constexpr int argc = 4;
    std::string firstArgument { "firstArgument" };
    std::string secondArgument { "secondArgument" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument, secondArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedSecondArgument, parsedThirdArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, parsedFirstArgument);
    EXPECT_EQ(secondArgument, parsedSecondArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenMultipleCommands_WillParseCorrectOne)
{
    auto commandWithOneMandatoryArg
        = UnparsedCommand::create("dummyCommand1"s, "dummyDescription"s, "dummyUsage"s).withArgs<std::string>();
    auto commandWithManyMandatoryArgs = UnparsedCommand::create("dummyCommand2"s, "dummyDescription"s, "dummyUsage"s)
                                            .withArgs<std::string, std::string>();
    auto commandWithOptionalArgs = UnparsedCommand::create("dummyCommand3"s, "dummyDescription"s, "dummyUsage"s)
                                       .withArgs<std::optional<std::string>, std::optional<std::string>>();
    std::string expectedCommand { "dummyCommand4" };
    auto commandWithMandatoryAndOptionalArgs
        = UnparsedCommand::create(expectedCommand, "dummyDescription"s, "dummyUsage"s)
              .withArgs<std::string, std::optional<std::string>>();
    constexpr int argc = 4;
    std::string firstArgument { "firstArgument" };
    std::string secondArgument { "secondArgument" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument, secondArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { commandWithOneMandatoryArg,
                          commandWithManyMandatoryArgs,
                          commandWithOptionalArgs,
                          commandWithMandatoryAndOptionalArgs };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(commandWithMandatoryAndOptionalArgs));
    auto [parsedFirstArgument, parsedSecondArgument] = parsedCommand.getArgs(commandWithMandatoryAndOptionalArgs);
    EXPECT_EQ(firstArgument, parsedFirstArgument);
    EXPECT_EQ(secondArgument, parsedSecondArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenHelpCalled_WillPrintUsageAndDescriptionOfAllCommands)
{
    std::string expectedDescription1 { "( ͡°( ͡° ͜ʖ( ͡° ͜ʖ ͡°)ʖ ͡°) ͡°)" };
    std::string expectedUsage1 { "ಠ╭╮ಠ" };
    auto command1 = UnparsedCommand::create("dummyCommand1"s, expectedDescription1, expectedUsage1);
    std::string expectedDescription2 { "༼ つ ◕_◕ ༽つ" };
    std::string expectedUsage2 { "(ಥ﹏ಥ)" };
    std::string expectedCommand { "dummyCommand2" };
    auto command2 = UnparsedCommand::create(expectedCommand, expectedDescription2, expectedUsage2);

    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand };
    auto argv = toArgv(arguments);
    std::tuple commands { command1, command2 };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command2));

    auto helpPrompt = parsedCommand.help();

    // Ensure all our rather "unique" strings are in the help prompt
    EXPECT_THAT(helpPrompt, testing::HasSubstr(expectedDescription1));
    EXPECT_THAT(helpPrompt, testing::HasSubstr(expectedUsage1));
    EXPECT_THAT(helpPrompt, testing::HasSubstr(expectedUsage2));
    EXPECT_THAT(helpPrompt, testing::HasSubstr(expectedDescription2));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCommandStartingWithHyphens_WillStillGetParsed)
{
    std::string expectedCommand { "--help" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s);
    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenUnknownOptionsSuppliedWithCommand_WillParseCommandAndIgnoreOptions)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::string>();
    constexpr int argc = 4;
    std::string firstArgument { "firstArgument" };
    std::string someOption { "-someOption" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, someOption, firstArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, parsedArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenUnknownOptions_WillReturnUnknownOptions)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s);
    constexpr int argc = 4;
    std::string someOption { "s" };
    std::string someOtherOption { "o" };
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "-" + someOption, "--" + someOtherOption };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto unknownOptions = parsedCommand.getUnknownOptions();
    std::unordered_set expectedUnknownOptions { someOption, someOtherOption };
    EXPECT_EQ(unknownOptions, expectedUnknownOptions);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenKnownOptions_WillReturnOptions)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedOption { "f" };
    std::string secondExpectedOption { "secondOption" };
    std::string thirdExpectedButNotReceivedOption { "someThirdOption" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string>()
                       .withOptions({ firstExpectedOption, secondExpectedOption, thirdExpectedButNotReceivedOption });
    auto otherCommand = UnparsedCommand::create("otherCommand"s, "dummyDescription"s).withOptions({ "secondOption" });
    constexpr int argc = 5;
    std::string firstArgument { "firstArgument" };
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              "-" + firstExpectedOption,
                                              "--" + secondExpectedOption,
                                              firstArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command, otherCommand };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, parsedArgument);
    EXPECT_TRUE(parsedCommand.hasOption(firstExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_FALSE(parsedCommand.hasOption(thirdExpectedButNotReceivedOption));
    EXPECT_EQ(parsedCommand.getUnknownOptions().size(), 0);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCompoundOptions_WillReturnOptions)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedOption { "a" };
    std::string secondExpectedOption { "b" };
    std::string thirdExpectedOption { "c" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::optional<std::string>>()
                       .withOptions({ firstExpectedOption, secondExpectedOption, thirdExpectedOption });
    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              "-" + firstExpectedOption + secondExpectedOption + thirdExpectedOption };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArgument] = parsedCommand.getArgs(command);
    EXPECT_FALSE(parsedArgument);
    EXPECT_TRUE(parsedCommand.hasOption(firstExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(thirdExpectedOption));
    EXPECT_EQ(parsedCommand.getUnknownOptions().size(), 0);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCompoundOptionsInRandomOrder_WillReturnOptions)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedOption { "a" };
    std::string secondExpectedOption { "b" };
    std::string thirdExpectedOption { "c" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::optional<std::string>>()
                       .withOptions({ firstExpectedOption, secondExpectedOption, thirdExpectedOption });
    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              "-" + secondExpectedOption + firstExpectedOption + thirdExpectedOption };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArgument] = parsedCommand.getArgs(command);
    EXPECT_FALSE(parsedArgument);
    EXPECT_TRUE(parsedCommand.hasOption(firstExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(thirdExpectedOption));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCompoundOptionsButOneMissing_WillReturnOptions)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedButNotReceived { "a" };
    std::string secondExpectedOption { "b" };
    std::string thirdExpectedOption { "c" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::optional<std::string>>()
                       .withOptions({ firstExpectedButNotReceived, secondExpectedOption, thirdExpectedOption });
    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              "-" + thirdExpectedOption + secondExpectedOption };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArgument] = parsedCommand.getArgs(command);
    EXPECT_FALSE(parsedArgument);
    EXPECT_FALSE(parsedCommand.hasOption(firstExpectedButNotReceived));
    EXPECT_TRUE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(thirdExpectedOption));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCompoundOptionsWithUnknownElement_WillReturnOptionAsUnknown)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedOption { "a" };
    std::string secondExpectedOption { "b" };
    std::string thirdExpectedOption { "c" };
    std::string unknownOption { "d" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::optional<std::string>>()
                       .withOptions({ firstExpectedOption, secondExpectedOption, thirdExpectedOption });
    constexpr int argc = 3;
    std::string compoundOptionWithUnknownElement = thirdExpectedOption + secondExpectedOption + unknownOption;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "-" + compoundOptionWithUnknownElement };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArgument] = parsedCommand.getArgs(command);
    EXPECT_FALSE(parsedArgument);
    EXPECT_FALSE(parsedCommand.hasOption(firstExpectedOption));
    EXPECT_FALSE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_FALSE(parsedCommand.hasOption(thirdExpectedOption));
    EXPECT_EQ(parsedCommand.getUnknownOptions(), std::unordered_set { compoundOptionWithUnknownElement });
}

TEST(CommandParserTest, ParsedCommandImpl_WhenOptionsBetweenArguments_WillParseAllCorrectly)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedOption { "a" };
    std::string secondExpectedOption { "b" };
    std::string thirdExpectedOption { "c" };
    std::string firstExpectedArgument { "firstArgument" };
    std::string secondExpectedArgument { "secondArgument" };
    std::string thirdExpectedArgument { "thirdArgument" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::string, std::optional<std::string>>()
                       .withOptions({ firstExpectedOption, secondExpectedOption, thirdExpectedOption });
    constexpr int argc = 8;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              "-" + firstExpectedOption,
                                              firstExpectedArgument,
                                              "--" + secondExpectedOption,
                                              secondExpectedArgument,
                                              thirdExpectedArgument,
                                              "-" + thirdExpectedOption };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedSecondArgument, thirdParsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(parsedFirstArgument, firstExpectedArgument);
    EXPECT_EQ(parsedSecondArgument, secondExpectedArgument);
    EXPECT_EQ(thirdParsedArgument, thirdExpectedArgument);
    EXPECT_TRUE(parsedCommand.hasOption(firstExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(thirdExpectedOption));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenOptionPassedForDifferentCommand_WillNotBeParsed)
{
    std::string expectedCommand { "dummyCommand" };
    std::string expectedOption { "a" };
    auto firstCommand = UnparsedCommand::create(expectedCommand, "dummyDescription"s);
    auto secondCommand = UnparsedCommand::create("secondCommand"s, "dummyDescription"s).withOptions({ expectedOption });
    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "-" + expectedOption };
    auto argv = toArgv(arguments);
    std::tuple commands { firstCommand, secondCommand };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(firstCommand));
    EXPECT_FALSE(parsedCommand.hasOption(expectedOption));
    EXPECT_EQ(parsedCommand.getUnknownOptions(), std::unordered_set { expectedOption });
}

TEST(CommandParserTest, ParsedCommandImpl_WhenTypeIsVector_WillAcceptMultipleArguments)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedArgument { "firstArgument" };
    std::string secondExpectedArgument { "secondArgument" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::vector<std::string>>();
    constexpr int argc = 4;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              firstExpectedArgument,
                                              secondExpectedArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArguments] = parsedCommand.getArgs(command);
    std::vector expectedArguments { firstExpectedArgument, secondExpectedArgument };
    EXPECT_EQ(parsedArguments, expectedArguments);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentAndVector_WillParseCorrectly)
{
    std::string expectedCommand { "dummyCommand" };
    std::string mandatoryArgument { "firstArgument" };
    std::string firstVectorArgument { "secondArgument" };
    std::string secondVectorArgument { "thirdArgument" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::vector<std::string>>();
    constexpr int argc = 5;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              mandatoryArgument,
                                              firstVectorArgument,
                                              secondVectorArgument };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedSecondArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(parsedFirstArgument, mandatoryArgument);
    std::vector expectedVectorArguments { firstVectorArgument, secondVectorArgument };
    EXPECT_EQ(parsedSecondArgument, expectedVectorArguments);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentAndVectorExpectedButArgumentFlagProvided_WillParseCorrectly)
{
    std::string expectedCommand { "dummyCommand" };
    std::string mandatoryArgument { "firstArgument" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::vector<std::string>>();
    constexpr int argc = 4;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, mandatoryArgument, "--hi" };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedFirstArgument, parsedVector] = parsedCommand.getArgs(command);
    EXPECT_EQ(parsedFirstArgument, mandatoryArgument);
    EXPECT_TRUE(parsedVector.empty());
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentIsSupportedNumericTypeWillParse)
{
    std::string expectedCommand { "dummyCommand" };
    int expectedInteger { std::numeric_limits<int>::min() };
    long expectedLong { std::numeric_limits<long>::min() };
    unsigned long expectedUnsignedLong { std::numeric_limits<unsigned long>::max() };
    long long expectedLongLong { std::numeric_limits<long long>::max() };
    unsigned long long expectedUnsignedLongLong { std::numeric_limits<unsigned long long>::max() };
    float expectedFloat { -164223.123f }; // std::to_string does not play well with floating point min()
    double expectedDouble { std::numeric_limits<double>::max() };
    long double expectedLongDouble { std::numeric_limits<long double>::max() };

    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<int, long, unsigned long, long long, unsigned long long, float, double, long double>();
    constexpr int argc = 10;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              std::to_string(expectedInteger),
                                              std::to_string(expectedLong),
                                              std::to_string(expectedUnsignedLong),
                                              std::to_string(expectedLongLong),
                                              std::to_string(expectedUnsignedLongLong),
                                              std::to_string(expectedFloat),
                                              std::to_string(expectedDouble),
                                              std::to_string(expectedLongDouble) };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto
        [parsedInteger,
         parsedLong,
         parsedUnsignedLong,
         parsedLongLong,
         parsedUnsignedLongLong,
         parsedFloat,
         parsedDouble,
         parsedLongDouble]
        = parsedCommand.getArgs(command);
    EXPECT_EQ(parsedInteger, expectedInteger);
    EXPECT_EQ(parsedLong, expectedLong);
    EXPECT_EQ(parsedUnsignedLong, expectedUnsignedLong);
    EXPECT_EQ(parsedLongLong, expectedLongLong);
    EXPECT_EQ(parsedUnsignedLongLong, expectedUnsignedLongLong);
    EXPECT_EQ(parsedFloat, expectedFloat);
    EXPECT_EQ(parsedDouble, expectedDouble);
    EXPECT_EQ(parsedLongDouble, expectedLongDouble);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentIsBoolean_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    bool expectedTrue { true };
    bool expectedFalse { false };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<bool, bool>();
    constexpr int argc = 4;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "true", "false" };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstArgument, secondArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, expectedTrue);
    EXPECT_EQ(secondArgument, expectedFalse);
}