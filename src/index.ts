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
    private _convert!: (...args: any[]) => string
    private _convertcb!: (...args: any[]) => void
    private cbConverter!: (result: ConverterResult) => void
    private cbConverterPointer?: number
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

    // Wrapper that allows run the callaback, without it being "memorized" in the wasm pointer table.
    private cbConverterWrapper(result: ConverterResult): void {
        if (this.cbConverter === undefined) {
            return
        }
        this.cbConverter(result)
    }

    public convertcb(fileName: string, fontSize: number, cbConverter: (result: ConverterResult) => void): void {
        if (this.emsModule == undefined) {
            throw 'FontConverter not Initiliazed.'
        }
        if (this._convertcb === undefined) {
            this._convertcb = this.emsModule.cwrap('convertcb', 'void', ['string', 'number', 'number', 'number', 'number', 'pointer'])

            this.cbConverterPointer = this.emsModule.addFunction((code, errorCode, errorMessage) => {
                const result: ConverterResult = {
                    code: this.emsModule?.AsciiToString(code),
                    errorCode: errorCode,
                    errorMessage: this.emsModule?.AsciiToString(errorMessage)
                }

                // The callback wrapper is executed
                this.cbConverterWrapper(result)
            }, 'viii')
        }

        // The callback is passed to the wrapper through a class variable
        this.cbConverter = cbConverter
        this._convertcb(fileName, fontSize, this.outType, this.firstChar, this.lastChar, this.cbConverterPointer)
    }

    async initialize(filePath?: string): Promise<void> {
        return new Promise<void>((resolve, reject) => {
            try {
                // Try load wasm module.
                this.emsModule = require('./fontconverter.js')({
                    noInitialRun: true,
                    wasmBinary: filePath,
                    locateFile: (path: string, prefix: string) => {
                        if (filePath === undefined) {
                            return prefix + path
                        }
                        return filePath
                    },
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
