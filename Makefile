web-wasm := docker run --rm --user=emscripten -it -v ${PWD}/build -w /build dockcross/web-wasm
rom-js := docker run -e TARGET=${TARGET} -it --entrypoint "" -w /work -v ${PWD}:/work rom-js:v0.1
rom-js-rom := docker run -e TARGET=${TARGET} -it -w /work -v ${PWD}:/work rom-js:v0.1
pitzDaily-dir := /work/offline/OpenFOAM/incompressible/simpleFoam/pitzDaily

all: install emcc-rom build
compile: install-compile third-party-compile emcc-third-party-compile emcc-rom-compile build-compile
test: test-install test-run

install:
	$(rom-js-rom) npm install
emcc-rom:
	$(rom-js-rom) ./emcc_rom.sh
build:
	$(rom-js-rom) npm run build
rom:
	$(rom-js-rom) /bin/bash -c "cd $(pitzDaily-dir) && ./Allrun"
test-install:
	$(rom-js) npm install --prefix tests/pitzDaily
test-run:
	$(rom-js) cd tests/pitzDaily && node pitzDaily.mjs 3.0 0.00001

install-compile:
	npm install
third-party-compile:
	./non_emcc_third_party.sh
emcc-third-party-compile:
	$(web-wasm) ./emcc_third_party.sh
emcc-rom-compile:
	$(web-wasm) ./emcc_rom.sh
build-compile:
	TARGET="node" npm run build
rom-compile:
	cd $(pitzDaily-dir) && ./Allrun
test-install-compile:
	npm install --prefix tests/pitzDaily
test-run-compile:
	cd tests/pitzDaily && node pitzDaily.mjs 3.0 0.00001
