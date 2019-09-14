import sys
import subprocess

build_target = "all"

if "rnjin" in sys.argv:
    build_target = "rnjin"

if "tests" in sys.argv:
    if build_target == "rnjin":
        build_target = "all"
    else:
        build_target = "tests"

output_log = "./logs/_scons.log"
build_result = 0

args = f"build={build_target}"

if "quiet" in sys.argv:
    args += " output=quiet"

command = "scons %s > %s" % (args, output_log)
build_process = subprocess.Popen(command, shell=True)

build_process.wait()
build_result = build_process.returncode    

if "run" in sys.argv and build_result == 0:
    if "tests" in sys.argv:
        run_process = subprocess.Popen(".\\tests.exe > logs\\_tests.log", shell=True)
    else:
        run_process = subprocess.Popen(".\\rnjin.exe", shell=True)