web-wasm := docker run --rm --user=emscripten -it -v ${PWD}/build -w /build dockcross/web-wasm
rom-js := docker run -e TARGET=${TARGET} -it --entrypoint "" -w /work -v ${PWD}:/work rom-js:v0.1
rom-js-rom := docker run -e TARGET=${TARGET} -it -w /work -v ${PWD}:/work rom-js:v0.1
pitzDaily-dir := /work/offline/OpenFOAM/incompressible/simpleFoam/pitzDaily

all: install emcc-rom build
test: test-install test-run

compiled-emcc: compiled-install compiled-emcc-third-party compiled-emcc-rom compiled-build
compiled-test: compiled-test-install compiled-test-run

install:
	$(rom-js) npm install
emcc-rom:
	$(rom-js) ./emcc_rom.sh
build:
	$(rom-js) npm run build
rom:
	$(rom-js-rom) /bin/bash -c "cd $(pitzDaily-dir) && ./Allrun"
test-install:
	$(rom-js) npm install --prefix tests/pitzDaily
test-run:
	$(rom-js) cd tests/pitzDaily && node pitzDaily.mjs 3.0 0.00001
compile-install:
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
	cd $(pitzDaily-dir) && ./Allrun
compiled-test-install:
	npm install --prefix tests/pitzDaily
compiled-test-run:
	cd tests/pitzDaily && node pitzDaily.mjs 3.0 0.00001
