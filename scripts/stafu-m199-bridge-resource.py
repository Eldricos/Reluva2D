#!/usr/bin/env python3
"""STAFU M199: read-only BRIDGE.RM archive and installed-file provenance.

Does not infer in-memory resource-record fields from ZIP metadata.
"""
import argparse
import hashlib
import pathlib
import struct
import sys
import zipfile
import zlib

EXPECTED_OVL_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'

def digest(b):
    return hashlib.sha256(b).hexdigest()

def line(tag, **fields):
    print('RM_M199_' + tag + ' ' + ' '.join(f'{k}={v}' for k,v in fields.items()))

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('installed', type=pathlib.Path, help='Installed STTNG folder')
    p.add_argument('cd', type=pathlib.Path, help='CD folder containing STTNG.ZIP')
    p.add_argument('--out', type=pathlib.Path, default=pathlib.Path('m199-output'))
    args = p.parse_args()
    ovl = args.installed / 'STTNG.OVL'
    archive = args.cd / 'STTNG.ZIP'
    if not ovl.is_file() or not archive.is_file():
        line('ERROR', missingOVL=int(not ovl.is_file()), missingZIP=int(not archive.is_file()))
        return 2
    ovl_data = ovl.read_bytes()
    ok = len(ovl_data) == 2008455 and digest(ovl_data) == EXPECTED_OVL_SHA
    line('OVL', bytes=len(ovl_data), sha256=digest(ovl_data), knownMatch=int(ok))
    if not ok:
        line('ERROR', reason='unexpectedOVL')
        return 2
    try:
        with zipfile.ZipFile(archive) as z:
            matches = [x for x in z.infolist() if pathlib.PurePosixPath(x.filename.replace('\\','/')).name.upper() == 'BRIDGE.RM']
            line('ARCHIVE', path=archive, entries=len(z.infolist()), bridgeMatches=len(matches))
            if not matches:
                line('ERROR', reason='bridgeNotFoundInArchive')
                return 2
            if len(matches) != 1:
                line('ERROR', reason='ambiguousBridgeEntries')
                return 2
            info = matches[0]
            line('ZIP_ENTRY', name=repr(info.filename), method=info.compress_type,
                 compressed=info.compress_size, uncompressed=info.file_size,
                 crc32=f'{info.CRC:08x}', flags=f'0x{info.flag_bits:04x}',
                 localHeaderOffset=info.header_offset)
            with archive.open(info) as f:
                payload = f.read()
            if len(payload) != info.file_size or (zlib.crc32(payload)&0xffffffff) != info.CRC:
                line('ERROR', reason='payloadValidationFailed')
                return 2
            line('PAYLOAD', bytes=len(payload), sha256=digest(payload), first32=payload[:32].hex())
            installed_candidates = [x for x in args.installed.rglob('*') if x.is_file() and x.name.upper() == 'BRIDGE.RM']
            line('INSTALLED', matches=len(installed_candidates))
            for i, file in enumerate(installed_candidates):
                data = file.read_bytes()
                line('INSTALLED_FILE', index=i, path=repr(str(file)), bytes=len(data), sha256=digest(data), archiveEqual=int(data == payload))
            # Only validated data is written, and never into original game folders.
            args.out.mkdir(parents=True, exist_ok=True)
            out = args.out / 'm199-bridge-rm-archive-payload.bin'
            out.write_bytes(payload)
            line('OUTPUT', file=out, bytes=len(payload), sha256=digest(payload))
            # Extract the local header independently to aid archive provenance checks.
            with open(archive, 'rb') as f:
                f.seek(info.header_offset)
                header = f.read(30)
                if len(header) != 30 or header[:4] != b'PK\x03\x04':
                    line('WARNING', reason='unexpectedLocalHeader')
                else:
                    fields = struct.unpack('<IHHHHHIIIHH', header)
                    name_len, extra_len = fields[-2:]
                    name_bytes = f.read(name_len)
                    extra_bytes = f.read(extra_len)
                    local = header + name_bytes + extra_bytes
                    local_path = args.out / 'm199-bridge-rm-local-header.bin'
                    local_path.write_bytes(local)
                    line('LOCAL_HEADER', bytes=len(local), nameBytes=name_bytes.hex(), extraBytes=len(extra_bytes), file=local_path)
    except (OSError, zipfile.BadZipFile, RuntimeError, NotImplementedError) as exc:
        line('ERROR', reason=repr(str(exc)))
        return 2
    line('LIMITATION', zipMethodNotRuntimeMode=1, runtimeRecordFieldsUnverified=1, paletteProvenanceUnverified=1)
    return 0

if __name__ == '__main__':
    sys.exit(main())
