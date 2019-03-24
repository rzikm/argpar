#include <argpar/argpar.h>

using namespace argpar;

// static instances for compilation purposes
static int_cfg int_config_;
static string_cfg str_config_;
static double_cfg dbl_config_;
static value_config value_config_;
static value_list_config value_list_config_;

string_cfg & string_cfg::from(std::vector<std::string> const & values)
{
	return *this;
}

string_cfg & string_cfg::with_default(std::string const & value)
{
	return *this;
}

int_cfg & int_cfg::between(int min, int max)
{
	return *this;
}

int_cfg & int_cfg::with_default(int value)
{
	return *this;
}

double_cfg & double_cfg::between(double min, double max)
{
	return *this;
}

double_cfg & double_cfg::with_default(double value)
{
	return *this;
}

int_cfg & value_config::int_val(std::string const & name, int * par_dest)
{
	return int_config_;
}

string_cfg & value_config::string_val(std::string const & name, std::string * par_dest)
{
	return str_config_;
}

double_cfg & value_config::double_val(std::string const & name, double * par_dest)
{
	return dbl_config_;
}

int_cfg & value_list_config::int_val(std::string const & name, std::vector<int> * par_dest)
{
	return int_config_;
}

string_cfg & value_list_config::string_val(std::string const & name, std::vector<std::string> * par_dest)
{
	return str_config_;
}

double_cfg & value_list_config::double_val(std::string const & name, std::vector<double> * par_dest)
{
	return dbl_config_;
}

value_config & parser::option(std::vector<std::string> const & aliases, std::string const & hint)
{
	return value_config_;
}

value_config & parser::option(std::vector<std::string> const & aliases, std::string const & hint,
                              bool * opt_dest)
{
	return value_config_;
}

value_config & parser::argument()
{
	return value_config_;
}

value_list_config & parser::argument_list()
{
	return value_list_config_;
}

void parser::parse(int p_argc, char ** p_argv)
{
}

void parser::print_help(std::ostream & stream)
{
}
