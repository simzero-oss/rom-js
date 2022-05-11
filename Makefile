# Copyright (c) 2022 Carlos Peña-Monferrer. All rights reserved.
# This work is licensed under the terms of the GNU LGPL v3.0 license.
# For a copy, see <https://opensource.org/licenses/LGPL-3.0>.

SHELL := /bin/bash

version=v1.0.0-rc.5
web-wasm := docker run --rm --user=emscripten -it -v ${PWD}:/build -w /build dockcross/web-wasm
rom-js-image := ghcr.io/simzero-oss/rom-js:$(version)
rom-js := docker run -e TARGET=${TARGET} -it --entrypoint "" -w /work -v ${PWD}:/work $(rom-js-image)
rom-js-rom := docker run -e TARGET=${TARGET} -it -w /work -v ${PWD}:/work $(rom-js-image)
openfoam-dir := third_party/openfoam/etc/bashrc
data-url := https://github.com/simzero-oss/cfd-xyz-data/raw/main/surrogates_$(version).tar.gz

all: install emcc-rom run-build
test: test-install test-run

compiled-emcc: compiled-install compiled-emcc-third-party compiled-emcc-rom compiled-build
compiled-test: compiled-test-install compiled-test-run

install:
	$(rom-js) npm install
emcc-rom:
	$(rom-js) ./emcc_rom.sh
run-build:
	$(rom-js) npm run build
rom:
	$(rom-js-rom) /bin/bash -c "cd /work/offline && ./Allrun ${CORES}"
rom-clean:
	$(rom-js-rom) /bin/bash -c "cd /work/offline && ./Allclean"
data:
	$(rom-js) curl -LJ0 $(data-url) -o surrogates.tar.gz
	tar -zxvf surrogates.tar.gz -C ./
test-install:
	$(rom-js) npm install --prefix tests/steady
test-run:
	$(rom-js) /bin/bash -c "cd tests/steady && ./Allrun"
compiled-install:
	npm install
compiled-non-emcc:
	./non_emcc_third_party.sh
compiled-emcc-third-party:
	$(web-wasm) ./emcc_third_party.sh
compiled-emcc-rom:
	$(web-wasm) ./emcc_rom.sh
compiled-build:
	TARGET="node" npm run build
compiled-rom:
	source $(openfoam-dir) && cd offline && ./Allrun ${CORES}
compiled-test-install:
	npm install --prefix tests/pitzDaily
compiled-test-run:
	cd tests/pitzDaily && node pitzDaily.mjs 10.0 0.00001 && node pitzDaily.mjs 5.0 0.00005 && node pitzDaily.mjs 15.0 0.0001
pack:
	tar -zcvf surrogates_$(version).tar.gz surrogates
