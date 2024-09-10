# -*- coding: utf-8 -*-
#
# Common (non-language-specific) configuration for Sphinx
#
# This file is imported from a language-specific conf.py (ie en/conf.py or
# zh_CN/conf.py)

#ESP_DOCS_PATH = os.environ['ESP_DOCS_PATH']

from esp_docs.conf_docs import *  # noqa: F403,F401

# format: {tag needed to include: documents to included}, tags are parsed from sdkconfig and peripheral_caps.h headers

extensions += [ 'sphinx_copybutton',
                'sphinxcontrib.wavedrom',
                'esp_docs.esp_extensions.dummy_build_system',
                'esp_docs.esp_extensions.run_doxygen',
                ]
# link roles config
github_repo = 'espressif/esp-dev-kits'

# context used by sphinx_idf_theme
html_context['github_user'] = 'espressif'
html_context['github_repo'] = 'esp-dev-kits'

languages = ['en', 'zh_CN']
idf_targets = ['esp32', 'esp32s2', 'esp32s3', 'esp32c3', 'esp32c6', 'esp32h2', 'esp32c2', 'esp32p4', 'esp32c5']

ESP32_DOCS = ['esp32/esp-wrover-kit/*.rst',
              'esp32/esp32-devkitc/*.rst',
              'esp32/esp32-ethernet-kit/*.rst',
              'esp32/esp32-lcdkit/*.rst',
              'esp32/esp32-meshkit-sensor/*.rst',
              'esp32/esp32-pico-devkitm-2/*.rst',
              'esp32/esp32-pico-kit/*.rst',
              'esp32/esp32-pico-kit-1/*.rst',
              'esp32/esp32-sense-kit/*.rst',
              'other/esp-module-prog-1-r/*.rst',
              'other/esp-module-prog-sub-02/*.rst',
              'other/esp-module-prog-sub-01&04/*.rst',
              'other/esp-prog/*.rst',
              'other/esp-prog/reference/*.rst'
              ]
ESP32C2_DOCS = ['esp8684/esp8684-devkitc-02/*.rst',
                'esp8684/esp8684-devkitm-1/*.rst',
                'other/esp-module-prog-1-r/*.rst',
                'other/esp-module-prog-sub-02/*.rst',
                'other/esp-module-prog-sub-01&04/*.rst'
                ]
ESP32C3_DOCS = ['esp32c3/esp32-c3-devkitc-02/*.rst',
                'esp32c3/esp32-c3-devkitm-1/*.rst',
                'esp32c3/esp32-c3-lcdkit/*.rst',
                'other/esp-module-prog-1-r/*.rst',
                'other/esp-module-prog-sub-02/*.rst',
                'other/esp-module-prog-sub-01&04/*.rst',
                'other/esp-prog/*.rst',
                'other/esp-prog/reference/*.rst'
                ]
ESP32C6_DOCS = ['esp32c6/esp32-c6-devkitc-1/*.rst',
                'esp32c6/esp32-c6-devkitm-1/*.rst',
                'other/esp-module-prog-1-r/*.rst',
                'other/esp-module-prog-sub-02/*.rst',
                'other/esp-module-prog-sub-01&04/*.rst'
                ]
ESP32H2_DOCS = ['esp32h2/esp32-h2-devkitm-1/*.rst',
                'other/esp-module-prog-1-r/*.rst',
                'other/esp-module-prog-sub-02/*.rst',
                'other/esp-module-prog-sub-01&04/*.rst'
                ]
ESP32P4_DOCS = ['esp32p4/esp32-p4-function-ev-board/*.rst'
                ]
ESP32C5_DOCS = ['esp32c5/esp32-c5-devkitc-1/*.rst'
                ]
ESP32S2_DOCS = ['esp32s2/esp32-s2-devkitc-1/*.rst',
                'esp32s2/esp32-s2-devkitm-1/*.rst',
                'esp32s2/esp32-s2-hmi-devkit-1/*.rst',
                'esp32s2/esp32-s2-hmi-devkit-1/reference/*.rst',
                'esp32s2/esp32-s2-kaluga-1/*.rst',
                'esp32s2/esp32-s2-touch-devkit-1/*.rst',
                'other/esp-module-prog-1-r/*.rst',
                'other/esp-module-prog-sub-02/*.rst',
                'other/esp-module-prog-sub-01&04/*.rst',
                'other/esp-prog/*.rst',
                'other/esp-prog/reference/*.rst'
                ]
ESP32S3_DOCS = ['esp32s3/esp32-s3-lcd-ev-board/*.rst',
                'esp32s3/esp32-s3-usb-bridge/*.rst',
                'esp32s3/esp32-s3-usb-otg/*.rst',
                'other/*.rst',
                'other/esp-module-prog-1-r/*.rst',
                'other/esp-module-prog-sub-02/*.rst',
                'other/esp-module-prog-sub-01&04/*.rst',
                'other/esp-prog/*.rst',
                'other/esp-prog/reference/*.rst'
                ]

conditional_include_dict = {'esp32':ESP32_DOCS,
                            'esp32c2':ESP32C2_DOCS,
                            'esp32c3':ESP32C3_DOCS,
                            'esp32c6':ESP32C6_DOCS,
                            'esp32s2':ESP32S2_DOCS,
                            'esp32s3':ESP32S3_DOCS,
                            'esp32h2':ESP32H2_DOCS,
                            'esp32p4':ESP32P4_DOCS,
                            'esp32c5':ESP32C5_DOCS
                            }

project_homepage = 'https://github.com/espressif/esp-dev-kits'

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
html_logo = "../_static/espressif-logo.svg"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['../_static']

# Extra options required by sphinx_idf_theme
project_slug = 'esp-dev-kits'

# Contains info used for constructing target and version selector
# Can also be hosted externally, see esp-idf for example
versions_url = './_static/docs_version.js'

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
exclude_patterns = ['_build','README.md']

# Final PDF filename will contains target and version
pdf_file_prefix = u'esp-dev-kits'

# The name of an image file (relative to this directory) to place at the top of
# the title page.
latex_logo = "../_static/espressif2.pdf"

# Measurement ID for Google Analytics

google_analytics_id = 'G-6GCDQQ87G0'
