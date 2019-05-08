#include "parser_fixture.h"

using plain_arguments = parser_fixture;
TEST_F(plain_arguments, argument_present)
{
	std::string arg;
	parser.argument().string_val("arg", &arg);

	parse({ "args" });

	ASSERT_EQ(arg, "args");
}

TEST_F(plain_arguments, argument_list_multiple)
{
	std::vector<std::string> args;
	parser.argument_list().string_val("arg", &args);

	parse({ "args","args2" });

	ASSERT_EQ(args.size(), 2);
	ASSERT_EQ(args[0], "args");
	ASSERT_EQ(args[1], "args2");
}

TEST_F(plain_arguments, too_many_arguments)
{
	ASSERT_THROW(parse({"extra"}), argpar::argpar_exception);
}

TEST_F(plain_arguments, too_few_arguments)
{
	int i;
	parser.argument().int_val("arg", &i);

	ASSERT_THROW(parse({}), argpar::missing_value);
}


TEST_F(plain_arguments, sets_default)
{
	int i;
	parser.argument().int_val("arg", &i).with_default(1);

	ASSERT_NO_THROW(parse({}));
	ASSERT_EQ(i, 1);
}

TEST_F(plain_arguments, explicit_separator)
{
	bool f_set = false;
	std::string s;
	parser.option({ "f" }, "", &f_set);
	parser.argument().string_val("arg", &s).with_default("default");

	ASSERT_NO_THROW(parse({"--", "-f"}));
	ASSERT_EQ(s, "-f");
	ASSERT_FALSE(f_set);
}
