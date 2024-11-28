#pragma once

namespace Services {
	extern bool HasRan;

	void WorkerThread();
	void CreatePresence();
	HRESULT Init();
}
