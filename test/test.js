const Module = require('../out/fontconverter.js')
const wasm = Module({
    //arguments: ['RobotoMono-Regular.ttf', '5'],
})
const func = wasm.cwrap('generateFont', 'string', ['string', 'number', 'number'])

wasm.onRuntimeInitialized = function() {
    console.log('JSON Output:')
    console.log(func('monaco.ttf', 5, 1))

    console.log('C++ Output:')
    console.log('-->> Llamando función en C++:')
    console.log(func('RobotoMono-Regular.ttf', 5, 0))
}
