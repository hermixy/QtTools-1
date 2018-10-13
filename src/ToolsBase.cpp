#include <QtTools/ToolsBase.hpp>

namespace QtTools
{
	namespace detail_qtstdstring
	{
		const std::codecvt_utf8<char16_t> _cvt;
		const std::codecvt_utf8<char16_t> & u8cvt = _cvt;

		class QStringWrapper
		{
		private:
			QString * str;

		public:
			QStringWrapper(QString & str)
				: str(&str) {}

		public:
			typedef std::size_t      size_type;
			typedef std::ptrdiff_t   difference_type;

			typedef char16_t           value_type;
			typedef const value_type * const_pointer;
			typedef value_type *       pointer;
			typedef value_type &       reference;
			typedef const value_type & const_reference;

			typedef pointer          iterator;
			typedef const_pointer    const_iterator;

			inline size_type size() const { return str->size(); }
			void   resize(size_type sz) { str->resize(qint(sz)); }

			value_type * data()             { return detail_qtstdstring::data(*str); }
			const value_type * data() const { return detail_qtstdstring::data(*str); }

			inline iterator begin() { return data(); }
			inline iterator end()   { return begin() + str->size(); }

			inline const_iterator begin() const { return data(); }
			inline const_iterator end() const   { return begin() + size(); }
		};
	}

	void QtRegisterStdString()
	{
		qRegisterMetaType<std::string>();
		QMetaType::registerComparators<std::string>();
	}

	void ToQString(const char * str, std::size_t len, QString & res)
	{
		using namespace detail_qtstdstring;
		QStringWrapper wr {res};
		auto s = boost::make_iterator_range_n(str, len);
		ext::codecvt_convert::from_bytes(u8cvt, s, wr);
	}

	void ToQString(const char * str, std::size_t len, QString & res, std::size_t maxSize, QChar truncChar /* = 0 */)
	{
		auto strEnd = str + len;
		len = std::min(maxSize, len);
		res.resize(qint(len));

		auto srcRng = boost::make_iterator_range_n(str, len);
		auto retRng = boost::make_iterator_range_n(detail_qtstdstring::data(res), res.size());

		const char * stopped_from;
		char16_t * stopped_to;
		std::tie(stopped_from, stopped_to) = ext::codecvt_convert::from_bytes(detail_qtstdstring::u8cvt, srcRng, retRng);

		auto sz = stopped_to - retRng.begin();

		/// if was truncation and truncChar is not 0
		if (truncChar != 0 && stopped_from != strEnd)
			res[qint(maxSize - 1)] = truncChar;

		res.resize(qint(sz));
	}
}

