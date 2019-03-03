#ifndef ARGPAR_H
#define ARGPAR_H

#include <string>
#include <vector>

namespace argpar
{
	template <typename opt_config>
	class _par_config_base
	{
	public:
		opt_config & requires(std::vector<std::string> required_options);
		opt_config & conflicts(std::vector<std::string> conflicted_options);
	};

	// forward declarations
	class opt_config;
	class int_par_config;
	class str_par_config;
	class str_list_par_config;

	class arg_parser
	{
	public:
		opt_config & opt(char short_name, char const * long_name, char const * hint, bool * pFlag);
		opt_config & opt(char short_name, char const * long_name, char const * hint);

		void parse(int * pArgc, char *** pArgv);

		void print_help(std::ostream & stream);
	};


	class opt_config : public _par_config_base<opt_config>
	{
	public:
		int_par_config & integer(char const * name, int * dest);
		str_par_config & string(char const * name, std::string * dest);
		str_list_par_config & string_list(char const * name, std::vector<std::string> * dest);
	};

	class str_list_par_config : public _par_config_base<str_list_par_config>
	{
	};

	class str_par_config : public _par_config_base<str_par_config>
	{
	public:
		str_par_config & with_default(char const * value);
		str_par_config & from(std::vector<std::string> const & allowed_values);
	};

	class int_par_config : public _par_config_base<int_par_config>
	{
	public:
		int_par_config & between(int min, int max);
		int_par_config & with_default(int value);
	};

}
#endif // ARGPAR_H
