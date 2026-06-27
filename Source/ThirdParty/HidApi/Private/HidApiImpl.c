/*
 * Single compiled translation unit for the vendored hidapi backend.
 *
 * The per-platform implementation files are kept with a .inl extension so that
 * UnrealBuildTool does not compile each of them on its own (which would define
 * the same hid_* symbols three times). Only the backend matching the current
 * platform is pulled in here.
 */

#if defined(_WIN32)
	#include "windows/hid_windows.inl"
#elif defined(__APPLE__)
	#include "mac/hid_mac.inl"
#elif defined(__linux__)
	#include "linux/hid_linux.inl"
#endif
