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

static define_export_struct()
{
    auto id, name;
    
    name = "LibraryExport";
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
        AddStrucMember(id, "pair_table",     0x18, FF_DWRD | FF_DATA | FF_0OFF,  -1, 4);
    }
    
    return name;
}

static define_pair_struct()
{
    auto id, name;
    
    name = "RtocPair";
    id = GetStrucIdByName(name);
    
    if (id == -1)
    {
        DelStruc(GetStrucIdByName(name));
        id = AddStrucEx(-1, name, 0);
        AddStrucMember(id, "function", 0x00, FF_DWRD | FF_DATA | FF_0OFF,  0, 4);
        AddStrucMember(id, "rtoc",     0x04, FF_DWRD | FF_DATA | FF_0OFF, -1, 4);
    }
    
    return name;
}

static define_exports(seg_start, seg_end)
{
    auto offset, size, index;
    auto function_count, library_name_offset, nid_table, pair_table;
    auto library_name;
    auto name;
    auto nid, pair_offset, function_offset;
    auto export_struct_name, pair_struct_name;
    
    export_struct_name = define_export_struct();
    pair_struct_name = define_pair_struct();
    
    size = seg_end - seg_start;
    
    for (offset = 0; offset < size; offset = offset + 0x1C)
    {
        if (Byte(seg_start + offset) != 0x1C)
        {
            Message("Can't define export: size at %X is not 0x1C!\n", seg_start + offset);
            break;
        }
    
        MakeUnknown(seg_start + offset, 0x1C, DOUNK_SIMPLE);
        MakeStruct(seg_start + offset, export_struct_name);
        
        function_count = Word(seg_start + offset + 0x06);
        if (function_count == 0)
        {
            continue;
        }
        
        library_name_offset = Dword(seg_start + offset + 0x10);
        
        // I'm sure there's a flag that indicates if this is a real export table
        // or not but for now, this will work.
        if (library_name_offset == 0)
        {
            continue;
        }
        
        nid_table = Dword(seg_start + offset + 0x14);
        pair_table = Dword(seg_start + offset + 0x18);
        
        library_name = GetString(library_name_offset, -1, ASCSTR_C);
        
        MakeUnknown(nid_table, 4 * function_count, 0);
        MakeDword(nid_table);
        MakeArray(nid_table, function_count);
        SetArrayFormat(nid_table, 0, 1, 0);
        name = form("%s_nid_table", library_name);
        MakeNameEx(nid_table, name, 0);
        
        MakeUnknown(pair_table, 4 * function_count, 0);
        MakeDword(pair_table);
        OpOff(pair_table, 0, 0);
        MakeArray(pair_table, function_count);
        SetArrayFormat(pair_table, 0, 1, 0);
        name = form("%s_pair_table", library_name);
        MakeNameEx(pair_table, name, 0);
        
        for (index = 0; index < function_count; index++)
        {
            pair_offset = Dword(pair_table + (index * 4));
            
            MakeUnknown(pair_offset, 8, DOUNK_SIMPLE);
            MakeStruct(pair_offset, pair_struct_name);
            MakeNameEx(pair_offset, lookup_nid(library_name, Dword(nid_table + (index * 4))) + "_pair", 0);
            
            function_offset = Dword(pair_offset + 0);
            
            MakeCode(function_offset);
            MakeFunction(function_offset, BADADDR);
            MakeNameEx(function_offset, lookup_nid(library_name, Dword(nid_table + (index * 4))), 0);
        }
    }
}
