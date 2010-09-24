#include "idc.idc"
#include "ps3libdoc.idc"

static lookup_nid(library, nid)
{
    auto name;
    name = ps3libdoc_name_for_nid(library, nid);
    if (name == BADADDR)
    {
        name = form("%s_UNK_%08X", library, nid);
    }
    return name;
}

static define_import_struct()
{
    auto id, name;
    
    name = "LibraryImport";
    id = GetStrucIdByName(name);
    
    if (id == -1)
    {
        DelStruc(GetStrucIdByName(name));
        id = AddStrucEx(-1, name, 0);
        AddStrucMember(id, "size",           0x00, FF_WORD | FF_DATA,             0, 2);
        AddStrucMember(id, "unknown02",      0x02, FF_WORD | FF_DATA,            -1, 2);
        AddStrucMember(id, "unknown04",      0x04, FF_WORD | FF_DATA,            -1, 2);
        AddStrucMember(id, "function_count", 0x06, FF_WORD | FF_DATA | FF_0NUMD, -1, 2);
        AddStrucMember(id, "unknown08",      0x08, FF_DWRD | FF_DATA,            -1, 4);
        AddStrucMember(id, "unknown0C",      0x0C, FF_DWRD | FF_DATA,            -1, 4);
        AddStrucMember(id, "name",           0x10, FF_DWRD | FF_DATA | FF_0OFF,  -1, 4);
        AddStrucMember(id, "nid_table",      0x14, FF_DWRD | FF_DATA | FF_0OFF,  -1, 4);
        AddStrucMember(id, "stub_table",     0x18, FF_DWRD | FF_DATA | FF_0OFF,  -1, 4);
        AddStrucMember(id, "unknown1C",      0x1C, FF_DWRD | FF_DATA,            -1, 4);
        AddStrucMember(id, "unknown20",      0x20, FF_DWRD | FF_DATA,            -1, 4);
        AddStrucMember(id, "unknown24",      0x24, FF_DWRD | FF_DATA,            -1, 4);
        AddStrucMember(id, "unknown28",      0x28, FF_DWRD | FF_DATA,            -1, 4);
    }
    
    return name;
}

static define_imports(seg_start, seg_end)
{
    auto offset, size, index;
    auto function_count, library_name_offset, nid_table, stub_table;
    auto library_name;
    auto name;
    auto nid, stub_offset;
    auto struct_name;
    
    struct_name = define_import_struct();
    size = seg_end - seg_start;
    
    for (offset = 0; offset < size; offset = offset + 0x2C)
    {
        if (Byte(seg_start + offset) != 0x2C)
        {
            Message("Can't define import: size at %X is not 0x2C!\n", seg_start + offset);
            break;
        }

        MakeUnknown(seg_start + offset, 0x2C, DOUNK_SIMPLE);
        MakeStruct(seg_start + offset, struct_name);
        
        function_count = Word(seg_start + offset + 0x06);
        if (function_count == 0)
        {
            continue;
        }
        
        library_name_offset = Dword(seg_start + offset + 0x10);
        nid_table = Dword(seg_start + offset + 0x14);
        stub_table = Dword(seg_start + offset + 0x18);
        
        library_name = GetString(library_name_offset, -1, ASCSTR_C);
        
        MakeUnknown(nid_table, 4 * function_count, 0);
        MakeDword(nid_table);
        MakeArray(nid_table, function_count);
        SetArrayFormat(nid_table, 0, 1, 0);
        name = form("%s_nid_table", library_name);
        MakeNameEx(nid_table, name, 0);
        
        MakeUnknown(stub_table, 4 * function_count, 0);
        MakeDword(stub_table);
        OpOff(stub_table, 0, 0);
        MakeArray(stub_table, function_count);
        SetArrayFormat(stub_table, 0, 1, 0);
        name = form("%s_stub_table", library_name);
        MakeNameEx(stub_table, name, 0);
        
        for (index = 0; index < function_count; index++)
        {
            stub_offset = Dword(stub_table + (index * 4));
            
            MakeUnkn(stub_offset, DOUNK_SIMPLE);
            MakeCode(stub_offset);
            MakeFunction(stub_offset, BADADDR);
            MakeNameEx(stub_offset, lookup_nid(library_name, Dword(nid_table + (index * 4))), 0);
        }
    }
}
