#include <gtest/gtest.h>
#include <argpar/argpar.h>

class test_fixture : public ::testing::Test
{
protected:
	test_fixture()
		: o_val(0)
		, m_val(0)
	{
	}

	void parse(std::vector<std::string> tokens)
	{
		std::vector<char *> args(tokens.size() + 2);
		char cmd[] = "cmd";
		args[0] = cmd;
		args[tokens.size() + 1] = nullptr;
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			args[i + 1] = tokens[i].data();
		}

		parser.parse((int) args.size() - 1, args.data());
	}

	int o_val;
	int m_val;
	argpar::parser parser;
};

using flag_option = test_fixture;
TEST_F(flag_option, error_on_parameter)
{
	parser.option({ "ff" }, "");
	
	ASSERT_THROW(parse({ "--ff=Value" }), argpar::bad_value);
}

using optional_parameter = test_fixture;
TEST_F(optional_parameter, condensed)
{
	parser.option({ "o" }, "").int_val("val", &o_val).with_default(2);
	parser.option({ "m" }, "").int_val("val", &m_val);

	ASSERT_NO_THROW(parse({"-o1", "-m", "1"}));

	ASSERT_EQ(o_val, 1);
	ASSERT_EQ(m_val, 1);
}

TEST_F(optional_parameter, not_condensed)
{
	parser.option({ "o" }, "").int_val("val", &o_val).with_default(2);
	parser.argument().int_val("arg", &m_val);

	ASSERT_NO_THROW(parse({"-o", "1"}));

	ASSERT_EQ(o_val, 2); // default
	ASSERT_EQ(m_val, 1); // set by the positional arg
}

using positional_arguments = test_fixture;
TEST_F(positional_arguments, too_few_arguments)
{
	parser.argument().int_val("arg", &m_val);

	ASSERT_THROW(parse({}), argpar::missing_value);
}

TEST_F(positional_arguments, too_many_arguments)
{
	ASSERT_THROW(parse({"extra"}), argpar::argpar_exception);
}

TEST_F(positional_arguments, sets_default)
{
	parser.argument().int_val("arg", &m_val).with_default(1);

	ASSERT_NO_THROW(parse({}));

	ASSERT_EQ(m_val, 1);
}

TEST_F(positional_arguments, mandatory_after_optional)
{
	parser.argument().int_val("arg", &m_val).with_default(1);
	parser.argument().int_val("arg2", &m_val);

	ASSERT_THROW(parse({}), std::logic_error);
}

TEST_F(positional_arguments, explicit_separator)
{
	bool f_set = false;
	std::string s;
	parser.option({ "f" }, "", &f_set);
	parser.argument().string_val("arg", &s).with_default("default");

	ASSERT_NO_THROW(parse({"--", "-f"}));
	ASSERT_EQ(s, "-f");
	ASSERT_FALSE(f_set);
}
