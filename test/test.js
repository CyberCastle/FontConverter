const Module = require('../dist/fontconverter.js')
const wasm = Module({
    //arguments: ["rRobotoMono-Regular.ttf", "5"],
    noInitialRun: true
})

function cb(code, errorCode, errorMessage) {
    console.log(wasm.AsciiToString(code))
    console.log('ErrorCode: ' + errorCode)
    console.log('Error Message:', wasm.AsciiToString(errorMessage))
}

const cbPtr = wasm.addFunction(cb, 'viii')
const func = wasm.cwrap('convert', 'string', ['string', 'number', 'number', 'number', 'number'])
const func_cb = wasm.cwrap('convertcb', null, ['string', 'number', 'number', 'number', 'number', 'pointer'])

wasm.onRuntimeInitialized = function() {
    console.log('JSON Output:')
    console.log(func('monaco.ttf', 5, 1, 50, 55))

    console.log('C++ Output:')
    func_cb('RobotoMono-Regular.ttf', 5, 0, null, null, cbPtr)

    console.log('C++ Output with error:')
    func_cb('font-doesnt-exist.ttf', 5, 0, null, null, cbPtr)
}
