import fio
import initiator
import json
import lib
import target
from datetime import datetime


def play(json_targets, json_inits, json_scenario):
    lib.printer.green(f"\n -- '{__name__}' has began --")

    raw_date = datetime.now()
    now_date = raw_date.strftime("%y%m%d_%H%M%S")
    skip_workload = False

    # validate arguments
    if 0 == len(json_targets):
        lib.printer.red(" TargetError: At least 1 target has to exist")
        return
    if 0 == len(json_inits):
        lib.printer.red(" InitiatorError: At least 1 initiator has to exist")
        return

    # target prepare
    targets = {}
    for json_target in json_targets:
        try:
            target_obj = target.manager.Target(json_target)
            target_name = json_target["NAME"]
        except KeyError:
            lib.printer.red(" TargetError: Target KEY is invalid")
            return
        if not target_obj.Prepare():
            skip_workload = True
            break
        targets[target_name] = target_obj

    # init prepare
    initiators = {}
    test_target = targets[next(iter(targets))]
    for json_init in json_inits:
        try:
            init_obj = initiator.manager.Initiator(json_init)
            init_name = json_init["NAME"]
        except KeyError:
            lib.printer.red(" InitiatorError: Initiator KEY is invalid")
            return
        if not init_obj.Prepare(True, test_target.subsystem_list):
            skip_workload = True
            break
        initiators[init_name] = init_obj

    # check auto generate
    if "yes" != test_target.use_autogen:
        lib.printer.red(f"{__name__} [Error] check [TARGET][AUTO_GENERATE][USE] is 'yes' ")
        skip_workload = True

    # run workload
    if not skip_workload:
        lib.printer.green(f" fio start")

        # readwrite, block size, io depth
        testcase = [["write", "128k", "4"],
                    ["read", "128k", "4"],
                    ["randwrite", "4k", "128"],
                    ["randread", "4k", "128"]
                    ]

        for tc in testcase:
            fio_cmdset = []
            rw = tc[0]
            bs = tc[1]
            iodepth = tc[2]
            output_name = f"{now_date}_fio_{bs}_{rw}"

            for key in initiators:
                init = initiators[key]
                test_fio = fio.manager.Fio(init.id, init.pw, init.nic_ssh)
                test_fio.opt["ioengine"] = "libaio"
                test_fio.opt["runtime"] = "15"
                test_fio.opt["ramp_time"] = "5"
                test_fio.opt["io_size"] = "8g"
                test_fio.opt["verify"] = "0"
                test_fio.opt["serialize_overlap"] = "1"
                test_fio.opt["time_based"] = "1"
                test_fio.opt["numjobs"] = "1"
                test_fio.opt["thread"] = "1"
                test_fio.opt["group_reporting"] = "1"
                test_fio.opt["direct"] = "1"
                test_fio.opt["readwrite"] = rw
                test_fio.opt["bs"] = bs
                test_fio.opt["iodepth"] = iodepth

                test_fio.opt["output"] = f"{init.output_dir}/{output_name}_{init.name}"
                test_fio.opt["output-format"] = "json"

                if "randwrite" == rw or "randread" == rw:
                    if test_fio.opt.get("norandommap"):
                        del test_fio.opt["norandommap"]
                else:
                    test_fio.opt["norandommap"] = "1"
                if "512-128k" == bs:
                    test_fio.opt["bsrange"] = bs
                else:
                    if test_fio.opt.get("bsrange"):
                        del test_fio.opt["bsrange"]

                for test_device in init.device_list:
                    test_fio.jobs.append(f" --name=job_{test_device} --filename={test_device}")
                    if not test_fio.Prepare():
                        skip_workload = True
                        break
                fio_cmdset.append(test_fio.cmd)

            if not skip_workload:
                try:
                    print(f" run -> {now_date}_fio_{bs}_{rw}")
                    fio.manager.parallel_run(fio_cmdset)
                except Exception as e:
                    lib.printer.red(f"{__name__} [Error] {e}")
                    skip_workload = True

                try:
                    for key in initiators:
                        init = initiators[key]
                        lib.subproc.sync_run(f"sshpass -p {init.pw} scp {init.id}@{init.nic_ssh}:{init.output_dir}/{output_name}_{init.name} {json_scenario['OUTPUT_DIR']}")
                except Exception as e:
                    lib.printer.red(f"{__name__} [Error] {e}")
                    skip_workload = True

        lib.printer.green(f" fio end")

    # init wrapup
    for key in initiators:
        initiators[key].Wrapup(True, test_target.subsystem_list)

    # target warpup
    for key in targets:
        if not targets[key].Wrapup():
            targets[key].ForcedExit()

    if skip_workload:
        lib.printer.red(f" -- '{__name__}' unexpected done --\n")
    else:
        lib.printer.green(f" -- '{__name__}' successfully done --\n")
