import os
import toml

def check_launchpad_apps(config):
    data = toml.loads(config)

    supported_apps = data['supported_apps']

    for app_name in supported_apps:
        if app_name not in data:
            error_str = f"Missing '{app_name}'"
            raise AssertionError(error_str)
        else:
            print(f"Found '{app_name}'")

    print("validation successful.")

if __name__ == '__main__':
    # Get the path to the directory of the current script
    script_directory = os.path.dirname(os.path.abspath(__file__))
    # Construct the path to the TOML file
    config_file_path = os.path.join(script_directory, '../../', 'launch.toml')
    # Read the TOML configuration from a file
    with open(config_file_path, 'r') as file:
        config = file.read()
    check_launchpad_apps(config)
