#include <QtTools/GuiQueue.hqt>

namespace QtTools
{
	auto GuiQueue::Take() -> action_list
	{
		decltype(m_actions) actions;
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			actions = std::move(m_actions);
			m_shouldEmit.store(true, std::memory_order_relaxed);
		}

		return actions;
	}

	void GuiQueue::AddAll(action_list actions)
	{
		bool should_emit;
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			m_actions.splice(m_actions.end(), actions);
			should_emit = m_shouldEmit.exchange(false, std::memory_order_relaxed);
		}

		if (should_emit) EmitActionsAvailiable();
	}

	void GuiQueue::Add(action_type action)
	{	
		action_list actions;
		actions.push_back(action);
		AddAll(std::move(actions));
	}

	void GuiQueue::EmitAccamulatedActions()
	{
		auto actions = Take();
		for (auto & action : actions)
			action();
	}

	GuiQueue::GuiQueue(QObject * parent)
		: QObject(parent)
	{
		connect(this, &GuiQueue::EmitActionsAvailiable,
				this, &GuiQueue::EmitAccamulatedActions,
				Qt::QueuedConnection);
	}

	GuiQueue::~GuiQueue() = default;
}
