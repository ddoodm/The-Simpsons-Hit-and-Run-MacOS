#!/usr/bin/env python3
"""One-shot migration helper: turn the Visual Studio projects into CMakeLists.

Run from the repository root. Regenerating is only needed if the .vcxproj source
lists change; the emitted CMakeLists files are the checked-in source of truth.
"""
import os
import xml.etree.ElementTree as ET

NS = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Sources that only make sense on Windows. Everything else in each project is
# either portable or already guarded by RAD_* preprocessor checks.
WINDOWS_ONLY = {
    'radcore': [
        'src/radcontroller/directinputcontroller.cpp',
        'src/radstacktrace/win32/stacktrace.cpp',
        # Winsock DECI2 transport. Only reachable when RADDEBUGCOMMUNICATION is
        # enabled in radoptions.hpp, which it is not; targetx.cpp supplies the
        # no-op public API either way.
        'src/raddebugcommunication/targetconnection.cpp',
        'src/raddebugcommunication/targetsocketchannel.cpp',
    ],
    # macOS shares the PC input path: keyboard, mouse and pad devices all come
    # from radcore's SDL backend, which reports the same DirectInput codes the
    # PC device wrappers already speak. Only the force-feedback effects are
    # dropped, since those are built directly on DIEFFECT.
    'SRR2': [
        '../game/input/basedamper.cpp',
        '../game/input/constanteffect.cpp',
        '../game/input/forceeffect.cpp',
        '../game/input/steeringspring.cpp',
        '../game/input/wheelrumble.cpp',
    ],
}

# Sources compiled only on macOS.
MACOS_ONLY = {}

LIBS = [
    dict(name='radmath', proj='src/libs/radmath/radmath.vcxproj',
         public=['.', 'radmath'], private=[], link=[]),
    dict(name='radcore', proj='src/libs/radcore/radcore.vcxproj',
         public=['include'], private=['src'],
         link=['srr2::sdl2', 'srr2::gl']),
    dict(name='radcontent', proj='src/libs/radcontent/radcontent.vcxproj',
         public=['include', '.'], private=['src', 'src/pch'],
         link=['radcore', 'radmath']),
    dict(name='radscript', proj='src/libs/radscript/radscript.vcxproj',
         public=['include'], private=['src/pch'],
         link=['radcore']),
    dict(name='radsound', proj='src/libs/radsound/radsound.vcxproj',
         public=['include'], private=['src/common', 'src/pch'],
         link=['radcore', 'srr2::openal']),
    dict(name='radmusic', proj='src/libs/radmusic/radmusic.vcxproj',
         public=['include'], private=['src', 'src/pch'],
         link=['radsound', 'radcore', 'radcontent']),
    dict(name='pure3d', proj='src/libs/pure3d/Pure3D.vcxproj',
         public=['.', 'p3d'], private=['../radcontent'],
         link=['radcore', 'radmath', 'radcontent',
               'srr2::png', 'srr2::gl', 'srr2::sdl2']),
    dict(name='poser', proj='src/libs/poser/poser.vcxproj',
         public=['include'], private=[],
         link=['pure3d', 'radmath', 'radcore', 'radcontent']),
    dict(name='sim', proj='src/libs/sim/sim.vcxproj',
         public=['.'], private=[],
         link=['pure3d', 'radmath', 'radcontent', 'poser', 'radcore']),
    dict(name='choreo', proj='src/libs/choreo/choreo.vcxproj',
         public=['include'], private=[],
         link=['pure3d', 'poser', 'sim', 'radmath', 'radcore', 'radcontent']),
    dict(name='scrooby', proj='src/libs/scrooby/ScroobyLib.vcxproj',
         public=['include', 'src'], private=[],
         link=['radmath', 'pure3d', 'radcore', 'radcontent'],
         defines=['SCROOBY_RUNTIME']),
    dict(name='radmovie', proj='src/libs/radmovie/radmovie.vcxproj',
         public=['include'], private=['src/common', 'src/pch'],
         link=['radcore', 'radsound', 'pure3d', 'radmath', 'radcontent',
               'srr2::ffmpeg']),
]

HEADER = "# Generated from {proj} by tools/gen_cmake_sources.py.\n"


def sources(proj_path, name):
    root = ET.parse(os.path.join(ROOT, proj_path)).getroot()
    out = []
    for c in root.findall('.//m:ClCompile[@Include]', NS):
        out.append(c.get('Include').replace('\\', '/'))
    win = set(WINDOWS_ONLY.get(name, []))
    common = [s for s in out if s not in win]
    # macOS sources are listed ahead of being written, so only emit the ones
    # that exist to keep the tree configurable at every step.
    proj_dir = os.path.dirname(os.path.join(ROOT, proj_path))
    mac = [s for s in MACOS_ONLY.get(name, [])
           if os.path.exists(os.path.join(proj_dir, s))]
    return sorted(common), sorted(win), sorted(mac)


def fmt_list(items, indent=4):
    pad = ' ' * indent
    return '\n'.join(pad + i for i in items)


def emit_lib(cfg):
    name, proj = cfg['name'], cfg['proj']
    common, win, mac = sources(proj, name)
    lines = [HEADER.format(proj=os.path.basename(proj))]
    lines.append(f'set({name}_SOURCES\n{fmt_list(common)})\n')
    if win:
        lines.append(f'if(WIN32)\n    list(APPEND {name}_SOURCES\n{fmt_list(win, 8)})\nendif()\n')
    if mac:
        lines.append(f'if(APPLE)\n    list(APPEND {name}_SOURCES\n{fmt_list(mac, 8)})\nendif()\n')

    lines.append(f'srr2_add_library({name}')
    lines.append(f'    SOURCES ${{{name}_SOURCES}}')
    if cfg['public']:
        lines.append('    PUBLIC_INCLUDES ' + ' '.join(cfg['public']))
    if cfg['private']:
        lines.append('    PRIVATE_INCLUDES_PLACEHOLDER')
    if cfg['link']:
        lines.append('    LINK ' + ' '.join(cfg['link']))
    lines.append(')')
    body = '\n'.join(lines)
    if cfg['private']:
        body = body.replace('    PRIVATE_INCLUDES_PLACEHOLDER',
                            '    INCLUDES ' + ' '.join(cfg['private']))
    if cfg.get('defines'):
        body += f"\n\ntarget_compile_definitions({name} PUBLIC {' '.join(cfg['defines'])})"
    body += '\n'
    path = os.path.join(ROOT, os.path.dirname(proj), 'CMakeLists.txt')
    with open(path, 'w') as fh:
        fh.write(body)
    print(f'{path}: {len(common)} sources (+{len(win)} win, +{len(mac)} mac)')


def emit_exe():
    common, win, mac = sources('src/srr2/SRR2.vcxproj', 'SRR2')
    with open(os.path.join(ROOT, 'src/srr2/sources.cmake'), 'w') as fh:
        fh.write('# Generated from SRR2.vcxproj by tools/gen_cmake_sources.py.\n')
        fh.write(f'set(SRR2_SOURCES\n{fmt_list(common)})\n\n')
        fh.write(f'if(WIN32)\n    list(APPEND SRR2_SOURCES\n{fmt_list(win, 8)})\nendif()\n\n')
        fh.write(f'if(APPLE)\n    list(APPEND SRR2_SOURCES\n{fmt_list(mac, 8)})\nendif()\n')
    print(f'src/srr2 sources: {len(common)} (+{len(win)} win, +{len(mac)} mac)')


if __name__ == '__main__':
    for cfg in LIBS:
        emit_lib(cfg)
    emit_exe()
