#pragma once
#include <type_traits>
#include <chrono>
#include <boost/chrono.hpp>
#include <QtCore/QDateTime>

namespace QtTools
{
	/// тут ряд функций для преобразования unix time в julian date,
	/// в форматах std::chrono, boost::chrono, QDateTime / double, QDate.

	/// unix time в QDate julian day.
	/// QDate хранит julian day как qint64.
	/// QDATE_JULIAN_DAY_FOR_UNIX_EPOCH должно быть равно 2440588
	const qint64 QDATE_JULIAN_DAY_FOR_UNIX_EPOCH = QDate(1970, 1, 1).toJulianDay();
	
	/// https://en.wikipedia.org/wiki/Julian_day
	/// 2440587.5
	/// разница в .5 вызвана в отчете точки начала дня:
	/// для julian day это полдень, для unix time это полночь
	const double JULIAN_DAY_FOR_UNIX_EPOCH = 2440587.5;

	/************************************************************************/
	/*                   julian / chrono -> QDate                           */
	/************************************************************************/
	/// QDate по факту обертка вокруг qint64 - in/out параметры не нужны, достаточно return формы
	inline QDate ToQDate(std::chrono::system_clock::time_point point)
	{
		auto days = std::chrono::duration_cast<std::chrono::hours>(point.time_since_epoch()).count() / 24;
		return QDate::fromJulianDay(days + QDATE_JULIAN_DAY_FOR_UNIX_EPOCH);
	}

	inline QDate ToQDate(boost::chrono::system_clock::time_point point)
	{
		auto days = boost::chrono::duration_cast<boost::chrono::hours>(point.time_since_epoch()).count() / 24;
		return QDate::fromJulianDay(days + QDATE_JULIAN_DAY_FOR_UNIX_EPOCH);
	}

	inline QDate ToQDate(double jd)
	{
		/// julian date: день начинается в полдень
		/// но QDate считает только дни(qint64), а unix эпоха считается как 2440588, вместо 2440587.5.
		/// округляем что бы быть консистентными. Возможно лишнее
		return QDate::fromJulianDay(std::round(jd));
	}

	/************************************************************************/
	/*                   julian / chrono -> QDateTime                       */
	/************************************************************************/
	/// QDateTime - qt shared data class, который создается на куче.
	/// предоставляем 2 версии сигнатур с in/out параметром и return формой
	inline void ToQDateTime(std::chrono::system_clock::time_point point, QDateTime & dt)
	{
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(point.time_since_epoch());
		dt.setMSecsSinceEpoch(msec.count());
	}
	
	inline void ToQDateTime(boost::chrono::system_clock::time_point point, QDateTime & dt)
	{
		auto msec = boost::chrono::duration_cast<boost::chrono::milliseconds>(point.time_since_epoch());
		dt.setMSecsSinceEpoch(msec.count());
	}

	inline void ToQDateTime(double jd, QDateTime & dt)
	{
		/// https://en.wikipedia.org/wiki/Julian_day
		/// (JD − 2440587.5) × 86400
		auto msec = static_cast<qint64>((jd - JULIAN_DAY_FOR_UNIX_EPOCH) * 24 * 60 * 60 * 1000);
		dt.setMSecsSinceEpoch(msec);
	}

	inline QDateTime ToQDateTime(std::chrono::system_clock::time_point point)
	{
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(point.time_since_epoch());
		return QDateTime::fromMSecsSinceEpoch(msec.count());
	}

	inline QDateTime ToQDateTime(boost::chrono::system_clock::time_point point)
	{
		auto msec = boost::chrono::duration_cast<boost::chrono::milliseconds>(point.time_since_epoch());
		return QDateTime::fromMSecsSinceEpoch(msec.count());
	}

	inline QDateTime ToQDateTime(double jd)
	{
		/// https://en.wikipedia.org/wiki/Julian_day
		/// (JD − 2440587.5) × 86400
		auto msec = static_cast<qint64>((jd - JULIAN_DAY_FOR_UNIX_EPOCH) * 24 * 60 * 60 * 1000);
		return QDateTime::fromMSecsSinceEpoch(msec);
	}

	/************************************************************************/
	/*                  chrono / QDate/QDateTime -> julian day              */
	/************************************************************************/
	inline double ToJulianDay(std::chrono::system_clock::time_point point)
	{
		auto days = std::chrono::duration_cast<std::chrono::hours>(point.time_since_epoch()).count() / 24;
		return days + QDATE_JULIAN_DAY_FOR_UNIX_EPOCH;
	}

	inline double ToJulianDay(boost::chrono::system_clock::time_point point)
	{
		auto days = boost::chrono::duration_cast<boost::chrono::hours>(point.time_since_epoch()).count() / 24;
		return days + QDATE_JULIAN_DAY_FOR_UNIX_EPOCH;
	}

	inline double ToJulianDay(const QDate & date)         { return date.toJulianDay(); }
	inline double ToJulianDay(const QDateTime & date)     { return ToJulianDay(date.date()); }

	/************************************************************************/
	/*          QDate/QDateTime / julian day -> chrono                      */
	/************************************************************************/
	inline std::chrono::system_clock::time_point ToStdChrono(double jd)
	{
		/// https://en.wikipedia.org/wiki/Julian_day
		/// (JD − 2440587.5) × 86400
		auto unix_time = static_cast<qint64>((jd - JULIAN_DAY_FOR_UNIX_EPOCH) * 24 * 60 * 60);
		return std::chrono::system_clock::from_time_t(unix_time);
	}

	inline std::chrono::system_clock::time_point ToStdChrono(QDate date)
	{
		/// https://en.wikipedia.org/wiki/Julian_day
		/// (JD − 2440587.5) × 86400
		auto unix_time = (date.toJulianDay() - QDATE_JULIAN_DAY_FOR_UNIX_EPOCH) * 24 * 60 * 60;
		return std::chrono::system_clock::from_time_t(unix_time);
	}

	inline std::chrono::system_clock::time_point ToStdChrono(const QDateTime & dt)
	{
		return std::chrono::system_clock::from_time_t(dt.toMSecsSinceEpoch() / 1000);
	}

	inline boost::chrono::system_clock::time_point ToBoostChrono(double jd)
	{
		/// https://en.wikipedia.org/wiki/Julian_day
		/// (JD − 2440587.5) × 86400
		auto unix_time = static_cast<qint64>((jd - JULIAN_DAY_FOR_UNIX_EPOCH) * 24 * 60 * 60);
		return boost::chrono::system_clock::from_time_t(unix_time);
	}

	inline boost::chrono::system_clock::time_point ToBoostChrono(QDate date)
	{
		/// https://en.wikipedia.org/wiki/Julian_day
		/// (JD − 2440587.5) × 86400
		auto unix_time = (date.toJulianDay() - QDATE_JULIAN_DAY_FOR_UNIX_EPOCH) * 24 * 60 * 60;
		return boost::chrono::system_clock::from_time_t(unix_time);
	}

	inline boost::chrono::system_clock::time_point ToBoostChrono(const QDateTime & dt)
	{
		return boost::chrono::system_clock::from_time_t(dt.toMSecsSinceEpoch() / 1000);
	}
}

