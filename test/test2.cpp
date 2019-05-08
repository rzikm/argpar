#include "parser_fixture.h"

using ParserTests = parser_fixture;
TEST_F(ParserTests, parse_emptyInput_exception)
{
	char* argv[1];
	argv[0] = nullptr;
	ASSERT_THROW(parser.parse(0, argv), std::invalid_argument);
}

TEST_F(ParserTests, parse_unknownOption_exception)
{
	ASSERT_THROW(parse({ "--bar" }), argpar::parse_error);
}

TEST_F(ParserTests, parse_mandatoryOptNotPresent_exception)
{
	parser.option({ "bar" }, "");

	ASSERT_THROW(parse({ "bar" }), argpar::missing_option);
}

TEST_F(ParserTests, parse_mandatoryIntParNotPresent_exception)
{
	int num;
	parser.option({ "bar" }, "").int_val("num", &num);

	ASSERT_THROW(parse({ "--bar" }), argpar::missing_value);
}

TEST_F(ParserTests, parse_mandatoryStringParNotPresent_exception)
{
	std::string str;

	parser.option({ "bar" }, "").string_val("string", &str);

	ASSERT_THROW(parse({ "--bar" }), argpar::missing_value);
}

TEST_F(ParserTests, parse_mandatoryDoubleParNotPresent_exception)
{
	double dou;
	parser.option({ "bar" }, "").double_val("double", &dou);

	ASSERT_THROW(parse({ "--bar" }), argpar::missing_value);
}

TEST_F(ParserTests, parse_stringToIntPar_exception)
{
	int val;
	parser.option({ "bar" }, "").int_val("val", &val);

	ASSERT_THROW(parse({ "--bar=abcd" }), argpar::bad_value);
}

TEST_F(ParserTests, parse_doubleToIntPar_exception)
{
	int val;
	parser.option({ "bar" }, "").int_val("val", &val);

	ASSERT_THROW(parse({ "--bar=4.2" }), argpar::bad_value);
}


TEST_F(ParserTests, option_createMandatory_notNull)
{
	argpar::value_config & val = parser.option({ "abcd" }, "hint");
	ASSERT_NE(nullptr, &val);
}

TEST_F(ParserTests, option_createOptional_notNull)
{
	bool flag;
	argpar::value_config & val = parser.option({ "o" }, "hint2", &flag);
	ASSERT_NE(nullptr, &val);
}

TEST_F(ParserTests, option_aliases_notNull)
{
	bool flag;
	argpar::value_config & val = parser.option({ "V", "version" }, "hint3", &flag);
	ASSERT_NE(nullptr, &val);
}

TEST_F(ParserTests, option_alreadyUsedAliases_exception)
{
	bool flag;
	parser.option({ "test" }, "");
	ASSERT_THROW(parser.option({ "x", "y", "test" }, "", &flag), std::invalid_argument);
}

TEST_F(ParserTests, option_sameAliasesInSameCreator_exception)
{
	ASSERT_THROW(parser.option({ "z", "z", "z" }, ""), std::invalid_argument);
}

TEST_F(ParserTests, option_noAliases_exception)
{
	ASSERT_THROW(parser.option({}, ""), std::invalid_argument);
}

TEST_F(ParserTests, argument_listCalledBefore_exception)
{
	parser.argument_list();
	ASSERT_THROW(parser.argument(), std::logic_error);
}

TEST_F(ParserTests, argumentList_listCalledBefore_exception)
{
	parser.argument_list();
	ASSERT_THROW(parser.argument_list(), std::logic_error);
}

using ValueConfigTests = parser_fixture;
TEST_F(ValueConfigTests, intVal_nullDest_exception)
{
	ASSERT_THROW(parser.option({ "optname" }, "hint").int_val("parname", nullptr), std::invalid_argument);
}

TEST_F(ValueConfigTests, intVal_emptyName_exception)
{
	int val;
	ASSERT_THROW(parser.option({ "optname" }, "hint").int_val("", &val), std::invalid_argument);
}

TEST_F(ValueConfigTests, intVal_nameDuplicity_notNull)
{
	int val;
	argpar::int_cfg & cfg = parser.option({ "optname" }, "hint").int_val("parname", &val);

	ASSERT_NE(nullptr, &cfg);
}


TEST_F(ValueConfigTests, string_val_nullDest_exception)
{
	ASSERT_THROW(parser.option({ "optname" }, "hint").string_val("parname", nullptr), std::invalid_argument);
}

TEST_F(ValueConfigTests, string_val_emptyName_exception)
{
	std::string val;
	ASSERT_THROW(parser.option({ "optname" }, "hint").string_val("", &val), std::invalid_argument);
}

TEST_F(ValueConfigTests, string_val_nameDuplicity_notNull)
{
	std::string val;
	argpar::string_cfg & cfg = parser.option({ "optname" }, "hint").string_val("parname", &val);

	ASSERT_NE(nullptr, &cfg);
}


TEST_F(ValueConfigTests, doubleVal_nullDest_exception)
{
	ASSERT_THROW(parser.option({ "optname" }, "hint").double_val("parname", nullptr), std::invalid_argument);
}

TEST_F(ValueConfigTests, doubleVal_emptyName_exception)
{
	double val;
	ASSERT_THROW(parser.option({ "optname" }, "hint").double_val("", &val), std::invalid_argument);
}

TEST_F(ValueConfigTests, doubleVal_nameDuplicity_notNull)
{
	double val;
	argpar::double_cfg & cfg = parser.option({ "optname" }, "hint").double_val("parname", &val);

	ASSERT_NE(nullptr, &cfg);
}

using TypeCfgTests = parser_fixture;
TEST_F(TypeCfgTests, stringCfg_default_equal)
{
	std::string val;
	std::string defaulted = "default";
	parser.option({ "optname" }, "hint").string_val("parname", &val).with_default(defaulted);

	parse({ "--optname" });
	ASSERT_EQ(defaulted, val);
}


