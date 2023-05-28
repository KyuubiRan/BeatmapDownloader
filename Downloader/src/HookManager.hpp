#pragma once

#include <map>
#include "detours.h"

class HookManager {
public:
	template <typename Fn>
	static void InstallHook(Fn original, Fn handler) {
		Enable(original, handler);
		s_FunctionMap[reinterpret_cast<void *>(handler)] = reinterpret_cast<void *>(original);
	}

	template <typename Fn>
	static void UninstallHook(Fn handler) {
		Disable(handler);
		s_FunctionMap.erase(reinterpret_cast<void *>(handler));
	}

	template <typename Fn>
	static Fn GetOriginal(Fn handler) {
		if (s_FunctionMap.count(reinterpret_cast<void *>(handler)) == 0) {
			LOGE("No such original function found!");
			return nullptr;
		}
		return reinterpret_cast<Fn>(s_FunctionMap[reinterpret_cast<void *>(handler)]);
	}

	static void UninstallAll() noexcept {
		for (const auto &[k, v] : s_FunctionMap) {
			Disable(k);
		}
		s_FunctionMap.clear();
	}

	template <typename ReturnType, typename... Params>
	static ReturnType CallOriginal(ReturnType(*handler)(Params...), Params... params) {
		auto original = GetOriginal(handler);
		return original ? original(params...) : ReturnType{};
	}

#ifndef _WIN64
	template <typename ReturnType, typename... Params>
	static ReturnType CallOriginal(ReturnType(__fastcall *handler)(Params...), Params... params) {
		auto original = GetOriginal(handler);
		return original ? original(params...) : ReturnType{};
	}

	template <typename ReturnType, typename... Params>
	static ReturnType CallOriginal(ReturnType(__stdcall *handler)(Params...), Params... params) {
		auto original = GetOriginal(handler);
		return original ? original(params...) : ReturnType{};
	}
#endif 

private:
	HookManager() = default;

	inline static std::map<void *, void *> s_FunctionMap{};

	template <typename Fn>
	static void Enable(Fn &original, Fn handler) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID &)original, (PVOID)handler);
		DetourTransactionCommit();
	}

	template <typename Fn>
	static void Disable(Fn handler) {
		Fn original = GetOriginal(handler);
		if (!original) {
			return;
		}

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID &)original, handler);
		DetourTransactionCommit();
	}
};
