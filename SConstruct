# -*- python -*-
import os
import os.path as op
import shutil as sh
import subprocess as sp
import random
from datetime import datetime

env = Environment()
mode = ARGUMENTS.get('mode', 'debug')
env['BUILD_DIR'] = env.Dir('build/%s' % mode)
env['TMP_DIR_PATTERN'] = '$BUILD_DIR/%{user}s.tmp.%{ts}s.%{salt}s/'
env['RANDOM'] = random.Random()
env['HEADER_DIR'] = env.Dir('$BUILD_DIR/include')
env['BIN_DIR'] = env.Dir('$BUILD_DIR/bin')
env['LIB_DIR'] = env.Dir('$BUILD_DIR/lib')
env.SetOption('duplicate','hard-soft-copy')
env.Decider('MD5')

# helper functions

def newTmpDir(env, usr):
    ts = datetime.utcnow()
    ts = ts.strftime('%Y%m%dT%H%M%S.%f')
    salt = env['RANDOM'].randint(0, 10000)
    d = env['TMP_DIR_PATTERN'] % (usr, ts, salt)
    os.makedirs(d)
    return d

env.AddMethod(newTmpDir)

def subDir(env, subd):
    env.SConscript('%s/SConscript' % subd, exports='env')

env.AddMethod(subDir)

_Glob = env.Glob
def Glob(pathname, ondisk=True, source=False, strings=False):
    fs = _Glob(pathname, ondisk, source, strings)
    fs.sort(key=lambda x:x.path)
    return fs
env.Glob = Glob

_Program = env.Program
def Program(env, target=None, source=None, **kwargs):
    p = _Program(target, source, **kwargs)
    env.Depends(p, '$LIB_DIR')
    env.Install('$BIN_DIR', p)
    return p
env.AddMethod(Program)

def Header(env, base, files):
    for f in files:
        src = env.File(f).abspath
        d = env.Dir('$HEADER_DIR').Dir(base)
        if not op.exists(d.abspath):
            os.makedirs(d.abspath)
        des = d.File(op.basename(src)).abspath
        os.symlink(src, des)
env.AddMethod(Header)

env['BUILDERS']['Object'] = env['BUILDERS']['SharedObject']
env['BUILDERS']['StaticObject'] = env['BUILDERS']['SharedObject']

# prepare build dir

def makeBuildDir():
    for d in [env['BUILD_DIR'], env['HEADER_DIR']]:
        d = d.abspath
        if not op.exists(d):
            os.makedirs(d)
        assert op.isdir(d)

def cleanBuildDir():
    buildDir = env['BUILD_DIR'].abspath
    for rt, dirs, files in os.walk(buildDir):
        try:
            dirs.remove('.git')
        except:
            pass
        for f in files:
            f = op.join(rt, f)
            if op.islink(f) or f.endswith('.gcno') or f.endswith('.gcda'):
                os.remove(f)

def firstDirname(p):
    x = p
    y = op.dirname(p)
    while len(y) > 0:
        x = y
        y = op.dirname(x)
    return x

def cloneFile(rt, fn):
    d = op.join(env['BUILD_DIR'].path, rt)
    if not op.exists(d):
        os.makedirs(d)
    os.symlink(op.abspath(op.join(rt, fn)), op.join(d, fn))
    
def cloneWorkSpace():
    buildDir = firstDirname(env['BUILD_DIR'].path)
    paths = os.listdir('.')
    for x in [buildDir, '.git', '.gitignore', '.sconsign.dblite', 'SConstruct']:
        try:
            paths.remove(x)
        except:
            pass
    for x in paths:
        if op.isfile(x):
            cloneFile('', x)
        if op.isdir(x):
            for rt, _, files in os.walk(x):
                for f in files:
                    cloneFile(rt, f)

makeBuildDir()
cleanBuildDir()
cloneWorkSpace()

flags = {
    'CFLAGS': ['--std=c11', '-pthread'],
    'CXXFLAGS': ['--std=c++11', '-pthread'],
    'CCFLAGS': ['-Wall', '-Wfloat-equal', '-pthread',
                '-g', '-gdwarf-4', 
                '-I%s' % env['HEADER_DIR'].path,
                '-Isrc'],
    'LINKFLAGS': ['-rdynamic', '-pthread',
                  '-Wl,-rpath=%s' % env['LIB_DIR'].abspath,
                  '-L%s' % env['LIB_DIR'].abspath]}
if mode == 'debug':
    flags['CCFLAGS'] += ['-O0', '--coverage']
    flags['LINKFLAGS'] += ['--coverage']
elif mode == 'release':
    flags['CCFLAGS'] += ['-O2', '-Werror', '-DNDEBUG']
env.MergeFlags(flags)
    
# gogogo

env.SConscript('$BUILD_DIR/SConscript', exports='env')

