#include <gtest/gtest.h>
#include <argpar/argpar.h>

class test_fixture : public ::testing::Test
{
protected:
	test_fixture()
		: o_val(0)
		, m_val(0)
	{
		parser.option({ "o" }, "").int_val("val", &o_val).with_default(2);
		parser.option({ "m" }, "").int_val("val", &m_val);
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

		parser.parse(args.size() - 1, args.data());
	}

	int o_val;
	int m_val;
	argpar::parser parser;
};

using optional_parameter = test_fixture;
TEST_F(optional_parameter, condensed)
{
	ASSERT_NO_THROW(parse({"-o1", "-m", "1"}));
	ASSERT_EQ(o_val, 1);
	ASSERT_EQ(m_val, 1);
}

TEST_F(optional_parameter, not_condensed)
{
	parser.argument().int_val("arg", &m_val);
	ASSERT_NO_THROW(parse({"-m", "0", "-o", "1"}));
	ASSERT_EQ(o_val, 2); // default
	ASSERT_EQ(m_val, 1); // set by the positional arg
}