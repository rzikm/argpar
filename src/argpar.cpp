#include <argpar/argpar.h>

argpar::string_cfg & argpar::string_cfg::from(std::vector<std::string> const & values)
{
	return *this;
}

argpar::string_cfg & argpar::string_cfg::with_default(std::string const & value)
{
	return *this;
}

argpar::int_cfg & argpar::int_cfg::between(int min, int max)
{
	return *this;
}

argpar::int_cfg & argpar::int_cfg::with_default(int value)
{
	return *this;
}

argpar::double_cfg & argpar::double_cfg::between(double min, double max)
{
	return *this;
}

argpar::double_cfg & argpar::double_cfg::with_default(double value)
{
	return *this;
}

argpar::int_cfg & argpar::value_config::int_val(std::string const & name, int * par_dest)
{
	return *new int_cfg;
}

argpar::string_cfg & argpar::value_config::string_val(std::string const & name, std::string * par_dest)
{
	return *new string_cfg;
}

argpar::double_cfg & argpar::value_config::double_val(std::string const & name, double * par_dest)
{
	return *new double_cfg;
}

argpar::int_cfg & argpar::value_list_config::int_val(std::string const & name, std::vector<int> * par_dest)
{
	return *new int_cfg;
}

argpar::string_cfg & argpar::value_list_config::string_val(std::string const & name, std::vector<std::string> * par_dest)
{
	return *new string_cfg;
}

argpar::double_cfg & argpar::value_list_config::double_val(std::string const & name, std::vector<double> * par_dest)
{
	return *new double_cfg;
}

argpar::value_config & argpar::parser::option(std::vector<std::string> const & aliases, std::string const & hint)
{
	return * new value_config;
}

argpar::value_config & argpar::parser::option(std::vector<std::string> const & aliases, std::string const & hint,
                              bool * opt_dest)
{
	return * new value_config;
}

argpar::value_config & argpar::parser::argument()
{
	return * new value_config;
}

argpar::value_list_config & argpar::parser::argument_list()
{
	return * new value_list_config;
}

void argpar::parser::parse(int p_argc, char ** p_argv)
{
}

void argpar::parser::print_help(std::ostream & stream)
{
}

char const * argpar::bad_value::value() const
{
	return nullptr;
}

char const * argpar::parse_error::name() const
{
	return nullptr;
}
