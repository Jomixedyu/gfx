#pragma once
#include "GFXGlobalConfig.h"
#include "GFXExtensions.h"
#include "GFXBuffer.h"
#include "GFXDescriptor.h"

#include <functional>

namespace gfx
{
	enum class GFXApi
	{
		Unknown,
		D3D12,
		Vulkan,
	};

	class GFXApplication
	{
	public:
		using LoopEvent = std::function<void(float)>;
		//using LoopEvent = void(*)(GFXApplication*, float);
		using ExitWindowEvent = bool(*)();
	public:
		virtual void Initialize() {}
		virtual void ExecLoop() {}
		virtual void RequestStop() {}
		virtual void Terminate() {}

		const GFXGlobalConfig& GetConfig() const { return m_config; }
		virtual GFXExtensions GetExtensionNames() = 0;

		LoopEvent OnLoop = nullptr;
		ExitWindowEvent OnExitWindow = nullptr;
	public:
		virtual GFXBuffer* CreateBuffer(GFXBufferUsage usage, size_t bufferSize) = 0;
		
		virtual intptr_t GetWindowHandle() = 0;
	protected:
		GFXApplication() {}
	protected:
		GFXGlobalConfig m_config{};
	};

	GFXApplication* CreateGFXApplication(GFXApi Api, GFXGlobalConfig config);
}