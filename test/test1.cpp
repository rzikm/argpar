#include <gtest/gtest.h>
#include <argpar/argpar.h>

TEST(option_synonyms, correct_short_name)
{
	bool has_version = false;
	argpar::parser parser;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version); 

	char* short_name[2] = {"./cmd", "-V" };
	parser.parse(2, short_name);

	ASSERT_TRUE(has_version);
}

TEST(option_synonyms, correct_long_name)
{
	bool has_version = false;
	argpar::parser parser;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version);

	char* long_name[2] = { "./cmd", "--version" };
	parser.parse(2, long_name);

	ASSERT_TRUE(has_version);
}

TEST(option_synonyms, wrong_short_name)
{
	bool has_version = false;
	argpar::parser parser;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version); 

	char* short_name[2] = {"./cmd",  "--V" };
	parser.parse(2, short_name);

	ASSERT_FALSE(has_version);
}

TEST(option, option_presence)
{
	bool one,two = false;
	argpar::parser parser;
	parser.option({ "one" }, "", &one);
	parser.option({ "two" }, "", &two);

	char* argv[2] = {"./cmd",  "--one" };
	parser.parse(2, argv);

	ASSERT_TRUE(one);
	ASSERT_FALSE(two);
}

TEST(option, option_param_presence)
{
	std::string format;
	argpar::parser parser;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.string_val("FORMAT", &format);

	char* argv[3] = {"./cmd",  "-f","forrrmat" };
	parser.parse(3, argv);

	ASSERT_EQ(format,"forrrmat");
}

TEST(option, option_param_not_present)
{
	std::string format;
	argpar::parser parser;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.string_val("FORMAT", &format).with_default("xxx");

	char* argv[2] = {"./cmd",  "-f" };
	parser.parse(2, argv);

	ASSERT_EQ(format, "xxx");
}

TEST(option, option_int_param)
{
	int format;
	argpar::parser parser;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.int_val("FORMAT", &format);

	char* argv[3] = {"./cmd",  "-f","12" };
	parser.parse(3, argv);

	ASSERT_EQ(format, 12);
}

TEST(option, option_double_param)
{
	double format;
	argpar::parser parser;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.double_val("FORMAT", &format);

	char* argv[3] = {"./cmd",  "-f","12.88" };
	parser.parse(3, argv);

	ASSERT_EQ(format, 12.88);
}

TEST(option, option_string_param_from_succes)
{
	std::string format;
	argpar::parser parser;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.string_val("FORMAT", &format).from({ "a","b","c" });

	char* argv[3] = {"./cmd",  "-f","c" };
	parser.parse(3, argv);

	ASSERT_EQ(format, "c");
}

TEST(option, option_string_param_from_fail)
{
	try
	{
		std::string format;
		argpar::parser parser;
		parser.option({ "f", "format" }, "Sets format for the output.")
			.string_val("FORMAT", &format).from({ "a","b","c" });

		char* argv[3] = {"./cmd",  "-f","cc" };
		parser.parse(3, argv);

		FAIL();
	}
	catch (argpar::bad_value& e)
	{
		ASSERT_STREQ(e.name(), "cc");
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

TEST(option, option_custom_param)
{
	size_t format;
	argpar::parser parser;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.custom_val<config_42>("FORMAT", &format);

	char* argv[3] = { "./cmd", "-f","12.88" };
	parser.parse(3, argv);

	ASSERT_EQ(format, 42);
}

TEST(option, option_param_between)
{
	int format;
	argpar::parser parser;
	parser.option({ "f", "format" }, "Sets format for the output.")
		.int_val("FORMAT", &format).between(0, 5);

	char* argv[3] = {"./cmd",  "-f","4" };
	parser.parse(3, argv);

	ASSERT_EQ(format, 4);
}

TEST(plain_arguments, argument_present)
{
	std::string arg;
	argpar::parser parser;
	parser.argument().string_val("arg", &arg);

	char* argv[2] = {"./cmd",  "args" };
	parser.parse(2, argv);

	ASSERT_EQ(arg, "arg");
}

TEST(plain_arguments, argument_list_present)
{
	std::vector<std::string> args;
	argpar::parser parser;
	parser.argument_list().string_val("arg", &args);

	char* argv[3] = {"./cmd",  "args","args2" };
	parser.parse(3, argv);

	ASSERT_EQ(args.size(), 2);
	ASSERT_EQ(args[0], "args");
	ASSERT_EQ(args[1], "args2");
}

TEST(parsing_exceptions, logic_error_argument_call)
{
	try
	{
		std::vector<int> v;
		int i;
		argpar::parser parser;
		parser.argument_list().int_val("d", &v);
		parser.argument().int_val("d", &i);

		char* argv[2] = {"./cmd",  "-f" };
		parser.parse(2, argv);

		FAIL();
	}
	catch (std::logic_error&)
	{
		SUCCEED();
	}
}

TEST(parsing_exceptions, logic_error_between)
{
	//not sure whether bad between throws this type of exception, so feel free to change the test
	try
	{
		std::vector<int> v;
		int i;
		argpar::parser parser;
		parser.argument_list().int_val("d", &v);
		parser.argument().int_val("d", &i).between(10,0);

		char* argv[2] = {"./cmd",  "-f" };
		parser.parse(2, argv);

		FAIL();
	}
	catch (std::logic_error&)
	{
		SUCCEED();
	}
}

TEST(parsing_exceptions, bad_option)
{
	try
	{
		argpar::parser parser;

		char* argv[2] = {"./cmd",  "-f" };
		parser.parse(2, argv);

		FAIL();
	}
	catch (argpar::bad_option& e)
	{
		ASSERT_STREQ(e.name(), "f");
	}
}

TEST(parsing_exceptions, bad_value)
{
	try
	{
		int format;
		argpar::parser parser;
		parser.option({ "f" }, "").int_val("val", &format);

		char* argv[3] = {"./cmd",  "-f","args" };
		parser.parse(3, argv);

		FAIL();
	}
	catch (argpar::bad_value& e)
	{
		ASSERT_STREQ(e.value(), "args");
	}
}

TEST(parsing_exceptions, bad_value_between)
{
	//not sure whether bad between throws this type of exception, so feel free to change the test
	try
	{
		int format;
		argpar::parser parser;
		parser.option({ "f" }, "").int_val("val", &format).between(1,2);

		char* argv[3] = {"./cmd",  "-f","18" };
		parser.parse(3, argv);

		FAIL();
	}
	catch (argpar::bad_value& e)
	{
		ASSERT_STREQ(e.value(), "18");
	}
}

TEST(parsing_exceptions, missing_option)
{
	try
	{
		int format;
		argpar::parser parser;
		parser.option({ "f" }, "").int_val("val", &format);

		char* argv[2] = {"./cmd",  "11" };
		parser.parse(2, argv);

		FAIL();
	}
	catch (argpar::missing_option& e)
	{
		ASSERT_STREQ(e.name(), "f");
	}
}

TEST(parsing_exceptions, missing_value)
{
	try
	{
		int format;
		argpar::parser parser;
		parser.option({ "f" }, "").int_val("val", &format);

		char* argv[2] = {"./cmd",  "-f" };
		parser.parse(2, argv);

		FAIL();
	}
	catch (argpar::missing_value& e)
	{
		ASSERT_STREQ(e.name(), "val");
	}
}