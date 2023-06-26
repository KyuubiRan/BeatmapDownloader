import http.client
import json
import os
import time

version_file = "./src/dlver.h"

class Version:
    def __init__(self):
        self.version = -1
        self.version_str = "unknown"


def update_version_file(ver: Version):
    with open(version_file, "w") as f:
        f.write("#pragma once\n")
        f.write("\n")
        f.write("#define LATEST_UPDATE_TIMESTAMP " + str(int(time.time())) + "\n")
        f.write("#define DOWNLOADER_VERSION " + str(ver.version) + "\n")
        f.write('#define DOWNLOADER_VERSION_STR "' + ver.version_str + '"\n')


def get_last_update_timestamp() -> int:
    with open(version_file, "r") as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("#define LATEST_UPDATE_TIMESTAMP"):
                return int(line.split(" ")[2])
    return 0


def create_default_version_file_if_not_exists():
    if os.path.exists(version_file):
        return

    with open(version_file, "w") as f:
        f.write("#pragma once\n")
        f.write("\n")
        f.write("#define LATEST_UPDATE_TIMESTAMP 0\n")
        f.write("#define DOWNLOADER_VERSION -1\n")
        f.write('#define DOWNLOADER_VERSION_STR "unknown"\n')


def get_version() -> Version:
    ver = Version()
    try:
        conn = http.client.HTTPSConnection("api.github.com")
        headers = {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36",
            "Accept": "application/vnd.github+json",
            "X-GitHub-Api-Version": "2022-11-28",
        }
        conn.request(
            "GET", "/repos/KyuubiRan/BeatmapDownloader/releases/latest", headers=headers
        )
        response = conn.getresponse()
        code = response.getcode()
        data = response.read()
        de = data.decode("utf-8")
        j = json.loads(de)

        if code != 200:
            print("get version failed, code: ", code)
            print("msg: ", j["message"])
            return ver

        version = j["tag_name"]
        ver.version_str = j["name"]
        ver.version = int(version)
        return ver
    except Exception as e:
        print(e)
        return ver


def main():
    create_default_version_file_if_not_exists()
    last_update_timestamp = get_last_update_timestamp()
    # 5 min
    if int(time.time()) - last_update_timestamp < 5 * 60:
        print("update too frequently, skip")
        return

    version = get_version()
    if version.version == -1:
        print("get version failed")
        return

    print(f"current version: {version.version_str}({version.version})")
    version.version += 1
    version.version_str = str(float(version.version_str) + 0.1)

    update_version_file(version)


if __name__ == "__main__":
    main()
