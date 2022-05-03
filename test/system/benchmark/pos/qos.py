import json
import lib
import pos


def CheckTestValidity(test_name, testcase):
    count = len(testcase)
    if count < 3:
        lib.printer.red(f"TEST NAME:{test_name}'s syntax is incorrect. Argument less than 3")
        return False

    limit_type = testcase[0]
    if limit_type != "reset" and limit_type != "bw" and limit_type != "iops":
        lib.printer.red(f"TEST NAME:{test_name}'s limit type is wrong (reset, bw, iops)")
        return False

    if limit_type == "reset":
        return True

    volume_list = []
    if type(testcase[1]) is list:
        volume_list = testcase[1]
        if limit_type != "reset" and (len(testcase[1]) != len(testcase[2])):
            lib.printer.red(f"TEST NAME:{test_name}'s number of volume list & limit value is not the same")
            return False
    else:
        limit_how = testcase[1]
        if limit_how != "rate" and limit_how != "value":
            lib.printer.red(f"TEST NAME:{test_name}'s limit how is wrong (rate, value)")
            return False
    return True


def GetLimitPerformance(base_perf, limit_type, limit_rate):
    value = float(base_perf[limit_type])
    limit_value = value * limit_rate / 100.0
    return limit_value


def GetQosCommandOption(testcase, base_perf):
    limit = {"type": "", "how": "", "value": 0.0}  # unit: kiops, MB/s
    volume_list = {"type": "", }

    limit["type"] = volume_list["type"] = testcase[0]
    if limit["type"] == "reset" or volume_list["type"] == "reset":
        testcase[2] = "0"
        return limit

    if type(testcase[1]) is list:
        count = len(testcase[1])
        for i in range(0, count):
            volume_list[testcase[1][i]] = int(testcase[2][i])
        return volume_list

    else:
        limit["value"] = float(testcase[2])

        limit["how"] = testcase[1]
        if limit["how"] == "rate" and limit["value"] != 0:
            limit_rate = limit["value"]
            limit["value"] = GetLimitPerformance(base_perf, limit["type"], limit_rate)
            if limit["type"] == "iops":
                limit["value"] /= 1000.0
        return limit


def ParseVolumeDict(volume_dict):
    volume_list = {}

    for key in volume_dict.keys():
        if key == "type":
            continue
        limit_value = volume_dict[key]
        if "-" in key:
            start, end = key.split("-")
            for i in range(int(start), int(end) + 1):
                volume_list[i] = limit_value
        else:
            volume_list[int(key)] = limit_value
    return volume_list


def SetQosToAllVolumes(test_target, limit):
    limit_type = limit["type"]
    limit_how = limit["how"]
    limit_value = limit["value"]
    total_limit_value = 0

    # Get total volume count
    vol_count = 0
    for array in test_target.array_volume_list:
        vol_count += len(test_target.array_volume_list[array])

    # Get limit value per volume if limit type == "rate"
    if limit_how == "rate":
        volume_limit_value = float(limit_value / vol_count)
        total_limit_value = limit_value
    else:
        volume_limit_value = float(limit_value)
        total_limit_value = volume_limit_value * vol_count

    if "yes" == test_target.use_autogen:
        for array in test_target.array_volume_list.keys():
            for vol_name in test_target.array_volume_list[array]:
                if -1 == pos.cli.set_qos(test_target.id, test_target.pw, test_target.nic_ssh, test_target.pos_cli, test_target.pos_dir, array, vol_name, limit_type, volume_limit_value):
                    return -1

    if limit_type == "reset" or volume_limit_value == 0:
        lib.printer.green(f" Reset qos of all volumes")
        return 0

    if limit_type == "iops":
        total_value = total_limit_value
        lib.printer.green(f" Throttle pos's {limit_type} to {total_value}k")
    else:
        lib.printer.green(f" Throttle pos's {limit_type} to {total_limit_value}MB/s")
    return total_limit_value


def SetQosToEachVolumes(test_target, limit):
    limit_type = limit["type"]

    volume_list = {}  # ex) {"1" 10, "3": 10 , "5": 30}
    volume_list = ParseVolumeDict(limit)

    # Support only one array with auto gen
    if "yes" == test_target.use_autogen:
        array = list(test_target.array_volume_list.keys())[0]
        for key in list(volume_list.keys()):
            limit_value = volume_list[key]
            vol_name = "VOL" + str(key)
            if -1 == pos.cli.set_qos(test_target.id, test_target.pw, test_target.nic_ssh, test_target.pos_cli, test_target.pos_dir, array, vol_name, limit_type, limit_value):
                lib.printer.red(" set qos failed")
    return volume_list


def CheckQosThrottled(json_result_file, limit_type, expected_value, prev_expected_value, base_perf):
    # Return value: [ throttled or not, actual_result(bw, iops) ]
    result = [True, []]

    # Load json file
    json_config_file = f"./{json_result_file}"
    with open(json_config_file, "r") as f:
        json_array = json.load(f)
    array_len = len(json_array)

    column_type = ""
    if limit_type == "iops":
        column_type = "rate"
    elif limit_type == "bw":
        column_type = "MB/sec"
    else:
        limit_type = "bw"
        column_type = "MB/sec"

    read_line_num = 3
    # Check result of last 3 in json file
    for i in range(0, min(read_line_num, array_len)):
        bw_value = float(json_array[array_len - i - 1]["MB/sec"])
        iops_value = float(json_array[array_len - i - 1]["rate"]) / 1000.0
        if column_type == "MB/sec":
            actual_value = bw_value
        elif column_type == "rate":
            actual_value = iops_value * 1000.0
        print(f"  BW: {bw_value} MB/sec, IOPS: {iops_value}k")
        if expected_value is None:
            continue

        # Check result with expected value
        if expected_value == -1:  # check if the same as prev perf
            if (prev_expected_value[limit_type] * 0.9 > actual_value) or (prev_expected_value[limit_type] * 1.1 < actual_value):
                print(f" Failed qos throttling: expected_value = {prev_expected_value[limit_type]} with o/h +-10% , actual_value = {actual_value}")
                result[0] = False
        elif expected_value == 0:  # check if the same as base perf
            if (base_perf[limit_type] * 0.9 > actual_value) or (base_perf[limit_type] * 1.1 < actual_value):
                print(f" Failed reset qos throttling: expected_value = {base_perf[limit_type]} with o/h +-10% , actual_value = {actual_value}")
                result[0] = False
        elif (actual_value > expected_value):
            print(f" Failed qos throttling: expected_value = {expected_value}, actual_value = {actual_value}")
            result[0] = False

    if array_len >= 1:
        prev_expected_value["bw"] = float(json_array[array_len - 1]["MB/sec"])
        prev_expected_value["iops"] = float(json_array[array_len - 1]["rate"])
    result[1] = prev_expected_value
    return result


def CheckThrottledResult(limit_type, volume_list, volId, json_file):
    limit_value = None
    if volId in volume_list:
        limit_value = volume_list[volId]

    if limit_value is None:
        CheckQosThrottled(json_file, limit_type, limit_value, {}, 0)
        return True
    else:
        if limit_type == "iops":
            limit_value *= 1000
        result = CheckQosThrottled(json_file, limit_type, limit_value, {}, 0)
        return result[0]


# vd_disk_name : {"/dev/nvme0n1": nvme001, ...}
def CheckEachVolumeThrottled(init_name, volume_limit_list, vd_disk_names, test_vdbench, workload_name, volume_id_list):
    limit_type = volume_limit_list["type"]

    volume_list = {}  # {"1" 10, "3": 10 , "5": 30}
    volume_list = ParseVolumeDict(volume_limit_list)
    print(f" volume id list with limit value : {volume_list}")
    throttling_success = False
    for disk_name in vd_disk_names:
        result = {}
        file_name = test_vdbench.CopyHtmlResult(vd_disk_names[disk_name], init_name)
        json_file = test_vdbench.ParseHtmlResult(file_name, workload_name)
        volId = volume_id_list[disk_name]
        print(f" -- Initiator: {init_name} , disk name: {disk_name} , volume id: {volId} -- ")
        throttling_success = CheckThrottledResult(limit_type, volume_list, volId, json_file)
        if (throttling_success is False):
            print(f" disk name: {disk_name} (volume id:{volId}) failed throttling")
