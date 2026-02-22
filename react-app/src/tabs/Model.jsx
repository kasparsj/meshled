import React from "react";
import { useColors } from "../hooks/useColors.js";
import { useModelData } from "../hooks/useModelData.js";
import { IntersectionProvider, useIntersectionContext } from "../contexts/IntersectionContext.jsx";
import LEDVisualization from "../components/LEDVisualization.jsx";
import ModelInfo from "../components/ModelInfo.jsx";
import AddIntersectionModal from "../components/AddIntersectionModal.jsx";
import RemoveIntersectionModal from "../components/RemoveIntersectionModal.jsx";

const ModelTab = () => {
    return (
        <IntersectionProvider>
            <ModelTabContent />
        </IntersectionProvider>
    );
};

const ModelTabContent = () => {
    const { 
        allPixels, 
        loading: colorsLoading, 
        refreshColors 
    } = useColors();
    
    const { 
        modelData, 
        loading: modelLoading, 
        refreshModelData 
    } = useModelData();

    const {
        openAddModal,
        openRemoveModal,
        removeIntersection
    } = useIntersectionContext();

    const handleRefresh = async () => {
        await Promise.all([
            refreshColors(),
            refreshModelData()
        ]);
    };

    const handleAddIntersection = (pixelIndex) => {
        openAddModal(pixelIndex);
    };

    const handleRemoveIntersection = (intersection) => {
        openRemoveModal(intersection);
    };

    const handleDirectRemoveIntersection = async (intersectionId, group) => {
        try {
            await removeIntersection(intersectionId, group);
            // Refresh data after successful removal
            await refreshModelData();
        } catch (error) {
            console.error('Failed to remove intersection:', error);
            // You could add a toast notification here
        }
    };

    const handleModalSuccess = async () => {
        // Refresh data after successful addition
        await refreshModelData();
    };

    const isLoading = colorsLoading || modelLoading;

    return (
        <div className="space-y-6">
            <h2 className="text-2xl font-bold">Model</h2>
            
            <LEDVisualization 
                allPixels={allPixels}
                loading={isLoading}
                onRefresh={handleRefresh}
                modelData={modelData}
                onAddIntersection={handleAddIntersection}
                onRemoveIntersection={handleRemoveIntersection}
            />
            
            <ModelInfo 
                modelData={modelData}
                onRemoveIntersection={handleDirectRemoveIntersection}
            />

            <AddIntersectionModal onSuccess={handleModalSuccess} />
            <RemoveIntersectionModal onSuccess={handleModalSuccess} />
        </div>
    );
};

export default ModelTab;