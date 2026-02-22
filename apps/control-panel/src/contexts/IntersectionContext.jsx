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

    const addIntersection = async (intersectionData) => {
        setIsLoading(true);
        try {
            const response = await deviceFetch('/add_intersection', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(intersectionData),
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || 'Failed to add intersection');
            }

            const result = await response.json();
            return result;
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
            const response = await deviceFetch('/remove_intersection', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ id: intersectionId, group }),
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || 'Failed to remove intersection');
            }

            const result = await response.json();
            return result;
        } catch (error) {
            console.error('Error removing intersection:', error);
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