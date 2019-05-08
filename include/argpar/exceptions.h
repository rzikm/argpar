#ifndef ARGPAR_EXCEPTIONS_H
#define ARGPAR_EXCEPTIONS_H

namespace argpar
{

class argpar_exception: public std::exception
{
public:
	argpar_exception(std::string const & message)
		: message_(message)
	{
	}

	char const * what() const noexcept override
	{
		return message_.c_str();
	}

	std::string const & message() const noexcept
	{
		return message_;
	}
private:
	std::string message_;
};

/**
 * Exception signalling that a command line argument cannot be parsed to the program
 * representation of the option parameter.
 */
class format_error : public argpar_exception
{
public:
		/**
	 * \param[in] msg Message giving further explanation of the error.
	 */
	format_error(std::string const & message)
		: argpar_exception(message)
	{
	}
};

/**
 * Base class of the exceptions possibly thrown when parsing command line arguments.
 */
class parse_error : public argpar_exception
{
public:
	/**
	 * Gets the name of the option or argument which caused the error.
	 * \return Returns C style string containing the name of the option which caused the error
	 */
	char const * name() const
	{
		return name_.c_str();
	}
protected:
	parse_error(std::string const & name, std::string const & message) 
		: name_(name)
		, argpar_exception(message)
	{
	}

private:
	std::string name_;
};

/**
 * Exception signaling that an unknown option was encountered.
 */
class bad_option : public parse_error
{
public:
	bad_option(std::string const & name)
		: argpar::parse_error(name, helpers::make_str("Unknown option: '", name, "'"))
	{
	}
};

/**
 * Exception signaling that an incompatible option parameter or argument was encountered.
 */
class bad_value : public parse_error
{
public:
	bad_value(std::string const & name, std::string const & value, std::string const & message)
		: parse_error(name, helpers::make_str("Invalid value for option '", name, "': ", message))
		, value_(value)
	{
	}
	char const * value() const
	{
		return value_.c_str();
	}
private:
	std::string value_;
};

/**
 * Exception signaling that a mandatory option was missing from command line args.
 */
class missing_option : public parse_error
{
public:
	missing_option(std::string const & name)
		: parse_error(name, helpers::make_str("Mandatory option missing: '", name, "'."))
	{
	}
};

/**
 * Exception signaling that a mandatory option parameter or positional was missing from command line args.
 */
class missing_value : public parse_error
{
public:
	missing_value(std::string const & name, bool is_option)
		: parse_error(name, helpers::make_str("Missing value for ", is_option ? "option" : "argument", " '", name, "'."))
	{
	}
};

}
#endif // ARGPAR_EXCEPTIONS_H
