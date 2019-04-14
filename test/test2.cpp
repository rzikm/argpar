#include <argpar/argpar.h>
#include <gtest/gtest.h>

#include <string.h>

// adaptors to make GTtest execute tests written for the obscure CPUnit framework
#define CPUNIT_TEST(fixture, test) TEST(fixture, test)

// GTest does not allow for exceptions to be thrown outside of test function, we therefore
// have to wrap it in try-cath block
#define TEST_WRAPPER_CLASS(fixture, test) fixture##test
#define CPUNIT_TEST_EX(fixture, test, exception) \
struct TEST_WRAPPER_CLASS(fixture,test) /* First, declare the wrapper class and method */\
{\
	static void run();\
};\
TEST(fixture, test) /* Use the actual body in a test */\
{\
	EXPECT_THROW(TEST_WRAPPER_CLASS(fixture,test)::run(), exception);\
}\
void TEST_WRAPPER_CLASS(fixture,test)::run() // the body will follow

#define CPUNIT_TEST_EX_ANY(fixture, test) CPUNIT_TEST_EX(fixture, test, std::exception) // we only use exceptions derived from std::exception anyway

// assertion adaptors
#define fail(msg) FAIL() << (msg)
#define assert_not_null(msg, val) ASSERT_NE(nullptr, val) << msg;
#define assert_true(msg, val) ASSERT_TRUE(val) << msg;

//Test<Jmeno tidy> pro tdy, 
//<jmeno toho metodu co testujeme>_<scenario>_<to chovn, kter oekvme>.

using namespace argpar;
// using namespace cpunit;

namespace ParserTests
{
	CPUNIT_TEST_EX(ParserTests, parse_emptyInput_exception, parse_error)
	{
		parser par;
		char * argv[1];
		argv[0] = NULL;
		par.parse(0, argv);
		fail("Expected parse_error exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, parse_unknownOption_exception, missing_option)
	{
		parser par;
		char * argv[3];
		argv[0] = "foo";
		argv[1] = "bar";
		argv[2] = NULL;
		int argc = 2;

		par.parse(argc, argv);
		fail("Expected missing_option exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, parse_mandatoryOptNotPresent_exception, missing_option)
	{
		parser par;
		char * argv[3];
		argv[0], "foo";
		argv[1], "bar";
		argv[2] = NULL;
		int argc = 2;

		par.option({ "bar" }, "");

		par.parse(argc, argv);
		fail("Expected missing_option exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, parse_mandatoryIntParNotPresent_exception, missing_value)
	{
		parser par;
		char * argv[3];
		argv[0], "foo";
		argv[1], "bar";
		argv[2] = NULL;
		int argc = 2;

		int num;
		std::string name = "num";

		par.option({ "bar" }, "").int_val(name, &num);

		par.parse(argc, argv);
		fail("Expected missing_value exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, parse_mandatoryStringParNotPresent_exception, missing_value)
	{
		parser par;
		char * argv[3];
		argv[0], "foo";
		argv[1], "bar";
		argv[2] = NULL;
		int argc = 2;

		std::string str;
		std::string name = "string";

		par.option({ "bar" }, "").string_val(name, &str);

		par.parse(argc, argv);
		fail("Expected missing_value exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, parse_mandatoryDoubleParNotPresent_exception, missing_value)
	{
		parser par;
		char * argv[3];
		argv[0], "foo";
		argv[1], "bar";
		argv[2] = NULL;
		int argc = 2;

		double dou;
		std::string name = "double";

		par.option({ "bar" }, "").double_val(name, &dou);

		par.parse(argc, argv);
		fail("Expected missing_value exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, parse_stringToIntPar_exception, bad_value)
	{
		parser par;
		char * argv[3];
		argv[0], "foo";
		argv[1], "bar=abcd";
		argv[2] = NULL;
		int argc = 2;

		int val;
		std::string name = "val";

		par.option({ "bar" }, "").int_val(name, &val);

		par.parse(argc, argv);
		fail("Expected bad_value exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, parse_doubleToIntPar_exception, bad_value)
	{
		parser par;
		char * argv[3];
		argv[0], "foo";
		argv[1], "bar=4.2";
		argv[2] = NULL;
		int argc = 2;

		int val;
		std::string name = "val";

		par.option({ "bar" }, "").int_val(name, &val);

		par.parse(argc, argv);
		fail("Expected bad_value exception to be thrown.");
	}


	CPUNIT_TEST(ParserTests, option_createMandatory_notNull)
	{
		parser par;
		value_config val = par.option({ "abcd" }, "hint");
		assert_not_null("Should not be null", &val);
	}

	CPUNIT_TEST(ParserTests, option_createOptional_notNull)
	{
		bool flag;
		parser par;
		value_config val = par.option({ "o" }, "hint2", &flag);
		assert_not_null("Should not be null", &val);
	}

	CPUNIT_TEST(ParserTests, option_aliases_notNull)
	{
		bool flag;
		parser par;
		value_config val = par.option({ "V", "version" }, "hint3", &flag);
		assert_not_null("Should not be null", &val);
	}

	CPUNIT_TEST_EX(ParserTests, option_alreadyUsedAliases_exception, std::invalid_argument)
	{
		bool flag;
		parser par;
		value_config val0 = par.option({ "test" }, "");
		value_config val1 = par.option({ "x", "y", "test" }, "", &flag);
		fail("Expected std::invalid_argument exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, option_sameAliasesInSameCreator_exception, std::invalid_argument)
	{
		parser par;
		value_config val = par.option({ "z", "z", "z" }, "");
		fail("Expected std::invalid_argument exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, option_noAliases_exception, std::invalid_argument)
	{
		parser par;
		par.option({}, "");
		fail("Expected std::invalid_argument exception to be thrown.");
	}


	CPUNIT_TEST_EX(ParserTests, argument_listCalledBefore_exception, std::logic_error)
	{
		parser par;
		par.argument_list();
		par.argument();
		fail("Expected std::logic_error exception to be thrown.");
	}

	CPUNIT_TEST_EX(ParserTests, argumentList_listCalledBefore_exception, std::logic_error)
	{
		parser par;
		par.argument_list();
		par.argument_list();
		fail("Expected std::logic_error exception to be thrown.");
	}

}

namespace ValueConfigTests
{
	CPUNIT_TEST_EX_ANY(ValueConfigTests, intVal_nullDest_exception)
	{
		parser par;
		par.option({ "optname" }, "hint").int_val("parname", nullptr);
		fail("Expected exception to be thrown.");
	}

	CPUNIT_TEST_EX_ANY(ValueConfigTests, intVal_emptyName_exception)
	{
		parser par;
		int val;
		par.option({ "optname" }, "hint").int_val("", &val);
		fail("Expected exception to be thrown.");
	}

	CPUNIT_TEST(ValueConfigTests, intVal_nameDuplicity_notNull)
	{
		parser par;
		int val;
		int_cfg cfg = par.option({ "optname" }, "hint").int_val("parname", &val);
		
		assert_not_null("Should not be null", &cfg);
	}

	
	CPUNIT_TEST_EX_ANY(ValueConfigTests, string_val_nullDest_exception)
	{
		parser par;
		par.option({ "optname" }, "hint").string_val("parname", nullptr);
		fail("Expected exception to be thrown.");
	}

	CPUNIT_TEST_EX_ANY(ValueConfigTests, string_val_emptyName_exception)
	{
		parser par;
		std::string val;
		par.option({ "optname" }, "hint").string_val("", &val);
		fail("Expected exception to be thrown.");
	}
	
	CPUNIT_TEST(ValueConfigTests, string_val_nameDuplicity_notNull)
	{
		parser par;
		std::string val;
		string_cfg cfg = par.option({ "optname" }, "hint").string_val("parname", &val);

		assert_not_null("Should not be null", &cfg);	
	}
	

	CPUNIT_TEST_EX_ANY(ValueConfigTests, doubleVal_nullDest_exception)
	{
		parser par;
		par.option({ "optname" }, "hint").double_val("parname", nullptr);
		fail("Expected exception to be thrown.");
	}

	CPUNIT_TEST_EX_ANY(ValueConfigTests, doubleVal_emptyName_exception)
	{
		parser par;
		double val;
		par.option({ "optname" }, "hint").double_val("", &val);
		fail("Expected exception to be thrown.");
	}

	CPUNIT_TEST(ValueConfigTests, doubleVal_nameDuplicity_notNull)
	{
		parser par;
		double val;
		double_cfg cfg = par.option({ "optname" }, "hint").double_val("parname", &val);
		
		assert_not_null("Should not be null", &cfg);
	}


	/*CPUNIT_TEST_EX_ANY(ValueConfigTests, customVal_nullDest_exception)
	{
		parser par;
		par.option({ "optname" }, "hint").custom_val<int>("parname", nullptr);
		fail("Expected exception to be thrown.");
	}

	CPUNIT_TEST_EX_ANY(ValueConfigTests, customVal_emptyName_exception)
	{
		parser par;
		auto val;
		par.option({ "optname" }, "hint").custom_val<int>("", &val);
		fail("Expected exception to be thrown.");
	}

	CPUNIT_TEST(ValueConfigTests, customVal_nameDuplicity_notNull)
	{
		parser par;
		auto val;
		int cfg = par.option({ "optname" }, "hint").custom_val<int>("parname", &val);
		
		assert_not_null("Should not be null", &cfg);
	}*/
}

namespace TypeCfgTests
{
	CPUNIT_TEST(TypeCfgTests, stringCfg_default_equal)
	{
		parser par;
		int argc = 2;
		char * argv[3];
		argv[0] = "foo";
		argv[1] = "option";
		argv[2] = NULL;

		std::string val;
		std::string defaulted = "default";

		par.option({ "optname" }, "hint").string_val("parname", &val).with_default(defaulted);

		par.parse(argc, argv);
		assert_true("Should be equal", val == defaulted);
	}


}
