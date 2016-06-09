#pragma once
#include <system_error>
#include <boost/system/system_error.hpp>


/// Категория ошибки - исключение, std::runtime_error и прочие(пока только std::runtime_error)
/// В ряде случаев могут возникать ошибки, для которых есть явно обозначенные коды, 
/// и они могут быть переданы как error_code.
/// 
/// Но при этом так же могут возникнуть ошибки вида std::runtime_error исключений, 
/// при этом они не имеют кода, а передать ошибку как-то нужно. 
/// Что бы не вводить еще один еще один флажок - мы вводим error_condition идентифицирующий исключения
/// но само исключение или его текст должны передаваться отдельно
namespace QtTools
{
	/// категория ошибки - исключения
	const boost::system::error_category &  boost_exception_category();
	const std::error_category           &  exception_category();

	enum class ExceptionError
	{
		RuntimeError /// std::runtime_error derived exception
	};
}

namespace std
{
	inline std::error_code make_error_code(QtTools::ExceptionError ev)
	{
		return std::error_code(static_cast<int>(ev), QtTools::exception_category());
	}

	template <>
	class std::is_error_code_enum<QtTools::ExceptionError> :
		public std::true_type {};
}

namespace boost {
namespace system 
{
	inline error_code make_error_code(QtTools::ExceptionError ev)
	{
		return error_code(static_cast<int>(ev), QtTools::boost_exception_category());
	}

	template <>
	class boost::system::is_error_code_enum<QtTools::ExceptionError> :
		public std::true_type {};
}}