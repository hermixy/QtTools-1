#pragma once
#include <memory>
#include <boost/signals2/slot.hpp>
#include <boost/signals2/connection.hpp>

namespace QtTools
{
	/// контроллер абстрактного соединения, thread-safe
	/// представляет соединение, работающее в отдельном потоке, текущем потоке или еще какой-то другой сущности.
	/// предоставляет методы для управления состоянием, имеет сигналы для извещения о изменении состояния.
	/// а именно:
	///   * подключение выполнено
	///   * выполнено отключение
	///   * произошла потери соединения
	///   * произошла ошибка соединения
	/// 
	/// Методы подключения/отключения по возможности должны не блокирующими, настолько насколько это возможно.
	/// (допустимо блокироваться на время не превышающее некий implementation defined лимит)
	/// действие может быть завершено тут же в методе Connect/Disconnect,
	/// а сигналам разрешено испускаться из любого потока(в том числе и этого же)
	/// 
	/// Таблица состояний:
	///     Start          Event               Next         Action
	///  +------------+------------------+-------------+------------
	///    Offline         Disconnect        Offline         none
	///    Offline         Disconnected      Offline         none
	///    Offline         Connect           Connecting      place connect request
	///    Offline         Connected         BadTransactionRequest
	///    
	///    Connecting      Connect           Connecting      none
	///    Connecting      Connected         Online          emit connected signal
	///    Connecting      Disconnect        Disconnecting   place disconnect request
	///    Connecting      Disconnected      Offline         emit signals: disconnect, connection error
	///    
	///    Online          Connect           Online          none
	///    Online          Connected         BadTransactionRequest
	///    Online          Disconnect        Disconnecting   place disconnect request
	///    Online          Disconnected      Offline         emit signals: disconnect, connection error, connection lost
	///    
	///    Disconnecting   Connect           BadConnectRequest
	///    Disconnecting   Connected         none            ignore, see below
	///    Disconnecting   Disconnect        Disconnecting   none
	///    Disconnecting   Disconnected      Offline         emit disconnect signal
	///    
	///    Может случится что соединение завершилось успешно, но в этот же момент автомат был переведен
	///    в состояние отключится - автомат должен проигнорировать такую ситуацию,
	///    но реализация должна следующим шагом отключится(в том смысл что запрос не должен быть утерян)
	///    
	///    В корректно реализованном ConnectionController BadTransactionRequest не должно случаться.
	///    Реализации вправе поступать в таких случаях implementation defined способом, но не игнорировать.
	///    Кинуть исключение, вызвать std::terminate, и.т.
	///    
	///    BadConnectRequest - это логическая ошибка использования автомата.
	///    Клиент не должен вызывать метод Connect, пока не дождется отключения.
	///    Реализация должна в таких случаях должна кидать std::logic_error derived exception
	class ConnectionController
	{
	public:
		typedef boost::signals2::slot<void()> ConnectedSlot;
		typedef boost::signals2::slot<void()> DisconnectedSlot;
		typedef boost::signals2::slot<void()> ConnectionLostSlot;
		typedef boost::signals2::slot<void()> ConnectionErrorSlot;

		enum StateType
		{
			Online,       // контроллер подключен
			Offline,      // контроллер отключен
			Connecting,   // контроллер выполняет подключение
			Disconnecting // контроллер выполняет отключение
		};

	public:
		/// текущее состояние
		virtual StateType GetState() = 0;

		/// выполнить попытку подключения
		/// @Throws std::logic_error смотри описание класса
		virtual void Connect() = 0;
		/// выполнить попытку отключения
		virtual void Disconnect() = 0;

		virtual boost::signals2::connection OnConnected(const ConnectedSlot & slot) = 0;
		virtual boost::signals2::connection OnDisconnected(const DisconnectedSlot & slot) = 0;
		virtual boost::signals2::connection OnConnectionLost(const ConnectionLostSlot & slot) = 0;
		virtual boost::signals2::connection OnConnectionError(const ConnectionErrorSlot & slot) = 0;
		
		virtual ~ConnectionController() = default;
	};
}
