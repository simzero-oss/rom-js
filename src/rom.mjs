import rom from '../build/rom.js'
import wasm from '../build/rom.wasm'

const Module = rom({
  wasmBinary: wasm
});

rom.ready = Module.then(module => {
  rom["Vector"] = module["Vector"]
  rom["VectorVector"] = module["VectorVector"]
  rom["reducedSteady"] = module["reducedSteady"]
})

export default rom
