# Documentation Source Folder

This folder contains source files of **ESP-IDF documentation** available in [English](https://docs.espressif.com/projects/esp-dev-kits/en/latest/index.html) and [中文](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/index.html).

The sources do not render well in GitHub and some information is not visible at all.

Use actual documentation generated within about 20 minutes on each commit:

# Hosted Documentation

* English: https://docs.espressif.com/projects/esp-dev-kits/en/latest/index.html
* 中文: https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/index.html

The above URLs are all for the master branch latest version.

# Building Documentation

The documentation is built using the python package `esp-docs`, which can be installed by running `pip install esp-docs`. Running `build-docs --help` will give a summary of available options. For more information see the `esp-docs` documentation at https://github.com/espressif/esp-docs/blob/master/README.md

If you need to compile html files in this repository, you can run:

```
cd docs
./build.sh
```

<details>
<summary>Deduplicating Static Resources</summary>

Sphinx copies the entire `docs/_static` directory into every language and target
site. This includes images for other boards, so a full documentation build
contains many copies of the same resources. gzip does not effectively eliminate
these copies, resulting in a large deployment archive.

`tools/ci/deduplicate_doc_static.py` shares identical `_static` resources across
the sites. It retains the first copy in sorted language/target/path order and
rewrites references in HTML, CSS, JavaScript and SVG to that copy. Text resources
with different relative dependencies remain separate. `_images`, `_downloads`
and PDFs are left unchanged.

Run the following commands from the repository root. To inspect potential
savings without changing files:

```bash
python tools/ci/deduplicate_doc_static.py docs/_build --version my-preview --dry-run
```

To optimize a build directory in place, omit `--dry-run`. Set `--version` to the
deployment directory name. Generated links use the deployed
`language/version/target` layout, so the optimized sites must be deployed
together rather than previewed individually from their build directories.

The CI preview deployment job uses the deployment wrapper:

```bash
python -u tools/ci/deduplicate_doc_static.py --deploy --allow-empty
```

`--allow-empty` lets preview deployment succeed without uploading when incremental
builds produced no HTML or final PDFs. Production deployment omits this flag and
requires documentation build outputs.

This mode uses the existing `esp-docs` deployment environment variables, including
`DOCS_BUILD_DIR` and `DOCS_DEPLOY_URL_BASE`. It optimizes a temporary staging copy
before packaging and uploading, preserving the original build artifacts for
local and GitLab Job previews. Links are generated separately for the requested
version and any additional `stable` deployment.

For standalone use, provide `--url-base https://host/projects/esp-dev-kits` if
pages contain absolute URLs to the deployment. Resources addressed through
dynamically assembled JavaScript URLs can be preserved with repeatable `--keep`
patterns, relative to the build root:

```bash
python -u tools/ci/deduplicate_doc_static.py --deploy --keep '*/*/html/_static/runtime/*'
```

The tool prints flushed progress lines every five seconds, including during
compression and upload, in both interactive terminals and CI logs. Use
`--progress-interval SECONDS` to change the interval. The final output reports
duplicate files, bytes saved, rewritten references and compressed archive size
when deploying.

</details>

# Preview

To preview the compiled html files locally:

* Start a local web page

    ```
    python3 -m http.server 8000 --directory _build/zh_CN/generic/html
    ```

* Enter the URL: http://localhost:8000/ for online preview
