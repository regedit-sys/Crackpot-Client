#pragma once

namespace AppManager {
	namespace Events {
		void OnTitleLoad(AppManager::C_AppInstance *pInstance);
		void OnDllLoad(AppManager::C_AppInstance *pInstance);
	}
}