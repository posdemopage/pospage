#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import test_result
import CREATE_VOL_NO_SPARE
import MOUNT_ARRAY_NO_SPARE

VOL_NAME = CREATE_VOL_NO_SPARE.VOL_NAME
VOL_SIZE = CREATE_VOL_NO_SPARE.VOL_SIZE
VOL_IOPS = CREATE_VOL_NO_SPARE.VOL_IOPS
VOL_BW = CREATE_VOL_NO_SPARE.VOL_BW

REMAINING_DEV = MOUNT_ARRAY_NO_SPARE.REMAINING_DEV
DETACH_ANOTHER_TARGET_DEV = MOUNT_ARRAY_NO_SPARE.DATA_DEV_2
ANY_DATA = MOUNT_ARRAY_NO_SPARE.ANY_DATA
ARRAYNAME = CREATE_VOL_NO_SPARE.ARRAYNAME

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    CREATE_VOL_NO_SPARE.execute()
    out = cli.mount_volume(CREATE_VOL_NO_SPARE.VOL_NAME, ARRAYNAME, "")
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    out = execute()
    set_result(out)
    pos.flush_and_kill_pos()