#ifndef NL_DEBUG_HPP
#define NL_DEBUG_HPP


// Macro pour activer un mode de debuggage pousse
#ifdef _DEBUG
#define NL_DEBUG
#endif // _DEBUG

enum DebugInterrupt {
	IndexOutOfRange,
	BadSizeArgument
};

void _nl_debug_break(enum DebugInterrupt di, const char* message);
void _nl_alloc_test(void* pointer, const char* str);



#ifdef NL_DEBUG

#define debug_break(di, str) _nl_debug_break(di, str)
#define alloc_test(ptr, str) _nl_alloc_test(ptr, str)

#else

#define debug_break(di, str)
#define alloc_test(ptr, str)

#endif // NL_DEBUG




#endif