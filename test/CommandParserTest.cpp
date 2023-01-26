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

TEST(CommandParserTest, ParsedCommandImpl_WhenOptionsSuppliedInMultipleSteps_WillParseAllOptions)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstExpectedOption { "f" };
    std::string secondExpectedOption { "secondOption" };
    std::string thirdExpectedOption { "someThirdOption" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withOptions({ firstExpectedOption, secondExpectedOption })
                       .withOptions({ thirdExpectedOption });
    constexpr int argc = 5;
    std::string firstArgument { "firstArgument" };
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              "-" + firstExpectedOption,
                                              "--" + secondExpectedOption,
                                              "-" + thirdExpectedOption };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    EXPECT_TRUE(parsedCommand.hasOption(firstExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_TRUE(parsedCommand.hasOption(thirdExpectedOption));
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
    long double expectedLongDouble { 123455678912349.1245678912349L };

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
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<bool, bool, bool, bool, bool, bool, bool, bool, bool, bool, bool, bool>();
    constexpr int argc = 14;
    std::array<std::string, argc> arguments {
        "binary"s, expectedCommand, //
        "true", // 1st
        "false", // 2nd
        "True", // 3rd
        "False", // 4th
        "TRUE", // 5th
        "1", // 6th
        "on", // 7th
        "off", // 8th
        "0", // 9th
        "TrUe", // 10th
        "yEs", // 11th
        "y" // 12th
    };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [first, second, third, fourth, fifth, sixth, seventh, eighth, ninth, tenth, eleventh, twelfth]
        = parsedCommand.getArgs(command);
    EXPECT_TRUE(first);
    EXPECT_FALSE(second);
    EXPECT_TRUE(third);
    EXPECT_FALSE(fourth);
    EXPECT_TRUE(fifth);
    EXPECT_TRUE(sixth);
    EXPECT_TRUE(seventh);
    EXPECT_FALSE(eighth);
    EXPECT_FALSE(ninth);
    EXPECT_TRUE(tenth);
    EXPECT_TRUE(eleventh);
    EXPECT_TRUE(twelfth);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenVectorOfIntegers_WillParse)
{
    std::string expectedCommand { "dummy-Command" };
    std::vector<int> expectedVector { 1, -2, 3, 4, 5 };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::vector<int>>();
    constexpr int argc = 7;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              std::to_string(expectedVector[0]),
                                              std::to_string(expectedVector[1]),
                                              std::to_string(expectedVector[2]),
                                              std::to_string(expectedVector[3]),
                                              std::to_string(expectedVector[4]) };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedVector] = parsedCommand.getArgs(command);
    EXPECT_EQ(parsedVector, expectedVector);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenVectorOfBooleans_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    std::vector<bool> expectedVector { true, false, true, false, true };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::vector<bool>>();
    constexpr int argc = 7;
    auto boolToString = [](bool value) { return value ? "true" : "false"; };
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              boolToString(expectedVector[0]),
                                              boolToString(expectedVector[1]),
                                              boolToString(expectedVector[2]),
                                              boolToString(expectedVector[3]),
                                              boolToString(expectedVector[4]) };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedVector] = parsedCommand.getArgs(command);
    EXPECT_EQ(parsedVector, expectedVector);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenOptionalInteger_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    int expectedInteger { -123 };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::optional<int>, std::optional<float>>();
    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, std::to_string(expectedInteger) };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstArgument, secondArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, expectedInteger);
    EXPECT_FALSE(secondArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenOptionalBooleanNotProvided_WillParse)
{
    // Let's make a separate test for std::optional<bool> since it can be misleading as it is `false` both when not
    // provided but also when provided as false
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::optional<bool>>();
    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstArgument] = parsedCommand.getArgs(command);
    EXPECT_FALSE(firstArgument.has_value()); // Explicitly calling has_value() to ensure it is not provided
}

TEST(CommandParserTest, ParsedCommandImpl_WhenNotEnoughNumericalArguments_WillNotParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<int, std::string, int>();
    constexpr int argc = 4;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "-1", "secondArgument" };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_FALSE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenTooManyNumericalArguments_WillNotParse)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<float, std::string, int>();
    constexpr int argc = 6;
    std::array<std::string, argc> arguments {
        "binary"s, expectedCommand, "-1.2", "secondArgument", "3", "extraArgument"
    };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_FALSE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenNumericalArgumentsAndOptions_WillParseCorrectly)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstOption { "firstFlag" };
    std::string secondOption { "secondFlag" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<double, std::string>()
                       .withOptions({ firstOption, secondOption });
    double firstDoubleArgument { -1.212156 };
    std::string secondStringArgument { "secondArgument" };
    constexpr int argc = 7;
    std::array<std::string, argc> arguments { "binary"s,           expectedCommand,
                                              "-" + firstOption,   std::to_string(firstDoubleArgument),
                                              "--" + secondOption, secondStringArgument,
                                              "-unknownOption" };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstArgument, secondArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, firstDoubleArgument);
    EXPECT_EQ(secondArgument, secondStringArgument);
    EXPECT_TRUE(parsedCommand.hasOption(firstOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondOption));
    EXPECT_EQ(parsedCommand.getUnknownOptions().size(), 1);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenBothNumericalAndStringArguments_WillParseCorrectly)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<
                           std::string,
                           float,
                           bool,
                           std::string,
                           int,
                           std::optional<std::string>,
                           std::optional<int>,
                           std::optional<long>>();
    std::string firstStringArgument { "firstArgument" };
    float secondFloatArgument { 1.2f };
    bool thirdBoolArgument { false };
    std::string fourthStringArgument { "fourthArgument" };
    int fifthIntArgument { -123 };
    std::string sixthStringArgument { "sixthArgument" };
    int seventhIntArgument { 456 };

    constexpr int argc = 9;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              firstStringArgument,
                                              std::to_string(secondFloatArgument),
                                              "false",
                                              fourthStringArgument,
                                              std::to_string(fifthIntArgument),
                                              sixthStringArgument,
                                              std::to_string(seventhIntArgument) };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto
        [firstArgument,
         secondArgument,
         thirdArgument,
         fourthArgument,
         fifthArgument,
         sixthArgument,
         seventhArgument,
         eighthArgument]
        = parsedCommand.getArgs(command);
    EXPECT_EQ(firstArgument, firstStringArgument);
    EXPECT_EQ(secondArgument, secondFloatArgument);
    EXPECT_EQ(thirdArgument, thirdBoolArgument);
    EXPECT_EQ(fourthArgument, fourthStringArgument);
    EXPECT_EQ(fifthArgument, fifthIntArgument);
    EXPECT_EQ(sixthArgument, sixthStringArgument);
    EXPECT_EQ(seventhArgument, seventhIntArgument);
    EXPECT_FALSE(eighthArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenInvalidNumericalArgument_WillCrash)
{
    std::string expectedCommand { "dummyCommand" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<int>();
    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "( ͡° ͜ʖ ͡°)" };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto shouldThrow = [&] { static_cast<void>(UnparsedCommand::parse(argc, argv.data(), commands)); };
    EXPECT_ANY_THROW(shouldThrow());
}

TEST(CommandParserTest, ParsedCommandImpl_WhenOptionSuppliedWithDashes_WillForgiveAndParse)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstOption { "--first-Flag" };
    std::string secondOption { "-secondFlag" };
    std::string thirdOption { "--thirdUnknownFlag" };
    auto command
        = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withOptions({ firstOption, secondOption });
    constexpr int argc = 5;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstOption, secondOption, thirdOption };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    EXPECT_TRUE(parsedCommand.hasOption(firstOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondOption));
    EXPECT_FALSE(parsedCommand.hasOption(thirdOption));
    EXPECT_EQ(parsedCommand.getUnknownOptions().size(), 1);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentHasAlias_WillParseAliasToo)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstAlias { "firstAlias" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withAliases({ firstAlias });
    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, firstAlias };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_TRUE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentHasMultipleAliases_WillParseAliasesToo)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstAlias { "firstAlias" };
    std::string secondAlias { "-secondAlias" };
    auto command
        = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withAliases({ firstAlias, secondAlias });
    constexpr int argc = 2;
    std::array<std::string, argc> arguments { "binary"s, secondAlias };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    EXPECT_TRUE(parsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentHasMultipleAliasesDeclaredInMultipleStages_WillParseAliasesToo)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstAlias { "firstAlias" };
    std::string secondAlias { "-secondAlias" };
    std::string thirdAlias { "thirdAlias" };
    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withAliases({ firstAlias, secondAlias })
                       .withAliases({ thirdAlias });
    constexpr int argc = 2;
    std::array<std::string, argc> firstArguments { "binary"s, secondAlias };
    std::array<std::string, argc> secondArguments { "binary"s, thirdAlias };

    auto firstArgv = toArgv(firstArguments);
    auto secondArgv = toArgv(secondArguments);
    std::tuple commands { command };

    auto firstParsedCommand = UnparsedCommand::parse(argc, firstArgv.data(), commands);
    EXPECT_TRUE(firstParsedCommand.is(command));
    auto secondParsedCommand = UnparsedCommand::parse(argc, secondArgv.data(), commands);
    EXPECT_TRUE(secondParsedCommand.is(command));
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentsSuppliedInMultipleStages_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstArgument { "firstArgument" };
    int secondArgument { 2 };

    auto command
        = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::string>().withArgs<int>();
    constexpr int argc = 4;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              firstArgument,
                                              std::to_string(secondArgument) };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstParsedArgument, secondParsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstParsedArgument, firstArgument);
    EXPECT_EQ(secondParsedArgument, secondArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentsAndOptionsSuppliedInAnyOrder_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstArgument { "firstArgument" };
    int secondArgument { 2 };
    std::string firstOption { "--first-Flag" };
    std::string secondOption { "-secondFlag" };
    std::string firstAlias { "firstAlias" };
    std::string secondAlias { "-secondAlias" };

    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withOptions({ firstOption })
                       .withArgs<std::string>()
                       .withAliases({ firstAlias })
                       .withArgs<int>()
                       .withAliases({ secondAlias })
                       .withArgs<std::optional<float>>()
                       .withOptions({ secondOption });
    constexpr int argc = 6;
    std::array<std::string, argc> arguments { "binary"s,     firstAlias,   firstOption,
                                              firstArgument, secondOption, std::to_string(secondArgument) };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstParsedArgument, secondParsedArgument, thirdParsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstParsedArgument, firstArgument);
    EXPECT_EQ(secondParsedArgument, secondArgument);
    EXPECT_FALSE(thirdParsedArgument);
    EXPECT_TRUE(parsedCommand.hasOption(firstOption));
    EXPECT_TRUE(parsedCommand.hasOption(secondOption));
}

struct AllowedCustomType {
    AllowedCustomType() = default;
    AllowedCustomType(std::string i)
        : id { prefix + i }
        , originalId { i }
    {
    }
    std::string prefix { "(¬‿¬)" };
    std::string id {};
    std::string originalId {};

    bool operator==(const AllowedCustomType& other) const { return id == other.id; }
};
static_assert(details::isAllowedCustomType<AllowedCustomType>::value);

struct TypeWithoutDefaultConstructor {
    TypeWithoutDefaultConstructor(std::string) {}
};
static_assert(!details::isAllowedCustomType<TypeWithoutDefaultConstructor>::value);

struct TypeWithoutStringConstructor {
    TypeWithoutStringConstructor() = default;
};
static_assert(!details::isAllowedCustomType<TypeWithoutStringConstructor>::value);

TEST(CommandParserTest, ParsedCommandImpl_WhenCustomTypeArgumentSupplied_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstArgument { "firstArgument" };
    AllowedCustomType secondArgument { "secondArgument" };

    auto command
        = UnparsedCommand::create(expectedCommand, "dummyDescription"s).withArgs<std::string, AllowedCustomType>();
    constexpr int argc = 4;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument, secondArgument.originalId };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstParsedArgument, secondParsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstParsedArgument, firstArgument);
    EXPECT_EQ(secondParsedArgument, secondArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCustomTypeArgumentOptional_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstArgument { "firstArgument" };
    AllowedCustomType secondArgument { "secondArgument" };

    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::optional<AllowedCustomType>, std::optional<AllowedCustomType>>();
    constexpr int argc = 4;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, firstArgument, secondArgument.originalId };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstParsedArgument, secondParsedArgument, thirdParsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstParsedArgument, firstArgument);
    EXPECT_EQ(secondParsedArgument, secondArgument);
    EXPECT_FALSE(thirdParsedArgument);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenCustomTypeArgumentVector_WillParse)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstArgument { "firstArgument" };
    AllowedCustomType secondArgument { "secondArgument" };
    AllowedCustomType thirdArgument { "thirdArgument" };

    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::vector<AllowedCustomType>>();
    constexpr int argc = 5;
    std::array<std::string, argc> arguments { "binary"s,
                                              expectedCommand,
                                              firstArgument,
                                              secondArgument.originalId,
                                              thirdArgument.originalId };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstParsedArgument, secondParsedArgument] = parsedCommand.getArgs(command);
    EXPECT_EQ(firstParsedArgument, firstArgument);
    std::vector expectedArguments { secondArgument, thirdArgument };
    EXPECT_EQ(secondParsedArgument, expectedArguments);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenArgumentStartingWithMoreThanTwoDashes_WillNotBeTreatedAsOption)
{
    std::string expectedCommand { "dummyCommand" };
    std::string firstArgument { "firstArgument" };
    std::string startWithTripleDashArg { "---secondArgument" }; // non-alphanumeric after double dash
    std::string tooShortToBeAnOptionArg { "-" };
    std::string containsSpacesArg { "--third argument" };
    std::string justTwoDashesArg { "--" };

    auto command = UnparsedCommand::create(expectedCommand, "dummyDescription"s)
                       .withArgs<std::string, std::string, std::string, std::string, std::string>();
    constexpr int argc = 7;
    std::array<std::string, argc> arguments {
        "binary"s,         expectedCommand, firstArgument, startWithTripleDashArg, tooShortToBeAnOptionArg,
        containsSpacesArg, justTwoDashesArg
    };

    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [firstParsedArgument, secondParsedArgument, thirdParsedArgument, fourthParsedArgument, fifthParsedArgument]
        = parsedCommand.getArgs(command);
    EXPECT_EQ(firstParsedArgument, firstArgument);
    EXPECT_EQ(secondParsedArgument, startWithTripleDashArg);
    EXPECT_EQ(thirdParsedArgument, tooShortToBeAnOptionArg);
    EXPECT_EQ(fourthParsedArgument, containsSpacesArg);
    EXPECT_EQ(fifthParsedArgument, justTwoDashesArg);
}

TEST(CommandParserTest, ParsedCommandImpl_WhenSubcommandsSupplied_WillParse)
{
    std::string groupCommand { "group" };
    std::string subcommand1 { "subcommand1" };
    std::string subcommand2 { "subcommand2" };
    std::tuple groupedCommands { UnparsedCommand::create(subcommand1, "dummyDescription"s).withArgs<std::string>(),
                                 UnparsedCommand::create(subcommand2, "dummyDescription"s) };

    auto group = UnparsedCommandGroup::create(groupCommand, "dummyDescription"s).withSubcommands(groupedCommands);

    std::string standAloneCommand { "standAloneCommand" };
    auto standAlone = UnparsedCommand::create(standAloneCommand, "").withArgs<std::string>();
    std::tuple commands { group, standAlone };

    constexpr int argc = 3;
    std::array<std::string, argc> arguments { "binary"s, groupCommand, subcommand2 };

    auto argv = toArgv(arguments);
    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);

    std::cout << "Test" << std::endl;
}
