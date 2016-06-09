#include <QtTools/DelayedExecutionTimer.hqt>

namespace QtTools
{
	DelayedExecutionTimer::DelayedExecutionTimer(int minimumTimeout, int maximumTimeout, QObject * parent/* = nullptr*/)
		: QObject(parent)
	{
		m_minimumTimeout = minimumTimeout;
		m_maximumTimeout = maximumTimeout;

		connect(m_minimumTimer, &QTimer::timeout, this, &DelayedExecutionTimer::OnTimeout);
		connect(m_maximumTimer, &QTimer::timeout, this, &DelayedExecutionTimer::OnTimeout);
	}

	void DelayedExecutionTimer::OnTriggered()
	{
		if (!m_maximumTimer->isActive() && m_maximumTimeout > 0)
			m_maximumTimer->start(m_maximumTimeout);

		if (m_minimumTimeout > 0)
		{
			m_minimumTimer->stop();
			m_minimumTimer->start(m_minimumTimeout);
		}
		else {
			Q_EMIT Trigger();
		}
	}

	void DelayedExecutionTimer::OnTimeout()
	{
		m_minimumTimer->stop();
		m_minimumTimer->stop();

		Q_EMIT Trigger();
	}
}
