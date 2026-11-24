# Child pipeline template. tools/ci/select_doc_targets.py substitutes @DOCTGT_LIST@
# with the targets that actually changed. Do not include this file directly.
#
# Keep ESP_IDF_DOC_ENV_IMAGE in sync with .gitlab/ci/docs.yml.

workflow:
  rules:
    - when: always

stages:
  - build_doc
  - deploy

include:
  - local: '.gitlab/ci/rules.yml'
  - local: '.gitlab/ci/deploy_docs.yml'
  - local: '.gitlab/ci/publish_docs_to_cdp.yml'

variables:
  ESP_IDF_DOC_ENV_IMAGE: "$CI_DOCKER_REGISTRY/esp-idf-doc-env-v5.4:1-1"
  DOC_TARGETS: '@DOCTGT_LIST@'
  LC_ALL: "C.UTF-8"

.build_docs_job: &build_docs_job
  image: $ESP_IDF_DOC_ENV_IMAGE
  stage: build_doc
  tags:
    - build_docs
  retry: 2
  parallel:
    matrix:
      - DOCLANG: ["en", "zh_CN"]
        DOCTGT: @DOCTGT_LIST@

build_docs_html:
  <<: *build_docs_job
  script:
    - cd docs
    - ./check_lang_folder_sync.sh
    - python -m pip install -r requirements.txt
    - echo "Building HTML docs for target $DOCTGT language $DOCLANG"
    - build-docs --skip-reqs-check -l $DOCLANG -t $DOCTGT
    - ARTIFACT_URL="$CI_JOB_URL/artifacts/file/docs/_build/$DOCLANG/$DOCTGT/html/index.html"
    - echo "esp-dev-kits documentation preview in HTML is available at $ARTIFACT_URL"
  artifacts:
    when: always
    paths:
      - docs/_build/$DOCLANG/$DOCTGT/*.txt
      - docs/_build/$DOCLANG/$DOCTGT/html/
    expire_in: 4 days

build_docs_pdf:
  <<: *build_docs_job
  script:
    - cd docs
    - ./check_lang_folder_sync.sh
    - python -m pip install -r requirements.txt
    - echo "Building PDF docs for target $DOCTGT language $DOCLANG"
    - build-docs --skip-reqs-check -bs latex -l $DOCLANG -t $DOCTGT
    - ARTIFACT_URL="$CI_JOB_URL/artifacts/file/docs/_build/$DOCLANG/$DOCTGT/latex/build/esp-dev-kits-$DOCLANG-master-$DOCTGT.pdf"
    - echo "esp-dev-kits documentation preview in PDF is available at $ARTIFACT_URL"
  artifacts:
    when: always
    paths:
      - docs/_build/$DOCLANG/$DOCTGT/latex/
      - docs/_build/$DOCLANG/$DOCTGT/*.txt
    expire_in: 4 days
