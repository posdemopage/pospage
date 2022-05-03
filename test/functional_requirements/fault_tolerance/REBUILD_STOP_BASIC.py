#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import MOUNT_VOL_BASIC_1
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA
SECOND_DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.SPARE
REMAINING_DEV = "unvme-ns-4"
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME


def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    fio.wait_fio(fio_proc)
    api.detach_ssd(DETACH_TARGET_DEV)

    if api.wait_situation(ARRAYNAME, "REBUILDING") is True:
        api.detach_ssd(SECOND_DETACH_TARGET_DEV)
        timeout = 80000 #80s
        if api.wait_situation(ARRAYNAME, "DEGRADED", timeout) is True:
            return "pass"
    return "fail"


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    result = execute()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)