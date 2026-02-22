import React from 'react';
import { Plus, Trash2 } from 'lucide-react';

const Pixel = ({ 
    color, 
    index, 
    isIntersection, 
    isConnection, 
    isGap, 
    intersection, 
    connection,
    onAddIntersection,
    onRemoveIntersection 
}) => {
    const getOverlayStyle = () => {
        if (isGap) {
            return 'border-2 border-red-400 opacity-50';
        }
        if (isIntersection) {
            return 'border-2 border-blue-400';
        }
        if (isConnection) {
            return 'border-2 border-green-400';
        }
        return 'border border-zinc-600';
    };

    const getTooltipText = () => {
        let tooltip = `LED ${index}: RGB(${color.r}, ${color.g}, ${color.b})`;
        
        if (isGap) {
            tooltip += '\n🔴 Gap - No LEDs in this range';
        }
        
        if (isIntersection) {
            tooltip += `\n🔵 Intersection ${intersection.id}`;
            tooltip += `\n   Group: ${intersection.group}`;
            tooltip += `\n   Ports: ${intersection.numPorts}`;
            tooltip += `\n   Full Range: ${intersection.topPixel} - ${intersection.bottomPixel}`;
            
            // Determine if this is the top or bottom pixel
            if (index === intersection.topPixel && index === intersection.bottomPixel) {
                tooltip += `\n   Position: Single pixel intersection`;
            } else if (index === intersection.topPixel) {
                tooltip += `\n   Position: Top pixel of intersection`;
            } else if (index === intersection.bottomPixel) {
                tooltip += `\n   Position: Bottom pixel of intersection`;
            }
        }
        
        if (isConnection) {
            tooltip += `\n🟢 Connection`;
            tooltip += `\n   Group: ${connection.group}`;
            tooltip += `\n   Full Range: ${connection.fromPixel} → ${connection.toPixel}`;
            tooltip += `\n   Total LEDs: ${connection.numLeds}`;
            tooltip += `\n   Direction: ${connection.pixelDir}`;
            
            // Determine if this is the from or to pixel
            if (index === connection.fromPixel && index === connection.toPixel) {
                tooltip += `\n   Position: Single pixel connection`;
            } else if (index === connection.fromPixel) {
                tooltip += `\n   Position: Start pixel (from)`;
            } else if (index === connection.toPixel) {
                tooltip += `\n   Position: End pixel (to)`;
            }
        }
        
        return tooltip;
    };

    const getTopIndicator = () => {
        if (isGap) {
            return <div className="absolute -top-0.5 left-1/2 transform -translate-x-1/2 w-1.5 h-0.5 bg-red-400 rounded-full" />;
        }
        if (isIntersection) {
            return <div className="absolute -top-0.5 left-1/2 transform -translate-x-1/2 w-1.5 h-0.5 bg-blue-400 rounded-full" />;
        }
        if (isConnection) {
            return <div className="absolute -top-0.5 left-1/2 transform -translate-x-1/2 w-1.5 h-0.5 bg-green-400 rounded-full" />;
        }
        return null;
    };

    const handleClick = () => {
        if (isIntersection) {
            // Show remove dialog for existing intersections
            if (onRemoveIntersection && intersection) {
                onRemoveIntersection(intersection);
            }
        } else {
            // Show add option for pixels without intersections
            if (onAddIntersection) {
                onAddIntersection(index);
            }
        }
    };

    return (
        <div className="flex flex-col items-center">
            <div className="relative group">
                <div
                    className={`w-3 h-3 rounded-sm ${getOverlayStyle()} cursor-pointer transition-all duration-150 hover:scale-110`}
                    style={{ backgroundColor: `rgb(${color.r}, ${color.g}, ${color.b})` }}
                    title={getTooltipText()}
                    onClick={handleClick}
                />
                {getTopIndicator()}
                
                {/* Hover overlay for click indication */}
                <div className="absolute inset-0 opacity-0 group-hover:opacity-100 transition-opacity duration-150 pointer-events-none">
                    <div className="absolute -top-6 left-1/2 transform -translate-x-1/2 bg-black bg-opacity-75 text-white text-xs px-2 py-1 rounded whitespace-nowrap">
                        {isIntersection ? (
                            <span className="flex items-center gap-1">
                                <Trash2 size={10} />
                                Remove
                            </span>
                        ) : (
                            <span className="flex items-center gap-1">
                                <Plus size={10} />
                                Add
                            </span>
                        )}
                    </div>
                </div>
            </div>
            {index % 5 === 0 && (
                <span className="text-xs text-zinc-400 mt-1">{index}</span>
            )}
        </div>
    );
};

export default Pixel;
