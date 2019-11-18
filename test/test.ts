import { FontConverter } from '../dist/index'

// Main function
;(async () => {
    const converter = new FontConverter(1)
    await converter.initialize()

    const code: string = converter.convert('./test/monaco.ttf', 5)
    console.log('JSON Output:')
    console.log(code)

    converter.setOutType(0)
    converter.setCharRange('A'.charCodeAt(0), 'Z'.charCodeAt(0))

    converter.convertcb('./test/monaco.ttf', 5, result => {
        console.log('C++ Output:')
        console.log(result.code)
    })

    converter.convertcb('font-doesnt-exist.ttf', 5, result => {
        console.log('C++ Output with error:')
        console.log('Error Code: ' + result.errorCode)
        console.log('Error Message: ' + result.errorMessage)
    })
})()
