#include "idc.idc"
#include "ps3_include_exports.idc"

static main()
{
	define_exports(ScreenEA(), ScreenEA() + 0x1C);
}
