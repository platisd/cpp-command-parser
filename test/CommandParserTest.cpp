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
