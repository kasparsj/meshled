import { useMemo } from 'react';

export const usePixelMapping = (modelData) => {
    return useMemo(() => {
        if (!modelData) {
            return {
                intersectionPixels: new Map(),
                connectionPixels: new Map(),
                gapPixels: new Set()
            };
        }

        const intersectionPixels = new Map();
        const connectionPixels = new Map();
        const gapPixels = new Set();

        // Map intersections - only show top and bottom pixels (if not -1)
        if (modelData.intersections) {
            modelData.intersections.forEach((intersection, index) => {
                const pixelsToShow = [];
                
                // Add top pixel if it's not -1
                if (intersection.topPixel !== -1) {
                    pixelsToShow.push(intersection.topPixel);
                }
                
                // Add bottom pixel if it's not -1 and different from top
                if (intersection.bottomPixel !== -1 && intersection.bottomPixel !== intersection.topPixel) {
                    pixelsToShow.push(intersection.bottomPixel);
                }
                
                pixelsToShow.forEach(pixel => {
                    intersectionPixels.set(pixel, {
                        ...intersection,
                        index,
                        type: 'intersection'
                    });
                });
            });
        }

        // Map connections - only show first and last pixels
        if (modelData.connections) {
            modelData.connections.forEach((connection, index) => {
                const pixelsToShow = [connection.fromPixel];
                
                // Add toPixel if it's different from fromPixel
                if (connection.toPixel !== connection.fromPixel) {
                    pixelsToShow.push(connection.toPixel);
                }
                
                pixelsToShow.forEach(pixel => {
                    // Don't override intersections with connections
                    if (!intersectionPixels.has(pixel)) {
                        connectionPixels.set(pixel, {
                            ...connection,
                            index,
                            type: 'connection'
                        });
                    }
                });
            });
        }

        // Map gaps to their pixel ranges
        if (modelData.gaps) {
            modelData.gaps.forEach(gap => {
                for (let pixel = gap.fromPixel; pixel <= gap.toPixel; pixel++) {
                    gapPixels.add(pixel);
                }
            });
        }

        return {
            intersectionPixels,
            connectionPixels,
            gapPixels
        };
    }, [modelData]);
};

export const getPixelInfo = (pixelIndex, intersectionPixels, connectionPixels, gapPixels) => {
    const info = {
        isIntersection: intersectionPixels.has(pixelIndex),
        isConnection: connectionPixels.has(pixelIndex),
        isGap: gapPixels.has(pixelIndex),
        intersection: intersectionPixels.get(pixelIndex),
        connection: connectionPixels.get(pixelIndex)
    };

    return info;
};