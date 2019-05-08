#include "parser_fixture.h"

using config_validation = parser_fixture;
TEST_F(config_validation, argument_without_value)
{
	parser.argument();
	ASSERT_THROW(parse({}), std::logic_error);
}

TEST_F(config_validation, argument_list_without_value)
{
	parser.argument_list();
	ASSERT_THROW(parse({}), std::logic_error);
}

TEST_F(config_validation, argument_after_argument_list)
{
	std::vector<int> v;
	int i;
	parser.argument_list().int_val("d", &v);
	ASSERT_THROW(parser.argument().int_val("d", &i), std::logic_error);
}

TEST_F(config_validation, argument_list_twice)
{
	std::vector<int> v;
	parser.argument_list().int_val("d", &v);
	ASSERT_THROW(parser.argument_list(), std::logic_error);
}

TEST_F(config_validation, mandatory_argument_after_optional)
{
	int i;
	parser.argument().int_val("arg", &i).with_default(1);
	parser.argument().int_val("arg2", &i);

	ASSERT_THROW(parse({}), std::logic_error);
}

TEST_F(config_validation, between_min_gt_max)
{
	std::vector<int> v;
	int i;
	parser.argument_list().int_val("d", &v);
	ASSERT_THROW(parser.argument().int_val("d", &i).between(10, 0), std::logic_error);
}
