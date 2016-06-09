#pragma once
#include <boost/config.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/signals2.hpp>

namespace viewed
{
	/// This class describes signal types used by container and views
	/// container holds and emits signals, view - connects to them and synchronize it's state to container
	/// This is a default interface. Your container's and views can use different signals and signal_range_type if they want,
	/// of course they probable will not able to use views/containers with different signal interface
	template <class Type>
	struct signal_types
	{
	protected:
		/// right now containers and views are only used from GUI thread - mutex not needed
		typedef boost::signals2::dummy_mutex signal_mutex_type;

	public:
		/// random access range of valid pointers(at least at moment of call) to value_type
		typedef boost::iterator_range<const Type **> signal_range_type;

		/// signal is emitted after new data is upserted into container, with 2 ranges of pointers. 
		///  * 1st points to elements that were updated, sorted by pointer value
		///  * 2nd to newly inserted, order is unspecified
		typedef typename boost::signals2::signal_type<
			void(signal_range_type updated, signal_range_type inserted), //updated will be sorted
			boost::signals2::keywords::mutex_type<signal_mutex_type>
		>::type upsert_signal_type;

		/// signal is emitted before data is erased from container, 
		/// with range of pointers to elements to erase, sorted by pointer value
		typedef typename boost::signals2::signal_type<
			void(signal_range_type), 
			boost::signals2::keywords::mutex_type<signal_mutex_type>
		>::type erase_signal_type;

		/// signal is emitted before container is cleared.
		typedef typename boost::signals2::signal_type<
			void(),
			boost::signals2::keywords::mutex_type<signal_mutex_type>
		>::type clear_signal_type;
	};
	
}
