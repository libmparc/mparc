import os
import SCons.Script as scons
# make me an environment
env = scons.Environment(
    ENV = {"PATH": os.environ["PATH"]},

    SCONS_CXX_STANDARD="c++11",
    SCONS_C_STANDARD="c99"
    )

TRUNIX = True

# flag things
install_path = None
if TRUNIX:
    flegs = ""
    with open("ccflegs.txt", "r", encoding="ascii") as f:
        flegs = f.read()

    sanitizers = os.environ.get("SANITIZERS", "")
    ldflags = os.environ.get("LDFLAGS", "")
    cppflags = os.environ.get("CPPFLAGS", "")

    install_path = os.environ.get("INSTALL", "/usr/local/bin/")


    cc = os.environ.get("CC", "gcc")
    cxx = os.environ.get("CXX", "g++")

    env.Replace(CC=cc)
    env.Replace(CXX=cxx)

    env.Append(LINKFLAGS=ldflags)
    env.Append(CPPDEFINES=cppflags)
    env.Append(CFLAGS=f"-std=c99 {sanitizers} {flegs}")
    env.Append(CXXFLAGS=f"-std=c++11 {sanitizers} {flegs}")


# library maker
lib = env.StaticLibrary(target="mparc.a", source="./lib/mparc.c")


# binary builders
programs = []
installers = []
if TRUNIX:
    l1 = lib.copy()
    l1.insert(0, "./src/main.c")
    programs.append(env.Program(target="mparc.exe", source=l1))

if TRUNIX:
    l2 = lib.copy()
    l2.insert(0, "./src/mainv2.c")
    programs.append(env.Program(target="mparc2.exe", source=l2))

if TRUNIX:
    l3 = lib.copy()
    l3.insert(0, "./src/cxmain.cpp")
    programs.append(env.Program(target="cxmparc.exe", source=l3))

for program in programs:
    env.Install(install_path, program)

env.Alias("install", install_path)
