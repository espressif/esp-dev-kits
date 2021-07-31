import os
import sys
# sys.path.insert(0, os.path.abspath('/home/zhouli/Work/esp-dev-kits/esp-docs/src'))
# sys.path.insert(0, os.path.abspath('/home/zhouli/Work/esp-dev-kits/esp-docs/src/esp_docs'))
from esp_docs.conf_docs import *  # noqa: F403,F401

languages = ['en', 'zh_CN']
idf_targets = ['esp32']

# link roles config
github_repo = 'espressif/esp-dev-kits'

# context used by sphinx_idf_theme
html_context['github_user'] = 'espressif'
html_context['github_repo'] = 'esp-docs'

# Extra options required by sphinx_idf_theme
project_slug = 'esp-dev-kits'
versions_url = 'https://dl.espressif.com/dl/esp-idf/idf_versions.js'
# versions_url = './idf_versions.js'
