import React, { useState } from "react";
import { useColors } from "../hooks/useColors.js";
import { useModelData } from "../hooks/useModelData.js";
import { IntersectionProvider, useIntersectionContext } from "../contexts/IntersectionContext.jsx";
import { useDevice } from "../contexts/DeviceContext.jsx";
import useRemoteTopology from "../hooks/useRemoteTopology.js";
import LEDVisualization from "../components/LEDVisualization.jsx";
import ModelInfo from "../components/ModelInfo.jsx";
import AddIntersectionModal from "../components/AddIntersectionModal.jsx";
import RemoveIntersectionModal from "../components/RemoveIntersectionModal.jsx";

const ModelTab = ({ devices = [] }) => {
    return (
        <IntersectionProvider>
            <ModelTabContent devices={devices} />
        </IntersectionProvider>
    );
};

const ModelTabContent = ({ devices }) => {
    const { selectedDevice } = useDevice();
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
        removeIntersection,
        addExternalPort,
        updateExternalPort,
        removeExternalPort,
        discoverPeers,
    } = useIntersectionContext();
    const { remoteDevices, loading: remoteLoading, error: remoteError } = useRemoteTopology(devices);
    const [discoveringPeers, setDiscoveringPeers] = useState(false);
    const [discoverPeersError, setDiscoverPeersError] = useState('');

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

    const handleAddExternalPort = async (payload) => {
        await addExternalPort(payload);
        await refreshModelData();
    };

    const handleUpdateExternalPort = async (payload) => {
        await updateExternalPort(payload);
        await refreshModelData();
    };

    const handleRemoveExternalPort = async (portId) => {
        await removeExternalPort(portId);
        await refreshModelData();
    };

    const handleDiscoverPeers = async () => {
        setDiscoverPeersError('');
        setDiscoveringPeers(true);
        try {
            await discoverPeers();
            await refreshModelData();
        } catch (error) {
            setDiscoverPeersError(error.message || 'Failed to start peer discovery');
        } finally {
            setDiscoveringPeers(false);
        }
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
                onAddExternalPort={handleAddExternalPort}
                onUpdateExternalPort={handleUpdateExternalPort}
                onRemoveExternalPort={handleRemoveExternalPort}
                remoteDevices={remoteDevices}
                remoteLoading={remoteLoading}
                remoteError={!selectedDevice ? '' : remoteError}
                onDiscoverPeers={handleDiscoverPeers}
                discoveringPeers={discoveringPeers}
                discoverPeersError={discoverPeersError}
            />

            <AddIntersectionModal onSuccess={handleModalSuccess} />
            <RemoveIntersectionModal onSuccess={handleModalSuccess} />
        </div>
    );
};

export default ModelTab;
