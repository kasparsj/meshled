/**
 * Firmware HTTP API response parsers and JSDoc "types" used by hooks.
 * Keeping parsing centralized helps catch contract drift early.
 */

/**
 * @typedef {{r:number,g:number,b:number,w:number}} ApiColor
 * @typedef {{colors:string[],positions:number[],colorRule:number,interMode:number,wrapMode:number,segmentation:number}} ApiPalette
 * @typedef {{id:number,visible:boolean,brightness:number,speed:number,fadeSpeed:number,easing:number,blendMode:number,behaviourFlags:number,offset:number,palette:ApiPalette}} ApiLayer
 * @typedef {{colors:ApiColor[],step:number,totalPixels:number}} ApiColorsResponse
 * @typedef {{id:number,group:number,numPorts:number,topPixel:number,bottomPixel:number,ports:Array<{id:number,type:string,direction:boolean,group:number,device?:string,targetId?:number}|null>}} ApiIntersection
 * @typedef {{group:number,fromPixel:number,toPixel:number,numLeds:number,pixelDir:number}} ApiConnection
 * @typedef {{id:number,defaultW:number,emitGroups:number,maxLength:number}|null} ApiModel
 * @typedef {{fromPixel:number,toPixel:number}} ApiGap
 * @typedef {{pixelCount:number,realPixelCount:number,modelCount:number,gapCount:number,intersections:ApiIntersection[],connections:ApiConnection[],models:ApiModel[],gaps:ApiGap[]}} ApiModelData
 */

const asNumber = (value, fallback = 0) => {
    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : fallback;
};

const asBoolean = (value, fallback = false) => {
    if (typeof value === 'boolean') return value;
    if (value === 'true') return true;
    if (value === 'false') return false;
    return fallback;
};

const asArray = (value) => (Array.isArray(value) ? value : []);

const parsePalette = (value = {}) => ({
    colors: asArray(value.colors).map((color) => String(color)),
    positions: asArray(value.positions).map((position) => asNumber(position, 0)),
    colorRule: asNumber(value.colorRule, -1),
    interMode: asNumber(value.interMode, 1),
    wrapMode: asNumber(value.wrapMode, 0),
    segmentation: asNumber(value.segmentation, 0),
});

/**
 * @param {unknown} payload
 * @returns {ApiLayer[]}
 */
export const parseLayersResponse = (payload) => {
    return asArray(payload).map((layer) => ({
        id: asNumber(layer?.id, 0),
        visible: asBoolean(layer?.visible, true),
        brightness: asNumber(layer?.brightness, 0),
        speed: asNumber(layer?.speed, 0),
        fadeSpeed: asNumber(layer?.fadeSpeed, 0),
        easing: asNumber(layer?.easing, 0),
        blendMode: asNumber(layer?.blendMode, 0),
        behaviourFlags: asNumber(layer?.behaviourFlags, 0),
        offset: asNumber(layer?.offset, 0),
        palette: parsePalette(layer?.palette),
    }));
};

/**
 * @param {unknown} payload
 * @returns {ApiColorsResponse}
 */
export const parseColorsResponse = (payload) => {
    const colors = asArray(payload?.colors).map((color) => ({
        r: asNumber(color?.r, 0),
        g: asNumber(color?.g, 0),
        b: asNumber(color?.b, 0),
        w: asNumber(color?.w, 0),
    }));
    return {
        colors,
        step: Math.max(1, asNumber(payload?.step, 1)),
        totalPixels: Math.max(0, asNumber(payload?.totalPixels, 0)),
    };
};

/**
 * @param {unknown} payload
 * @returns {ApiModelData}
 */
export const parseModelDataResponse = (payload) => ({
    pixelCount: asNumber(payload?.pixelCount, 0),
    realPixelCount: asNumber(payload?.realPixelCount, 0),
    modelCount: asNumber(payload?.modelCount, 0),
    gapCount: asNumber(payload?.gapCount, 0),
    intersections: asArray(payload?.intersections).map((intersection) => ({
        id: asNumber(intersection?.id, 0),
        group: asNumber(intersection?.group, 0),
        numPorts: asNumber(intersection?.numPorts, 0),
        topPixel: asNumber(intersection?.topPixel, 0),
        bottomPixel: asNumber(intersection?.bottomPixel, -1),
        ports: asArray(intersection?.ports).map((port) => {
            if (port == null) return null;
            return {
                id: asNumber(port.id, 0),
                type: String(port.type || 'internal'),
                direction: asBoolean(port.direction, false),
                group: asNumber(port.group, 0),
                device: port.device ? String(port.device) : undefined,
                targetId: port.targetId == null ? undefined : asNumber(port.targetId, 0),
            };
        }),
    })),
    connections: asArray(payload?.connections).map((connection) => ({
        group: asNumber(connection?.group, 0),
        fromPixel: asNumber(connection?.fromPixel, 0),
        toPixel: asNumber(connection?.toPixel, 0),
        numLeds: asNumber(connection?.numLeds, 0),
        pixelDir: asNumber(connection?.pixelDir, 0),
    })),
    models: asArray(payload?.models).map((model) => {
        if (model == null) return null;
        return {
            id: asNumber(model.id, 0),
            defaultW: asNumber(model.defaultW, 0),
            emitGroups: asNumber(model.emitGroups, 0),
            maxLength: asNumber(model.maxLength, 0),
        };
    }),
    gaps: asArray(payload?.gaps).map((gap) => ({
        fromPixel: asNumber(gap?.fromPixel, 0),
        toPixel: asNumber(gap?.toPixel, 0),
    })),
});
