#include "parser_fixture.h"

using option_synonyms = parser_fixture;
TEST_F(option_synonyms, finds_by_short_name)
{
	bool has_version = false;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version);

	parse({ "-V" });

	ASSERT_TRUE(has_version);
}

TEST_F(option_synonyms, finds_by_long_name)
{
	bool has_version = false;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version);

	parse({ "--version" });

	ASSERT_TRUE(has_version);
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

TEST_F(option, bad_value_when_argument_to_flag)
{
	parser.option({ "ff" }, "");
	
	ASSERT_THROW(parse({ "--ff=Value" }), argpar::bad_value);
}

TEST_F(option, parses_condensed)
{
	parser.option({ "o" }, "").int_val("val", &o_val).with_default(2);
	parser.option({ "m" }, "").int_val("val", &m_val);

	ASSERT_NO_THROW(parse({"-o1", "-m", "1"}));

	ASSERT_EQ(o_val, 1);
	ASSERT_EQ(m_val, 1);
}

TEST_F(option, parses_not_condensed)
{
	parser.option({ "o" }, "").int_val("val", &o_val).with_default(2);
	parser.argument().int_val("arg", &m_val);

	ASSERT_NO_THROW(parse({"-o", "1"}));

	ASSERT_EQ(o_val, 2); // default
	ASSERT_EQ(m_val, 1); // set by the positional arg
}

TEST_F(option, dupliate_alias)
{
	bool flag;
	parser.option({ "test" }, "");
	ASSERT_THROW(parser.option({ "x", "test" }, "", &flag), std::invalid_argument);
}

TEST_F(option, multiple_same_aliases)
{
	ASSERT_THROW(parser.option({ "z", "z", "z" }, ""), std::invalid_argument);
}

TEST_F(option, no_alias)
{
	ASSERT_THROW(parser.option({}, ""), std::invalid_argument);
}

TEST_F(option, value_config_not_null)
{
	argpar::value_config & val = parser.option({ "abcd" }, "hint");
	ASSERT_NE(nullptr, &val);
}
