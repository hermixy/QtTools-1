#pragma once
#include <QtTools/ConnectionController.hpp>
#include <boost/config.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/mutex.hpp>

namespace QtTools
{
	/// Реализация ConnectionController автомата(на switch'ах),
	/// с делегированием запросов наследнику по средствам virtual methods
	///
	/// протокол работы с сервером:
	///   * контроллер инициирует запрос на подключение с помощью do_ConnectRequest
	///     когда подключение выполнено, серверная часть уведомляет с помощью вызова NotifyConnected
	///   * контроллер инициирует запрос на отключение с помощью do_DisconnectRequest
	///     когда отключение выполнено, серверная часть уведомляет с помощью вызова NotifyDisconnected
	///   * при потери соединения серверная часть уведомляет с помощью NotifyDisconnected
	///   причина отключения хранится в серверной части и является специфичной для типа подключения
	///   
	/// NOTE: ConnectionController по факту является интерфейсом.
	///       Что бы поддерживать параллельные иерархии интерфейсов, реализаций - virtual наследование
	class AbstractConnectionController : public virtual ConnectionController
	{
	protected:
		typedef boost::mutex mutex_type;
		typedef boost::unique_lock<mutex_type> unique_lock;

		typedef boost::signals2::signal< ConnectedSlot::signature_type       > ConnectedSig;
		typedef boost::signals2::signal< DisconnectedSlot::signature_type    > DisconnectedSig;
		typedef boost::signals2::signal< ConnectionErrorSlot::signature_type > ConnectionErrorSig;
		typedef boost::signals2::signal< ConnectionLostSlot::signature_type  > ConnectionLostSig;

		mutex_type m_mutex;           /// мьютекс защищающий автомат, может быть использован наследником
	
	private:
		/// state private, наследник не должен влиять
		StateType m_state = Offline;  /// состояние автомата
	
	protected:
		/// сигналы
		ConnectedSig m_connectedSignal;
		DisconnectedSig m_disconnectedSignal;
		ConnectionErrorSig m_connectionErrorSignal;
		ConnectionLostSig m_connectionLostSignal;
		
	protected:
		/// обработчик ситуации BadTransactionRequest(смотри описание ConnectionController)
		/// реализация по-умолчанию кидает std::logic_error
		virtual void BOOST_NORETURN OnBadTransaction();
		/// обработчик ситуации BadConnectRequest(смотри описание ConnectionController)
		/// реализация по-умолчанию кидает std::logic_error
		virtual void BOOST_NORETURN OnBadConnectRequest();

		/// реализация запроса на подключение, передается лок автомата.
		/// состояние уже переведено.
		/// реализация может манипулировать локом как угодно,
		/// вызывающему лок более не требуется, впрочем он будет снят автоматически, если еще не снят
		virtual void do_ConnectRequest(unique_lock & lk) = 0;
		/// реализация запроса на отключение, передается лок автомата.
		/// состояние уже переведено.
		/// реализация может манипулировать локом как угодно,
		/// вызывающему лок более не требуется, впрочем он будет снят автоматически, если еще не снят
		virtual void do_DisconnectRequest(unique_lock & lk) = 0;

		/// Derived класс уведомляет с помощью этого метода об успешном подключении. ThreadSafe.
		/// На вход должен быть передан захваченный лок (lk.owns_lock() == true)
		/// в процессе снимает лок(перед вызовом сигналов)
		void NotifyConnected(unique_lock & lk);
		/// Derived класс уведомляет с помощью этого метода об отключении/потери соединения. ThreadSafe.
		/// На вход должен быть передан захваченный лок (lk.owns_lock() == true)
		/// в процессе снимает лок(перед вызовом сигналов)
		void NotifyDisconnected(unique_lock & lk);

		/// реализация автомата для события Connect.
		/// На вход должен быть передан захваченный лок (lk.owns_lock() == true)
		void DoConnect(unique_lock & lk);
		/// реализация автомата для события Disconnect.
		/// На вход должен быть передан захваченный лок (lk.owns_lock() == true)
		void DoDisconnect(unique_lock & lk);
		/// возвращает текущее состояние.
		/// На вход должен быть передан захваченный лок (lk.owns_lock() == true)
		StateType GetState(unique_lock & lk) { return  m_state; }

	public:
		/// текущее состояние
		StateType GetState() override final;
		/// выполнить попытку подключения
		void Connect() override final;
		/// выполнить попытку отключения
		void Disconnect() override final;

		boost::signals2::connection OnConnected(const ConnectedSlot & slot) override
		{ return m_connectedSignal.connect(slot); }
		boost::signals2::connection OnDisconnected(const DisconnectedSlot & slot) override
		{ return m_disconnectedSignal.connect(slot); }
		boost::signals2::connection OnConnectionLost(const ConnectionLostSlot & slot) override
		{ return m_connectionLostSignal.connect(slot); }
		boost::signals2::connection OnConnectionError(const ConnectionErrorSlot & slot) override
		{ return m_connectionErrorSignal.connect(slot); }
	};
}
