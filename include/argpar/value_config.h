#ifndef ARGPAR_VALUE_CONFIG_H
#define ARGPAR_VALUE_CONFIG_H

#include "helpers.h"
#include "value_handler.h"
#include "exceptions.h"
#include "option.h"

namespace argpar
{

/**
 * Class which can be used as a mixin for custom parameter types to provide functionality for
 * default values.
 * \par Expected usage:
 * class my_custom_cfg : public argpar::cfg_base<my_custom_cfg, my_custom_value> { ... }
 * \tparam TConfig Class used to configure the parameter, see argpar::value_config::custom_val.
 * \tparam TValue Type of the value of the parameter, see argpar::value_config::custom_val.
 */
template<typename TConfig, typename TValue>
class cfg_base : private helpers::no_copy_move
{
	cfg_base(cfg_base const &) = delete;
protected:
	cfg_base()
	{
	}

public:
	using value_type = TValue;
	using container = std::vector<TValue>;

	bool has_default() const
	{
		return value_.has_value();
	}

	value_type get_default() const
	{
		return value_.value();
	}

	/**
	 * Configures the parameter to be optional, using the specified value when none is supplied.
	 * \param[in] value Default value of the parameter.
	 * \return Returns reference to this object for method chaining.
	 */
	TConfig & with_default(const value_type & value)
	{
		value_ = value;
		return *(TConfig *)this;
	}
private:
	std::optional<value_type> value_;
};

/**
 * Class for configuring a string value of a plain argument or option parameter.
 */
class string_cfg : public cfg_base<string_cfg, std::string>
{
public:
	/**
	 * Constrains the parameter to specified set of values.
	 * \param[in] values Allowed set of values.
	 * \return Returns reference to this object for method chaining.
	 */
	string_cfg & from(std::vector<std::string> const & values)
	{
		if (values.empty()) throw std::invalid_argument("Values array must be nonempty");
		values_.resize(values.size());
		std::copy(values.begin(), values.end(), values_.begin());
		return *this;
	}

	std::string parse(std::string const & value) const
	{
		if (!values_.empty() && std::find(values_.begin(), values_.end(), value) == values_.end())
		{
			throw argpar::format_error(helpers::make_str("Value '", value, "' is not allowed."));
		}

		return value;
	}

private:
	//! List of allowed values. If empty, any value is allowed
	std::vector<std::string> values_;
};

template<typename TConfig, typename TValue>
class cfg_integral_base : public cfg_base<TConfig, TValue>
{
public:
	/**
	 * Constrains the parameter to fit between specified two values.
	 * \param[in] min Minimum value of the parameter (inclusive).
	 * \param[in] max Maximum value of the parameter (inclusive).
	 * \return Returns reference to this object for method chaining.
	 */
	TConfig & between(TValue min, TValue max)
	{
		if (min > max) throw std::logic_error("Min cannot be greater than max");
		min_value_ = min;
		max_value_ = max;
		return *(TConfig *)this;
	}

	TValue parse(std::string const & value) const
	{
		TValue val;
		bool valid = true;

		std::stringstream ss(value);
		ss >> val;
		if (!ss || !ss.eof()) // either read failed or not read entire string
			throw argpar::format_error(helpers::make_str("Value '", value, "' does not represent a valid number."));

		if (!valid || val > max_value_ || val < min_value_)
			throw argpar::format_error(helpers::make_str("Value '", value, "' is out of bounds."));

		return val;
	}

private:
	TValue min_value_ = std::numeric_limits<TValue>::min();
	TValue max_value_ = std::numeric_limits<TValue>::max();
};

/**
 * Class for configuring an integer value of a plain argument or option parameter.
 */
class int_cfg : public cfg_integral_base<int_cfg, int>
{
};

/**
 * Class for configuring a double value of a plain argument or option parameter.
 */
class double_cfg : public cfg_integral_base<double_cfg, double>
{
};

class value_list_config : private helpers::no_copy_move
{
public:
	/**
	 * Configures the list to contain integer arguments.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the container where the parsed arguments should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	int_cfg & int_val(std::string const & name, std::vector<int> * dest)
	{
		return custom_val<int_cfg>(name, dest);
	}

	/**
	 * Configures the list to contain string arguments.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the container where the parsed arguments should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	string_cfg & string_val(std::string const & name, std::vector<std::string> * dest)
	{
		return custom_val<string_cfg>(name, dest);
	}

	/**
	 * Configures the list to accept a floating point number arguments.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the container where the parsed arguments should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	double_cfg & double_val(std::string const & name, std::vector<double> * dest)
	{
		return custom_val<double_cfg>(name, dest);
	}

	/**
	 * Configures the option or argument to accept a parameter of custom type.
	 * \tparam     TConfig  Type used to configure the parameter fulfilling following requirements
	 *    (most of which can be fulfilled by publicly deriving from argpar::cfg_base<TConfig, TValue>):
	 *  - public parameterless constructor
	 *  - define TConfig::value_type to be the target type of the value. The type must be
	 *    movable or copyable.
	 *  - define TConfig::container to be the type used to store the values.
	 *  - public instance method TConfig::value_type TConfig::parse(std::string const &) const, which
	 *    parses the given string parameter and returns an instance of the target type. This method
	 *    should throw argpar::format_error to indicate that incompatible option parameter was used.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns a reference to an internally constructed TConfig object for further
	 * configuration.
	 */
	template<typename TConfig>
	TConfig & custom_val(std::string const & name, typename TConfig::container * dest)
	{
		if (name.empty()) throw std::invalid_argument("Name cannot be empty");
		if (dest == nullptr) throw std::invalid_argument("Destination pointer cannot be nullptr");
		if (this->handler_) throw std::logic_error("A value has already been configured");
		std::unique_ptr<detail::multi_value_handler<TConfig>> handler = std::make_unique<detail::multi_value_handler<TConfig>>(name, dest);
		TConfig * ret = &handler->get_config();
		this->handler_ = std::move(handler);
		return *ret;
	}

protected:
	//! Pointer to the value handler, nullptr if no value configured (Option is only a flag).
	std::unique_ptr<detail::value_handler> handler_;
};

/*
 * Interface for configuring the parameter of the command line option. If no method on this class is
 * called, then it is assumed that the option has no parameter.
 */
class value_config : private helpers::no_copy_move
{
public:
	/**
	 * Configures the option or argument to accept an integer parameter.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	int_cfg & int_val(std::string const & name, int * dest)
	{
		return custom_val<int_cfg>(name, dest);
	}

	/**
	 * Configures the option or argument to accept a string parameter.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	string_cfg & string_val(std::string const & name, std::string * dest)
	{
		return custom_val<string_cfg>(name, dest);
	}

	/**
	 * Configures the option or argument to accept a floating point number parameter.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	double_cfg & double_val(std::string const & name, double * dest)
	{
		return custom_val<double_cfg>(name, dest);
	}

	/**
	 * Configures the option or argument to accept a parameter of custom type.
	 * \tparam     TConfig  Type used to configure the parameter fulfilling following requirements
	 *    (most of which can be fulfilled by publicly deriving from argpar::cfg_base<TConfig, TValue>):
	 *  - public parameterless constructor
	 *  - define TConfig::value_type to be the target type of the value. The type must be
	 *    movable or copyable.
	 *  - public instance method TConfig::value_type TConfig::parse(std::string const &) const, which
	 *    parses the given string parameter and returns an instance of the target type. This method
	 *    should throw argpar::format_error to indicate that incompatible option parameter was used.
	 *    public instance method std::optional<TConfig::value_type> TConfig::default() const, which
	 *    returns a default instance or std::nullopt if no default is to be provided. The default
	 *    will be requested during the argpar::parser::parse call only.
	 *  - public instance method bool TConfig::has_default() const, which returns whether the
	 *    configured parameter/option has default value configured
	 *  - public instance method TConfig::value_type get_default() const, which returns the default if
	 *    the above method returns true.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns a reference to an internally constructed TConfig object for further
	 * configuration of the parameter.
	 */
	template<typename TConfig>
	TConfig & custom_val(std::string const & name, typename TConfig::value_type * dest)
	{
		if (name.empty()) throw std::invalid_argument("Name cannot be empty");
		if (dest == nullptr) throw std::invalid_argument("Destination pointer cannot be nullptr");
		if (this->handler_) throw std::logic_error("A value has already been configured");
		std::unique_ptr<detail::single_value_handler<TConfig>> handler = std::make_unique<detail::single_value_handler<TConfig>>(name, dest);
		TConfig * ret = &handler->get_config();
		this->handler_ = std::move(handler);
		return *ret;
	}
protected:
	//! Pointer to the value handler, nullptr if no value configured (Option is only a flag).
	std::unique_ptr<detail::value_handler> handler_;
};

}
#endif // ARGPAR_VALUE_CONFIG_H
