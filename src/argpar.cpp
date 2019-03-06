#include <argpar/argpar.h>

// static instances for compilation purposes
static argpar::integer_par_config int_config_;
static argpar::string_par_config str_config_;
static argpar::double_par_config dbl_config_;
static argpar::opt_config opt_config_;

argpar::string_par_config& argpar::string_par_config::from(std::vector<std::string> const& values)
{
	return *this;
}

argpar::string_par_config& argpar::string_par_config::with_default(std::string const& value)
{
	return *this;
}

argpar::integer_par_config& argpar::integer_par_config::between(int min, int max)
{
	return *this;
}

argpar::integer_par_config& argpar::integer_par_config::with_default(int value)
{
	return *this;
}

argpar::double_par_config& argpar::double_par_config::between(double min, double max)
{
	return *this;
}

argpar::double_par_config& argpar::double_par_config::with_default(double value)
{
	return *this;
}

argpar::integer_par_config& argpar::opt_config::integer_par(std::string const& name, int* par_dest)
{
	return int_config_;
}

argpar::string_par_config& argpar::opt_config::string_par(std::string const& name, std::string* par_dest)
{
	return str_config_;
}

argpar::double_par_config& argpar::opt_config::double_par(std::string const& name, double* par_dest)
{
	return dbl_config_;
}

argpar::opt_config& argpar::arg_parser::option(std::vector<std::string> const& aliases, std::string const& hint)
{
	return opt_config_;
}

argpar::opt_config& argpar::arg_parser::option(std::vector<std::string> const& aliases, std::string const& hint,
	bool* opt_dest)
{
	return opt_config_;
}

void argpar::arg_parser::parse(int* p_argc, char*** p_argv)
{
}

void argpar::arg_parser::print_help(std::ostream& stream)
{
}
