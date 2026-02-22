import React from 'react';
import { RefreshCw } from 'lucide-react';
import { usePixelMapping, getPixelInfo } from '../hooks/usePixelMapping.js';
import Pixel from './Pixel.jsx';

const LEDVisualization = ({ allPixels, loading, onRefresh, modelData, onAddIntersection, onRemoveIntersection }) => {
    const { intersectionPixels, connectionPixels, gapPixels } = usePixelMapping(modelData);

    return (
        <div className="bg-zinc-700 p-4 rounded-lg">
            {/* Legend */}
            <div className="flex gap-4 mb-4 text-xs">
                <div className="flex items-center gap-1">
                    <div className="w-3 h-3 border-2 border-blue-400 rounded-sm"></div>
                    <span className="text-blue-400">Intersections (top/bottom)</span>
                </div>
                <div className="flex items-center gap-1">
                    <div className="w-3 h-3 border-2 border-green-400 rounded-sm"></div>
                    <span className="text-green-400">Connections (first/last)</span>
                </div>
                <div className="flex items-center gap-1">
                    <div className="w-3 h-3 border-2 border-red-400 rounded-sm opacity-50"></div>
                    <span className="text-red-400">Gaps</span>
                </div>
            </div>

            <div className="flex flex-nowrap gap-0.5 overflow-x-auto pb-6">
                {allPixels.map((color, index) => {
                    const pixelInfo = getPixelInfo(index, intersectionPixels, connectionPixels, gapPixels);
                    
                    return (
                        <Pixel
                            key={index}
                            color={color}
                            index={index}
                            isIntersection={pixelInfo.isIntersection}
                            isConnection={pixelInfo.isConnection}
                            isGap={pixelInfo.isGap}
                            intersection={pixelInfo.intersection}
                            connection={pixelInfo.connection}
                            onAddIntersection={onAddIntersection}
                            onRemoveIntersection={onRemoveIntersection}
                        />
                    );
                })}
            </div>

            <div className="mt-4 flex gap-2">
                <button
                    onClick={onRefresh}
                    disabled={loading}
                    className="bg-sky-600 hover:bg-sky-700 disabled:opacity-50 disabled:cursor-not-allowed px-4 py-2 rounded-lg flex items-center gap-2"
                >
                    <RefreshCw size={18} className={loading ? "animate-spin" : ""} />
                    {loading ? "Loading..." : "Refresh"}
                </button>
            </div>
        </div>
    );
};

export default LEDVisualization;