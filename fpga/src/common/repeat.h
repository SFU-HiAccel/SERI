// Macros to generate repetitive code.
#define REPEAT_1( macro ) macro(0)
#define REPEAT_2( macro ) REPEAT_1(macro) macro(1)
#define REPEAT_3( macro ) REPEAT_2(macro) macro(2)
#define REPEAT_4( macro ) REPEAT_3(macro) macro(3)
#define REPEAT_5( macro ) REPEAT_4(macro) macro(4)
#define REPEAT_6( macro ) REPEAT_5(macro) macro(5)
#define REPEAT_7( macro ) REPEAT_6(macro) macro(6)
#define REPEAT_8( macro ) REPEAT_7(macro) macro(7)

#define REPEAT( macro, n ) REPEAT_(macro, n)
#define REPEAT_( macro, n ) REPEAT_##n(macro)

#define FOR_LAST_1( macro ) macro(0)
#define FOR_LAST_2( macro ) macro(1)
#define FOR_LAST_3( macro ) macro(2)
#define FOR_LAST_4( macro ) macro(3)
#define FOR_LAST_5( macro ) macro(4)
#define FOR_LAST_6( macro ) macro(5)
#define FOR_LAST_7( macro ) macro(6)
#define FOR_LAST_8( macro ) macro(7)

#define FOR_LAST( macro, n ) FOR_LAST_(macro, n)
#define FOR_LAST_( macro, n ) FOR_LAST_##n(macro)

#define STRINGIFY(x) #x

#define PRAGMA(x) _Pragma(STRINGIFY(x))
