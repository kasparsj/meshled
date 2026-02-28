import React, { createContext, useContext, useState } from 'react';
import { useDevice } from './DeviceContext';

const IntersectionContext = createContext();

export const useIntersectionContext = () => {
    const context = useContext(IntersectionContext);
    if (!context) {
        throw new Error('useIntersectionContext must be used within an IntersectionProvider');
    }
    return context;
};

export const IntersectionProvider = ({ children }) => {
    const { deviceFetch } = useDevice();
    const [isLoading, setIsLoading] = useState(false);
    const [showAddModal, setShowAddModal] = useState(false);
    const [showRemoveModal, setShowRemoveModal] = useState(false);
    const [selectedPixel, setSelectedPixel] = useState(null);
    const [selectedIntersection, setSelectedIntersection] = useState(null);

    const postJson = async (path, body) => {
        const response = await deviceFetch(path, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(body),
        });

        if (!response.ok) {
            let message = `Request failed (${response.status})`;
            try {
                const errorData = await response.json();
                message = errorData.error || message;
            } catch {
                // Keep default message when body is not JSON.
            }
            throw new Error(message);
        }

        try {
            return await response.json();
        } catch {
            return { success: true };
        }
    };

    const addIntersection = async (intersectionData) => {
        setIsLoading(true);
        try {
            return await postJson('/add_intersection', intersectionData);
        } catch (error) {
            console.error('Error adding intersection:', error);
            throw error;
        } finally {
            setIsLoading(false);
        }
    };

    const removeIntersection = async (intersectionId, group) => {
        setIsLoading(true);
        try {
            return await postJson('/remove_intersection', { id: intersectionId, group });
        } catch (error) {
            console.error('Error removing intersection:', error);
            throw error;
        } finally {
            setIsLoading(false);
        }
    };

    const addExternalPort = async (payload) => {
        setIsLoading(true);
        try {
            return await postJson('/add_external_port', payload);
        } catch (error) {
            console.error('Error adding external port:', error);
            throw error;
        } finally {
            setIsLoading(false);
        }
    };

    const updateExternalPort = async (payload) => {
        setIsLoading(true);
        try {
            return await postJson('/update_external_port', payload);
        } catch (error) {
            console.error('Error updating external port:', error);
            throw error;
        } finally {
            setIsLoading(false);
        }
    };

    const removeExternalPort = async (portId) => {
        setIsLoading(true);
        try {
            return await postJson('/remove_external_port', { portId });
        } catch (error) {
            console.error('Error removing external port:', error);
            throw error;
        } finally {
            setIsLoading(false);
        }
    };

    const openAddModal = (pixelIndex) => {
        setSelectedPixel(pixelIndex);
        setShowAddModal(true);
    };

    const closeAddModal = () => {
        setSelectedPixel(null);
        setShowAddModal(false);
    };

    const openRemoveModal = (intersection) => {
        setSelectedIntersection(intersection);
        setShowRemoveModal(true);
    };

    const closeRemoveModal = () => {
        setSelectedIntersection(null);
        setShowRemoveModal(false);
    };

    const value = {
        isLoading,
        showAddModal,
        showRemoveModal,
        selectedPixel,
        selectedIntersection,
        addIntersection,
        removeIntersection,
        addExternalPort,
        updateExternalPort,
        removeExternalPort,
        openAddModal,
        closeAddModal,
        openRemoveModal,
        closeRemoveModal,
    };

    return (
        <IntersectionContext.Provider value={value}>
            {children}
        </IntersectionContext.Provider>
    );
};
