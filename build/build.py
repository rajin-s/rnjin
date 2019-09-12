import sys
import subprocess

build_target = "all"

if len(sys.argv) > 1:
    build_target = sys.argv[1]

output_log = "./logs/scons.log"
build_result = 0

build_process = subprocess.Popen("scons build=%s > %s" % (build_target, output_log), shell=True)

build_process.wait()
build_result = build_process.returncode    

if "run" in sys.argv and build_result == 0:
    if "tests" in sys.argv:
        run_process = subprocess.Popen(".\\tests.exe > logs\\_tests.log", shell=True)
    else:
        run_process = subprocess.Popen(".\\rnjin.exe", shell=True)