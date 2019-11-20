import sys
import subprocess
import os

build_target = "all"

if "rnjin" in sys.argv:
    build_target = "rnjin"

if "tests" in sys.argv:
    if build_target == "rnjin":
        build_target = "all"
    else:
        build_target = "tests"

output_log     = "./logs/_scons.log"
engine_run_log = "./logs/_run.log"
tests_run_log  = "./logs/_tests.log"

engine_executable = "rnjin.exe"
tests_executable  = "tests.exe"

build_result = 0

args = f"build={build_target}"

if "quietly" in sys.argv:
    args += " output=quiet"

if not "singlecore" in sys.argv:
    args += " -j 4"

if "clean" in sys.argv:
    clean_command = f"scons -c {args}"
    os.system(f"echo {clean_command}")
    clean_process = subprocess.Popen(clean_command, shell=True)
    clean_process.wait()

command       = "scons %s >> %s" % (args, output_log)
os.system(f"echo {command} > {output_log}")
build_process = subprocess.Popen(command, shell=True)

build_process.wait()
build_result = build_process.returncode

if build_result == 0:
    print("Build succeeded")
    if "run" in sys.argv:
        # Get arguments to pass to executable (anything after --)
        start = -1
        for (i, arg) in enumerate(sys.argv):
            if arg == "--":
                start = i + 1
                break

        run_args = ""
        if start >= 0 and start < len(sys.argv):
            for arg in sys.argv[start:]:
                run_args = f"{run_args} {arg}"

        if "tests" in sys.argv:
            command = f"{tests_executable}{run_args} > {tests_run_log}"
        else:
            command = f"{engine_executable}{run_args} > {engine_run_log}"

        print(f"running {command}")
        run_process = subprocess.Popen(command, shell=True)
else:
    print("Build failed")
