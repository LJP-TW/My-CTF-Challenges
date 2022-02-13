#!/usr/bin/env python3
import pefile

pe = pefile.PE('giveUflag.dll')

entrypoint  = pe.OPTIONAL_HEADER.AddressOfEntryPoint
imagebase   = pe.OPTIONAL_HEADER.ImageBase
sizeofimage = pe.OPTIONAL_HEADER.SizeOfImage

section_cnt = len(pe.sections)
sections = b''
for sec in pe.sections:
    sections += sec.__pack__()

import_cnt = len(pe.DIRECTORY_ENTRY_IMPORT)
import_size = pe.OPTIONAL_HEADER.DATA_DIRECTORY[1].Size
import_rva  = pe.OPTIONAL_HEADER.DATA_DIRECTORY[1].VirtualAddress
import_dir = pe.get_data(rva=import_rva, length=import_size)

reloc_size = pe.OPTIONAL_HEADER.DATA_DIRECTORY[5].Size
reloc_rva  = pe.OPTIONAL_HEADER.DATA_DIRECTORY[5].VirtualAddress
reloc_dir  = pe.get_data(rva=reloc_rva, length=reloc_size)

tls_rva  = pe.OPTIONAL_HEADER.DATA_DIRECTORY[9].VirtualAddress

export_size = pe.OPTIONAL_HEADER.DATA_DIRECTORY[0].Size
export_rva  = pe.OPTIONAL_HEADER.DATA_DIRECTORY[0].VirtualAddress
export_dir  = pe.get_data(rva=export_rva, length=export_size)

with open('giveUflag.dll', 'rb') as f:
    dll = list(f.read())

offset_section_end = list(pe.sections)[-1].get_file_offset()+0x28
for i in range(offset_section_end):
    dll[i] = 0

offset_import = pe.get_offset_from_rva(import_rva)
for i in range(import_size):
    dll[offset_import + i] = 0

offset_reloc = pe.get_offset_from_rva(reloc_rva)
for i in range(reloc_size):
    dll[offset_reloc + i] = 0

offset_export = pe.get_offset_from_rva(export_rva)
for i in range(0x24):
    dll[offset_export + i] = 0

dll = bytes(dll)

with open('payload_dll.h', 'wb+') as f:
    f.write(b'#pragma once\n')

    f.write(f'#define PAYLOAD_ENTRY {entrypoint}\n'.encode())
    f.write(f'#define PAYLOAD_IMAGEBASE {imagebase}\n'.encode())
    f.write(f'#define PAYLOAD_SIZE_OF_IMAGE {sizeofimage}\n'.encode())

    f.write(f'#define PAYLOAD_SECTION_CNT {section_cnt}\n'.encode())

    f.write(b'#define PAYLOAD_SECTIONS {')
    for b in sections:
        f.write(f'{int(b)}, '.encode())
    f.write(b'}\n')

    f.write(f'#define PAYLOAD_IMPORT_CNT {import_cnt}\n'.encode())

    f.write(b'#define PAYLOAD_IMPORTS {')
    for b in import_dir:
        f.write(f'{int(b)}, '.encode())
    f.write(b'}\n')

    f.write(f'#define PAYLOAD_RELOC_SIZE {reloc_size}\n'.encode())

    f.write(b'#define PAYLOAD_RELOC {')
    for b in reloc_dir:
        f.write(f'{int(b)}, '.encode())
    f.write(b'}\n')

    f.write(f'#define PAYLOAD_TLS_RVA {tls_rva}\n'.encode())

    f.write(f'#define PAYLOAD_EXPORT_SIZE {export_size}\n'.encode())

    f.write(b'#define PAYLOAD_EXPORTS {')
    for b in export_dir:
        f.write(f'{int(b)}, '.encode())
    f.write(b'}\n')

    f.write(b'#define PAYLOAD_DLL {')
    for b in dll:
        f.write(f'{int(b)}, '.encode())
    f.write(b'}\n')

    f.write(f'#define PAYLOAD_DLL_LEN {len(dll)}\n'.encode())
