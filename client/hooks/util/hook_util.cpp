#include "common.hpp"
#include "game/game.hpp"
#include "hooks/util/hook_util.hpp"

#include <utility/nt.hpp>

namespace Client::Hook {
	void* Util::InitDbgHelp() {
		static class InitDbgHelp_ {
		public:
			InitDbgHelp_() {
				SymInitialize(GetCurrentProcess(), nullptr, FALSE);
			}

			~InitDbgHelp_() {
				SymCleanup(GetCurrentProcess());
			}
		} initDbgHelp;
		return &initDbgHelp;
	}

	LONG WINAPI Util::TopLevelExceptionFilter(EXCEPTION_POINTERS* inf) {
		(void)InitDbgHelp();

		const char* exceptionCodeName = "<unknown>";
		switch (inf->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
			exceptionCodeName = "Access violation";
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			exceptionCodeName = "Array bounds exceeded";
			break;
		case EXCEPTION_BREAKPOINT:
			exceptionCodeName = "Breakpoint";
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			exceptionCodeName = "Data type misalignment";
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			exceptionCodeName = "Float denormal operand";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			exceptionCodeName = "Float / 0";
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			exceptionCodeName = "Float inexact result";
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			exceptionCodeName = "Float invalid operation";
			break;
		case EXCEPTION_FLT_OVERFLOW:
			exceptionCodeName = "Float overflow";
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			exceptionCodeName = "Float stack check";
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			exceptionCodeName = "Float underflow";
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			exceptionCodeName = "Illegal instruction";
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			exceptionCodeName = "In-page error";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			exceptionCodeName = "Int / 0";
			break;
		case EXCEPTION_INT_OVERFLOW:
			exceptionCodeName = "Integer overflow";
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			exceptionCodeName = "Invalid disposition";
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			exceptionCodeName = "Non-continuable exception";
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			exceptionCodeName = "Private instruction";
			break;
		case EXCEPTION_SINGLE_STEP:
			exceptionCodeName = "Single step";
			break;
		case EXCEPTION_STACK_OVERFLOW:
			exceptionCodeName = "Stack overflow";
			break;
		}

		Common::Utility::NT::Library gameLib = Common::Utility::NT::Library();
		Common::Utility::NT::Library modLib = Common::Utility::NT::Library(Client::g_Module);

		std::string message = "Unhandled exception caught. Instead of giving you absolutely nothing:\n";
		message += std::format("{} region: 0x{:08X} -> 0x{:08X}\n", gameLib.GetName(),
			reinterpret_cast<std::size_t>(gameLib.GetPtr()), reinterpret_cast<std::size_t>(gameLib.GetPtr()) + gameLib.GetOptionalHeader()->SizeOfImage);
		message += std::format("{} region: 0x{:08X} -> 0x{:08X}\n", modLib.GetName(),
			reinterpret_cast<std::size_t>(modLib.GetPtr()), reinterpret_cast<std::size_t>(modLib.GetPtr()) + modLib.GetOptionalHeader()->SizeOfImage);
		message += "---- Exception Record ----\n";
		message += std::format("Exception code: {} (0x{:08X})\n", exceptionCodeName, inf->ExceptionRecord->ExceptionCode);
		message += std::format("Exception flags: 0x{:08X}\n", inf->ExceptionRecord->ExceptionFlags);
		message += std::format("Exception address: {}\n", AndRel(reinterpret_cast<std::size_t>(inf->ExceptionRecord->ExceptionAddress)));
		message += std::format("Number of parameters: {}\n", inf->ExceptionRecord->NumberParameters);
		for (auto x = 0ul; x < std::min(inf->ExceptionRecord->NumberParameters, 15ul /*EXCEPTION_MAXIMUM_PARAMETERS*/); x++) {
			message += std::format(" > Parameter {}: {}\n", x, AndRel(inf->ExceptionRecord->ExceptionInformation[x]));
		}
		message += "----- Context Record -----\n";
		message += std::format("P1Home: {}\n", AndRel(inf->ContextRecord->P1Home));
		message += std::format("P2Home: {}\n", AndRel(inf->ContextRecord->P2Home));
		message += std::format("P3Home: {}\n", AndRel(inf->ContextRecord->P3Home));
		message += std::format("P4Home: {}\n", AndRel(inf->ContextRecord->P4Home));
		message += std::format("P5Home: {}\n", AndRel(inf->ContextRecord->P5Home));
		message += std::format("P6Home: {}\n", AndRel(inf->ContextRecord->P6Home));

		message += std::format("Context flags: 0x{:08X}\n", inf->ContextRecord->ContextFlags);
		message += std::format("MxCsr: 0x{:08X}\n", inf->ContextRecord->MxCsr);

		message += std::format("SegCs: 0x{:04X}\n", inf->ContextRecord->SegCs);
		message += std::format("SegDs: 0x{:04X}\n", inf->ContextRecord->SegDs);
		message += std::format("SegEs: 0x{:04X}\n", inf->ContextRecord->SegEs);
		message += std::format("SegFs: 0x{:04X}\n", inf->ContextRecord->SegFs);
		message += std::format("SegGs: 0x{:04X}\n", inf->ContextRecord->SegGs);
		message += std::format("SegSs: 0x{:04X}\n", inf->ContextRecord->SegSs);
		message += std::format("EFlags: 0x{:08X}\n", inf->ContextRecord->EFlags);

		message += std::format("Dr0: {}\n", AndRel(inf->ContextRecord->Dr0));
		message += std::format("Dr1: {}\n", AndRel(inf->ContextRecord->Dr1));
		message += std::format("Dr2: {}\n", AndRel(inf->ContextRecord->Dr2));
		message += std::format("Dr3: {}\n", AndRel(inf->ContextRecord->Dr3));
		message += std::format("Dr6: {}\n", AndRel(inf->ContextRecord->Dr6));
		message += std::format("Dr7: {}\n", AndRel(inf->ContextRecord->Dr7));

		message += std::format("Rax: {}\n", AndRel(inf->ContextRecord->Rax));
		message += std::format("Rcx: {}\n", AndRel(inf->ContextRecord->Rcx));
		message += std::format("Rdx: {}\n", AndRel(inf->ContextRecord->Rdx));
		message += std::format("Rbx: {}\n", AndRel(inf->ContextRecord->Rbx));
		message += std::format("Rsp: {}\n", AndRel(inf->ContextRecord->Rsp));
		message += std::format("Rbp: {}\n", AndRel(inf->ContextRecord->Rbp));
		message += std::format("Rsi: {}\n", AndRel(inf->ContextRecord->Rsi));
		message += std::format("Rdi: {}\n", AndRel(inf->ContextRecord->Rdi));
		message += std::format("R8: {}\n", AndRel(inf->ContextRecord->R8));
		message += std::format("R9: {}\n", AndRel(inf->ContextRecord->R9));
		message += std::format("R10: {}\n", AndRel(inf->ContextRecord->R10));
		message += std::format("R11: {}\n", AndRel(inf->ContextRecord->R11));
		message += std::format("R12: {}\n", AndRel(inf->ContextRecord->R12));
		message += std::format("R13: {}\n", AndRel(inf->ContextRecord->R13));
		message += std::format("R14: {}\n", AndRel(inf->ContextRecord->R14));
		message += std::format("R15: {}\n", AndRel(inf->ContextRecord->R15));

		message += std::format("Rip: {}\n", AndRel(inf->ContextRecord->Rip));

		message += std::format("VectorControl: {}\n", AndRel(inf->ContextRecord->VectorControl));

		message += std::format("DebugControl: {}\n", AndRel(inf->ContextRecord->DebugControl));
		message += std::format("LastBranchToRip: {}\n", AndRel(inf->ContextRecord->LastBranchToRip));
		message += std::format("LastBranchFromRip: {}\n", AndRel(inf->ContextRecord->LastBranchFromRip));
		message += std::format("LastExceptionToRip: {}\n", AndRel(inf->ContextRecord->LastExceptionToRip));
		message += std::format("LastExceptionFromRip: {}\n", AndRel(inf->ContextRecord->LastExceptionFromRip));

		message += "------- Other Info -------\n";
		STACKFRAME64 stackFrame = { 0 };
		stackFrame.AddrPC.Offset = inf->ContextRecord->Rip;
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = inf->ContextRecord->Rbp;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = inf->ContextRecord->Rsp;
		stackFrame.AddrStack.Mode = AddrModeFlat;

		std::size_t functionCount = 0;
		while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(),
			&stackFrame, inf->ContextRecord, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			functionCount++;
			message += std::format("Function address [{}]: {}\n", functionCount, AndRel(stackFrame.AddrPC.Offset));
		}

		MessageBoxA(nullptr, message.c_str(), "t9-mod exception handler", MB_OK);
		return EXCEPTION_EXECUTE_HANDLER;
	}
}
