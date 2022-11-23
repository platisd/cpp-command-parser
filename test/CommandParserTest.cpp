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
    std::string compoundOptionWithUknownElement = thirdExpectedOption + secondExpectedOption + unknownOption;
    std::array<std::string, argc> arguments { "binary"s, expectedCommand, "-" + compoundOptionWithUknownElement };
    auto argv = toArgv(arguments);
    std::tuple commands { command };

    auto parsedCommand = UnparsedCommand::parse(argc, argv.data(), commands);
    ASSERT_TRUE(parsedCommand.is(command));
    auto [parsedArgument] = parsedCommand.getArgs(command);
    EXPECT_FALSE(parsedArgument);
    EXPECT_FALSE(parsedCommand.hasOption(firstExpectedOption));
    EXPECT_FALSE(parsedCommand.hasOption(secondExpectedOption));
    EXPECT_FALSE(parsedCommand.hasOption(thirdExpectedOption));
    EXPECT_EQ(parsedCommand.getUnknownOptions(), std::unordered_set { compoundOptionWithUknownElement });
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