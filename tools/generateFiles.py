# SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

import re
import sys
import subprocess
import os
import rtoml
import shutil

# Root toml object
toml_obj = {'esp_toml_version': 1.0, 'firmware_images_url': f'https://dl.espressif.com/AE/esp-dev-kits/', 'supported_apps': []}

class App:

    def __init__(self, app):
        # App directory
        self.app_dir = app
        if app:
            # Name of the app
            self.name = app.split('/')[-1]
            # Name of the board
            self.boards = app.split('/')[1]
        # build_info_dict['target'] (esp32, esp32s2, etc)
        # build_info_dict['sdkconfig']
        # build_info_dict['build_dir']
        # build_info_dict['idf_version']
        self.build_info = []
        # App version
        self.app_version = ''

current_app = App(None)

# Regex to get the app_dir
def get_app_dir(line):
    return re.search(r'"app_dir"\s*:"([^,]*)",', line).group(1) if re.search(r'"app_dir"\s*:([^,]*)",', line) else None

# Regex to get the target
def get_target(line):
    return re.search(r'"target"\s*:"([^,]*)",', line).group(1) if re.search(r'"target"\s*:"([^,]*)",', line) else None

# Regex to get the kit
def get_sdkconfig(line):
    return re.search(r'"config_name"\s*:"([^,]*)",', line).group(1) if re.search(r'"config_name"\s*:"([^,]*)",', line) else None

def find_build_dir(
    app_path: str,
    target: str,
    config: str,
) -> list:
    """
    Check local build dir with the following priority:

    1. <app_path>/${IDF_VERSION}/build_<target>_<config>
    2. <app_path>/${IDF_VERSION}/build_<target>
    3. <app_path>/build_<target>_<config>
    4. <app_path>/build

    Args:
        app_path: app path
        target: target
        config: config

    Returns:
        valid build directory
    """

    # list all idf versions
    idf_version = ['4.3','4.4','5.0','5.1','5.2','5.3','5.4']

    assert target
    assert config
    check_dirs = []
    for i in idf_version:
        check_dirs.append((os.path.join(i, f'build_{target}_{config}'), f"v{i}"))
        check_dirs.append((os.path.join(i, f'build_{target}'), f"v{i}"))
    check_dirs.append((f'build_{target}_{config}', ''))
    check_dirs.append(('build', ''))
    build_dir = []
    for check_dir in check_dirs:
        binary_path = os.path.join(app_path, check_dir[0])
        if os.path.isdir(binary_path):
            print(f'find valid binary path: {binary_path}, idf version: {check_dir[1]}')
            build_dir.append(check_dir)
    return build_dir

def find_app_version(app_path: str)->str:
    pattern = r'(^|\n)version: "?([0-9]+).([0-9]+).([0-9]+)"?'

    for root, dirs, files in os.walk(app_path):
        for file_name in files:
            if file_name == 'idf_component.yml':
                file_path = os.path.join(root, file_name)

                try:
                    with open(file_path, 'r') as file:
                        content = file.read()
                    match = re.search(pattern, content)
                    if match:
                        major_version = match.group(2)
                        minor_version = match.group(3)
                        patch_version = match.group(4)
                        return f"{major_version}.{minor_version}.{patch_version}"
                    else:
                        return ""
                except Exception as e:
                    return f"error: {str(e)}"
    return ""

# Squash the json into a list of apps
def squash_json(input_str):
    global current_app
    # Split the input into lines
    lines = input_str.splitlines()
    output_list = []
    for line in lines:
        # Get the app_dir
        app = get_app_dir(line)
        # If its a not a None and not the same as the current app
        if current_app.app_dir != app:
            # Save the previous app
            if current_app.app_dir and current_app.build_info:
                output_list.append(current_app.__dict__)
            current_app = App(app)

        current_app.app_version = find_app_version(current_app.app_dir)
        build_dir = find_build_dir(current_app.app_dir, get_target(line), get_sdkconfig(line) if get_sdkconfig(line) else 'default')
        for dir in build_dir:
            build_info_dict = {}
            build_info_dict['target'] = get_target(line)
            build_info_dict['sdkconfig'] = get_sdkconfig(line)
            build_info_dict['build_dir'] = dir[0]
            build_info_dict['idf_version'] = dir[1]
            current_app.build_info.append(build_info_dict)

    # Append last app
    output_list.append(current_app.__dict__)
    print(output_list)
    return output_list

# Merge binaries for each app
def merge_binaries(apps):
    os.makedirs('binaries', exist_ok=True)
    for app in apps:
        # If we are merging binaries for kits
        if app.get('boards'):
            board = app['boards']
            for build_info in app['build_info']:
                target = build_info['target']
                sdkconfig = build_info['sdkconfig']
                build_dirs = build_info['build_dir']
                idf_version = build_info['idf_version']
                app_version = app['app_version']
                bin_name = f'{app["name"]}-{board}-{target}' + (f'-{idf_version}' if idf_version else '') + (f'-{app_version}' if app_version else '') + '.bin'
                cmd = ['esptool.py', '--chip', target, 'merge_bin', '-o', bin_name, '@flash_args']
                cwd = os.path.join(app.get("app_dir"), build_dirs)
                subprocess.run(cmd, cwd=cwd)
                print(f'Merged binaries for {bin_name}')
                shutil.move(f'{cwd}/{bin_name}', 'binaries')

# Write a single app to the toml file
def write_app(app):
    # If we are working with kits
    if app.get('boards'):
            board = app["boards"]
            for build_info in app['build_info']:
                idf_version = build_info['idf_version']
                app_version = app['app_version']
                bin_name = f'{app["name"]}-{board}-{build_info["target"]}' + (f'-{idf_version}' if idf_version else '') + (f'-{app_version}' if app_version else '') + '.bin'
                support_app = f'{app["name"]}-{board}' + (f'-{idf_version}' if idf_version else '')
                toml_obj[support_app] = {}
                toml_obj[support_app]['chipsets'] = [build_info["target"]]
                toml_obj[support_app][f'image.{build_info["target"]}'] = bin_name
                toml_obj[support_app]['android_app_url'] = ''
                toml_obj[support_app]['ios_app_url'] = ''

# Create the config.toml file
def create_config_toml(apps):
    for app in apps:
        if app.get('boards'):
            for build_info in app['build_info']:
                if build_info['idf_version']:
                    toml_obj['supported_apps'].extend([f'{app["name"]}-{app["boards"]}-{build_info["idf_version"]}'])
                else :
                    toml_obj['supported_apps'].extend([f'{app["name"]}-{app["boards"]}'])
        for app in apps:
            write_app(app)

        with open('binaries/config.toml', 'w') as toml_file:
            rtoml.dump(toml_obj, toml_file)

        # This is a workaround to remove the quotes around the image.<string> in the config.toml file as dot is not allowed in the key by default
        with open('binaries/config.toml', 'r') as toml_file:
            fixed = replace_image_string(toml_file.read())

        with open('binaries/config.toml', 'w') as toml_file:
            toml_file.write(fixed)

def replace_image_string(text):
    # Define the regular expression pattern to find "image.<string>"
    pattern = r'"(image\.\w+)"'
    # Use re.sub() to replace the matched pattern with image.<string>
    result = re.sub(pattern, r'\1', text)
    return result

# Get the output json file from idf_build_apps, process it, merge binaries and create config.toml
with open(sys.argv[1], 'r') as file:
    apps = squash_json(file.read())
    merge_binaries(apps)
    create_config_toml(apps)
