#include "idc.idc"
#include "ps3_include_imports.idc"

static main()
{
	define_imports(ScreenEA(), ScreenEA() + 0x2C);
}
