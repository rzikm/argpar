#include "parser_fixture.h"

using parsing_exceptions = parser_fixture;
TEST_F(parsing_exceptions, empty_input)
{
	char* argv[1];
	argv[0] = nullptr;
	ASSERT_THROW(parser.parse(0, argv), std::invalid_argument);
}

TEST_F(parsing_exceptions, bad_option_when_unknown)
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

TEST_F(parsing_exceptions, bad_value_when_cannot_parse)
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

TEST_F(parsing_exceptions, parse_doubleToIntPar_exception)
{
	try
	{
		int val;
		parser.option({ "bar" }, "").int_val("val", &val);

		parse({ "--bar=4.2" });
		FAIL();
	}
	catch (argpar::bad_value& e)
	{
		ASSERT_STREQ(e.value(), "4.2");
		ASSERT_STREQ(e.name(), "bar");
	}
}

TEST_F(parsing_exceptions, bad_value_when_not_between)
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
