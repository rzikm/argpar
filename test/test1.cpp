#include "parser_fixture.h"

using option_synonyms = parser_fixture;
TEST_F(option_synonyms, correct_short_name)
{
	bool has_version = false;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version);

	parse({ "-V" });

	ASSERT_TRUE(has_version);
}

TEST_F(option_synonyms, correct_long_name)
{
	bool has_version = false;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version);

	parse({ "--version" });

	ASSERT_TRUE(has_version);
}

TEST_F(option_synonyms, wrong_short_name)
{
	bool has_version = false;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version);

	ASSERT_THROW(parse({ "--V" }), argpar::argpar_exception);

	ASSERT_FALSE(has_version);
}

using option = parser_fixture;
TEST_F(option, option_presence)
{
	bool one, two = false;
	parser.option({ "one" }, "", &one);
	parser.option({ "two" }, "", &two);

	parse({ "--one" });

	ASSERT_TRUE(one);
	ASSERT_FALSE(two);
}

TEST_F(option, option_param_presence)
{
	std::string format;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.string_val("FORMAT", &format);

	parse({ "-f","forrrmat" });

	ASSERT_EQ(format, "forrrmat");
}

TEST_F(option, option_param_not_present)
{
	std::string format;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.string_val("FORMAT", &format).with_default("xxx");

	parse({ "-f" });

	ASSERT_EQ(format, "xxx");
}

TEST_F(option, option_int_param)
{
	int format;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.int_val("FORMAT", &format);

	parse({ "-f","12" });

	ASSERT_EQ(format, 12);
}

TEST_F(option, option_double_param)
{
	double format;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.double_val("FORMAT", &format);

	parse({ "-f","12.88" });

	ASSERT_EQ(format, 12.88);
}

TEST_F(option, option_string_param_from_succes)
{
	std::string format;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.string_val("FORMAT", &format).from({ "a","b","c" });

	parse({ "-f","c" });

	ASSERT_EQ(format, "c");
}

TEST_F(option, option_string_param_from_fail)
{
	try
	{
		std::string format;
		parser.option({ "f", "format" }, "Sets format for the output.")
			.string_val("FORMAT", &format).from({ "a","b","c" });

		parse({ "-f","cc" });

		FAIL();
	}
	catch (argpar::bad_value& e)
	{
		ASSERT_STREQ(e.name(), "f");
		ASSERT_STREQ(e.value(), "cc");
	}
}

class config_42 : public argpar::cfg_base<config_42, size_t>
{
public:

	// value_type typedef is inherited from base_cfg
	value_type parse(std::string const & value) const
	{
		return (size_t)42;
	}
};

TEST_F(option, option_custom_param)
{
	size_t format;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.custom_val<config_42>("FORMAT", &format);

	parse({ "-f","12.88" });

	ASSERT_EQ(format, 42);
}

TEST_F(option, option_param_between)
{
	int format;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.int_val("FORMAT", &format).between(0, 5);

	parse({ "-f","4" });

	ASSERT_EQ(format, 4);
}

using plain_arguments = parser_fixture;
TEST_F(plain_arguments, argument_present)
{
	std::string arg;
	parser.argument().string_val("arg", &arg);

	parse({ "args" });

	ASSERT_EQ(arg, "args");
}

TEST_F(plain_arguments, argument_list_present)
{
	std::vector<std::string> args;
	parser.argument_list().string_val("arg", &args);

	parse({ "args","args2" });

	ASSERT_EQ(args.size(), 2);
	ASSERT_EQ(args[0], "args");
	ASSERT_EQ(args[1], "args2");
}

using parsing_exceptions = parser_fixture;
TEST_F(parsing_exceptions, logic_error_argument_call)
{
	std::vector<int> v;
	int i;
	parser.argument_list().int_val("d", &v);
	ASSERT_THROW(parser.argument().int_val("d", &i), std::logic_error);
}

TEST_F(parsing_exceptions, logic_error_between)
{
	std::vector<int> v;
	int i;
	parser.argument_list().int_val("d", &v);
	ASSERT_THROW(parser.argument().int_val("d", &i).between(10, 0), std::logic_error);
}

TEST_F(parsing_exceptions, bad_option)
{
	try
	{
		parse({ "-f" });

		FAIL();
	}
	catch (argpar::bad_option& e)
	{
		ASSERT_STREQ(e.name(), "f");
	}
}

TEST_F(parsing_exceptions, bad_value)
{
	try
	{
		int format;
		parser.option({ "f" }, "").int_val("val", &format);

		parse({ "-f","args" });

		FAIL();
	}
	catch (argpar::bad_value& e)
	{
		ASSERT_STREQ(e.value(), "args");
	}
}

TEST_F(parsing_exceptions, bad_value_between)
{
	try
	{
		int format;
		parser.option({ "f" }, "").int_val("val", &format).between(1, 2);

		parse({ "-f","18" });

		FAIL();
	}
	catch (argpar::bad_value& e)
	{
		ASSERT_STREQ(e.value(), "18");
	}
}

TEST_F(parsing_exceptions, missing_option)
{
	try
	{
		int format;
		parser.option({ "f" }, "").int_val("val", &format);

		parse({ "11" });

		FAIL();
	}
	catch (argpar::missing_option& e)
	{
		ASSERT_STREQ(e.name(), "f");
	}
}

TEST_F(parsing_exceptions, missing_value)
{
	try
	{
		int format;
		parser.option({ "f" }, "").int_val("val", &format);

		parse({ "-f" });

		FAIL();
	}
	catch (argpar::missing_value& e)
	{
		ASSERT_STREQ(e.name(), "f");
	}
}
