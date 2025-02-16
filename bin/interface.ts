import FFLBinary from "./ffl_wasm";

// Sorry, I'm sort of a noob when it comes to typing functions.

interface EmscriptenInterface {
    HEAP8: Int8Array,
    HEAPU8: Uint8Array,
    HEAPU32: Uint32Array,
    HEAPU16: Uint16Array,
    HEAPF32: Float32Array,
    GL: {
        createContext: Function,
        makeContextCurrent: Function
    },
    ccall: Function
};
export interface FFLTextureInterface {
    width: number | undefined,
    height: number | undefined,
    data: Uint8Array | undefined
};
export interface FFLMeshInterface {
    vertexCount: number,
    indexCount: number,
    color: {
        r: number,
        g: number,
        b: number
    },
    offset: number,
    positionData: Float32Array,
    texCoordData: Float32Array,
    normalData: Float32Array,
    tangentData: Float32Array,
    indexData: Uint16Array
}

export default class FFLInterface {
    async init(FFLBuffer: Uint8Array, FFLResolution?: number) {
        return new Promise<void>(async resolve => {
            this.binary = await FFLBinary();
            
            this.glContext = this.binary?.GL.createContext(
                document.createElement("canvas"), 
                {majorVersion: 1}
            );
            this.binary?.GL.makeContextCurrent(this.glContext);

            let bufferAddress = this.binary?.ccall("init", ["number"], ["number"], [FFLBuffer.byteLength]);
            this.binary?.HEAPU8.set(FFLBuffer, bufferAddress);
            this.miiAddress = this.binary?.ccall("init", ["number"], ["number"], [FFLResolution]);
            
            return resolve();
        })
    };
    setMii(FFLMiiBuffer: Uint8Array, FFLAllExpressions: boolean = false): boolean  {
        this.binary?.HEAPU8.set(FFLMiiBuffer, this.miiAddress);
        let status: boolean = this.binary?.ccall("mii", ["boolean"], ["boolean"], [FFLAllExpressions]);
        
        if (status) {
            this.binary?.ccall("drawXlu");
            this.binary?.ccall("drawFaceline");
            return true;
        } else
            return false;
    };
    drawFaceMask(FFLExpression: number = 0) {
        this.binary?.ccall("drawFaceTexture", null, ["number"], [FFLExpression]);
    };

    getTexture(FFLTexture: string): FFLTextureInterface | undefined {
        if (!this.binary) return;
        // TODO: add a success in the wrapper
        let textureAddress = this.binary?.ccall("exportTexture", ["number"], ["string"], [FFLTexture]);
        
        let width = this.binary?.HEAPU32[textureAddress / 4];
        let height = this.binary?.HEAPU32[(textureAddress / 4) + 1];

        if (!width || !height)
            return;

        return {
            data: this.binary?.HEAPU8.slice(
                this.binary?.HEAPU32[(textureAddress / 4) + 2],
                this.binary?.HEAPU32[(textureAddress / 4) + 2] + (width * height * 4)
            ), width, height
        };
    };
    getMesh(FFLMesh: string): FFLMeshInterface | undefined {
        if (!this.binary) return;
        
        let pointer = this.binary?.ccall("getMesh", "number", ["string"], [FFLMesh]);
        let vertexCount = this.binary?.HEAPU32[pointer / 4];
        let indexCount = this.binary?.HEAPU32[(pointer / 4) + 1];

        let data = {
            color: {
                r: this.binary?.HEAPF32[(pointer / 4) + 2] * 255,
                g: this.binary?.HEAPF32[(pointer / 4) + 3] * 255,
                b: this.binary?.HEAPF32[(pointer / 4) + 4] * 255
            },
            offset: (pointer / 4) + 5,
            vertexCount, indexCount
        } as FFLMeshInterface;

        data.positionData = this.binary?.HEAPF32.slice(this.binary?.HEAPU32[data.offset] / 4, (this.binary?.HEAPU32[data.offset] / 4 + vertexCount * 4)); data.offset++;
        data.texCoordData = this.binary?.HEAPU32[data.offset] !== 0 ? this.binary?.HEAPF32.slice(this.binary?.HEAPU32[data.offset] / 4, (this.binary?.HEAPU32[data.offset] / 4 + vertexCount * 2)) : new Float32Array(0); data.offset++;
        data.normalData = this.binary?.HEAPU32[data.offset] !== 0 ? this.binary?.HEAPF32.slice(this.binary?.HEAPU32[data.offset] / 4, (this.binary?.HEAPU32[data.offset] / 4 + vertexCount * 3)) : new Float32Array(0); data.offset++;
        data.tangentData = this.binary?.HEAPU32[data.offset] !== 0 ? this.binary?.HEAPF32.slice(this.binary?.HEAPU32[data.offset] / 4, (this.binary?.HEAPU32[data.offset] / 4 + vertexCount * 3)) : new Float32Array(0); data.offset++;

        data.indexData = this.binary?.HEAPU16.slice(this.binary?.HEAPU32[data.offset] / 2, (this.binary?.HEAPU32[data.offset] / 2 + (indexCount * 2))); data.offset++;

        return data;
    };

    private glContext: Object | undefined;
    private miiAddress: number = 0;
    private binary: EmscriptenInterface | undefined;
};