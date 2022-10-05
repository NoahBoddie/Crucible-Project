#pragma once

#include "SerialComponent.h"


namespace Crucible
{
	template <class State>
	struct StatedData
	{
		bool _isLocked;
		State _currentState;
		State _previousState;
		std::optional<State> _releaseState;//If specified, this stateObj will change to this state when
	};

	template<class State>
	class StatedObject : public SerialComponent<StatedData<State>>
	{
	public:
		static_assert(std::is_enum<State>::value || std::is_integral<State>::value);
		
		bool IsLocked() { return this->data._isLocked; }
		bool SetLockState(bool lockActive)
		{
			//May not need to lock it mid transit
			if (_isInTransit)
				return false;

			if (lockActive)
			{
				if (this->data._isLocked)
					return false;

				this->data._isLocked = true;
			}
			else
			{
				if (!this->data._isLocked)
					return false;

				this->data._isLocked = false;

				if (this->data._releaseState)
				{
					GoToState(*this->data._releaseState);
					this->data._releaseState = {};
				}
					
			}

			return true;
		}
		void ClearReleaseState()
		{
			this->data._releaseState = std::nullopt_t;
		}

		State GetCurrentState() { return this->data._currentState; }
		State GetPreviousState() { return this->data._previousState; }
		std::optional<State> GetGoingToState() { return _goingToState; }
		std::optional<State> GetReleaseState() { return this->data._releaseState; }

		State GoToState(State newState, bool procOnRelease = false)
		{
			if (this->data._isLocked)
			{
				if (procOnRelease)
					this->data._releaseState = newState;

				//Utility::DebugNotification(std::string("No: is locked").c_str());
				return this->data._currentState;
			}
			else if (newState == this->data._currentState) {
				//Utility::DebugNotification(std::format("No: is same state {} vs {}", (int)newState, (int)this->data._currentState).c_str());
				return newState;
			}
			else if (_isInTransit) {
				//Utility::DebugNotification(std::string("No: In transit").c_str());
				return this->data._currentState;
			}
			bool wasInTransit = _isInTransit;

			_isInTransit = true;
			_goingToState = newState;
			OnStateFinish();
			_isInTransit = wasInTransit;
			
			if (!_goingToState || _goingToState.value() == this->data._currentState) {
				//Utility::DebugNotification(std::string("No: Going state later deactivated").c_str());
				return this->data._currentState;//Should probably report when this happens
			}
			this->data._previousState = this->data._currentState;
			this->data._currentState = _goingToState.value();
			_goingToState = std::nullopt;
			OnStateBegin();//Because of this, be careful switching states on beginnings.
			//Utility::DebugNotification(std::string("finish?").c_str());

			return this->data._currentState;//Returns the current state for notification if it ended successfully.
		}

	protected:
		
		virtual void OnStateBegin() {}
		virtual void OnStateFinish() {}

		void SetGoingState(State newState)
		{
			if (!_isInTransit)
				return;

			_goingToState = newState;
		}

		inline void SetGoingToState(State newState) { SetGoingState(newState); }

	private:
		bool _isInTransit;
		std::optional<State> _goingToState;
	};

}