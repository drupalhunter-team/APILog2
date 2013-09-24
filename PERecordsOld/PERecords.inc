StartRec(MZTITLE)
	charh(Signature, 2)_
	word(PartPag)_
    word(Page_count)_
    word(Relo_count)_
    word(Header_size)_
    word(Minimum_memory)_
    word(Maximum_memory)_
	word(ReloSS)_
    word(ExeSP)_
    word(Check_sum)_
    word(ExeIP)_
    word(ReloCS)_
    word(Table_offset)_
    word(Overlay)_
    reserved(Reserved, 8)_
	cardinal(PE_offset)
EndRec(MZTITLE)

StartRec(PETITLE)
    charh(Signature, 4)_
    word(CPU_type)_
    word(Number_of_objects)_
    cardinal(Date_time)_
    reserved(Reserved1, 2)_
    word(NT_length)_
    word(Flags)_
    reservedb(Reserved2, 2)_
    word(Linker_version)_
    reserved(Reserved3, 3)_
    cardinal(Entrypoint_RVA)_
    reserved(Reserved4, 2)_
    cardinal(Image_base)_
    cardinal(Object_align)_
    cardinal(File_align)_
    cardinal(OS_version)_
    cardinal(User_version)_
    cardinal(NT_version)_
    reserved(Reserved5, 1)_
    cardinal(Program_size)_
    cardinal(Full_title_size)_
    cardinal(Check_sum)_
    word(NT_sub_system)_
    word(DLL_flags)_
    cardinal(Stack_size)_
    cardinal(Minimal_stack_size)_
    cardinal(Heap_size)_
    cardinal(Minimal_heap_size)_
    reserved(Reserved6, 1)_
    cardinal(Array_size)_
    pointer(Export_table_offset)_
    cardinal(Export_table_size)_
    pointer(Import_table_offset)_
    cardinal(Import_table_size)_
    pointer(Resource_table_offset)_
    cardinal(Resource_table_size)_
    cardinal(Exception_table_offset)_
    cardinal(Exception_table_size)_
    cardinal(WarrTabOffset)_
    cardinal(WarrTabSize)_
    pointer(AddrTabOffset)_
    cardinal(AddrTabSize)_
    pointer(Debug_table_offset)_
    cardinal(Debug_table_size)_
    cardinal(DescrTabOffset)_
    cardinal(DescrTabSize)_
    cardinal(MachSpecOffset)_
    cardinal(MachSpecSize)_
    cardinal(TLSOffset)_
    cardinal(TLSSize)_
    cardinal(LoadCFGOffset)_
    cardinal(LoadCFGSize)_
    reserved(Reserved7, 2)_
    cardinal(IATOffset)_
    cardinal(IATSize)_
	reserved(Reserved8, 6)
EndRec(PETITLE)

StartRec(OBJECTTABLE)
    char(Name, 8)_
    cardinal(Virtual_size)_
    pointer(RVA)_
    cardinal(Phisical_size)_
    cardinal(Offset)_
    reserved(Reserved, 3)_
    cardinal(Flags)
EndRec(OBJECTTABLE)

StartRec(IMPORTDIRECTORYTABLE)
    pointer(Lookup_offset)_
    cardinal(Time_date_stamp)_
    cardinal(Forward_chain)_
    pointer(Library_name_offset)_
    pointer(Address_table_offset)
EndRec(IMPORTDIRECTORYTABLE)

StartRec(EXPORTDIRECTORYTABLE)
    cardinal(Flags)_
    cardinal(Time_date_stamp)_
    cardinal(Version)_
    pointer(Name_RVA)_
    cardinal(Ordinal_base)_
    cardinal(Number_of_funcs)_
    cardinal(Number_of_names)_
    pointer(Address_table_offset)_
    pointer(Name_pointers_RVA)_
    pointer(Ordinal_table_RVA)
EndRec(EXPORTDIRECTORYTABLE)

StartRec(EXPORTEDENTRY)
	pchar_d(Name, "Имя")_
	cardinal_d(Ordinal, "Ординал")_
	pointer_d(Address, "Адрес входа")
EndRec(EXPORTEDENTRY)

PreDefRec(IMPORTEDLIB)

StartRec(IMPORTEDENTRY)
	pchar_d(Name, "Имя")_
	cardinal_d(Ordinal, "Ординал")_
	pointer_d(Address, "Адрес связи")_
	precord(Library, IMPORTEDLIB)
EndRec(IMPORTEDENTRY)

StartRec(IMPORTEDLIB)
	pchar_d(Name, "Библиотека")_
	records(Entries, IMPORTEDENTRY)
EndRec(IMPORTEDLIB)