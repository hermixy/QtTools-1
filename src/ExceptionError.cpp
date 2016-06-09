#include <QtTools/ExceptionError.hpp>

namespace QtTools
{
	struct BoostExceptionCategory :
		std::error_category,
		boost::system::error_category
	{
		virtual const char * name() const BOOST_SYSTEM_NOEXCEPT
		{
			return "cpp_exception";
		}

		virtual std::string message(int ev) const
		{
			switch (static_cast<ExceptionError>(ev))
			{
			case ExceptionError::RuntimeError:
				return "std::runtime_error";

			default:
				return "unkown_exception";
			}
		}
	};

	static const BoostExceptionCategory CatInstance;

	const boost::system::error_category & boost_exception_category()
	{
		return CatInstance;
	}

	const std::error_category & exception_category()
	{
		return CatInstance;
	}
}