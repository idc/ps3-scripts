#include "idc.idc"
#include "ps3_include_imports.idc"

static main()
{
    auto lib_stub_seg, lib_stub_start, lib_stub_end;
    
    lib_stub_seg = SegByBase(SegByName(".lib.stub"));
    lib_stub_start = SegStart(lib_stub_seg);
    lib_stub_end = SegEnd(lib_stub_seg);
    
    if (lib_stub_start == BADADDR || lib_stub_end == BADADDR)
    {
        Message("Could not find .lib.stub segment location.\n");
    }
    else
    {
        Message("Defining library stubs...\n");
        define_imports(lib_stub_start, lib_stub_end);
        Message("...done\n");
    }
}
