web-wasm := docker run --rm --user=emscripten -it -v ${PWD}:/build -w /build dockcross/web-wasm
rom-js := docker run -e TARGET=${TARGET} -it --entrypoint "" -w /work -v ${PWD}:/work rom-js:v0.1
pitzDaily-dir := /build/offline/OpenFOAM/incompressible/simpleFoam/pitzDaily

compile: install-compile third-party-compile emcc-third-party-compile rom-compile build-compile

prebuilt: install-prebuilt rom-prebuilt build-prebuilt

test: test-install test-run

install-compile:
	npm install
third-party-compile:
	./non_emcc_third_party.sh
emcc-third-party-compile:
	$(web-wasm) ./emcc_third_party.sh
rom-compile:
	$(web-wasm) ./emcc_rom.sh
build-compile:
	TARGET="node" npm run build

install-prebuilt:
	$(rom-js) npm install
rom-prebuilt:
	$(rom-js) ./emcc_rom.sh
build-prebuilt:
	$(rom-js) npm run build

pitzDaily:
	$(rom-js) cd $(pitzDaily) && ./Allrun
test-install:
	npm install --prefix tests/pitzDaily
test-run:
	cd tests/pitzDaily && node pitzDaily.mjs 3.0 0.00001
