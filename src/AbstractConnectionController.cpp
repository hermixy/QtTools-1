#include <QtTools/AbstractConnectionController.hpp>

namespace QtTools
{
	BOOST_NORETURN void AbstractConnectionController::OnBadConnectRequest()
	{
		throw std::logic_error("AbstractConnectionController: bad connect request");
	}

	BOOST_NORETURN void AbstractConnectionController::OnBadTransaction()
	{
		throw std::logic_error("AbstractConnectionController: bad transition");
	}

	void AbstractConnectionController::DoConnect(unique_lock & lk)
	{
		assert(lk.owns_lock());
		switch (m_state)
		{
		case Disconnecting:
			/// запрос на подключение, при еще не отработанном запросе на отключение. это ошибка
			OnBadConnectRequest();

		case Offline:
			m_state = Connecting;
			do_ConnectRequest(lk);
			return;

		case Online:
		case Connecting:
			/// подключение уже выполняется или выполнено - ничего не делаем
			return;

		default:
			OnBadTransaction();
		}
	}

	void AbstractConnectionController::DoDisconnect(unique_lock & lk)
	{
		assert(lk.owns_lock());
		switch (m_state)
		{
		case Offline:
		case Disconnecting:
			return;

		case Online:
		case Connecting:
			m_state = Disconnecting;
			do_DisconnectRequest(lk);
			return;

		default:
			OnBadTransaction();
		}
	}

	void AbstractConnectionController::NotifyConnected(unique_lock & lk)
	{
		assert(lk.owns_lock());
		switch (m_state)
		{
		case Disconnecting:
			// может быть ситуация, когда был запрос на подключение, мы начали подключатся,
			// пришел запрос на отключение, он отработал, и после мы подключились.
			// просто игнорируем, мы отключимся следующим шагом
			lk.unlock();
			return;

		case Connecting:
			m_state = Online;
			lk.unlock();
			m_connectedSignal();
			return;

			// такого быть не должно
			//case Offline:
			//case Online:
		default:
			OnBadTransaction();
		}
	}

	void AbstractConnectionController::NotifyDisconnected(unique_lock & lk)
	{
		assert(lk.owns_lock());
		switch (m_state)
		{
		case Connecting:
			m_state = Offline;
			lk.unlock();
			m_disconnectedSignal();
			m_connectionErrorSignal();
			return;

		case Disconnecting:
			m_state = Offline;
			lk.unlock();
			m_disconnectedSignal();
			return;

		case Online:
			m_state = Offline;
			lk.unlock();
			m_disconnectedSignal();
			m_connectionErrorSignal();
			m_connectionLostSignal();
			return;

		// case Offline:
		default:
			OnBadTransaction();
		}
	}

	ConnectionController::StateType AbstractConnectionController::GetState()
	{
		unique_lock lk(m_mutex);
		return m_state;
	}

	void AbstractConnectionController::Connect()
	{
		unique_lock lk(m_mutex);
		DoConnect(lk);
	}

	void AbstractConnectionController::Disconnect()
	{
		unique_lock lk(m_mutex);
		DoDisconnect(lk);
	}
}
