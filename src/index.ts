export enum OutputConverterType {
    CPlus = 0,
    Json = 1
}

export interface ConverterResult {
    readonly code?: string
    readonly errorCode: number
    readonly errorMessage?: string
}

export class FontConverter {
    private emsModule?: EmscriptenModule
    private _convert!: (...args: any[]) => any
    private _convertcb!: (...args: any[]) => any
    private _callbackPtr?: number
    private firstChar?: number
    private lastChar?: number

    constructor(private outType: OutputConverterType = 0) {}

    public setOutType(outType: OutputConverterType) {
        this.outType = outType
    }

    public setCharRange(firstChar: number, lastChar: number) {
        this.firstChar = firstChar
        this.lastChar = lastChar
    }

    public convert(fileName: string, fontSize: number): string {
        if (this.emsModule == undefined) {
            throw 'FontConverter not Initiliazed.'
        }
        if (this._convert === undefined) {
            this._convert = this.emsModule.cwrap('convert', 'string', ['string', 'number', 'number', 'number', 'number'])
        }
        return this._convert(fileName, fontSize, this.outType, this.firstChar, this.lastChar)
    }

    private _callbackFunc(result: ConverterResult, cb: (result: ConverterResult) => void): void {
        cb(result)
    }

    public convertcb(fileName: string, fontSize: number, cb: (result: ConverterResult) => void): void {
        if (this.emsModule == undefined) {
            throw 'FontConverter not Initiliazed.'
        }
        if (this._convertcb === undefined) {
            this._convertcb = this.emsModule.cwrap('convertcb', 'void', ['string', 'number', 'number', 'number', 'number', 'pointer'])

            this._callbackPtr = this.emsModule.addFunction((code, errorCode, errorMessage) => {
                const result: ConverterResult = {
                    code: this.emsModule?.AsciiToString(code),
                    errorCode: errorCode,
                    errorMessage: this.emsModule?.AsciiToString(errorMessage)
                }
                this._callbackFunc(result, cb)
            }, 'viii')
        }
        this._convertcb(fileName, fontSize, this.outType, this.firstChar, this.lastChar, this._callbackPtr)
    }

    async initialize(): Promise<void> {
        return new Promise<void>((resolve, reject) => {
            try {
                // Try load wasm module.
                this.emsModule = require('./fontconverter.js')({
                    //arguments: ['RobotoMono-Regular.ttf', '5'],
                    noInitialRun: true,
                    onRuntimeInitialized: () => {
                        // An Emscripten is a then-able that resolves with itself, causing an infite loop when you
                        // wrap it in a real promise. Delete the `then` prop solves this for now.
                        // https://github.com/kripken/emscripten/issues/5820
                        console.info('WASM FontConverter module loaded.')
                        delete (this.emsModule as any).then
                        resolve()
                    }
                })
            } catch (e) {
                // Error to load wasm module.s
                reject(e)
            }
        })
    }
}

;(async () => {
    const converter = new FontConverter(1)
    await converter.initialize()

    //console.log(converter.convert('./test/monaco.ttf', 5))

    converter.convertcb('./test/monaco.ttf', 5, result => {
        console.log('-- eee0: ' + result.errorCode)
    })

    converter.convertcb('./test/4monaco.ttf', 5, result => {
        console.log('-- eee1: ' + result.errorCode)
    })
    converter.convertcb('./test/monaco.ttf', 5, result => {
        console.log('-- eee2: ' + result.errorCode)
    })
    converter.convertcb('./test/monaco.ttf', 5, result => {
        console.log('-- eee4: ' + result.errorCode)
    })
    converter.convertcb('./test/4monaco.ttf', 5, result => {
        console.log('-- eee5: ' + result.errorCode)
    })
    //console.log('pasé!!!')
})()
