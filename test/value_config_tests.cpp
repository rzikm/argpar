#include "parser_fixture.h"

using value_configuration = parser_fixture;
template<typename T>
using access_func = T & (argpar::value_config::*)(std::string const &, typename T::value_type *);

template<typename T, access_func<T> Accessor>
struct memfun
{
	using value_type = typename T::value_type;
	using cfg_type = T;
};

template<typename T> // T is a memfun defined above
class value_cfg_fixture;

template<typename T, access_func<T> Accessor> 
class value_cfg_fixture<memfun<T, Accessor>> : public parser_fixture
{
protected:
	static constexpr access_func<T> make_val = Accessor;
};

using types = ::testing::Types<
	memfun<argpar::int_cfg, &argpar::value_config::int_val>,
	memfun<argpar::double_cfg, &argpar::value_config::double_val>,
	memfun<argpar::string_cfg, &argpar::value_config::string_val>
>;

TYPED_TEST_SUITE(value_cfg_fixture, types);
TYPED_TEST(value_cfg_fixture, dest_nullptr)
{
	ASSERT_THROW((this->parser.option({ "optname" }, "hint").*this->make_val)("parname", nullptr), std::invalid_argument);
}

TYPED_TEST(value_cfg_fixture, empty_name)
{
	typename TypeParam::value_type val;
	ASSERT_THROW((this->parser.option({ "optname" }, "hint").*this->make_val)("", &val), std::invalid_argument);
}

TYPED_TEST(value_cfg_fixture, config_not_nullptr)
{
	typename TypeParam::value_type val;
	typename TypeParam::cfg_type & cfg = (this->parser.option({ "optname" }, "hint").*this->make_val)("parname", &val);

	ASSERT_NE(nullptr, &cfg);
}
